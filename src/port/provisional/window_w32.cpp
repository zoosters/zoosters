// CreateGameWindow (0x519f00) for the windowed build (RegisterClassExA with the icon, WS_POPUP full screen or a centred bordered
// window, CreateWindowExA, g_mainWnd), plus the harness side: a watchdog thread that closes the window after N presented frames.
// Deviation: the window starts bordered and takes the window mode of the options (windowed, borderless, exclusive) from there;
// without the options only PORT_FULLSCREEN=1 makes it full screen (the game is always full screen on the PC).
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../debugui/debugui.h"
#include "platform.h"

extern "C" FILE *port_log;
extern void *g_mainWnd;   // GLOBAL: 0x62f138 g_mainWnd
extern unsigned char g_wndWindowed;   // GLOBAL: 0x62a5b0 g_wndWindowed (WinMain clears it: the PC game is always full screen)
extern "C" int port_frames_presented(void);   // port/shim/rwcore.cpp (RwCameraShowRaster count)
extern "C" void port_set_video_mode(int w, int h);   // port/shim/rwcore.cpp: the one windowed video mode the game's SelectVideoMode sees
static int g_maxFrames = -1;
static DWORD g_startTick;

// Input backend of mouse.cpp: raw input (unaccelerated, what DirectInput 8 reads) arrives as WM_INPUT on the game window. A
// wrapper in front of GameWndProc decodes it and passes every message on unchanged (the game leaves WM_INPUT to DefWindowProc).
extern "C" void port_mouse_raw_move(int dx, int dy);
extern "C" void port_mouse_raw_wheel(int delta);
extern "C" void port_mouse_raw_button(int button, int down);
extern "C" void port_mouse_raw_reset(void);
static WNDPROC g_gameWndProc;
static void rawMouse(const RAWMOUSE &m)
{
	if (m.usFlags & MOUSE_MOVE_ABSOLUTE) {        // tablets, remote desktop, VMs: 0..65535 over the (virtual) screen
		static LONG lastX = -1, lastY = -1;
		int vd = (m.usFlags & MOUSE_VIRTUAL_DESKTOP) != 0;
		int w = GetSystemMetrics(vd ? SM_CXVIRTUALSCREEN : SM_CXSCREEN), h = GetSystemMetrics(vd ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);
		LONG x = MulDiv(m.lLastX, w, 65535), y = MulDiv(m.lLastY, h, 65535);
		if (lastX >= 0) port_mouse_raw_move(x - lastX, y - lastY);
		lastX = x; lastY = y;
	} else if (m.lLastX || m.lLastY) {
		port_mouse_raw_move(m.lLastX, m.lLastY);
	}
	static const USHORT down[5] = { RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_3_DOWN, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_5_DOWN };
	static const USHORT up[5] = { RI_MOUSE_BUTTON_1_UP, RI_MOUSE_BUTTON_2_UP, RI_MOUSE_BUTTON_3_UP, RI_MOUSE_BUTTON_4_UP, RI_MOUSE_BUTTON_5_UP };
	for (int b = 0; b < 5; b++) {                 // DirectInput order: left, right, middle, side buttons
		if (m.usButtonFlags & down[b]) port_mouse_raw_button(b, 1);
		if (m.usButtonFlags & up[b]) port_mouse_raw_button(b, 0);
	}
	if (m.usButtonFlags & RI_MOUSE_WHEEL) port_mouse_raw_wheel((SHORT)m.usButtonData);
}
extern "C" int port_real_input_allowed(void);   // mouse.cpp: false in scripted runs (PORT_INPUT / PORT_KEYS / PORT_MOUSE)
#ifdef ZOOSTERS_OPTIONS
extern "C" void port_window_toggle_fullscreen(void);   // port/options/display.cpp
extern "C" int port_window_mode(void);                 // port/options/display.cpp: 0 windowed, 1 borderless, 2 exclusive full screen
#endif
static void exclusiveFocus(bool active);
static LRESULT CALLBACK portWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_ACTIVATEAPP) exclusiveFocus(wp != 0);   // the game sees it too
	if (((msg >= WM_KEYFIRST && msg <= WM_KEYLAST) || (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)) && !port_real_input_allowed())
		return DefWindowProcA(hwnd, msg, wp, lp);   // a scripted run gets no real keyboard / mouse messages (someone using the PC)
#ifdef ZOOSTERS_OPTIONS
	if (msg == WM_SYSKEYDOWN && wp == VK_RETURN) {   // Alt+Enter: windowed <-> full screen (the repeat of a held key is ignored)
		if (!(lp & (1 << 30))) port_window_toggle_fullscreen();
		return 0;
	}
#endif
	if (msg == WM_INPUT) {
		RAWINPUT ri;
		UINT size = sizeof ri;
		if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, &ri, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1 && ri.header.dwType == RIM_TYPEMOUSE)
			rawMouse(ri.data.mouse);
	} else if (msg == WM_KILLFOCUS || (msg == WM_ACTIVATE && LOWORD(wp) == WA_INACTIVE)) {
		port_mouse_raw_reset();
	}
#ifdef ZOOSTERS_DEBUGUI
	long long eaten;   // the debug overlay sees every message first; what it uses (clicks on its windows, typing) the game does not get
	if (port_debugui_wndproc(hwnd, msg, (unsigned long long)wp, (long long)lp, &eaten)) return 0;
#endif
	return CallWindowProcA(g_gameWndProc, hwnd, msg, wp, lp);
}

// ---- window metrics for the resolution policy (widescreen.cpp port_wide_resolution); the headless platform (window.cpp) has fallbacks
static const DWORD kGameWindowStyle = 0x80C80000;   // WS_POPUP | WS_CAPTION | WS_SYSMENU: the bordered window CreateGameWindow makes
static int g_dispMode;                // port_platform_apply_display: 0 windowed, 1 borderless, 2 exclusive
static DEVMODEW g_exclusiveMode;      // the display mode of 2
static DEVMODEW g_desktopMode;        // the mode before 2 changed it, restored as is (the registry's can be a stale other one)

// The desktop's display mode: while exclusive full screen has changed it, the one the display returns to.
static bool desktopMode(DEVMODEW *dm)
{
	if (g_dispMode == 2) { *dm = g_desktopMode; return dm->dmPelsWidth && dm->dmPelsHeight; }
	memset(dm, 0, sizeof *dm);
	dm->dmSize = sizeof *dm;
	return EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, dm) && dm->dmPelsWidth && dm->dmPelsHeight;
}

// Per-monitor DPI awareness (Windows 10 1703+, else system aware), before any window exists: sizes and display queries are then
// physical pixels, not DWM-scaled ones.
extern "C" void port_platform_dpi_aware(void)
{
	static int done;
	if (done) return;
	done = 1;
	typedef BOOL (WINAPI *SetCtx)(HANDLE);
	HMODULE u = GetModuleHandleA("user32.dll");
	SetCtx f = u ? (SetCtx)GetProcAddress(u, "SetProcessDpiAwarenessContext") : 0;
	BOOL ok = f ? f((HANDLE)-4) : FALSE;   // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
	if (!ok) ok = SetProcessDPIAware();
	if (port_log) fprintf(port_log, "window_w32: DPI awareness %s\n", ok ? "set" : "not set");
}

// Pixels per display point: the window's DPI / 96 (1.5 at 150 %). GetDpiForWindow needs Windows 10 1607+; before that (or without
// a window) the screen DC's DPI. Returns 1 on success.
extern "C" int port_platform_display_scale(float *scale)
{
	typedef UINT (WINAPI *GetDpi)(HWND);
	HMODULE u = GetModuleHandleA("user32.dll");
	GetDpi f = u ? (GetDpi)GetProcAddress(u, "GetDpiForWindow") : 0;
	UINT dpi = f && g_mainWnd ? f((HWND)g_mainWnd) : 0;
	if (!dpi) {
		HDC dc = GetDC(0);
		dpi = dc ? (UINT)GetDeviceCaps(dc, 88 /* LOGPIXELSY */) : 0;
		if (dc) ReleaseDC(0, dc);
	}
	*scale = dpi ? dpi / 96.0f : 1.0f;
	return dpi != 0;
}

// The primary display's current mode in physical pixels. Returns 1 on success.
extern "C" int port_platform_desktop_size(int *w, int *h)
{
	DEVMODEW dm;
	if (!desktopMode(&dm)) return 0;
	*w = (int)dm.dmPelsWidth; *h = (int)dm.dmPelsHeight;
	return 1;
}

// The primary monitor's work area (screen minus task bar) and the extra width / height the window frame adds to a client size.
extern "C" int port_platform_work_area(int *w, int *h, int *frameW, int *frameH)
{
	POINT origin = { 0, 0 };
	MONITORINFO mi;
	memset(&mi, 0, sizeof mi);
	mi.cbSize = sizeof mi;
	if (!GetMonitorInfoW(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &mi)) return 0;
	RECT fr = { 0, 0, 0, 0 };
	AdjustWindowRect(&fr, kGameWindowStyle, FALSE);
	*w = mi.rcWork.right - mi.rcWork.left; *h = mi.rcWork.bottom - mi.rcWork.top;
	DEVMODEW dm;
	if (g_dispMode == 2 && desktopMode(&dm)) {   // the desktop's work area: its size less what the task bar takes now
		*w = (int)dm.dmPelsWidth - ((mi.rcMonitor.right - mi.rcMonitor.left) - *w);
		*h = (int)dm.dmPelsHeight - ((mi.rcMonitor.bottom - mi.rcMonitor.top) - *h);
	}
	*frameW = fr.right - fr.left; *frameH = fr.bottom - fr.top;
	return *w > 0 && *h > 0;
}

// The primary monitor's full rectangle (full screen sizes). Same coordinate space as the window whatever the DPI mode. Returns 1
// on success.
extern "C" int port_platform_screen_size(int *w, int *h)
{
	POINT origin = { 0, 0 };
	MONITORINFO mi;
	memset(&mi, 0, sizeof mi);
	mi.cbSize = sizeof mi;
	if (!GetMonitorInfoW(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &mi)) return 0;
	*w = mi.rcMonitor.right - mi.rcMonitor.left; *h = mi.rcMonitor.bottom - mi.rcMonitor.top;
	return *w > 0 && *h > 0;
}

// ---- display changes at run time (platform.h): the window's style and size. librw resets the D3D9 device to the new client size at
// the next camera update, the camera rasters follow through port_camera_resize (rwcore.cpp). Exclusive full screen is a display mode
// change (ChangeDisplaySettingsEx) under the borderless window, not an exclusive device; the desktop mode comes back while the game
// is in the background (WM_ACTIVATEAPP in portWndProc).
extern "C" int port_platform_display_modes(int *wh, int max)
{
	DEVMODEW dm;
	int n = 0;
	for (DWORD i = 0; ; i++) {
		memset(&dm, 0, sizeof dm);
		dm.dmSize = sizeof dm;
		if (!EnumDisplaySettingsW(NULL, i, &dm)) break;
		if (dm.dmBitsPerPel != 32) continue;
		int w = (int)dm.dmPelsWidth, h = (int)dm.dmPelsHeight, k = 0;
		while (k < n && !(wh[2 * k] == w && wh[2 * k + 1] == h)) k++;
		if (k == n && n < max) { wh[2 * n] = w; wh[2 * n + 1] = h; n++; }
	}
	return n;
}
// The 32-bit mode of w x h nearest the desktop's refresh rate.
static bool findMode(int w, int h, DEVMODEW *out)
{
	DEVMODEW desk, dm;
	int want = desktopMode(&desk) ? (int)desk.dmDisplayFrequency : 60, best = -1;
	for (DWORD i = 0; ; i++) {
		memset(&dm, 0, sizeof dm);
		dm.dmSize = sizeof dm;
		if (!EnumDisplaySettingsW(NULL, i, &dm)) break;
		if (dm.dmBitsPerPel != 32 || (int)dm.dmPelsWidth != w || (int)dm.dmPelsHeight != h) continue;
		int diff = abs((int)dm.dmDisplayFrequency - want);
		if (best < 0 || diff < best) { best = diff; *out = dm; }
	}
	if (best < 0) return false;
	out->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	return true;
}
static void monitorRect(RECT *r, bool work)
{
	MONITORINFO mi;
	memset(&mi, 0, sizeof mi);
	mi.cbSize = sizeof mi;
	GetMonitorInfoW(MonitorFromWindow((HWND)g_mainWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
	*r = work ? mi.rcWork : mi.rcMonitor;
}
static void restoreDesktop(void) { ChangeDisplaySettingsExW(NULL, &g_desktopMode, NULL, 0, NULL); }
static void setBorderless(void)
{
	HWND hw = (HWND)g_mainWnd;
	RECT m;
	monitorRect(&m, false);
	SetWindowLongPtrA(hw, GWL_STYLE, WS_POPUP | WS_VISIBLE);
	SetWindowLongPtrA(hw, GWL_EXSTYLE, 0);
	SetWindowPos(hw, HWND_TOP, m.left, m.top, m.right - m.left, m.bottom - m.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}
static void setWindowed(int w, int h)
{
	HWND hw = (HWND)g_mainWnd;
	RECT rc = { 0, 0, w, h }, wa;
	AdjustWindowRect(&rc, kGameWindowStyle, FALSE);
	int cx = rc.right - rc.left, cy = rc.bottom - rc.top;
	monitorRect(&wa, true);
	SetWindowLongPtrA(hw, GWL_STYLE, kGameWindowStyle | WS_VISIBLE);
	SetWindowLongPtrA(hw, GWL_EXSTYLE, 0);
	SetWindowPos(hw, HWND_NOTOPMOST, wa.left + (wa.right - wa.left - cx) / 2, wa.top + (wa.bottom - wa.top - cy) / 2, cx, cy, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}
extern "C" int port_platform_apply_display(int mode, int w, int h)
{
	if (!g_mainWnd || mode < 0 || mode > 2) return 0;
	bool ok = true;
	if (mode == 2) {
		DEVMODEW dm;
		if (g_dispMode != 2) desktopMode(&g_desktopMode);
		ok = findMode(w, h, &dm) && ChangeDisplaySettingsExW(NULL, &dm, NULL, CDS_FULLSCREEN, NULL) == DISP_CHANGE_SUCCESSFUL;
		if (ok) { g_exclusiveMode = dm; setBorderless(); }
	} else {
		if (g_dispMode == 2) restoreDesktop();
		if (mode == 1) setBorderless();
		else if (w > 0 && h > 0) setWindowed(w, h);
		else ok = false;
	}
	RECT cr = { 0, 0, 0, 0 };
	GetClientRect((HWND)g_mainWnd, &cr);
	if (ok && mode != 1 && (abs(cr.right - w) > 1 || abs(cr.bottom - h) > 1)) ok = false;
	if (ok) g_dispMode = mode;   // g_wndWindowed stays 1: this backend owns the window (GameWndProc's WM_SETFOCUS would maximise it)
	if (port_log) fprintf(port_log, "window_w32: apply display mode %d %dx%d -> %s, client %ldx%ld\n", mode, w, h, ok ? "ok" : "FAILED", cr.right, cr.bottom);
	return ok;
}
// Exclusive full screen in the background: the desktop's mode while another application is in front, the game's when it returns.
static void exclusiveFocus(bool active)
{
	if (g_dispMode != 2) return;
	if (active) {
		ChangeDisplaySettingsExW(NULL, &g_exclusiveMode, NULL, CDS_FULLSCREEN, NULL);
		setBorderless();
	} else {
		restoreDesktop();
	}
}

int CreateGameWindow(const char *cls, const char *title, int x, int y, int w, int h, void *wndProc, void *hInst, int icon, bool fullscreen)
{
#ifdef ZOOSTERS_OPTIONS
	int wmode = port_window_mode();   // the window is made bordered, then port_platform_apply_display sets the stored mode
	fullscreen = 0;
#else
	const char *fs = getenv("PORT_FULLSCREEN");
	if (!(fs && fs[0] == '1')) fullscreen = 0;
#endif
	// the port's window is bordered: tell the game, or GameWndProc's WM_SETFOCUS maximises it (ShowWindow 3) and librw resizes
	// the back buffer to the desktop while the camera's Z raster keeps the video mode size (a smaller depth-stencil surface: the
	// D3D9 driver dropped every triangle draw)
	if (!fullscreen) g_wndWindowed = 1;
	WNDCLASSEXA wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = 0x40;
	g_gameWndProc = (WNDPROC)wndProc;
	wc.lpfnWndProc = portWndProc;
	wc.hInstance = (HINSTANCE)hInst;
	if (icon) wc.hIcon = LoadIconA((HINSTANCE)hInst, MAKEINTRESOURCEA(icon));
	wc.lpszClassName = cls;
	RegisterClassExA(&wc);
	DWORD style, exStyle = 0;
	int cx = w, cy = h;
	if (!fullscreen) {
		style = kGameWindowStyle;
		RECT rc = { 0, 0, w, h };
		AdjustWindowRect(&rc, style, FALSE);
		cx = rc.right - rc.left; cy = rc.bottom - rc.top;
		if (x == -1) x = (GetSystemMetrics(SM_CXSCREEN) - cx) / 2;
		if (y == -1) y = (GetSystemMetrics(SM_CYSCREEN) - cy) / 2;
	} else {
		style = WS_POPUP; exStyle = WS_EX_TOPMOST;
		x = 0; y = 0; cx = GetSystemMetrics(SM_CXSCREEN); cy = GetSystemMetrics(SM_CYSCREEN);
	}
	SetLastError(0);
	g_mainWnd = CreateWindowExA(exStyle, cls, title, style, x, y, cx, cy, 0, 0, (HINSTANCE)hInst, 0);
	if (port_log) fprintf(port_log, "window_w32: CreateGameWindow(class=%s title=%s %dx%d fullscreen=%d) -> hwnd %p (err %lu)\n", cls, title, w, h, fullscreen, g_mainWnd, GetLastError());
	if (g_mainWnd) {
#ifdef ZOOSTERS_OPTIONS
		if (wmode == 2 && !port_platform_apply_display(2, w, h)) {
			if (port_log) fprintf(port_log, "window_w32: exclusive full screen %dx%d unavailable, borderless instead\n", w, h);
			port_platform_apply_display(1, 0, 0);
		} else if (wmode == 1) {
			port_platform_apply_display(1, 0, 0);
		}
#endif
		// the windowed mode the game picks must be the client size (Settings\Display\Resolution), or its camera rasters keep the
		// shim's 800x600 and draw into a corner
		RECT cr;
		if (GetClientRect((HWND)g_mainWnd, &cr) && cr.right > 0 && cr.bottom > 0) port_set_video_mode(cr.right, cr.bottom);
		RAWINPUTDEVICE rid = { 0x01, 0x02, 0, (HWND)g_mainWnd };   // generic desktop / mouse, while the window is in front
		BOOL ok = RegisterRawInputDevices(&rid, 1, sizeof rid);
		if (port_log) fprintf(port_log, "window_w32: raw mouse input %s\n", ok ? "registered" : "not available");
		ShowWindow((HWND)g_mainWnd, SW_SHOW); UpdateWindow((HWND)g_mainWnd);
	}
	return g_mainWnd != 0;
}

// ---- input queries of the platform seam (port/provisional/platform.h)
extern "C" int port_platform_focused(void) { return g_mainWnd && GetForegroundWindow() == (HWND)g_mainWnd; }

extern "C" int port_platform_client_size(int *w, int *h)
{
	RECT rc;
	if (g_mainWnd && GetClientRect((HWND)g_mainWnd, &rc)) { *w = rc.right - rc.left; *h = rc.bottom - rc.top; return 1; }
	*w = 800; *h = 600;
	return 0;
}

// A key's DIK code is its scan code; extended keys (E0 prefix) carry bit 7, as DirectInput reports them. MAPVK_VK_TO_VSC_EX marks only
// some of them (right Ctrl / Alt): the arrows, Insert / Delete / Home / End / Page Up / Down come back as the keypad's codes
// (VK_UP -> 0x48, DIK_NUMPAD8, not DIK_UP 0xC8: the menus' arrow bindings never fired), so those virtual keys are extended here.
static bool extendedVk(int vk)
{
	switch (vk) {
	case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME: case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
	case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK: case VK_RCONTROL: case VK_RMENU: case VK_LWIN: case VK_RWIN: case VK_APPS:
		return true;
	}
	return false;
}
extern "C" void port_platform_keyboard(unsigned char *cur)
{
	for (int vk = 8; vk < 0xff; vk++) {
		if (vk == 0x10 || vk == 0x11 || vk == 0x12) continue;   // VK_SHIFT / CONTROL / MENU: the left / right keys report themselves
		if (GetAsyncKeyState(vk) & 0x8000) {
			unsigned sc = MapVirtualKeyA(vk, 4);   // MAPVK_VK_TO_VSC_EX: 0xE0xx for (some) extended keys
			unsigned dik = (sc & 0x7f) | ((sc & 0xff00) == 0xe000 || extendedVk(vk) ? 0x80 : 0);
			if (sc & 0x7f) cur[dik] = 0x80;
		}
	}
}

extern "C" int port_platform_pointer(int *x, int *y, int *leftDown)
{
	*leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
	CURSORINFO ci; ci.cbSize = sizeof ci;
	// GetCursorInfo, not GetCursorPos (its IAT slot is patched to port_mouse_get in the address-bound port). Bound at run time:
	// the VC98 user32.lib predates GetCursorInfo.
	typedef BOOL (WINAPI *GetCursorInfoFn)(PCURSORINFO);
	static GetCursorInfoFn pGetCursorInfo; static int bound;
	if (!bound) { bound = 1; HMODULE u = GetModuleHandleA("user32.dll"); if (u) pGetCursorInfo = (GetCursorInfoFn)GetProcAddress(u, "GetCursorInfo"); }
	if (!g_mainWnd || !pGetCursorInfo || !pGetCursorInfo(&ci)) return 0;
	POINT p = ci.ptScreenPos;
	if (!ScreenToClient((HWND)g_mainWnd, &p)) return 0;
	int w, h;
	port_platform_client_size(&w, &h);
	if (p.x < 0 || p.y < 0 || p.x >= w || p.y >= h) return 0;
	*x = p.x; *y = p.y;
	return 1;
}

extern "C" void port_platform_post_pointer(unsigned msg, int x, int y, int leftDown)
{
	if (g_mainWnd) PostMessageA((HWND)g_mainWnd, msg, leftDown ? MK_LBUTTON : 0, MAKELPARAM(x, y));
}
extern "C" void port_platform_request_close(void) { if (g_mainWnd) PostMessageA((HWND)g_mainWnd, WM_CLOSE, 0, 0); }

// ---- harness side (same entry points as the headless shim)
static DWORD WINAPI watchdog(void *)
{
	for (;;) {
		Sleep(50);
		int late = GetTickCount() - g_startTick > 120000;
		if (late && g_mainWnd) {
			if (port_log) fprintf(port_log, "window_w32: watchdog timeout after %d presented frames, posting WM_CLOSE\n", port_frames_presented());
			port_platform_request_close();
			return 0;
		}
	}
}
// The frame limit is checked on the game thread after every Present: WM_CLOSE is posted right after present number `frames`, so
// every run ends after the same frame (the watchdog thread used to post it 0-50 ms late).
extern "C" void (*port_present_hook)(void);
static void (*g_prevPresentHook)(void);
static int g_closePosted;
static void limitHook(void)
{
	if (g_prevPresentHook) g_prevPresentHook();
	if (!g_closePosted && port_frames_presented() >= g_maxFrames && g_mainWnd) {
		g_closePosted = 1;
		if (port_log) fprintf(port_log, "window_w32: frame limit after %d presented frames, posting WM_CLOSE\n", port_frames_presented());
		port_platform_request_close();
	}
}
// n <= 0: play mode (a person at the keyboard): no frame limit and no 120 s timeout, the window stays until it is closed
extern "C" void port_win_set_frames(int n)
{
	g_maxFrames = n;
	g_startTick = GetTickCount();
	if (n > 0) {
		g_prevPresentHook = port_present_hook;
		port_present_hook = &limitHook;
		CreateThread(0, 0, watchdog, 0, 0, 0);
	}
}
extern "C" int port_win_pumps(void) { return port_frames_presented(); }
