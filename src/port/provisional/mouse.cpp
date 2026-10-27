// The port's mouse (the keyboard side is port_keyboard_frame in game_stubs.cpp).
// The game reads the mouse two ways. Pointer sprite: Cursor_Show 0x462eb0 creates the Hud2D sprite "bx_MousePointer" at GetCursorPos()
// scaled client -> 640x480 when a controller buffer asks for a visible cursor; GameWndProc then moves it on WM_MOUSEMOVE (also
// hit-testing menu items) and handles WM_LBUTTONDOWN/UP. DirectInput mouse: InputManager::Poll reads a DIMOUSESTATE2 (relative lX/lY
// in mickeys, lZ 120 per notch, buttons) once a frame and GetMouseControl scales it by m_mouseSens.
// The normal build runs both from src/game over dinput_port.cpp, which calls port_mouse_frame below; the address-bound port still has
// a zero GetMouseControl stand-in (game_stubs.cpp). The original is full screen (client == screen coordinates); the port's window is
// bordered, so port_run.cpp patches the game's GetCursorPos to port_mouse_get.
// Platform seam: backends feed the relative mouse through port_mouse_raw_move / _wheel / _button / _reset (window_w32.cpp: WM_INPUT
// raw input, unaccelerated like DirectInput 8; window_sdl.cpp: SDL mouse events).
// Sources: the real mouse while the window has the focus and no input script is given (port_real_input; PORT_REALINPUT=1 forces the
// real keyboard and mouse on next to a script, =0 off), or PORT_MOUSE="<entry>[;<entry>...]", frames = presented frames:
//   <frame>:<x>,<y>[:c]        pointer at client pixel x,y from that frame on (":c" = a left click: down at that frame, up 3 frames
//                              later); also posts WM_MOUSEMOVE / WM_LBUTTONDOWN / WM_LBUTTONUP to the window (the real window gets
//                              those from Windows) and moves the relative mouse by the pointer's change, as a real mouse would
//   <frame>:d<dx>,<dy>[:<n>]   relative movement of dx,dy mickeys in each of the n frames (default 1) from that frame on
//   <frame>:w<notches>         wheel turned by that many notches (lZ = 120 per notch) in that frame
//   <frame>:b<button>[:<n>]    DirectInput button (0 left, 1 right, 2 middle, 3/4 side) held for n frames (default 3)
// and the `mouse` / `wheel` / `mbutton` lines of a PORT_INPUT script (game_stubs.cpp; trace frames in gate runs).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../debugui/debugui.h"
#include "platform.h"

// Window message numbers the platform backend posts for the scripted pointer (the game's own protocol with GameWndProc; the Win32 values)
enum { PORT_WM_MOUSEMOVE = 0x0200, PORT_WM_LBUTTONDOWN = 0x0201, PORT_WM_LBUTTONUP = 0x0202 };
struct PortPoint { int x, y; };   // POINT (LONG = 32 bits)
#if defined(_WIN32) && !defined(ZOOSTERS_OS_SDL)   // the Windows lanes patch the user32 import with it (port_run.cpp); the portable build calls it directly
#define PORT_MOUSE_CALL __stdcall
#else
#define PORT_MOUSE_CALL
#endif

extern "C" FILE *port_log;
extern "C" int port_frames_presented(void);
extern void *g_mainWnd;   // GLOBAL: 0x62f138 g_mainWnd

enum { INJ_POS, INJ_MOVE, INJ_WHEEL, INJ_BUTTON };
struct Inj { int frame, kind, x, y, n, click, done; };
static Inj g_inj[256];
static int g_ninj = -1;           // -1 = not parsed yet
static int g_nposInj;             // absolute entries (they own the pointer position)
static PortPoint g_pos = { 400, 300 };
static PortPoint g_last = { -1, -1 };
static int g_lastBtn;
static int g_lastFrame = -1;      // frame of the previous port_mouse_frame (scripted entries apply once per frame)

static void parse(void)
{
	g_ninj = 0;
	const char *e = getenv("PORT_MOUSE");
	while (e && *e && g_ninj < 256) {
		Inj in = { 0, INJ_POS, 0, 0, 1, 0, -1 };
		int n = 0;
		if (sscanf(e, "%d:%n", &in.frame, &n) < 1 || !n) break;
		e += n;
		n = 0;
		if (*e == 'd') {
			in.kind = INJ_MOVE;
			if (sscanf(e + 1, "%d,%d%n", &in.x, &in.y, &n) < 2) break;
			e += 1 + n;
			if (e[0] == ':') { n = 0; if (sscanf(e + 1, "%d%n", &in.n, &n) < 1) break; e += 1 + n; }
		} else if (*e == 'w') {
			in.kind = INJ_WHEEL;
			if (sscanf(e + 1, "%d%n", &in.x, &n) < 1) break;
			e += 1 + n;
		} else if (*e == 'b') {
			in.kind = INJ_BUTTON;
			in.n = 3;
			if (sscanf(e + 1, "%d%n", &in.x, &n) < 1 || in.x < 0 || in.x > 7) break;
			e += 1 + n;
			if (e[0] == ':') { n = 0; if (sscanf(e + 1, "%d%n", &in.n, &n) < 1) break; e += 1 + n; }
		} else {
			if (sscanf(e, "%d,%d%n", &in.x, &in.y, &n) < 2) break;
			e += n;
			if (e[0] == ':' && e[1] == 'c') { in.click = 1; e += 2; }
			g_nposInj++;
		}
		if (in.n < 1) in.n = 1;
		g_inj[g_ninj++] = in;
		while (*e == ';' || *e == ' ') e++;
	}
	if (g_ninj && port_log) fprintf(port_log, "mouse: PORT_MOUSE %d entries%s\n", g_ninj, e && *e ? " (stopped at a malformed entry)" : "");
}

static int windowSize(int *w, int *h) { return port_platform_client_size(w, h); }
static int focused(void) { return port_platform_focused(); }

// Real keyboard and mouse: only while the window is in front (DISCL_FOREGROUND), and not in scripted runs (PORT_INPUT, PORT_KEYS or
// PORT_MOUSE given), so that a test run does not pick up whoever touches the PC; PORT_REALINPUT=1 / 0 overrides.
// port_real_input_allowed: the scripted-run half alone (the window backends drop the real keyboard / mouse messages they would pass to the
// game's window procedure in a scripted run, whatever the focus: SDL reports a synthetic motion when its window appears under the cursor).
extern "C" int port_real_input_allowed(void)
{
	static int allowed = -1;
	if (allowed < 0) {
		const char *r = getenv("PORT_REALINPUT");
		if (r && r[0]) allowed = r[0] == '1';
		else allowed = !(getenv("PORT_INPUT") || getenv("PORT_KEYS") || getenv("PORT_MOUSE"));
		if (port_log) fprintf(port_log, "mouse: real keyboard / mouse %s\n", allowed ? "on (while focused)" : "off (input script given)");
	}
	return allowed;
}
extern "C" int port_real_input(void)
{
	return port_real_input_allowed() && focused();
}

// ---- the relative mouse, fed by the platform backend (any thread that pumps the window: the game's main thread)
static long g_rawDx, g_rawDy, g_rawWheel;
static unsigned char g_rawButtons[8];

extern "C" void port_mouse_raw_move(int dx, int dy) { g_rawDx += dx; g_rawDy += dy; }
extern "C" void port_mouse_raw_wheel(int delta) { g_rawWheel += delta; }   // 120 per notch, positive = away from the user
extern "C" void port_mouse_raw_button(int button, int down) { if (button >= 0 && button < 8) g_rawButtons[button] = down ? 0x80 : 0; }
extern "C" void port_mouse_raw_reset(void)      // focus lost: nothing held, nothing pending
{
	g_rawDx = g_rawDy = g_rawWheel = 0;
	memset(g_rawButtons, 0, sizeof g_rawButtons);
}

// current pointer position in client pixels; *btn = left button state (scripted pointer or the real one)
static PortPoint current(int *btn)
{
	if (g_ninj < 0) parse();
	int f = port_frames_presented();
	*btn = 0;
	if (g_nposInj > 0) {
		int k = -1;
		for (int i = 0; i < g_ninj; i++)
			if (g_inj[i].kind == INJ_POS && f >= g_inj[i].frame && (k < 0 || g_inj[i].frame >= g_inj[k].frame)) k = i;
		if (k >= 0) {
			g_pos.x = g_inj[k].x; g_pos.y = g_inj[k].y;
			if (g_inj[k].click && f >= g_inj[k].frame && f < g_inj[k].frame + 3) *btn = 1;
		}
		return g_pos;
	}
	if (port_real_input()) {
		int x, y;
		if (port_platform_pointer(&x, &y, btn)) { g_pos.x = x; g_pos.y = y; }
	}
	return g_pos;
}

extern "C" int PORT_MOUSE_CALL port_mouse_get(PortPoint *pt)       // replaces USER32!GetCursorPos for the game
{
	int b;
	PortPoint p = current(&b);
	if (pt) *pt = p;
	{ static int n; if (n++ < 6 && port_log) fprintf(port_log, "mouse: GetCursorPos by the game at frame %d -> %d,%d\n", port_frames_presented(), p.x, p.y); }
	return 1;
}

// The mouse of one InputManager poll as a DIMOUSESTATE2 (lX, lY, lZ, rgbButtons[8]): relative movement since the previous poll.
// Also posts the scripted pointer's window messages.
struct PortMouseState { int lX, lY, lZ; unsigned char rgbButtons[8]; };
extern "C" void port_input_mouse(int *state);
extern "C" void port_mouse_frame(PortMouseState *st)
{
	memset(st, 0, sizeof *st);
	int b;
	PortPoint p = current(&b);
	int f = port_frames_presented();
	int newFrame = f != g_lastFrame;
	g_lastFrame = f;
	if (g_nposInj > 0) {
		if (p.x != g_last.x || p.y != g_last.y) port_platform_post_pointer(PORT_WM_MOUSEMOVE, p.x, p.y, b);
		if (b && !g_lastBtn) port_platform_post_pointer(PORT_WM_LBUTTONDOWN, p.x, p.y, 1);
		if (!b && g_lastBtn) port_platform_post_pointer(PORT_WM_LBUTTONUP, p.x, p.y, 0);
		if (g_last.x >= 0) { st->lX = p.x - g_last.x; st->lY = p.y - g_last.y; }
		st->rgbButtons[0] = b ? 0x80 : 0;
	}
	if (p.x != g_last.x || p.y != g_last.y || b != g_lastBtn) {
		static int n; if (n++ < 12 && port_log) fprintf(port_log, "mouse: frame %d client %d,%d button %d\n", f, p.x, p.y, b);
	}
	g_last = p; g_lastBtn = b;
	for (int i = 0; i < g_ninj; i++) {
		Inj &in = g_inj[i];
		if (in.kind == INJ_POS || f < in.frame || f >= in.frame + in.n) continue;
		if (in.kind == INJ_BUTTON) { st->rgbButtons[in.x] = 0x80; continue; }
		if (!newFrame || in.done == f) continue;      // movement and wheel: once per frame
		in.done = f;
		if (in.kind == INJ_MOVE) { st->lX += in.x; st->lY += in.y; }
		else st->lZ += in.x * 120;
		{ static int n; if (n++ < 12 && port_log) fprintf(port_log, "mouse: frame %d scripted %s %d %d\n", f, in.kind == INJ_MOVE ? "move" : "wheel", in.x, in.y); }
	}
	port_input_mouse(&st->lX);                    // PORT_INPUT `mouse` / `wheel` / `mbutton` lines (game_stubs.cpp)
#ifdef ZOOSTERS_DEBUGUI
	if (port_debugui_blocks_mouse()) g_rawDx = g_rawDy = g_rawWheel = 0;   // the overlay is open: the pointer is its cursor
#endif
	if (port_real_input()) {
		st->lX += g_rawDx; st->lY += g_rawDy; st->lZ += g_rawWheel;
		for (int k = 0; k < 8; k++) st->rgbButtons[k] |= g_rawButtons[k];
	}
	g_rawDx = g_rawDy = g_rawWheel = 0;
}

// address-bound port (game_stubs.cpp stand-in Poll): the DIMOUSESTATE block of the InputManager
extern "C" void port_mouse_update(void *input) { port_mouse_frame((PortMouseState *)input); }
