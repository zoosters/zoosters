#include "framework/mainloop/gameclock.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/platform.h"
#include "zoosters/os.h"

static inline int ShowCursor(int show) { return zoosters::os::ShowCursor(show != 0); }
static inline void ShowWindow(void *wnd, int cmd) { zoosters::os::ShowWindow(wnd, cmd); }
static inline void InvalidateRect(void *wnd, const void *, int) { zoosters::os::InvalidateWindow(wnd); }
static inline void ValidateRect(void *wnd, const void *) { zoosters::os::ValidateWindow(wnd); }
static inline WinLResult DefWindowProcA(void *wnd, unsigned msg, WinWParam wParam, WinLParam lParam)
{
	return (WinLResult)zoosters::os::DefaultWindowProc(wnd, msg, (uintptr_t)wParam, (intptr_t)lParam);
}
static inline void SetCapture(void *wnd) { zoosters::os::CaptureMouse(wnd, wnd != 0); }
static inline void PostQuitMessage(int) { zoosters::os::RequestQuit(); }

// 0x464370
WinLResult __stdcall GameWndProc(void *hwnd, unsigned msg, WinWParam wParam, WinLParam lParam)
{
	short x, y;
	switch (msg) {
	case 7:
		App_SetActive();
		TFBHook_Resume();
		ShowCursor(0);
		ShowWindow(g_mainWnd, g_wndWindowed ? 1 : 3);
		return 0;
	case 8:
		ShowWindow(g_mainWnd, 6);
		ShowCursor(1);
		TFBHook_Pause();
		App_SetInactive();
		InvalidateRect(0, 0, 1);
		return 0;
	case 2:
	case 0x10:
		g_quitRequested = 1;
		PostQuitMessage(0);
		return 0;
	case 0xf:
		ValidateRect(hwnd, 0);
		return 0;
	case 0x100:
		if (g_cfgQuitOnInput)
			g_quitRequested = 1;
		if (g_cfgKeyReqA) {
			g_wndRequestFlag = 1;
			g_wndRequestA = 0x10;
			g_wndRequestB = 0;
		}
		if (g_cfgKeyReqB) {
			g_wndRequestFlag = 1;
			g_wndRequestA = 4;
			g_wndRequestB = 0;
		}
		return DefWindowProcA(hwnd, msg, wParam, lParam);
	case 0x112:
		if (wParam == 0xf100)
			return 0;
		return DefWindowProcA(hwnd, msg, wParam, lParam);
	case 0x201:
		if (g_cfgQuitOnInput)
			g_quitRequested = 1;
		if (g_cursorShown) {
			x = (short)lParam;
			y = (short)((unsigned)lParam >> 16);
			g_mouseButtonDown = 1;
			SetCapture(g_mainWnd);
			MenuPointer_Update(x, y, true, true);
		}
		return 0;
	case 0x202:
		if (g_cursorShown) {
			x = (short)lParam;
			y = (short)((unsigned)lParam >> 16);
			g_mouseButtonDown = 0;
			SetCapture(0);
			if (MenuPointer_Update(x, y, false, true)) {
				g_wndRequestFlag = 1;
				g_wndRequestA = 0;
				g_wndRequestB = 0;
			}
		}
		return 0;
	case 0x200:
		if (g_cursorShown) {
			x = (short)lParam;
			y = (short)((unsigned)lParam >> 16);
			MenuPointer_Update(x, y, g_mouseButtonDown != 0, false);
		}
		return 0;
	}
	return DefWindowProcA(hwnd, msg, wParam, lParam);
}
