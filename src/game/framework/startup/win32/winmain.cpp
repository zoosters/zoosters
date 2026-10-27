#include <stdlib.h>
#include "framework/startup/win32/cfgobj.h"
#include "framework/mainloop/mainloop.h"
#include "framework/startup/startup.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/fileio.h"
#include "framework/startup/win32/platform.h"
#include "framework/startup/win32/textdict.h"
#include "zoosters/os.h"

// GLOBAL: 0x559e6c _atoi

struct RECTW { int left, top, right, bottom; };
struct POINTW { int x, y; };
static inline int GetClientRect(void *wnd, RECTW *rc)
{
	int w = 0, h = 0;
	int ok = zoosters::os::ClientSize(wnd, &w, &h);
	rc->left = rc->top = 0;
	rc->right = w;
	rc->bottom = h;
	return ok;
}
static inline int ClientToScreen(void *wnd, POINTW *pt) { return zoosters::os::ClientToScreen(wnd, &pt->x, &pt->y); }
static inline void SetCursorPos(int x, int y) { zoosters::os::SetCursorPos(x, y); }
static inline void ShowWindow(void *wnd, int cmd) { zoosters::os::ShowWindow(wnd, cmd); }
static inline int ShowCursor(int show) { return zoosters::os::ShowCursor(show != 0); }
static inline void UpdateWindow(void *wnd) { zoosters::os::UpdateWindow(wnd); }
static inline void Sleep(unsigned ms) { zoosters::os::SleepMs(ms); }
static inline void MessageBoxA(void *, const char *text, const char *caption, unsigned) { zoosters::os::ShowError(caption, text); }

int SymbolRegistry_GetSize();
void LoadScreen_FreeTextures();
int GetRumbleOff();
#ifdef ZOOSTERS_OPTIONS
extern "C" void port_options_load_assets(void);   // port/options/bridge/options_bridge.cpp
#endif
#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide_resolution(int *w, int *h);   // port/fixes/widescreen.cpp
#endif
#ifdef ZOOSTERS_FIX_BUGS
extern "C" void port_display_fit(int *w, int *h);   // port/fixes/display.cpp
#endif

// 0x4646f0
int __stdcall WinMain(void *hInstance, void *hPrev, char *cmdLine, int showCmd)
{
	char resText[13];
	RECTW centerRect;
	RECTW clientRect;
	VideoModeRequest mode;
	char widthText[13];
	void *mutex;
	char heightText[13];
	bool alreadyRunning;
	mutex = zoosters::os::InstanceLock("Madagascar Running", &alreadyRunning);
	if (alreadyRunning) {
		zoosters::os::InstanceUnlock(mutex);
		return 0;
	}
	g_audioDevice = Settings_Create();
	g_unlockLevels = g_audioDevice->GetInt("Settings\\Options\\UnlockLevels", 0) == 1;
	int xPos = 0;
#ifdef ZOOSTERS_WIDESCREEN   // no stored resolution: the desktop's, not 800x600
	for (int i = 0; i < 13; i++)
		resText[i] = 0;
	g_audioDevice->GetString("Settings\\Display\\Resolution", "", resText, 12);
#else
	g_audioDevice->GetString("Settings\\Display\\Resolution", "800x600", resText, 12);
#endif
	while (resText[xPos] != 'x' && xPos < 12)
		xPos++;
	for (int j = 0; j < 12; j++) {
		widthText[j] = ' ';
		heightText[j] = ' ';
		if (j < xPos)
			widthText[j] = resText[j];
		else if (j > xPos)
			heightText[j - xPos - 1] = resText[j];
	}
	g_resWidth = atoi(widthText);
	g_resHeight = atoi(heightText);
#ifdef ZOOSTERS_WIDESCREEN
	port_wide_resolution((int *)&g_resWidth, (int *)&g_resHeight);
#endif
	g_distClip = g_audioDevice->GetInt("Settings\\Display\\DistanceClipping", 0) == 1;
	g_distance = g_audioDevice->GetInt("Settings\\Display\\Distance", 100);
	g_wndWindowed = 0;
#ifdef ZOOSTERS_FIX_BUGS   // tier fixes and up: never change the display mode, fit the stored size to the screen instead
	port_display_fit((int *)&g_resWidth, (int *)&g_resHeight);
#else
	{
		uint32_t desktopW, desktopH;
		if (zoosters::os::DesktopMode(&desktopW, &desktopH)) {
			if (g_resWidth > desktopW || g_resHeight > desktopH) {
				if (!zoosters::os::SetDisplayMode(g_resWidth, g_resHeight))
					zoosters::os::ShowError("Error", "Display change failed");
			}
		}
	}
#endif
	CreateGameWindow("RWSConsoleD3D8", TextDict_Find(g_dict, "ID_WINNAME"), 0, 0, g_resWidth, g_resHeight, (void *)GameWndProc, hInstance, 0x67, !g_wndWindowed);
	DAT_0062a5dc = g_mainWnd;
	if (g_mainWnd) {
		GetClientRect(DAT_0062a5dc, &clientRect);
		if (g_wndWindowed) {
			mode.width = clientRect.right - clientRect.left;
			mode.height = clientRect.bottom - clientRect.top;
		} else {
			mode.width = g_resWidth;
			mode.height = g_resHeight;
		}
		mode.depth = 0x20;
		mode.flags = 1;
		if (RWS::StartUp::Open(0, 0, 0x1000000, &mode, 0x10, DAT_0062a5dc)) {
			RWS::StartUp::LoadGameDatabaseFile("levels\\global.stream", false);
#ifdef ZOOSTERS_OPTIONS   // our own menu textures (Mod\Levels\options.stream), loaded once after the global stream
			port_options_load_assets();
#endif
			g_globalSymbolCount = SymbolRegistry_GetSize();
			GetClientRect(DAT_0062a5dc, &centerRect);
			POINTW center;
			center.x = (centerRect.right - centerRect.left) / 2;
			center.y = (centerRect.bottom - centerRect.top) / 2;
			ClientToScreen(DAT_0062a5dc, &center);
			SetCursorPos(center.x, center.y);
			g_mainLoopEntered = 1;
			if (g_wndWindowed)
				ShowWindow(g_mainWnd, 1);
			ShowCursor(0);
			UpdateWindow(DAT_0062a5dc);
			while (!g_quitRequested) {
				PumpMessages();
				while (!g_frameReady && !g_quitRequested) {
					PumpMessages();
					Sleep(10);
				}
				RWS::MainLoop::Poll();
			}
			LoadScreen_FreeTextures();
			RWS::StartUp::Close();
		}
	}
	g_audioDevice->SetOption("Settings\\Options\\Effect", g_effectsVolume);
	g_audioDevice->SetOption("Settings\\Options\\Music", g_musicVolume);
	g_audioDevice->SetOption("Settings\\Options\\RumbleOff", GetRumbleOff());
	FileIo_Reset();
	TextDict_Free(g_dict);
	Platform_Shutdown();
#ifndef ZOOSTERS_FIX_BUGS   // tier fixes and up never changed the mode
	if (!zoosters::os::RestoreDisplayMode())
		zoosters::os::ShowError("Error", "Display reset failed");
#endif
	zoosters::os::InstanceUnlock(mutex);
	return 1;
}
