// Widescreen (--tier qol defines ZOOSTERS_WIDESCREEN). The
// game's CameraHelper::SetFieldOfView (0x44ea40) keeps the horizontal field of view (view window x = tan(fov / 2)) and takes the
// vertical one from a fixed 640 x 448 frame (y = 0.7 * x), whatever the screen: 7% too tall at 4:3, 1.24x too wide at 16:9. Hor+
// with 4:3 as the reference: y = 0.75 * x, and x follows the camera's raster (x = y * width / height). At 4:3 the horizontal
// field is the game's.
#include <stdlib.h>
#include <stdio.h>

struct PortV2 { float x, y; };
extern "C" void *RwCameraGetRaster(const void *cam);
extern "C" int RwRasterGetWidth(const void *raster);
extern "C" int RwRasterGetHeight(const void *raster);
extern "C" void *RwCameraSetViewWindow(void *cam, const PortV2 *vw);
extern "C" FILE *port_log;


// ---- Resolution. The game starts at its registry value Settings\\Display\\Resolution, "800x600" when there is none. Unset here
// = follow the desktop's mode (never written back); a stored value is an explicit override, used as is. Called by WinMain with
// the parsed value (0 x 0 when missing), before the window exists. Windowed: the largest client size of the desktop's aspect that
// fits the work area with the frame; full screen (Settings\\Port\\WindowMode, PORT_FULLSCREEN=1): the desktop's size.
extern "C" void port_platform_dpi_aware(void);
extern "C" int port_platform_desktop_size(int *w, int *h);
extern "C" void port_display_fit(int *w, int *h);   // port/fixes/display.cpp
extern "C" int port_window_mode(void);               // port/options/display.cpp: 0 windowed, 1 borderless, 2 exclusive full screen (PORT_FULLSCREEN=1: 1)

extern "C" void port_wide_resolution(int *w, int *h)
{
	port_platform_dpi_aware();   // physical pixels for the queries below and for the window
	if (*w > 0 && *h > 0) {
		if (port_log) fprintf(port_log, "widescreen: resolution %dx%d (stored value)\n", *w, *h);
		return;
	}
	int dw = 0, dh = 0;
	if (!port_platform_desktop_size(&dw, &dh) || dw <= 0 || dh <= 0) {
		*w = 800; *h = 600;
		if (port_log) fprintf(port_log, "widescreen: resolution 800x600 (no desktop size, the game's default)\n");
		return;
	}
	*w = dw; *h = dh;
	if (port_window_mode() != 0) {
		if (port_log) fprintf(port_log, "widescreen: resolution %dx%d (desktop %dx%d, full screen)\n", *w, *h, dw, dh);
		return;
	}
	port_display_fit(w, h);   // windowed: the desktop's aspect scaled into the work area (port/fixes/display.cpp)
	if (port_log) fprintf(port_log, "widescreen: resolution %dx%d (desktop %dx%d, windowed fit)\n", *w, *h, dw, dh);
}

// Called by GameCamera::UpdateGameCamera right after the game's SetFieldOfView(zoom, camera); returns 1 when it replaced the view window.
extern "C" int port_widescreen_view_window(void *camera, float zoom)
{
	const void *r = RwCameraGetRaster(camera);
	if (!r) return 0;
	int w = RwRasterGetWidth(r), h = RwRasterGetHeight(r);
	if (w <= 0 || h <= 0) return 0;
	PortV2 vw;
	vw.y = zoom * 0.75f;
	vw.x = vw.y * (float)w / (float)h;
	RwCameraSetViewWindow(camera, &vw);
	return 1;
}

// ---- 2D. The game lays out 2D elements on a 640 x 480 grid stretched over the frame buffer: Rt2d through Rt2dDeviceSetMetric(0, 0, 640, 480), the HUD's quads through
// x * g_hudScreenD + g_hudScreenB (Hud2D::BeginFrame: D = width / 640, B = 0), the mouse through x * 640 / client width. In the widescreen build the grid keeps its 4:3 shape:
//   pixel = W * (0.5 + (x / 640 - 0.5) * k + s),   k = (4/3) / (W/H) (1 at 4:3 or narrower)
// with s per element: 0 (centred, the default: menus, loading screens, the pointer), -(1 - k) / 2 or +(1 - k) / 2 (HUD elements near the left / right edge move to it),
// or k = 1 (elements as wide as the screen stay stretched: fades, backdrops).
#include <string.h>
#include "zoosters/settings.h"
enum { WIDE_CENTER, WIDE_STRETCH, WIDE_LEFT, WIDE_RIGHT };
static int g_wideMode = WIDE_CENTER;
static int g_menuSeen, g_menuActive;   // a menu sprite was drawn in this / the previous HUD pass: no edge anchoring (menus are laid out as one 4:3 page)

// ---- HUD scale. The game sizes the HUD as a fraction of the screen, so on a large or sharp display it reads big. Every in-level
// HUD element (not a menu page, not full width, not the pointer) is scaled by S around its anchor (its edge / corner, the screen
// centre for centred elements); the margin to the edge shrinks with it. S = [Settings\Port] HudScale in percent, 0 or missing =
// automatic: window height in display points (pixels / the OS display scale) against a reference height, clamped to 0.5 .. 1.
// PORT_HUD_SCALE=<percent> overrides the setting, PORT_HUD_REF=<points> the reference (tuning).
extern "C" int port_platform_display_scale(float *scale);   // pixels per display point (port/provisional/window_*.cpp)
static const float kHudRefPoints = 640.0f;   // the HUD keeps the game's size up to this height
static int g_scaleThis;                      // the current element is scaled
static float g_anchorX, g_anchorY;           // its anchor on the grid, as fractions (0 = left / top .. 1 = right / bottom)
static float g_hudCachedH = -1.0f, g_hudCached = 1.0f;   // by window height; reset by port_wide_settings_changed
static float HudScale(float H)
{
	if (H == g_hudCachedH)
		return g_hudCached;
	int percent = 0;
	const char *e = getenv("PORT_HUD_SCALE");
	if (e && *e)
		percent = atoi(e);
	else if (zoosters::settings::GetInt("Settings\\Port", "HudScale", &percent) != zoosters::settings::Found::Yes)
		percent = 0;
	float s;
	if (percent > 0) {
		s = percent / 100.0f;
	} else {
		float scale = 1.0f, ref = kHudRefPoints;
		if (!port_platform_display_scale(&scale) || scale <= 0.0f)
			scale = 1.0f;
		const char *r = getenv("PORT_HUD_REF");
		if (r && atof(r) > 0.0)
			ref = (float)atof(r);
		s = ref / (H / scale);
	}
	s = s < 0.5f ? 0.5f : s > 1.0f ? 1.0f : s;
	if (port_log)
		fprintf(port_log, "widescreen: HUD scale %.3f (%s, %.0f px high)\n", s, percent > 0 ? "setting" : "automatic", H);
	g_hudCachedH = H;
	g_hudCached = s;
	return s;
}

// ---- HUD width. [Settings\Port] HudWidth = 1 anchors the edge elements to a centred 16:9 box when the screen is wider, 0 (default) to the
// screen edges. The HUD slides in from off the grid (x < 0 or > 640 at level start), which the box mode would show in the strip
// between box and screen edge, so the shift blends from the screen edge to the box edge with the share of the sprite that is on the
// grid: fully off = screen edge (stays hidden), on the grid = box edge, no jump in between.
static int g_hudWidth = -1;   // -1 = not read yet
static float g_spriteLeft, g_spriteWidth;
static int HudWidthBox(void)
{
	if (g_hudWidth < 0) {
		int v = 0;
		const char *e = getenv("PORT_HUD_WIDTH");
		if (e && *e) v = atoi(e);
		else if (zoosters::settings::GetInt("Settings\\Port", "HudWidth", &v) != zoosters::settings::Found::Yes) v = 0;
		g_hudWidth = v == 1;
		if (port_log) fprintf(port_log, "widescreen: HUD width %s\n", g_hudWidth ? "16:9 box" : "full");
	}
	return g_hudWidth;
}
extern "C" void port_wide_settings_changed(void)
{
	g_hudWidth = -1;
	g_hudCachedH = -1.0f;
}
// shift of an edge element: |s| = 0.5 (edge width - k); edge width 1 = the screen, less = the 16:9 box
static float EdgeShift(float W, float H, float kk, int right)
{
	float full = 0.5f * (1.0f - kk), edge = full;
	if (HudWidthBox() && g_spriteWidth > 0.0f) {
		float bw = (16.0f / 9.0f) * H / W;
		if (bw < 1.0f) {
			float onGrid = right ? (640.0f - g_spriteLeft) : (g_spriteLeft + g_spriteWidth);
			float t = onGrid / g_spriteWidth;
			t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
			edge = full + t * (0.5f * (bw - kk) - full);
		}
	}
	return right ? edge : -edge;
}
static void MapBase(float W, float H, float *k, float *s)
{
	*k = 1.0f; *s = 0.0f;
	if (W <= 0.0f || H <= 0.0f || g_wideMode == WIDE_STRETCH) return;
	float kk = 4.0f * H / (3.0f * W);
	if (kk >= 1.0f) return;
	*k = kk;
	if (g_wideMode == WIDE_LEFT) *s = EdgeShift(W, H, kk, 0);
	else if (g_wideMode == WIDE_RIGHT) *s = EdgeShift(W, H, kk, 1);
}
// The element's grid x as u (0..1): pixel = W (0.5 + (u - 0.5) k + s). Scaled around the anchor a: u' = a + (u - a) S, i.e. k S and
// s + (a - 0.5)(1 - S) k.
static void ScaleX(float H, float *k, float *s)
{
	if (!g_scaleThis) return;
	float S = HudScale(H);
	*s += (g_anchorX - 0.5f) * (1.0f - S) * *k;
	*k *= S;
}
extern "C" void port_wide2d_map(float W, float H, float *k, float *s)
{
	MapBase(W, H, k, s);
	if (W > 0.0f && H > 0.0f)
		ScaleX(H, k, s);
}
// y of the current element: pixel = H (v ky + cy) for the grid's v = y / 480 (y down); 1 and 0 unless the element is scaled.
extern "C" void port_wide2d_map_y(float H, float *ky, float *cy)
{
	*ky = 1.0f; *cy = 0.0f;
	if (!g_scaleThis || H <= 0.0f) return;
	float S = HudScale(H);
	*ky = S;
	*cy = g_anchorY * (1.0f - S);
}

// The HUD quads' x scale and offset (g_hudScreenD, g_hudScreenB) for the current element
extern "C" void port_wide2d_hud(float W, float H, float *B, float *D)
{
	float k, s;
	port_wide2d_map(W, H, &k, &s);
	*D = W * k * 0.0015625f;
	*B = W * (0.5f * (1.0f - k) + s);
}

// The HUD quads' y scale and offset (g_hudScreenE, g_hudScreenC) for the current element: E = H / 480 unscaled
extern "C" void port_wide2d_hud_y(float H, float *C, float *E)
{
	float ky, cy;
	port_wide2d_map_y(H, &ky, &cy);
	*E = H * ky * (1.0f / 480.0f);
	*C = H * cy;
}

// PORT_SPRITE_TRACE=<file>: one line per HUD pass, "<presented frame>\t<name>:<mode><s>|..." for every sprite drawn (mode c / x / l / r =
// centre / stretch / left / right edge, s = HUD-scaled): what an external checker reads to find sprites that vanish, appear or
// change layout for a frame or two (menu page changes).
extern "C" int port_frames_presented(void);
static FILE *g_trace;
static char g_traceLine[16384];
static int g_traceLen = -1, g_traceFrame;   // the pass's frame: taken at its first sprite (the line is written at the next pass)
static void traceFlush(void)
{
	if (g_traceLen < 0) return;
	if (g_trace) { fprintf(g_trace, "%d\t%s\n", g_traceFrame, g_traceLine); fflush(g_trace); }
	g_traceLen = -1;
}
extern "C" void port_wide2d_trace(const char *name)
{
	static int init;
	if (!init) { init = 1; const char *p = getenv("PORT_SPRITE_TRACE"); if (p && *p) g_trace = fopen(p, "w"); }
	if (!g_trace) return;
	if (g_traceLen < 0) { g_traceLen = 0; g_traceLine[0] = 0; g_traceFrame = port_frames_presented(); }
	static const char modes[] = { 'c', 'x', 'l', 'r' };
	int n = snprintf(g_traceLine + g_traceLen, sizeof g_traceLine - g_traceLen, "%s%s:%c%s", g_traceLen ? "|" : "", name ? name : "?",
	                 modes[g_wideMode], g_scaleThis ? "s" : "");
	if (n > 0 && g_traceLen + n < (int)sizeof g_traceLine) g_traceLen += n;
}

// Hud2D::BeginFrame: a new HUD pass
extern "C" void port_wide2d_frame(void)
{
	traceFlush();
	g_menuActive = g_menuSeen;
	g_menuSeen = 0;
	g_wideMode = WIDE_CENTER;
	g_scaleThis = 0;
}

extern "C" void port_wide2d_anchor(float left, float width, float top, float height);
// PORT_WIDE2D_LOG=<presented frame>: every sprite of that frame's HUD pass in the port log (name, box in grid units, edge mode,
// HUD-scale anchor, scaled or not). PORT_WIDE2D_LOG=<text> (not a number): every sprite whose name contains the text, logged
// whenever its line changes. The debug overlay's F10 logs the next frame (port_wide2d_log_frame).
extern "C" int port_frames_presented(void);
static const char *g_logName;
static float g_logBox[4];
static int frame = -2;   // LogSprite's frame to log (-1 none)
extern "C" void port_wide2d_log_frame(int f)
{
	if (frame == -2) frame = -1;   // (PORT_WIDE2D_LOG not read yet: the key wins)
	frame = f;
	if (port_log) fprintf(port_log, "wide2d: HUD layout of frame %d requested (g_menuActive %d, g_menuSeen %d)\n", f, g_menuActive, g_menuSeen);
}
static void LogSprite(const char *what)
{
	static const char *match;
	if (frame == -2) {
		const char *e = getenv("PORT_WIDE2D_LOG");
		frame = -1;
		if (e && *e >= '0' && *e <= '9') frame = atoi(e);
		else if (e && *e) match = e;
	}
	if (!port_log) return;
	if (match ? !(g_logName && strstr(g_logName, match)) : (frame < 0 || port_frames_presented() != frame)) return;
	static const char *const modes[] = { "centre", "stretch", "left", "right" };
	char line[256];
	snprintf(line, sizeof line, "%-28s box %6.1f %6.1f %6.1f %6.1f  %-7s %s anchor %.1f %.1f %s", g_logName ? g_logName : "(none)", g_logBox[0],
	         g_logBox[1], g_logBox[2], g_logBox[3], modes[g_wideMode], g_scaleThis ? "scaled" : "unscaled", g_anchorX, g_anchorY, what);
	if (match) {                         // name mode: only changes (one last line per sprite name)
		static char last[16][256];
		static const char *names[16];
		int i = 0;
		while (i < 16 && names[i] && strcmp(names[i], g_logName) != 0) i++;
		if (i == 16) return;
		if (names[i] && strcmp(last[i], line) == 0) return;
		names[i] = g_logName;
		strcpy(last[i], line);
	}
	fprintf(port_log, "wide2d: frame %d %s\n", port_frames_presented(), line);
}
// SpriteManager's DrawSprite, before one sprite: its name and extent in 640 x 480 units (y down) choose the mode and the HUD-scale anchor
// The menu pages: a pass that draws one of these is laid out as one 4:3 page (no edge moves, no HUD scale). The names of SpriteManager's
// control-buffer check (spritemanager_frame.cpp NoteControlSprite), the pause menu's confirmation messages, and the port's options screen.
static bool isMenuSprite(const char *name)
{
	static const char *const menu[] = { "menu_frame_top", "menu_master", "game_menu_master", "Shop_menu_master", "Quit_Confirmation_Msg",
		"Match_Confirmation_Msg", "Exit_to_Map_Confirm_Msg", "Exit_Zoovenir_Confirm_Msg", "Msg_Box_Bot", "AUTO_Msg_Box_Top",
		"menu_checklist_bkTop", "Menu_Slider_Bkgnd", "MEM_Title_menu_title", "MEM_New_Game_title", "MEM_Load_Game_title", "MEM_prompt_select",
		"Rave_Prompt_O_InstReplay" };
	for (unsigned i = 0; i < sizeof menu / sizeof menu[0]; i++)
		if (strcmp(name, menu[i]) == 0) return true;
	return strncmp(name, "port_opt_", 9) == 0;
}
// SpriteManager::Render, before it draws: this pass's sprites decide menu or HUD. Deciding from the previous pass laid out the
// first frame of a new page the old way (the Exit to Map dialog).
extern "C" void port_wide2d_scan_begin(void) { g_menuActive = 0; }
extern "C" void port_wide2d_scan(const char *name) { if (name && isMenuSprite(name)) g_menuActive = 1; }

// The in-level HUD by sprite name prefix: lives and heads, health, the collectible counters, minigame counters and timers, the
// trading cards flying into their counter. Only these take the HUD scale, and behind a menu page they keep their edge anchoring.
// Everything else named (dialogs, prompts and their bar, zone and title banners, popups) stays at the game's size: a dialog's
// box and its text are separate sprites, and scaling one of them pulls the dialog apart.
static bool IsHudName(const char *name)
{
	static const char *const hud[] = { "Head_", "HeadLives_", "headframe_", "Health_", "health_", "MiniTotemCounter", "PiggyBank",
		"MangoCounter", "Mini_", "R1_", "Ring", "score_number", "Global_Leader_HP", "Vehicle_Counter", "Powerstone_", "TradingCardFlying_",
		"Ballon Counter", "Get Your Powerstones" };   // the last two: the card counter and the goal text on Mini_TextBacking
	for (unsigned i = 0; i < sizeof hud / sizeof hud[0]; i++)
		if (strncmp(name, hud[i], strlen(hud[i])) == 0) return true;
	return false;
}
// Side panels outside that list, not HUD-scaled: the golf scoreboard, the shuffleboard score columns, the rave's score meter, the
// ring goal text, the dev build's fps and build lines. Every other named sprite keeps the centred 4:3 layout: a box near x 160 / 480
// is no sign of a side panel (the rave's outer catchers, the shuffleboard power bar against its frame, the map's place labels).
static bool IsEdgeName(const char *name)
{
	static const char *const edge[] = { "#: ", "Words: ", "Golfer_", "HUD_Alex", "HUD_heads_bkgnd", "hole_info_bkgnd", "UIpuck-",
		"Score_Meter_", "BlackBar", "Bar_Line", "SP_HUB_", "Jump Through Rings", "bx_fps", "bx_build" };
	for (unsigned i = 0; i < sizeof edge / sizeof edge[0]; i++)
		if (strncmp(name, edge[i], strlen(edge[i])) == 0) return true;
	return strncmp(name, "Number", 6) == 0 && strstr(name, "NINE");   // shuffleboard scores: Number1NINE .. Number9blueNINE
}
// The side edge a box sits next to: left of 160 = left, right of 480 = right, both or neither = centre.
static int EdgeMode(float left, float right)
{
	if (left < 160.0f && right > 480.0f) return WIDE_CENTER;
	return left < 160.0f ? WIDE_LEFT : right > 480.0f ? WIDE_RIGHT : WIDE_CENTER;
}
extern "C" void port_wide2d_sprite(const char *name, float left, float width, float top, float height)
{
	g_wideMode = WIDE_CENTER;
	g_scaleThis = 0;
	if (width < 0.0f) { left += width; width = -width; }   // a mirrored sprite (negative scale: the map's title_frame_bottom is title_frame_top
	if (height < 0.0f) { top += height; height = -height; }  // flipped) has its box the other way round
	g_spriteLeft = left; g_spriteWidth = width;
	g_logName = name;
	g_logBox[0] = left; g_logBox[1] = width; g_logBox[2] = top; g_logBox[3] = height;
	if (name) {
		if (isMenuSprite(name)) g_menuSeen = g_menuActive = 1;
		if (strcmp(name, "bx_MousePointer") == 0) return;
	}
	float right = left + width;
	bool hud = name && IsHudName(name);
	bool onMenuPage = g_menuActive && !hud;
	if (width >= 600.0f) g_wideMode = WIDE_STRETCH;
	else if (hud || (!onMenuPage && (!name || IsEdgeName(name)))) g_wideMode = EdgeMode(left, right);   // unnamed: children in HUD groups, the radar
	else g_wideMode = WIDE_CENTER;
	// HUD scale: not menus, full-width elements or the pointer (returned above). The anchor comes from port_wide2d_anchor (the
	// group's root), else from this element.
	if (onMenuPage || g_wideMode == WIDE_STRETCH || height <= 0.0f || (name && !hud)) return;
	g_scaleThis = 1;
	port_wide2d_anchor(left, width, top, height);
}
// The HUD-scale anchor of the current element from a box in grid units (y down): the root of its sprite group, so a group
// (Marty's wheel and lives count, an icon and its number) shrinks around one point and keeps its arrangement. x: the box's side
// (left of 160 = left edge, right of 480 = right edge, both or neither = centre); y: the third the box's centre sits in.
extern "C" void port_wide2d_anchor(float left, float width, float top, float height)
{
	if (width < 0.0f) { left += width; width = -width; }   // mirrored sprites, as in port_wide2d_sprite
	if (height < 0.0f) { top += height; height = -height; }
	float right = left + width, cy = top + 0.5f * height;
	// A group sits on the edge its root does. The mango counter (child of the lives number, child of the head frame) starts at x 178:
	// alone that is centre, which left it behind the head at 16:9 and flipped it from the left edge during the slide-in.
	if (g_scaleThis) {
		int root = EdgeMode(left, right);
		if (root != WIDE_CENTER) g_wideMode = root;
	}
	g_anchorX = (left < 160.0f && right > 480.0f) ? 0.5f : left < 160.0f ? 0.0f : right > 480.0f ? 1.0f : 0.5f;
	g_anchorY = cy < 160.0f ? 0.0f : cy > 320.0f ? 1.0f : 0.5f;
	LogSprite(g_scaleThis ? "" : "(anchor only)");
}
extern "C" void port_wide2d_reset(void) { g_wideMode = WIDE_CENTER; g_scaleThis = 0; }

// A rectangle the game places in stretched pixels (x = W * fraction, the radar): the same rectangle on the 4:3 grid, moved to the side edge
// it sits next to. x and w in pixels, in and out.
extern "C" void port_wide2d_rect(float W, float H, float *x, float *w)
{
	if (W <= 0.0f) return;
	int saved = g_wideMode, savedScale = g_scaleThis;
	float left = *x * 640.0f / W, width = *w * 640.0f / W;
	port_wide2d_sprite(0, left, width, 0.0f, 0.0f);   // no height: not HUD-scaled (the radar's y is the game's)
	float k, s;
	port_wide2d_map(W, H, &k, &s);
	g_wideMode = saved;
	g_scaleThis = savedScale;
	*x = W * (0.5f + (left / 640.0f - 0.5f) * k + s);
	*w = width / 640.0f * W * k;
}

// The window procedure's client x for the menu pointer: the pixel the stretched 640 x 480 grid would have under the pointer
extern "C" int port_wide2d_mouse_x(int x, int clientW, int clientH)
{
	int saved = g_wideMode, savedScale = g_scaleThis;
	g_wideMode = WIDE_CENTER;
	g_scaleThis = 0;                       // the pointer maps on the unscaled grid
	float k, s;
	port_wide2d_map((float)clientW, (float)clientH, &k, &s);
	g_wideMode = saved;
	g_scaleThis = savedScale;
	if (k >= 1.0f) return x;
	return (int)(clientW * 0.5f + (x - clientW * 0.5f) / k);
}
