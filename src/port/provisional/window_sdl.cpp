// SDL3 window backend (--window sdl, the default with --platform gl3); counterpart of window_w32.cpp behind platform.h, like re3's skel/glfw/glfw.cpp next to skel/win/win.cpp.
//   CreateGameWindow (0x519f00)  one SDL window (SDL_WINDOW_OPENGL for librw gl3, which creates its context on it)
//   PumpMessages (0x51a100)      replaced (SDL_REPLACES leaves out pumpmessages.cpp): SDL events become the window messages GameWndProc (0x464370) handles, so the game's own
//                                focus / quit / menu pointer logic runs unchanged: focus gained 7, lost 8, close 0x10, key down 0x100, pointer 0x200 / 0x201 / 0x202 (client pixels)
//   input                        keyboard as DirectInput codes (SDL scancodes are USB HID usages, DIK codes set-1 scan codes); mouse motion / buttons / wheel into mouse.cpp
// Differs from window_w32.cpp: mouse deltas are SDL's (system acceleration applied; Win32 raw input is unaccelerated); the screen saver is held off with SDL_DisableScreenSaver.
#include <SDL3/SDL.h>
#ifdef _WIN32   // not <windows.h>: its WinMain declaration clashes with the game's (appstate.h)
struct PortRect { long left, top, right, bottom; };
extern "C" __declspec(dllimport) int __stdcall AdjustWindowRect(PortRect *rect, unsigned long style, int menu);
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "framework/startup/win32/appstate.h"   // GameWndProc's signature, g_mainWnd, g_wndWindowed, g_quitRequested

extern "C" FILE *port_log;
extern "C" int port_frames_presented(void);           // port/shim/rwcore.cpp (RwCameraShowRaster count)
extern "C" void port_set_video_mode(int w, int h);    // port/shim/rwcore.cpp: the one windowed video mode the game's SelectVideoMode sees
extern "C" void port_mouse_raw_move(int dx, int dy);
extern "C" void port_mouse_raw_wheel(int delta);
extern "C" void port_mouse_raw_button(int button, int down);
extern "C" void port_mouse_raw_reset(void);
#ifdef ZOOSTERS_OPTIONS
extern "C" int port_window_mode(void);                // port/options/display.cpp: 0 windowed, 1 borderless, 2 exclusive full screen
extern "C" void port_window_toggle_fullscreen(void);  // port/options/display.cpp: windowed <-> the last full screen mode
extern "C" void port_display_size_changed(void);      // port/options/display.cpp: the game's screen-sized state follows the window
#endif
extern "C" void port_audio_focus(int focused);   // port/shim/rwaudio_al.cpp: mute while unfocused
#define SLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); fflush(port_log); } } while (0)

typedef WinLResult (__stdcall *GameWndProcFn)(void *hwnd, unsigned msg, WinWParam wParam, WinLParam lParam);
static SDL_Window *g_win;
static GameWndProcFn g_wndProc;
static int g_focusSeen;
static bool g_comboEnter;   // Alt+Enter was taken by the window: the game does not see that Enter press (events or polled state)
static int g_sizeEvent;   // the drawable changed size (our own display change, or the system's): the game's rasters follow in PumpMessages

// PORT_HIDDEN=1: the window is never shown; the game runs and renders as usual
static bool windowHidden()
{
	const char *v = SDL_getenv("PORT_HIDDEN");
	return v && *v == '1';
}

// the SDL window, for librw gl3 (port/shim/rwcore.cpp RwEngineOpen passes it in EngineOpenParams::window)
extern "C" SDL_Window *port_sdl_window(void) { return g_win; }

static int videoUp(void)
{
	static int state = -1;
	if (state < 0) {
		const char *bg = SDL_getenv("PORT_BACKGROUND");   // scripted runs (port_run.cpp): do not take the focus from the user's other work
		if (bg && *bg == '1') {
			SDL_SetHint(SDL_HINT_MAC_BACKGROUND_APP, "1");               // macOS: no Dock icon, the app is not activated
			SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "0");       // the window does not take the focus when it appears
		}
		state = SDL_InitSubSystem(SDL_INIT_VIDEO) ? 1 : 0;
		SLOG("window_sdl: SDL %d.%d.%d video %s%s%s\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION, state ? "up, driver " : "FAILED: ",
		     state ? SDL_GetCurrentVideoDriver() : SDL_GetError(), "");
	}
	return state;
}

// ---- display queries (platform.h). SDL makes the process DPI aware itself (physical pixels on Windows).
extern "C" void port_platform_dpi_aware(void) { videoUp(); }
extern "C" int port_platform_desktop_size(int *w, int *h)
{
	if (!videoUp()) return 0;
	const SDL_DisplayMode *m = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
	if (!m || m->w <= 0 || m->h <= 0) return 0;
	float d = m->pixel_density > 0.0f ? m->pixel_density : 1.0f;
	*w = (int)(m->w * d + 0.5f); *h = (int)(m->h * d + 0.5f);
	return 1;
}
// Display pixels per window coordinate: 2 on a Retina Mac (SDL's window / display coordinates are points there), 1 on Windows.
// Every port_platform_* size is in physical pixels (widescreen.cpp), so the game's resolution, RW's video mode and the GL
// drawable share one coordinate space; only SDL's window calls take points.
static float pixelDensity(void)
{
	float d = g_win ? SDL_GetWindowPixelDensity(g_win) : 0.0f;
	if (d <= 0.0f) {
		const SDL_DisplayMode *m = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
		d = m && m->pixel_density > 0.0f ? m->pixel_density : 1.0f;
	}
	return d;
}
static int toDisplayPixels(int v) { return (int)(v * pixelDensity() + 0.5f); }
// Pixels per display point (HUD scale, port/fixes/widescreen.cpp): pixel density x the OS content scale (macOS Retina 2; Windows
// 1.5 at 150 %).
extern "C" int port_platform_display_scale(float *scale)
{
	float s = 0.0f;
	if (g_win)
		s = SDL_GetWindowDisplayScale(g_win);
	else if (videoUp())                    // before the window (the debug overlay sets up early): the primary display's
		s = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()) * pixelDensity();
	*scale = s > 0.0f ? s : 1.0f;
	return s > 0.0f;
}
extern "C" int port_platform_screen_size(int *w, int *h)
{
	SDL_Rect r;
	if (!videoUp() || !SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &r)) return 0;
	*w = toDisplayPixels(r.w); *h = toDisplayPixels(r.h);
	return r.w > 0 && r.h > 0;
}
extern "C" int port_platform_work_area(int *w, int *h, int *frameW, int *frameH)
{
	SDL_Rect r;
	if (!videoUp() || !SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &r)) return 0;
	*w = toDisplayPixels(r.w); *h = toDisplayPixels(r.h);
	int top = 0, left = 0, bottom = 0, right = 0;
	if (g_win && SDL_GetWindowBordersSize(g_win, &top, &left, &bottom, &right)) { *frameW = toDisplayPixels(left + right); *frameH = toDisplayPixels(top + bottom); }
	else {   // no window yet (the resolution policy runs before CreateGameWindow)
#ifdef _WIN32
		PortRect fr = { 0, 0, 0, 0 };   // the frame of SDL's fixed-size window style (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), as window_w32.cpp measures its own
		AdjustWindowRect(&fr, 0x00ca0000, 0);
		*frameW = fr.right - fr.left; *frameH = fr.bottom - fr.top;
#else
		*frameW = 0; *frameH = toDisplayPixels(28);   // macOS: title bar only (28 points)
#endif
	}
	return *w > 0 && *h > 0;
}

// ---- display changes at run time (platform.h). Sizes are physical pixels; SDL's window calls take points (pixelDensity).
extern "C" int port_platform_display_modes(int *wh, int max)
{
	int count = 0, n = 0;
	if (!videoUp()) return 0;
	SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &count);
	for (int i = 0; modes && i < count; i++) {
		float d = modes[i]->pixel_density > 0.0f ? modes[i]->pixel_density : 1.0f;
		int w = (int)(modes[i]->w * d + 0.5f), h = (int)(modes[i]->h * d + 0.5f), k = 0;
		while (k < n && !(wh[2 * k] == w && wh[2 * k + 1] == h)) k++;
		if (k == n && n < max) { wh[2 * n] = w; wh[2 * n + 1] = h; n++; }
	}
	SDL_free(modes);
	return n;
}
// The exclusive mode of w x h pixels nearest the desktop's refresh rate; the mode is copied into *out.
static bool findMode(int w, int h, SDL_DisplayMode *out)
{
	const SDL_DisplayMode *desk = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
	float want = desk ? desk->refresh_rate : 60.0f, bestDiff = 1e9f;
	int count = 0;
	bool found = false;
	SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &count);
	for (int i = 0; modes && i < count; i++) {
		float d = modes[i]->pixel_density > 0.0f ? modes[i]->pixel_density : 1.0f;
		if ((int)(modes[i]->w * d + 0.5f) != w || (int)(modes[i]->h * d + 0.5f) != h) continue;
		float diff = modes[i]->refresh_rate > want ? modes[i]->refresh_rate - want : want - modes[i]->refresh_rate;
		if (diff < bestDiff) { bestDiff = diff; *out = *modes[i]; found = true; }
	}
	SDL_free(modes);
	return found;
}
extern "C" int port_platform_apply_display(int mode, int w, int h)
{
	if (!g_win) return 0;
	bool full = (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN) != 0, ok = true;
	if (mode == 0) {
		if (full) ok = SDL_SetWindowFullscreen(g_win, false);
		if (ok) SDL_SyncWindow(g_win);   // a size set while the window is still leaving full screen is dropped
		if (ok && w > 0 && h > 0) {
			float d = pixelDensity();
			SDL_SetWindowSize(g_win, (int)(w / d + 0.5f), (int)(h / d + 0.5f));
			SDL_SetWindowPosition(g_win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		}
	} else if (mode == 1) {
		ok = SDL_SetWindowFullscreenMode(g_win, 0) && SDL_SetWindowFullscreen(g_win, true);
	} else {
		SDL_DisplayMode m;
		ok = findMode(w, h, &m) && SDL_SetWindowFullscreenMode(g_win, &m) && SDL_SetWindowFullscreen(g_win, true);
	}
	if (ok) SDL_SyncWindow(g_win);
	int pw = 0, ph = 0;
	SDL_GetWindowSizeInPixels(g_win, &pw, &ph);
	bool isFull = (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN) != 0;
	if (ok && isFull != (mode != 0)) ok = false;
	if (ok) g_wndWindowed = mode == 0;   // GameWndProc and WinMain read it
	if (ok && mode != 1 && (abs(pw - w) > 1 || abs(ph - h) > 1)) ok = false;   // odd sizes round through points
	SLOG("window_sdl: apply display mode %d %dx%d -> %s, window %dx%d pixels%s%s\n", mode, w, h, ok ? "ok" : "FAILED", pw, ph, ok ? "" : ": ", ok ? "" : SDL_GetError());
	return ok;
}

// ---- input (platform.h)
extern "C" int port_platform_focused(void) { return g_win && SDL_GetKeyboardFocus() == g_win; }
extern "C" int port_platform_client_size(int *w, int *h)
{
	if (g_win && SDL_GetWindowSizeInPixels(g_win, w, h) && *w > 0 && *h > 0) return 1;
	*w = 800; *h = 600;
	return 0;
}
// window coordinates (points on macOS) -> client pixels
static void toPixels(float x, float y, int *px, int *py)
{
	int w = 0, h = 0, pw = 0, ph = 0;
	SDL_GetWindowSize(g_win, &w, &h);
	SDL_GetWindowSizeInPixels(g_win, &pw, &ph);
	*px = (int)(w > 0 ? x * pw / w : x);
	*py = (int)(h > 0 ? y * ph / h : y);
}
extern "C" int port_platform_pointer(int *x, int *y, int *leftDown)
{
	float fx = 0, fy = 0;
	SDL_MouseButtonFlags b = SDL_GetMouseState(&fx, &fy);
	*leftDown = (b & SDL_BUTTON_LMASK) ? 1 : 0;
	if (!g_win || SDL_GetMouseFocus() != g_win) return 0;
	int px, py, w, h;
	toPixels(fx, fy, &px, &py);
	port_platform_client_size(&w, &h);
	if (px < 0 || py < 0 || px >= w || py >= h) return 0;
	*x = px; *y = py;
	return 1;
}

// SDL scancode (USB HID usage) -> DirectInput key code (set-1 scan code, 0x80 = E0-extended), 0 = not mapped
static unsigned char hidToDik(int sc)
{
	static const unsigned char letters[26] = { 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, 0x19,
	                                           0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c };   // a..z
	if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) return letters[sc - SDL_SCANCODE_A];
	if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0) return (unsigned char)(0x02 + sc - SDL_SCANCODE_1);   // 1..9, 0 = 0x02..0x0b
	if (sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F10) return (unsigned char)(0x3b + sc - SDL_SCANCODE_F1);
	switch (sc) {
	case SDL_SCANCODE_RETURN: return 0x1c;        case SDL_SCANCODE_ESCAPE: return 0x01;       case SDL_SCANCODE_BACKSPACE: return 0x0e;
	case SDL_SCANCODE_TAB: return 0x0f;           case SDL_SCANCODE_SPACE: return 0x39;        case SDL_SCANCODE_MINUS: return 0x0c;
	case SDL_SCANCODE_EQUALS: return 0x0d;        case SDL_SCANCODE_LEFTBRACKET: return 0x1a;  case SDL_SCANCODE_RIGHTBRACKET: return 0x1b;
	case SDL_SCANCODE_BACKSLASH: return 0x2b;     case SDL_SCANCODE_NONUSHASH: return 0x2b;    case SDL_SCANCODE_SEMICOLON: return 0x27;
	case SDL_SCANCODE_APOSTROPHE: return 0x28;    case SDL_SCANCODE_GRAVE: return 0x29;        case SDL_SCANCODE_COMMA: return 0x33;
	case SDL_SCANCODE_PERIOD: return 0x34;        case SDL_SCANCODE_SLASH: return 0x35;        case SDL_SCANCODE_CAPSLOCK: return 0x3a;
	case SDL_SCANCODE_F11: return 0x57;           case SDL_SCANCODE_F12: return 0x58;          case SDL_SCANCODE_F13: return 0x64;
	case SDL_SCANCODE_F14: return 0x65;           case SDL_SCANCODE_F15: return 0x66;          case SDL_SCANCODE_PRINTSCREEN: return 0xb7;
	case SDL_SCANCODE_SCROLLLOCK: return 0x46;    case SDL_SCANCODE_PAUSE: return 0xc5;        case SDL_SCANCODE_INSERT: return 0xd2;
	case SDL_SCANCODE_HOME: return 0xc7;          case SDL_SCANCODE_PAGEUP: return 0xc9;       case SDL_SCANCODE_DELETE: return 0xd3;
	case SDL_SCANCODE_END: return 0xcf;           case SDL_SCANCODE_PAGEDOWN: return 0xd1;     case SDL_SCANCODE_RIGHT: return 0xcd;
	case SDL_SCANCODE_LEFT: return 0xcb;          case SDL_SCANCODE_DOWN: return 0xd0;         case SDL_SCANCODE_UP: return 0xc8;
	case SDL_SCANCODE_NUMLOCKCLEAR: return 0x45;  case SDL_SCANCODE_KP_DIVIDE: return 0xb5;    case SDL_SCANCODE_KP_MULTIPLY: return 0x37;
	case SDL_SCANCODE_KP_MINUS: return 0x4a;      case SDL_SCANCODE_KP_PLUS: return 0x4e;      case SDL_SCANCODE_KP_ENTER: return 0x9c;
	case SDL_SCANCODE_KP_1: return 0x4f;          case SDL_SCANCODE_KP_2: return 0x50;         case SDL_SCANCODE_KP_3: return 0x51;
	case SDL_SCANCODE_KP_4: return 0x4b;          case SDL_SCANCODE_KP_5: return 0x4c;         case SDL_SCANCODE_KP_6: return 0x4d;
	case SDL_SCANCODE_KP_7: return 0x47;          case SDL_SCANCODE_KP_8: return 0x48;         case SDL_SCANCODE_KP_9: return 0x49;
	case SDL_SCANCODE_KP_0: return 0x52;          case SDL_SCANCODE_KP_PERIOD: return 0x53;    case SDL_SCANCODE_NONUSBACKSLASH: return 0x56;
	case SDL_SCANCODE_APPLICATION: return 0xdd;   case SDL_SCANCODE_LCTRL: return 0x1d;        case SDL_SCANCODE_LSHIFT: return 0x2a;
	case SDL_SCANCODE_LALT: return 0x38;          case SDL_SCANCODE_LGUI: return 0xdb;         case SDL_SCANCODE_RCTRL: return 0x9d;
	case SDL_SCANCODE_RSHIFT: return 0x36;        case SDL_SCANCODE_RALT: return 0xb8;         case SDL_SCANCODE_RGUI: return 0xdc;
	default: return 0;
	}
}
extern "C" void port_platform_keyboard(unsigned char *dik)
{
	int n = 0;
	const bool *keys = SDL_GetKeyboardState(&n);
	if (g_comboEnter && !(n > SDL_SCANCODE_RETURN && keys[SDL_SCANCODE_RETURN])) g_comboEnter = false;
	for (int sc = 0; sc < n; sc++)
		if (keys[sc] && !(g_comboEnter && sc == SDL_SCANCODE_RETURN)) { unsigned char d = hidToDik(sc); if (d) dik[d] = 0x80; }
}

// ---- the window procedure side: events -> GameWndProc
static WinLParam pointLParam(int x, int y) { return (WinLParam)(unsigned)((x & 0xffff) | ((unsigned)(y & 0xffff) << 16)); }
static void dispatch(unsigned msg, WinWParam wp, WinLParam lp) { if (g_wndProc) g_wndProc(g_mainWnd, msg, wp, lp); }

struct PostedPointer { unsigned msg; int x, y, down; };
static PostedPointer g_posted[64];
static int g_nPosted;
extern "C" void port_platform_post_pointer(unsigned msg, int x, int y, int leftDown)
{
	if (g_nPosted < 64) { PostedPointer p = { msg, x, y, leftDown }; g_posted[g_nPosted++] = p; }
}
extern "C" void port_platform_request_close(void)   // any thread (the watchdog): SDL_PushEvent is thread-safe
{
	SDL_Event e;
	memset(&e, 0, sizeof e);
	e.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&e);
}

extern "C" int port_real_input_allowed(void);   // mouse.cpp: false in scripted runs (PORT_INPUT / PORT_KEYS / PORT_MOUSE)
#ifdef ZOOSTERS_DEBUGUI
// The debug overlay (port/debugui) takes Win32 window messages (port_debugui_wndproc, as window_w32.cpp feeds it): SDL's events become
// the same messages with Windows virtual-key codes. 1 = the overlay used the event (the game does not get it).
#include "../debugui/debugui.h"
static int toVirtualKey(SDL_Keycode k)
{
	if (k >= 'a' && k <= 'z') return (int)(k - 'a' + 'A');
	if (k >= '0' && k <= '9') return (int)k;
	if (k >= SDLK_F1 && k <= SDLK_F12) return 0x70 + (int)(k - SDLK_F1);
	switch (k) {
	case SDLK_BACKSPACE: return 0x08; case SDLK_TAB: return 0x09; case SDLK_RETURN: return 0x0D; case SDLK_ESCAPE: return 0x1B;
	case SDLK_SPACE: return 0x20; case SDLK_PAGEUP: return 0x21; case SDLK_PAGEDOWN: return 0x22; case SDLK_END: return 0x23;
	case SDLK_HOME: return 0x24; case SDLK_LEFT: return 0x25; case SDLK_UP: return 0x26; case SDLK_RIGHT: return 0x27; case SDLK_DOWN: return 0x28;
	case SDLK_INSERT: return 0x2D; case SDLK_DELETE: return 0x2E; case SDLK_LSHIFT: case SDLK_RSHIFT: return 0x10;
	case SDLK_LCTRL: case SDLK_RCTRL: case SDLK_LGUI: case SDLK_RGUI: return 0x11;   // Cmd as Ctrl (macOS)
	case SDLK_LALT: case SDLK_RALT: return 0x12; case SDLK_COMMA: return 0xBC; case SDLK_MINUS: return 0xBD; case SDLK_PERIOD: return 0xBE;
	}
	return 0;
}
static int debugui(unsigned msg, unsigned long long wp, long long lp)
{
	long long r = 0;
	return port_debugui_wndproc(g_win, msg, wp, lp, &r);
}
static int debuguiEvent(const SDL_Event &e)
{
	switch (e.type) {
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP: {
		int vk = toVirtualKey(e.key.key);
		if (!vk) return 0;
		return debugui(e.type == SDL_EVENT_KEY_DOWN ? 0x0100 : 0x0101, (unsigned long long)vk, e.key.repeat ? (1ll << 30) : 0);
	}
	case SDL_EVENT_TEXT_INPUT: {                     // UTF-8 -> WM_CHAR per UTF-16 unit (the overlay's text fields)
		const unsigned char *s = (const unsigned char *)e.text.text;
		int used = 0;
		while (s && *s) {
			unsigned c = *s++, n = c >= 0xf0 ? 3 : c >= 0xe0 ? 2 : c >= 0xc0 ? 1 : 0;
			c &= n == 3 ? 0x07 : n == 2 ? 0x0f : n == 1 ? 0x1f : 0x7f;
			while (n-- && (*s & 0xc0) == 0x80) c = (c << 6) | (*s++ & 0x3f);
			if (c >= 0x10000) { c -= 0x10000; used |= debugui(0x0102, 0xd800 + (c >> 10), 0); c = 0xdc00 + (c & 0x3ff); }
			used |= debugui(0x0102, c, 0);
		}
		return used;
	}
	case SDL_EVENT_MOUSE_MOTION: {
		int x, y;
		toPixels(e.motion.x, e.motion.y, &x, &y);
		return debugui(0x0200, 0, (long long)((unsigned)(y & 0xffff) << 16 | (unsigned)(x & 0xffff)));
	}
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP: {
		bool down = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
		unsigned msg = e.button.button == SDL_BUTTON_LEFT ? (down ? 0x0201 : 0x0202) : e.button.button == SDL_BUTTON_RIGHT ? (down ? 0x0204 : 0x0205)
		             : e.button.button == SDL_BUTTON_MIDDLE ? (down ? 0x0207 : 0x0208) : 0;
		return msg ? debugui(msg, 0, 0) : 0;
	}
	case SDL_EVENT_MOUSE_WHEEL:
		return debugui(0x020A, (unsigned long long)((unsigned)(short)(e.wheel.y * 120.0f) & 0xffff) << 16, 0);
	case SDL_EVENT_WINDOW_FOCUS_GAINED: debugui(0x0007, 0, 0); return 0;
	case SDL_EVENT_WINDOW_FOCUS_LOST: debugui(0x0008, 0, 0); return 0;
	}
	return 0;
}
#endif
static void translate(const SDL_Event &e)
{
	static float wheelAcc, dxAcc, dyAcc;
	switch (e.type) {
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_MOUSE_MOTION:
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
	case SDL_EVENT_MOUSE_WHEEL:
		if (!port_real_input_allowed())
			return;   // a scripted run gets no real keyboard / mouse (also not SDL's synthetic motion when the window appears under the cursor)
		break;
	default:
		break;
	}
#ifdef ZOOSTERS_DEBUGUI
	if (debuguiEvent(e))
		return;                          // the overlay used it
#endif
	switch (e.type) {
	case SDL_EVENT_QUIT:
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		if (!g_quitRequested) dispatch(0x10, 0, 0);               // WM_CLOSE: g_quitRequested = 1
		break;
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		g_sizeEvent = 1;
		break;
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		g_focusSeen = 1;
		port_audio_focus(1);
		dispatch(7, 0, 0);                                        // WM_SETFOCUS: App_SetActive, frames run
		break;
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		port_mouse_raw_reset();
		port_audio_focus(0);                                      // as DirectSound did for the game: silent while unfocused
		dispatch(8, 0, 0);                                        // WM_KILLFOCUS: the game pauses (and asks to minimise: zoosters::os::ShowWindow)
		break;
	case SDL_EVENT_KEY_DOWN:
#ifdef ZOOSTERS_OPTIONS
		if (e.key.key == SDLK_RETURN && (e.key.mod & SDL_KMOD_ALT)) {
			if (!e.key.repeat) { g_comboEnter = true; port_window_toggle_fullscreen(); }
			break;
		}
#endif
		dispatch(0x100, (WinWParam)e.key.key, 0);                 // WM_KEYDOWN (the game only reads "a key was pressed")
		break;
	case SDL_EVENT_MOUSE_MOTION: {
		if (e.motion.windowID != SDL_GetWindowID(g_win)) break;
		dxAcc += e.motion.xrel; dyAcc += e.motion.yrel;
		int dx = (int)dxAcc, dy = (int)dyAcc;
		dxAcc -= dx; dyAcc -= dy;
		if (dx || dy) port_mouse_raw_move(dx, dy);
		int x, y;
		toPixels(e.motion.x, e.motion.y, &x, &y);
		dispatch(0x200, (e.motion.state & SDL_BUTTON_LMASK) ? 1 : 0, pointLParam(x, y));
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP: {
		if (e.button.windowID != SDL_GetWindowID(g_win)) break;
		static const int dinputButton[6] = { -1, 0, 2, 1, 3, 4 };     // SDL left, middle, right, X1, X2 -> DirectInput left, right, middle, 4, 5
		int b = e.button.button < 6 ? dinputButton[e.button.button] : e.button.button - 1;
		port_mouse_raw_button(b, e.button.down);
		if (e.button.button == SDL_BUTTON_LEFT) {
			int x, y;
			toPixels(e.button.x, e.button.y, &x, &y);
			dispatch(e.button.down ? 0x201 : 0x202, e.button.down ? 1 : 0, pointLParam(x, y));
		}
		break;
	}
	case SDL_EVENT_MOUSE_WHEEL: {
		wheelAcc += e.wheel.y * (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -120.0f : 120.0f);
		int d = (int)wheelAcc;
		wheelAcc -= d;
		if (d) port_mouse_raw_wheel(d);
		break;
	}
	default:
		break;
	}
}

// Pointer lock. The original ran full screen with an exclusive DirectInput mouse, so the pointer could not leave the game; a
// window needs the OS lock for that: SDL's relative mouse mode. The pointer stays put and only deltas arrive (re-centring it each
// frame did not hold). Locked while the game is played: the window has the focus and the game's own pointer sprite is off
// (g_cursorShown: menus show it and need absolute positions); scripted runs never lock.
static void updatePointerLock()
{
	static bool locked;
	bool want = g_win && port_real_input_allowed() && SDL_GetKeyboardFocus() == g_win && !g_cursorShown;
#ifdef ZOOSTERS_DEBUGUI
	if (port_debugui_blocks_mouse())
		want = false;                    // the debug overlay is open: it needs the pointer
#endif
	if (want == locked)
		return;
	if (!SDL_SetWindowRelativeMouseMode(g_win, want)) {
		SLOG("window_sdl: relative mouse mode %d failed: %s\n", (int)want, SDL_GetError());
		return;
	}
	locked = want;
	SLOG("window_sdl: pointer %s\n", want ? "locked (playing)" : "free (menu / unfocused)");
}

// Replaces the game's PumpMessages (0x51a100: screen saver timer reset every 200 pumps, then PeekMessage / Translate / Dispatch).
void PumpMessages()
{
	if (!g_focusSeen && g_win) {   // Windows sends WM_SETFOCUS to the newly shown window (window_w32.cpp runs from it); SDL may not report
		g_focusSeen = 1;           // a focus change for the first activation, so the game gets it once here (else g_frameReady stays 0)
		dispatch(7, 0, 0);
	}
	for (int i = 0; i < g_nPosted; i++) dispatch(g_posted[i].msg, g_posted[i].down ? 1 : 0, pointLParam(g_posted[i].x, g_posted[i].y));
	g_nPosted = 0;
	SDL_Event e;
	while (SDL_PollEvent(&e)) translate(e);
#ifdef ZOOSTERS_OPTIONS
	if (g_sizeEvent) { g_sizeEvent = 0; port_display_size_changed(); }
#endif
	updatePointerLock();
}

int CreateGameWindow(const char *cls, const char *title, int x, int y, int w, int h, void *wndProc, void *hInst, int icon, bool fullscreen)
{
	(void)cls; (void)x; (void)y; (void)hInst; (void)icon;
#ifdef ZOOSTERS_OPTIONS
	int wmode = port_window_mode();
	fullscreen = wmode != 0;
#else
	const char *fs = getenv("PORT_FULLSCREEN");
	if (!(fs && fs[0] == '1')) fullscreen = 0;
#endif
	if (!fullscreen) g_wndWindowed = 1;   // as window_w32.cpp: GameWndProc's WM_SETFOCUS would maximise a "full screen" window
	g_wndProc = (GameWndProcFn)wndProc;
	if (!videoUp()) return 0;
	SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	if (windowHidden())
		flags |= SDL_WINDOW_HIDDEN;
#ifdef RW_GL3
	flags |= SDL_WINDOW_OPENGL;
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);     // the D3D8 device RW opens: D24S8 (SDL's default is 16-bit depth, no stencil)
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif
	float density = pixelDensity();       // the game's w x h are pixels: the window is that size in points / density (Retina: half)
	g_win = SDL_CreateWindow(title ? title : "Madagascar", (int)(w / density + 0.5f), (int)(h / density + 0.5f), flags);
	if (!g_win) { SLOG("window_sdl: SDL_CreateWindow failed: %s\n", SDL_GetError()); return 0; }
#ifdef ZOOSTERS_DEBUGUI
	SDL_StartTextInput(g_win);           // typed characters for the debug overlay's text fields
#endif
#ifdef _WIN32
	g_mainWnd = SDL_GetPointerProperty(SDL_GetWindowProperties(g_win), SDL_PROP_WINDOW_WIN32_HWND_POINTER, 0);   // the game's Win32 calls use it
#else
	g_mainWnd = g_win;
#endif
	SDL_DisableScreenSaver();
#ifdef ZOOSTERS_OPTIONS
	if (wmode == 2 && !port_platform_apply_display(2, w, h)) {
		SLOG("window_sdl: exclusive full screen %dx%d unavailable, borderless instead\n", w, h);
		port_platform_apply_display(1, 0, 0);
	}
#endif
	int pw = w, ph = h;
	SDL_GetWindowSizeInPixels(g_win, &pw, &ph);
#ifdef ZOOSTERS_OPTIONS
	if (fullscreen && pw > 0 && ph > 0) { g_resWidth = (unsigned)pw; g_resHeight = (unsigned)ph; }   // WinMain sizes the camera from them
#endif
	if (pw > 0 && ph > 0) port_set_video_mode(pw, ph);
	SLOG("window_sdl: CreateGameWindow(title=%s %dx%d fullscreen=%d) -> SDL window %p (%dx%d pixels), native %p\n", title ? title : "", w, h,
	     (int)fullscreen, (void *)g_win, pw, ph, g_mainWnd);
	return g_mainWnd != 0;
}

// ---- harness side (same entry points as window_w32.cpp): frame limit after `frames` presents, 120 s watchdog
extern "C" void (*port_present_hook)(void);
static void (*g_prevPresentHook)(void);
static int g_maxFrames = -1, g_closePosted;
static Uint64 g_startTicks;
static void limitHook(void)
{
	if (g_prevPresentHook) g_prevPresentHook();
	if (!g_closePosted && port_frames_presented() >= g_maxFrames) {
		g_closePosted = 1;
		SLOG("window_sdl: frame limit after %d presented frames, requesting close\n", port_frames_presented());
		port_platform_request_close();
	}
}
static int SDLCALL watchdog(void *)
{
	for (;;) {
		SDL_Delay(50);
		if (SDL_GetTicks() - g_startTicks > 120000) {
			SLOG("window_sdl: watchdog timeout after %d presented frames, requesting close\n", port_frames_presented());
			port_platform_request_close();
			return 0;
		}
	}
}
extern "C" void port_win_set_frames(int n)
{
	g_maxFrames = n;
	g_startTicks = SDL_GetTicks();
	if (n > 0) {
		g_prevPresentHook = port_present_hook;
		port_present_hook = &limitHook;
		SDL_DetachThread(SDL_CreateThread(watchdog, "port watchdog", 0));
	}
}
extern "C" int port_win_pumps(void) { return port_frames_presented(); }
