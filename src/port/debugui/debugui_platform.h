// The overlay's few Win32 dependencies for every platform. Windows: <windows.h>. Elsewhere (window_sdl.cpp feeds
// port_debugui_wndproc the same WM_ messages and VK_ codes): the message / key constants, the types the overlay uses, and
// SDL-backed GetKeyState (modifiers), QueryPerformanceCounter / Frequency, GetClientRect and PostMessageA (WM_CLOSE). Files the
// overlay writes go through port_debugui_file (debugui_rw.cpp), not GetModuleFileNameA.
#ifndef PORT_DEBUGUI_PLATFORM_H
#define PORT_DEBUGUI_PLATFORM_H
#ifdef _WIN32
#include <windows.h>
#define DEBUGUI_SEP "\\"
#else
#include <sys/stat.h>
#define DEBUGUI_SEP "/"
#include <SDL3/SDL.h>
#include <stdint.h>

typedef void *HWND;
typedef unsigned long long WPARAM;
typedef long long LPARAM;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef union { struct { uint32_t LowPart; int32_t HighPart; } u; long long QuadPart; } LARGE_INTEGER;
typedef struct { LONG left, top, right, bottom; } RECT;
#define LOWORD(l) ((unsigned short)((unsigned long long)(l) & 0xffff))
#define HIWORD(l) ((unsigned short)(((unsigned long long)(l) >> 16) & 0xffff))
#define WHEEL_DELTA 120
#define MAX_PATH 1024

enum {
	WM_SETFOCUS = 0x0007, WM_KILLFOCUS = 0x0008, WM_CLOSE = 0x0010, WM_KEYDOWN = 0x0100, WM_KEYUP = 0x0101, WM_CHAR = 0x0102,
	WM_SYSKEYDOWN = 0x0104, WM_SYSKEYUP = 0x0105, WM_MOUSEMOVE = 0x0200, WM_LBUTTONDOWN = 0x0201, WM_LBUTTONUP = 0x0202,
	WM_RBUTTONDOWN = 0x0204, WM_RBUTTONUP = 0x0205, WM_MBUTTONDOWN = 0x0207, WM_MBUTTONUP = 0x0208, WM_MOUSEWHEEL = 0x020A,
};
enum {
	VK_BACK = 0x08, VK_TAB = 0x09, VK_RETURN = 0x0D, VK_SHIFT = 0x10, VK_CONTROL = 0x11, VK_MENU = 0x12, VK_ESCAPE = 0x1B, VK_SPACE = 0x20,
	VK_PRIOR = 0x21, VK_NEXT = 0x22, VK_END = 0x23, VK_HOME = 0x24, VK_LEFT = 0x25, VK_UP = 0x26, VK_RIGHT = 0x27, VK_DOWN = 0x28,
	VK_INSERT = 0x2D, VK_DELETE = 0x2E, VK_F1 = 0x70, VK_F12 = 0x7B, VK_OEM_COMMA = 0xBC, VK_OEM_MINUS = 0xBD, VK_OEM_PERIOD = 0xBE,
};

extern "C" int port_platform_client_size(int *w, int *h);   // port/provisional/window_sdl.cpp: the client area in pixels
extern "C" void port_platform_request_close(void);

static inline short GetKeyState(int vk)                    // only the modifiers are asked for (the high bit = down)
{
	SDL_Keymod m = SDL_GetModState();
	bool down = vk == VK_SHIFT ? (m & SDL_KMOD_SHIFT) != 0 : vk == VK_CONTROL ? (m & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) != 0
	          : vk == VK_MENU ? (m & SDL_KMOD_ALT) != 0 : false;   // Cmd counts as Ctrl on macOS (copy / paste / undo in the overlay)
	return down ? (short)0x8000 : 0;
}
static inline int QueryPerformanceCounter(LARGE_INTEGER *t) { t->QuadPart = (long long)SDL_GetPerformanceCounter(); return 1; }
static inline int QueryPerformanceFrequency(LARGE_INTEGER *t) { t->QuadPart = (long long)SDL_GetPerformanceFrequency(); return 1; }
static inline int GetClientRect(HWND, RECT *r)
{
	int w = 0, h = 0;
	port_platform_client_size(&w, &h);
	r->left = r->top = 0;
	r->right = w;
	r->bottom = h;
	return w > 0 && h > 0;
}
static inline int CreateDirectoryA(const char *path, void *) { return mkdir(path, 0755) == 0; }
static inline int PostMessageA(HWND, unsigned msg, WPARAM, LPARAM)
{
	if (msg == WM_CLOSE)
		port_platform_request_close();
	return 1;
}
#endif

// A file the overlay writes (debugui.ini, recordings, captures): next to the exe on Windows, in the user folder elsewhere (a
// macOS bundle is read-only). Returns out, or 0 when no folder is known.
const char *port_debugui_file(const char *name, char *out, int size);
#endif
