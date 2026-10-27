// Split from platform.cpp so that a port window backend can replace it: the SDL build compiles window_sdl.cpp's PumpMessages instead.
// CFLAGS: /GS
#include "zoosters/base.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/platform.h"
extern "C" {
__declspec(dllimport) int __stdcall SystemParametersInfoA(unsigned action, unsigned param, void *vparam, unsigned winini);
__declspec(dllimport) int __stdcall PeekMessageA(void *msg, void *hwnd, unsigned lo, unsigned hi, unsigned remove);
__declspec(dllimport) int __stdcall TranslateMessage(const void *msg);
__declspec(dllimport) long __stdcall DispatchMessageA(const void *msg);
}
// GLOBAL: 0x5ca1d8 _imp__PeekMessageA
// GLOBAL: 0x5ca1dc _imp__DispatchMessageA
// GLOBAL: 0x5ca1e0 _imp__TranslateMessage

enum {
	SPI_SETSCREENSAVETIMEOUT = 0x0f,
	PM_REMOVE = 1,
	kScreenSaverResetPeriod = 200   // pumps between two screen saver resets
};

struct WinMsg {   // MSG
	void *hwnd;
	unsigned message;
	WinWParam wParam;
	WinLParam lParam;
	unsigned time;
	long x;
	long y;
};

extern unsigned g_pumpCounter;               // GLOBAL: 0x62f150 g_pumpCounter

// Re-setting the screen saver timeout to its current value restarts the idle timer.
// 0x51a100
void PumpMessages()
{
	WinMsg msg;
	if (g_screenSaverTimeout) {
		if (g_pumpCounter++ >= kScreenSaverResetPeriod) {
			g_pumpCounter = 0;
			SystemParametersInfoA(SPI_SETSCREENSAVETIMEOUT, g_screenSaverTimeout, 0, 0);
		}
	}
	while (PeekMessageA(&msg, g_mainWnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}
