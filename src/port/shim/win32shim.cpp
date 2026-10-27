// Headless Win32 for the start-up chain (WinMain -> window -> message pump -> GameWndProc): user32 / advapi32 / winmm imports;
// kernel32 stays real. The "window" is a fake handle with a message queue: WM_SETFOCUS right after creation, WM_CLOSE after
// `port_win_set_frames(n)` empty pumps. Registry reads fail (ERROR_FILE_NOT_FOUND) so CfgObj uses its defaults; timeGetTime is a
// deterministic counter.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" FILE *port_log;
#define WLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)
typedef long (__stdcall *WndProcFn)(void *hwnd, unsigned msg, unsigned wParam, long lParam);
struct Msg { void *hwnd; unsigned message; unsigned wParam; long lParam; unsigned time; long x, y; };
struct Rect { long left, top, right, bottom; };

static Msg g_queue[64];
static int g_qhead, g_qtail;
static void *g_hwnd;
static WndProcFn g_wndProc;
static int g_width = 800, g_height = 600;
static int g_maxPumps = -1;          // -1 = never post WM_CLOSE
static int g_emptyPumps;             // PeekMessage calls that found the queue empty (one per PumpMessages, i.e. per frame)
static int g_closePosted;
static unsigned g_ms = 1000;         // timeGetTime
static int g_cursorCount;

static void post(unsigned m, unsigned w, long l)
{
	int next = (g_qtail + 1) % 64;
	if (next == g_qhead) return;
	Msg &q = g_queue[g_qtail];
	q.hwnd = g_hwnd; q.message = m; q.wParam = w; q.lParam = l; q.time = g_ms; q.x = 0; q.y = 0;
	g_qtail = next;
}

// ---- harness API
extern "C" void port_win_set_frames(int n) { g_maxPumps = n; }
extern "C" int port_win_pumps(void) { return g_emptyPumps; }
extern "C" void port_win_post(unsigned msg, unsigned wParam, long lParam) { post(msg, wParam, lParam); }
extern "C" void *port_win_create(int w, int h, void *wndProc)
{
	g_hwnd = (void *)0x10001;
	g_wndProc = (WndProcFn)wndProc;
	if (w > 0) g_width = w;
	if (h > 0) g_height = h;
	WLOG("win32: window %dx%d wndproc=%p\n", g_width, g_height, wndProc);
	post(7, 0, 0);                   // WM_SETFOCUS: GameWndProc -> App_SetActive (g_frameReady = 1)
	return g_hwnd;
}

// ---- user32: messages
static int __stdcall PeekMessageA_(Msg *m, void *hwnd, unsigned lo, unsigned hi, unsigned remove)
{
	(void)hwnd; (void)lo; (void)hi;
	if (g_qhead == g_qtail) {
		g_emptyPumps++;
		g_ms += 16;
		if (g_maxPumps >= 0 && g_emptyPumps > g_maxPumps && !g_closePosted) {
			g_closePosted = 1;
			WLOG("win32: %d pumps done, posting WM_CLOSE\n", g_emptyPumps - 1);
			post(0x10, 0, 0);
		} else {
			return 0;
		}
	}
	*m = g_queue[g_qhead];
	if (remove & 1) g_qhead = (g_qhead + 1) % 64;
	return 1;
}
static int __stdcall TranslateMessage_(const Msg *) { return 0; }
static long __stdcall DispatchMessageA_(const Msg *m)
{
	if (!g_wndProc) return 0;
	WLOG("win32: dispatch msg %04x w=%x l=%lx\n", m->message, m->wParam, m->lParam);
	return g_wndProc(m->hwnd, m->message, m->wParam, m->lParam);
}
static long __stdcall SendMessageA_(void *hwnd, unsigned msg, unsigned w, long l) { return g_wndProc ? g_wndProc(hwnd, msg, w, l) : 0; }
static long __stdcall DefWindowProcA_(void *, unsigned, unsigned, long) { return 0; }
static void __stdcall PostQuitMessage_(int code) { WLOG("win32: PostQuitMessage(%d)\n", code); post(0x12, (unsigned)code, 0); }
static int __stdcall DestroyWindow_(void *) { WLOG("win32: DestroyWindow\n"); return 1; }
static int __stdcall ShowWindow_(void *, int cmd) { WLOG("win32: ShowWindow(%d)\n", cmd); return 1; }
static int __stdcall UpdateWindow_(void *) { return 1; }
static int __stdcall ShowCursor_(int show) { g_cursorCount += show ? 1 : -1; return g_cursorCount; }
// MapVirtualKeyA: headless has no keyboard layout, every mapping is 0 (empty key names); the windowed build uses the real user32.
static unsigned __stdcall MapVirtualKeyA_(unsigned code, unsigned type) { (void)code; (void)type; return 0; }
// The provisional InputManager::Poll / mouse read the real keyboard only while the window has focus, which headless never has (input is
// PORT_KEYS).
static void *__stdcall GetForegroundWindow_(void) { return 0; }
static short __stdcall GetAsyncKeyState_(int vk) { (void)vk; return 0; }
static void *__stdcall SetCapture_(void *) { return 0; }
static int __stdcall SetCursorPos_(int, int) { return 1; }
static int __stdcall GetCursorPos_(long *pt) { pt[0] = g_width / 2; pt[1] = g_height / 2; return 1; }
static int __stdcall ClientToScreen_(void *, long *) { return 1; }
static int __stdcall GetClientRect_(void *, Rect *r) { r->left = 0; r->top = 0; r->right = g_width; r->bottom = g_height; return 1; }
static int __stdcall InvalidateRect_(void *, const Rect *, int) { return 1; }
static int __stdcall ValidateRect_(void *, const Rect *) { return 1; }
static int __stdcall PtInRect_(const Rect *r, long x, long y) { return x >= r->left && x < r->right && y >= r->top && y < r->bottom; }
static int __stdcall MessageBoxA_(void *, const char *text, const char *caption, unsigned type)
{
	printf("win32: MessageBox [%s] %s\n", caption ? caption : "", text ? text : "");
	WLOG("win32: MessageBox [%s] %s (type %x)\n", caption ? caption : "", text ? text : "", type);
	return 1;
}
static char *__stdcall CharUpperA_(char *s)
{
	if ((unsigned)(size_t)s >> 16 == 0) return s;         // a single character packed in the pointer
	for (char *p = s; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 32;
	return s;
}
static int __stdcall SystemParametersInfoA_(unsigned action, unsigned param, void *pv, unsigned)
{
	switch (action) {
	case 0x0e: *(int *)pv = 0; return 1;                                   // SPI_GETSCREENSAVEACTIVE
	case 0x3a: ((unsigned *)pv)[0] = 8; ((unsigned *)pv)[1] = 0; return 1; // SPI_GETSTICKYKEYS
	case 0x70: *(int *)pv = 10; return 1;                                  // SPI_GETMOUSESPEED
	default: (void)param; return 1;
	}
}
// DEVMODEA: dmSize +0x24, dmFields +0x28, dmBitsPerPel +0x68, dmPelsWidth +0x6c, dmPelsHeight +0x70 (size 0x9c)
static int __stdcall EnumDisplaySettingsA_(const char *, unsigned, unsigned char *dm)
{
	memset(dm, 0, 0x9c);
	*(unsigned short *)(dm + 0x24) = 0x9c;
	*(unsigned *)(dm + 0x28) = 0x1c0000;
	*(unsigned *)(dm + 0x68) = 32;
	*(unsigned *)(dm + 0x6c) = 1024;
	*(unsigned *)(dm + 0x70) = 768;
	return 1;
}
static long __stdcall ChangeDisplaySettingsA_(void *dm, unsigned flags) { WLOG("win32: ChangeDisplaySettings(%p, %x)\n", dm, flags); return 0; }

// ---- advapi32: registry (nothing stored: every lookup is "not found", every write succeeds silently)
static long __stdcall RegOpenKeyA_(void *, const char *sub, void **out) { WLOG("win32: RegOpenKey %s -> not found\n", sub ? sub : ""); *out = 0; return 2; }
static long __stdcall RegCreateKeyA_(void *, const char *sub, void **out) { WLOG("win32: RegCreateKey %s\n", sub ? sub : ""); *out = (void *)0x20001; return 0; }
static long __stdcall RegQueryValueExA_(void *, const char *, unsigned *, unsigned *, unsigned char *, unsigned *) { return 2; }
static long __stdcall RegSetValueExA_(void *, const char *name, unsigned, unsigned type, const unsigned char *data, unsigned size)
{
	if (type == 4 && size == 4) WLOG("win32: RegSetValue %s = %d\n", name ? name : "", *(const int *)data);
	else WLOG("win32: RegSetValue %s (type %u, %u bytes)\n", name ? name : "", type, size);
	return 0;
}

// ---- winmm
static unsigned __stdcall timeGetTime_(void) { return g_ms++; }
static unsigned __stdcall timeBeginPeriod_(unsigned) { return 0; }
static unsigned __stdcall timeEndPeriod_(unsigned) { return 0; }
static unsigned __stdcall timeGetDevCaps_(unsigned *caps, unsigned) { caps[0] = 1; caps[1] = 1000000; return 0; }

// The exe imports these like any Win32 program; the headless versions replace the import slots before main (port_run.cpp port_iat_hook).
extern "C" void *port_iat_hook(const char *dll, const char *function, void *fn);
static const struct { const char *dll, *function; void *impl; } g_headless[] = {
	{ "user32.dll", "PeekMessageA", (void *)&PeekMessageA_ },
	{ "user32.dll", "TranslateMessage", (void *)&TranslateMessage_ },
	{ "user32.dll", "DispatchMessageA", (void *)&DispatchMessageA_ },
	{ "user32.dll", "SendMessageA", (void *)&SendMessageA_ },
	{ "user32.dll", "DefWindowProcA", (void *)&DefWindowProcA_ },
	{ "user32.dll", "PostQuitMessage", (void *)&PostQuitMessage_ },
	{ "user32.dll", "DestroyWindow", (void *)&DestroyWindow_ },
	{ "user32.dll", "ShowWindow", (void *)&ShowWindow_ },
	{ "user32.dll", "UpdateWindow", (void *)&UpdateWindow_ },
	{ "user32.dll", "ShowCursor", (void *)&ShowCursor_ },
	{ "user32.dll", "MapVirtualKeyA", (void *)&MapVirtualKeyA_ },
	{ "user32.dll", "GetForegroundWindow", (void *)&GetForegroundWindow_ },
	{ "user32.dll", "GetAsyncKeyState", (void *)&GetAsyncKeyState_ },
	{ "user32.dll", "SetCapture", (void *)&SetCapture_ },
	{ "user32.dll", "SetCursorPos", (void *)&SetCursorPos_ },
	{ "user32.dll", "GetCursorPos", (void *)&GetCursorPos_ },
	{ "user32.dll", "ClientToScreen", (void *)&ClientToScreen_ },
	{ "user32.dll", "GetClientRect", (void *)&GetClientRect_ },
	{ "user32.dll", "InvalidateRect", (void *)&InvalidateRect_ },
	{ "user32.dll", "ValidateRect", (void *)&ValidateRect_ },
	{ "user32.dll", "PtInRect", (void *)&PtInRect_ },
	{ "user32.dll", "MessageBoxA", (void *)&MessageBoxA_ },
	{ "user32.dll", "CharUpperA", (void *)&CharUpperA_ },
	{ "user32.dll", "SystemParametersInfoA", (void *)&SystemParametersInfoA_ },
	{ "user32.dll", "EnumDisplaySettingsA", (void *)&EnumDisplaySettingsA_ },
	{ "user32.dll", "ChangeDisplaySettingsA", (void *)&ChangeDisplaySettingsA_ },
	{ "advapi32.dll", "RegOpenKeyA", (void *)&RegOpenKeyA_ },
	{ "advapi32.dll", "RegCreateKeyA", (void *)&RegCreateKeyA_ },
	{ "advapi32.dll", "RegQueryValueExA", (void *)&RegQueryValueExA_ },
	{ "advapi32.dll", "RegSetValueExA", (void *)&RegSetValueExA_ },
	{ "winmm.dll", "timeGetTime", (void *)&timeGetTime_ },
	{ "winmm.dll", "timeBeginPeriod", (void *)&timeBeginPeriod_ },
	{ "winmm.dll", "timeEndPeriod", (void *)&timeEndPeriod_ },
	{ "winmm.dll", "timeGetDevCaps", (void *)&timeGetDevCaps_ },
};
static struct HeadlessInstall {
	HeadlessInstall() { for (unsigned i = 0; i < sizeof g_headless / sizeof g_headless[0]; i++) port_iat_hook(g_headless[i].dll, g_headless[i].function, g_headless[i].impl); }
} g_headlessInstall;
