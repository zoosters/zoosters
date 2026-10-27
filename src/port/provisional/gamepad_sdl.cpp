// Game controllers through SDL3's gamepad API, for dinput_port.cpp. The game sees one pad (slot 0): the first connected SDL gamepad at each poll, so a pad plugged in later works too.
// With the SDL window its event loop (PumpMessages) updates the pads; with the win32 window nothing reads SDL's events, so they are
// off and every poll updates the pads itself (SDL_UpdateGamepads, also what notices a pad plugged in or out).
// gamecontrollerdb.txt next to the exe (macOS bundle: Contents/Resources) adds the community mappings.
// The state is laid out as the Logitech RumblePad 2 / Dual Action that the launcher's preset 0 was written for (the bindings:
// src/game/framework/startup/win32/input/controls.cpp kPortPadPreset):
//   lX lY  left stick (and the d-pad, digital)    lZ lRz  right stick (x, y)    lRx lRy  left / right trigger (0..1000)
//   rgdwPOV[0]  d-pad (hundredths of a degree, 0xffffffff centred)
//   buttons  0 west (X / Square)  1 south (A / Cross)  2 east (B / Circle)  3 north (Y / Triangle)  4 / 5 shoulders  6 / 7 triggers
//            8 Start  9 Back / Select  10 / 11 stick clicks
// Axes are -1000..1000 (what the game asks for with DIPROP_RANGE); the game has no dead zone, so the sticks get one here.
// Camera (right stick): the game's horizontal direction is inverted against modern games, its vertical one is not. Default: x the
// modern way, y as the game. [Settings\Port] PadInvertCameraX / PadInvertCameraY = 1 inverts that axis against the default.
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "framework/startup/win32/input/dinput.h"
#include "zoosters/settings.h"

extern "C" FILE *port_log;
extern "C" int port_platform_focused(void);

static int g_up;                      // 1 = SDL's gamepad subsystem is up, -1 = it failed
static SDL_Gamepad *g_pad;
static Uint64 g_nextLook;             // no pad: when to look for one again
static float g_low, g_high;           // the rumble the game asked for (0..1; low = the heavy motor)
static int g_rumbling;                // the motors run (re-armed before SDL's duration ends)
static Uint64 g_rumbleUntil;
static const float kDeadZone = 0.2f;  // of the stick's travel, radial (the game has none: a resting stick drifted the camera)

static bool up(void)
{
	if (g_up)
		return g_up > 0;
	g_up = SDL_InitSubSystem(SDL_INIT_GAMEPAD) ? 1 : -1;
	if (g_up < 0) {
		if (port_log) fprintf(port_log, "gamepad: SDL gamepad subsystem failed: %s\n", SDL_GetError());
		return false;
	}
#ifndef PORT_WINDOW_SDL
	SDL_SetJoystickEventsEnabled(false);
	SDL_SetGamepadEventsEnabled(false);
	SDL_UpdateGamepads();
#endif
	const char *base = SDL_GetBasePath();
	char path[1024];
	snprintf(path, sizeof path, "%sgamecontrollerdb.txt", base ? base : "");
	int n = SDL_AddGamepadMappingsFromFile(path);
	if (port_log) fprintf(port_log, "gamepad: SDL gamepads up; %s: %d mappings%s\n", path, n < 0 ? 0 : n, n < 0 ? " (not found: SDL's own only)" : "");
	return true;
}

static void applyRumble(SDL_Gamepad *p)
{
	bool on = (g_low > 0.0f || g_high > 0.0f) && port_platform_focused();   // silent while in the background, as the sound
	Uint16 lo = on ? (Uint16)(g_low * 65535.0f) : 0, hi = on ? (Uint16)(g_high * 65535.0f) : 0;
	bool ok = SDL_RumbleGamepad(p, lo, hi, on ? 30000 : 0);   // the game's effect has no end: re-armed in port_gamepad_state
	static int logged;
	if (port_log && logged < 24 && (on || g_rumbling)) {
		logged++;
		fprintf(port_log, "gamepad: rumble %.2f / %.2f%s -> %s%s\n", g_low, g_high, on ? "" : " (off: unfocused or stopped)", ok ? "ok" : "FAILED: ",
		        ok ? "" : SDL_GetError());
	}
	g_rumbling = on;
	g_rumbleUntil = SDL_GetTicks() + 25000;
}

static SDL_Gamepad *pad(void)
{
	if (!up())
		return 0;
	if (g_pad && SDL_GamepadConnected(g_pad))
		return g_pad;
	if (g_pad) {
		if (port_log) fprintf(port_log, "gamepad: '%s' disconnected\n", SDL_GetGamepadName(g_pad));
		SDL_CloseGamepad(g_pad);
		g_pad = 0;
	}
	Uint64 now = SDL_GetTicks();
	if (now < g_nextLook)
		return 0;
	g_nextLook = now + 500;
	int n = 0;
	SDL_JoystickID *ids = SDL_GetGamepads(&n);
	if (ids && n > 0)
		g_pad = SDL_OpenGamepad(ids[0]);
	SDL_free(ids);
	if (g_pad) {
		SDL_PropertiesID props = SDL_GetGamepadProperties(g_pad);
		bool rumble = SDL_GetBooleanProperty(props, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
		const char *path = SDL_GetGamepadPath(g_pad);   // HIDAPI devices have a HID path; the macOS GameController (MFi) driver has none
		if (port_log) fprintf(port_log, "gamepad: '%s' connected (%d pad%s, rumble %s; SDL %d.%d.%d, type %d, vendor %04x product %04x, %s)\n",
		                      SDL_GetGamepadName(g_pad), n, n == 1 ? "" : "s", rumble ? "yes" : "no", SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
		                      SDL_MICRO_VERSION, (int)SDL_GetGamepadType(g_pad), SDL_GetGamepadVendor(g_pad), SDL_GetGamepadProduct(g_pad),
		                      path ? path : "no HID path (not HIDAPI)");
		applyRumble(g_pad);
		const char *t = SDL_getenv("PORT_PAD_RUMBLE_TEST");   // test of the SDL end alone: both motors at half for a second at connect
		if (t && *t == '1') {
			bool ok = SDL_RumbleGamepad(g_pad, 0x8000, 0x8000, 1000);
			if (port_log) fprintf(port_log, "gamepad: PORT_PAD_RUMBLE_TEST -> %s%s\n", ok ? "ok" : "FAILED: ", ok ? "" : SDL_GetError());
		}
	}
	return g_pad;
}

static float axisUnit(SDL_Gamepad *p, SDL_GamepadAxis a)
{
	return SDL_GetGamepadAxis(p, a) / 32767.0f;
}

// a stick with the radial dead zone, rescaled so the rest of the travel still reaches +-1000
static void stick(SDL_Gamepad *p, SDL_GamepadAxis ax, SDL_GamepadAxis ay, int *x, int *y)
{
	float fx = axisUnit(p, ax), fy = axisUnit(p, ay), len = sqrtf(fx * fx + fy * fy);
	if (len <= kDeadZone) {
		*x = *y = 0;
		return;
	}
	float scale = (len > 1.0f ? 1.0f : (len - kDeadZone) / (1.0f - kDeadZone)) / len;
	*x = (int)(fx * scale * 1000.0f);
	*y = (int)(fy * scale * 1000.0f);
	*x = *x > 1000 ? 1000 : *x < -1000 ? -1000 : *x;
	*y = *y > 1000 ? 1000 : *y < -1000 ? -1000 : *y;
}

// The camera inversion against the default above: [Settings\Port] PadInvertCameraX / Y, read at the first use; the options screen
// changes them live (port_gamepad_invert_camera(axis, value): axis 0 = x, 1 = y; value -1 only reads the settings).
extern "C" int port_pad_invert_x = -1, port_pad_invert_y = -1;
extern "C" void port_gamepad_invert_camera(int axis, int value)
{
	if (port_pad_invert_x < 0) {
		int x = 0, y = 0;
		zoosters::settings::GetInt("Settings\\Port", "PadInvertCameraX", &x);
		zoosters::settings::GetInt("Settings\\Port", "PadInvertCameraY", &y);
		port_pad_invert_x = x != 0;
		port_pad_invert_y = y != 0;
	}
	if (value >= 0) (axis ? port_pad_invert_y : port_pad_invert_x) = value != 0;
}

// The pad's state as DIJOYSTATE2 (layout above); all released while the window is in the background (DISCL_FOREGROUND). Returns 1
// when a pad is connected.
extern "C" int port_gamepad_state(DIJOYSTATE2 *s)
{
	memset(s, 0, sizeof *s);
	for (int i = 0; i < 4; i++)
		s->rgdwPOV[i] = 0xffffffffu;
#ifndef PORT_WINDOW_SDL
	if (up()) SDL_UpdateGamepads();
#endif
	SDL_Gamepad *p = pad();
	if (!p)
		return 0;
	if (g_rumbling != ((g_low > 0.0f || g_high > 0.0f) && port_platform_focused()) || (g_rumbling && SDL_GetTicks() >= g_rumbleUntil))
		applyRumble(p);
	if (!port_platform_focused())
		return 1;
	stick(p, SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY, &s->lX, &s->lY);
	stick(p, SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY, &s->lZ, &s->lRz);
	port_gamepad_invert_camera(0, -1);   // (reads the settings once)
	if (!port_pad_invert_x) s->lZ = -s->lZ;    // modern: stick right turns the view right (the game's own is the other way)
	if (port_pad_invert_y) s->lRz = -s->lRz;   // the game's vertical is the modern default
	float lt = axisUnit(p, SDL_GAMEPAD_AXIS_LEFT_TRIGGER), rt = axisUnit(p, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
	s->lRx = (int)(lt * 1000.0f);
	s->lRy = (int)(rt * 1000.0f);
	bool up = SDL_GetGamepadButton(p, SDL_GAMEPAD_BUTTON_DPAD_UP), down = SDL_GetGamepadButton(p, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
	bool left = SDL_GetGamepadButton(p, SDL_GAMEPAD_BUTTON_DPAD_LEFT), right = SDL_GetGamepadButton(p, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
	int dx = right - left, dy = down - up;
	if (dx || dy) {   // the hat, and the d-pad as the left stick too: the game binds no hat direction (menus and walking by d-pad)
		static const unsigned kPov[3][3] = { { 31500, 0, 4500 }, { 27000, 0xffffffffu, 9000 }, { 22500, 18000, 13500 } };   // [dy + 1][dx + 1]
		s->rgdwPOV[0] = kPov[dy + 1][dx + 1];
		if (dx) s->lX = dx * 1000;
		if (dy) s->lY = dy * 1000;
	}
	static const SDL_GamepadButton kButtons[12] = {
		SDL_GAMEPAD_BUTTON_WEST, SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_NORTH,
		SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, SDL_GAMEPAD_BUTTON_INVALID, SDL_GAMEPAD_BUTTON_INVALID,
		SDL_GAMEPAD_BUTTON_START, SDL_GAMEPAD_BUTTON_BACK, SDL_GAMEPAD_BUTTON_LEFT_STICK, SDL_GAMEPAD_BUTTON_RIGHT_STICK };
	for (int i = 0; i < 12; i++)
		if (kButtons[i] != SDL_GAMEPAD_BUTTON_INVALID && SDL_GetGamepadButton(p, kButtons[i]))
			s->rgbButtons[i] = 0x80;
	if (lt > 0.5f) s->rgbButtons[6] = 0x80;
	if (rt > 0.5f) s->rgbButtons[7] = 0x80;
	return 1;
}

// The game's force feedback (dinput_port.cpp PadEffect): the two motors as 0..1 (low = heavy, high = light), 0 / 0 = stop
extern "C" void port_gamepad_rumble(float low, float high)
{
	g_low = low < 0.0f ? 0.0f : low > 1.0f ? 1.0f : low;
	g_high = high < 0.0f ? 0.0f : high > 1.0f ? 1.0f : high;
	if (SDL_Gamepad *p = pad())
		applyRumble(p);
}

// A short buzz of both motors (the options screen confirms Vibration On with it); the game's own rumble returns with its next change.
extern "C" void port_gamepad_pulse(float strength, int ms)
{
	SDL_Gamepad *p = pad();
	if (!p || !port_platform_focused())
		return;
	Uint16 v = (Uint16)((strength < 0.0f ? 0.0f : strength > 1.0f ? 1.0f : strength) * 65535.0f);
	SDL_RumbleGamepad(p, v, v, (Uint32)ms);
}

// Brings the subsystem up early (DirectInput's device enumeration at start-up), so a pad that is already connected is seen at once.
// Returns 1 when gamepads can be read at all (the port's DirectInput then offers its pad).
extern "C" int port_gamepad_init(void)
{
	pad();
	return g_up > 0;
}
