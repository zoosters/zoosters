// CreateGameWindow (0x519f00) for the windowed build (RegisterClassExA with the icon, WS_POPUP full screen or a centred bordered
// window, CreateWindowExA, g_mainWnd), plus the harness side: a watchdog thread that closes the window after N presented frames.
// Deviation: the port forces a bordered window unless PORT_FULLSCREEN=1 (the game is always full screen on the PC).
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
#endif
static LRESULT CALLBACK portWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
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
	memset(&dm, 0, sizeof dm);
	dm.dmSize = sizeof dm;
	if (!EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm) || dm.dmPelsWidth == 0 || dm.dmPelsHeight == 0) return 0;
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

// The D3D9 back buffer follows the window only at device creation: no display changes at run time.
extern "C" int port_platform_display_modes(int *wh, int max) { (void)wh; (void)max; return 0; }
extern "C" int port_platform_apply_display(int mode, int w, int h) { (void)mode; (void)w; (void)h; return 0; }

int CreateGameWindow(const char *cls, const char *title, int x, int y, int w, int h, void *wndProc, void *hInst, int icon, bool fullscreen)
{
	const char *fs = getenv("PORT_FULLSCREEN");
	if (!(fs && fs[0] == '1')) fullscreen = 0;
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

// A key's DIK code is its scan code; extended keys (E0 prefix) carry bit 7, as DirectInput reports them.
extern "C" void port_platform_keyboard(unsigned char *cur)
{
	for (int vk = 8; vk < 0xff; vk++) {
		if (vk == 0x10 || vk == 0x11 || vk == 0x12) continue;   // VK_SHIFT / CONTROL / MENU: the left / right keys report themselves
		if (GetAsyncKeyState(vk) & 0x8000) {
			unsigned sc = MapVirtualKeyA(vk, 4);   // MAPVK_VK_TO_VSC_EX: 0xE0xx for extended keys
			unsigned dik = (sc & 0x7f) | ((sc & 0xff00) == 0xe000 ? 0x80 : 0);
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
