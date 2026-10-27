// ZOOSTERS_OPTIONS display settings (display.h). Stores: Settings\Display\Resolution ("<w>x<h>", empty = desktop; WinMain reads it at
// start), Settings\Port\WindowMode (0 windowed, 1 borderless, 2 exclusive full screen), Settings\Port\HudScale, Settings\Port\HudWidth.
// PORT_FULLSCREEN=1 forces borderless full screen at start; later changes in the menu win.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "bridge/options_bridge.h"
#include "strings.h"
#include "zoosters/settings.h"

extern "C" int port_platform_display_modes(int *wh, int max);
extern "C" int port_platform_apply_display(int mode, int w, int h);
extern "C" int port_platform_desktop_size(int *w, int *h);
extern "C" int port_platform_work_area(int *w, int *h, int *frameW, int *frameH);
extern "C" int port_platform_client_size(int *w, int *h);
extern "C" void port_display_fit(int *w, int *h);          // port/fixes/display.cpp
extern "C" void port_wide_settings_changed(void);          // port/fixes/widescreen.cpp
extern "C" void port_display_size_changed(void);

namespace {
const char *kResKey = "Settings\\Display\\Resolution";
const int kMaxModes = 64, kApplyDelay = 20;   // frames of quiet before a change is applied

int g_mode = -1;                // window mode in force (port_window_mode)
bool g_init;
int g_lastFull = 1;             // the full screen mode Alt+Enter returns to
struct Cfg { int mode, res; } g_want, g_active;
bool g_pending;
int g_countdown;
int g_codes[kMaxModes + 1];
char g_text[kMaxModes + 1][16];
const char *g_labels[kMaxModes + 1];
int g_count;

int code(int w, int h) { return (w << 16) | h; }
int codeW(int c) { return (c >> 16) & 0xffff; }
int codeH(int c) { return c & 0xffff; }

int loadMode()
{
	const char *fs = getenv("PORT_FULLSCREEN");
	if (fs && fs[0] == '1') return opt_display::MODE_BORDERLESS;
	int v = 0;
	if (zoosters::settings::GetInt("Settings\\Port", "WindowMode", &v) != zoosters::settings::Found::Yes || v < 0 || v >= opt_display::MODE_COUNT) v = 0;
#ifdef __APPLE__
	if (v == opt_display::MODE_FULLSCREEN) v = opt_display::MODE_BORDERLESS;   // exclusive full screen leaves the Space machinery: borderless only
#endif
	return v;
}

int loadResolution()
{
	char s[16] = "";
	int w = 0, h = 0;
	if (opt_bridge::ConfigGetString(kResKey, s, sizeof s) && sscanf(s, "%dx%d", &w, &h) == 2 && w > 0 && h > 0 && w < 65536 && h < 65536)
		return code(w, h);
	return 0;
}

void init()
{
	if (g_init) return;
	g_init = true;
	g_want.mode = g_active.mode = g_mode >= 0 ? g_mode : loadMode();
	g_want.res = g_active.res = loadResolution();
	if (g_active.mode != opt_display::MODE_WINDOWED) g_lastFull = g_active.mode;
}

// The selector's list. Windowed: only sizes whose window fits the work area. Borderless: the desktop only. keepCurrent adds the
// current size when the display does not list it (a stored value, a window size from the fit).
void rebuild(bool keepCurrent)
{
	int wh[2 * kMaxModes], n = 0, aw = 0, ah = 0;
	g_count = 0;
	g_codes[g_count++] = 0;
	if (g_want.mode != opt_display::MODE_BORDERLESS) {
		n = port_platform_display_modes(wh, kMaxModes);
		if (g_want.mode == opt_display::MODE_WINDOWED) {
			int ww, wht, fx, fy;
			if (port_platform_work_area(&ww, &wht, &fx, &fy)) { aw = ww - fx; ah = wht - fy; }
		}
		for (int i = 0; i < n; i++) {
			int w = wh[2 * i], h = wh[2 * i + 1];
			if (w < 640 || h < 480 || w > 65535 || h > 65535) continue;
			if (aw > 0 && (w > aw || h > ah)) continue;
			g_codes[g_count++] = code(w, h);
		}
		for (int i = 1; i < g_count; i++)   // ascending by width, then height
			for (int j = i; j > 1 && g_codes[j - 1] > g_codes[j]; j--) { int t = g_codes[j]; g_codes[j] = g_codes[j - 1]; g_codes[j - 1] = t; }
		bool listed = false;
		for (int i = 1; i < g_count; i++) listed |= g_codes[i] == g_want.res;
		if (g_want.res && !listed) {
			if (keepCurrent && g_count <= kMaxModes) g_codes[g_count++] = g_want.res;
			else g_want.res = 0;
		}
	}
	for (int i = 0; i < g_count; i++) {
		if (i == 0) snprintf(g_text[0], sizeof g_text[0], "%s", opt_strings::Str("OPT_DESKTOP", "Desktop"));
		else snprintf(g_text[i], sizeof g_text[i], "%dx%d", codeW(g_codes[i]), codeH(g_codes[i]));
		g_labels[i] = g_text[i];
	}
}

void desktopFit(int mode, int *w, int *h)
{
	*w = *h = 0;
	if (!port_platform_desktop_size(w, h)) { *w = *h = 0; return; }
	if (mode == opt_display::MODE_WINDOWED) port_display_fit(w, h);   // as WinMain does for an unset resolution
}

bool applyNow()
{
	g_pending = false;
	if (g_want.mode == g_active.mode && (g_want.mode == opt_display::MODE_BORDERLESS || g_want.res == g_active.res)) return true;
	int w = codeW(g_want.res), h = codeH(g_want.res), pw = 0, ph = 0;
	port_platform_client_size(&pw, &ph);
	if (g_want.mode == opt_display::MODE_BORDERLESS) w = h = 0;
	else if (!w) desktopFit(g_want.mode, &w, &h);
	opt_bridge::Log("display: apply mode %d %dx%d (was mode %d, window %dx%d)", g_want.mode, w, h, g_active.mode, pw, ph);
	if (!port_platform_apply_display(g_want.mode, w, h)) {
		opt_bridge::Log("display: refused, back to mode %d %dx%d", g_active.mode, pw, ph);
		g_want = g_active;
		port_platform_apply_display(g_active.mode, g_active.mode == opt_display::MODE_BORDERLESS ? 0 : pw, g_active.mode == opt_display::MODE_BORDERLESS ? 0 : ph);
		rebuild(true);
		port_display_size_changed();
		return false;
	}
	g_active = g_want;
	g_mode = g_want.mode;
	rebuild(true);
	port_display_size_changed();
	return true;
}

void schedule() { g_pending = true; g_countdown = kApplyDelay; }

int storeInt(const char *name, int def)
{
	int v = def;
	if (zoosters::settings::GetInt("Settings\\Port", name, &v) != zoosters::settings::Found::Yes) v = def;
	return v;
}
}  // namespace

extern "C" int port_window_mode(void)
{
	if (g_mode < 0) g_mode = loadMode();
	return g_mode;
}

extern "C" void port_display_size_changed(void)
{
	int w = 0, h = 0, cw = 0, ch = 0;
	if (!opt_bridge::ScreenSize(&cw, &ch)) return;   // no camera yet
	if (!port_platform_client_size(&w, &h) || (w == cw && h == ch)) return;
	opt_bridge::Log("display: screen %dx%d -> %dx%d", cw, ch, w, h);
	opt_bridge::ScreenResized(w, h);
	port_wide_settings_changed();   // the automatic HUD scale follows the height
}

// Alt+Enter (window backends): windowed <-> the last full screen mode (borderless unless exclusive was chosen), applied at once and stored
extern "C" void port_window_toggle_fullscreen(void)
{
	init();
	if (g_active.mode != opt_display::MODE_WINDOWED) g_lastFull = g_active.mode;
	opt_display::Flush();
	opt_display::SetWindowMode(g_active.mode == opt_display::MODE_WINDOWED ? g_lastFull : opt_display::MODE_WINDOWED);
	opt_display::Flush();
	opt_display::CommitWindowMode();
}

namespace opt_display {

void Refresh()
{
	init();
	g_want = g_active;
	g_pending = false;
	// the window is the truth: a stored size the fit or the system changed shows as what it is
	rebuild(true);
}

void Tick()
{
	if (g_pending && --g_countdown <= 0) applyNow();
}

void Flush()
{
	if (g_pending) applyNow();
}

int Resolution() { init(); return g_want.mode == MODE_BORDERLESS ? 0 : g_want.res; }
void SetResolution(int c)
{
	init();
	if (g_want.mode == MODE_BORDERLESS || c == g_want.res) return;
	g_want.res = c;
	schedule();
}
void CommitResolution()
{
	init();
	char s[16] = "";
	if (g_want.res) snprintf(s, sizeof s, "%dx%d", codeW(g_want.res), codeH(g_want.res));
	opt_bridge::ConfigSetString(kResKey, s);
}
int ResolutionCount() { init(); if (!g_count) rebuild(true); return g_count; }
const char *const *ResolutionLabels() { ResolutionCount(); return g_labels; }
int ResolutionIndex()
{
	ResolutionCount();
	int c = Resolution();
	for (int i = 0; i < g_count; i++) if (g_codes[i] == c) return i;
	return 0;
}
int ResolutionCodeAt(int i) { ResolutionCount(); return i >= 0 && i < g_count ? g_codes[i] : 0; }

int ModeCount()
{
#ifdef __APPLE__
	return 2;
#else
	return MODE_COUNT;
#endif
}
bool WideScreen()
{
	int w = 0, h = 0;
	return port_platform_client_size(&w, &h) && h > 0 && (float)w / (float)h > 16.0f / 9.0f + 0.01f;
}
int WindowMode() { init(); return g_want.mode; }
void SetWindowMode(int m)
{
	init();
	if (m < 0 || m >= ModeCount() || m == g_want.mode) return;
	g_want.mode = m;
	rebuild(false);   // the new mode's list: a size it does not offer drops back to the desktop
	schedule();
}
void CommitWindowMode() { init(); opt_bridge::ConfigSetInt("Settings\\Port\\WindowMode", g_want.mode); }

int HudScale() { return storeInt("HudScale", 0); }
void SetHudScale(int p)
{
	zoosters::settings::SetInt("Settings\\Port", "HudScale", p);
	port_wide_settings_changed();
}
int HudWidth() { return storeInt("HudWidth", 0); }
void SetHudWidth(int v)
{
	zoosters::settings::SetInt("Settings\\Port", "HudWidth", v);
	port_wide_settings_changed();
}

}  // namespace opt_display
