#include <stdarg.h>
#include <stdio.h>
#include "framework/startup/win32/cfgobj.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/fileio.h"
#include "framework/startup/win32/platform.h"
#include "framework/startup/win32/textdict.h"
#include "zoosters/os.h"

void SetRumbleOff(int rumbleOff);

// 0x4634e0
CfgObj *Settings_Create()
{
	Platform_Init();
	g_dict = TextDict_Load("data\\mdgdict.bnx");
	FileIoConfig params;
	params.m_threadPriority = 1;
	params.m_param = 3;
	params.m_limit = 16384.0f;
	FileIo_Init(&params);
	CfgObj *cfg = new CfgObj("Activision", "Madagascar");
	g_effectsVolume = cfg->GetInt("Settings\\Options\\Effect", 75);
	g_musicVolume = cfg->GetInt("Settings\\Options\\Music", 75);
	SetRumbleOff(cfg->GetInt("Settings\\Options\\RumbleOff", 0));
	g_detailedShadows = cfg->GetInt("Settings\\Display\\DetailedShadows", 1) != 0;
	return cfg;
}

// 0x4635d0
int DebugMessageHandler(const char *fmt, ...)
{
	char buf[0x400];
	va_list ap;
	va_start(ap, fmt);
	int n = _vsnprintf(buf, 0x400, fmt, ap);
	zoosters::os::DebugPrint(n < 0 ? "Failed to output FULL debug message\n" : buf);
	return 1;
}

// 0x463620
void __stdcall WindowCloseCallback(int code, int)
{
	if (code == 1) {
		GameWndProc(DAT_0062a5dc, 0x10, 0, 0);   // SendMessage(WM_CLOSE) from the window's own thread calls the window procedure directly
	}
}

// GLOBAL: 0x62f138 g_mainWnd
// GLOBAL: 0x62a5dc DAT_0062a5dc
// GLOBAL: 0x62a5ac g_dict
// GLOBAL: 0x617fc8 g_effectsVolume
// GLOBAL: 0x617fc4 g_musicVolume
// GLOBAL: 0x606cc4 g_detailedShadows
// GLOBAL: 0x62a5c0 g_unlockLevels
// GLOBAL: 0x62a5c8 g_resWidth
// GLOBAL: 0x62a5cc g_resHeight
// GLOBAL: 0x62a59c g_distClip
// GLOBAL: 0x62a5a0 g_distance
// GLOBAL: 0x62a5b0 g_wndWindowed
// GLOBAL: 0x62a5d0 g_mainLoopEntered
// GLOBAL: 0x62f140 g_quitRequested
// GLOBAL: 0x62f141 g_frameReady
// GLOBAL: 0x602ea0 g_globalSymbolCount
// GLOBAL: 0x62f1dc g_pfnCloseHandle
// GLOBAL: 0x62a5b1 g_wndRequestFlag
// GLOBAL: 0x62a5b4 g_wndRequestA
// GLOBAL: 0x62a5b8 g_wndRequestB
// GLOBAL: 0x62a5c1 g_cfgQuitOnInput
// GLOBAL: 0x62a5c2 g_cfgKeyReqA
// GLOBAL: 0x62a5c3 g_cfgKeyReqB
// GLOBAL: 0x62a5d1 g_mouseButtonDown
// GLOBAL: 0x62a5d2 g_cursorShown
// GLOBAL: 0x62a5d4 g_cursorSprite
// GLOBAL: 0x62a5d8 g_cursorSaved
