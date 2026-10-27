// Win32 platform layer: window, message pump, accessibility settings, user data folder, file I/O emulation, DirectSound.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_PLATFORM_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_PLATFORM_H

// window.cpp. Full screen is a borderless topmost popup; otherwise the client area is w x h (x or y == -1 centres it).
// Returns the handle, also kept in g_mainWnd; 0 on failure.
int CreateGameWindow(const char *className, const char *title, int x, int y, int w, int h, void *wndProc, void *hInstance,
                     int iconId, bool fullscreen);   // 0x519f00

// platform.cpp
void Platform_Init();   // saves the mouse speed, sticky keys and screen saver timeout; switches the sticky-keys hotkey off
void PumpMessages();   // pumpmessages.cpp
extern unsigned g_screenSaverTimeout;   // at start-up, seconds; 0 when unknown
void Platform_Shutdown();
void App_SetActive();   // WM_SETFOCUS
void App_SetInactive();

// userdata.cpp. Writes "<My Documents>\<section>" to buf, or `fallback` when the shell folder is unknown. 0 when it doesn't fit.
char GetUserDataPath(const char *section, const char *fallback, char *buf, int size);

// dsound.cpp: the object the movie player (Bink) opens its sound device on.
struct DirectSound;
extern DirectSound *g_dsoundObj;
void DSound_Init(void *hwnd);
void DSound_Release();

#endif
