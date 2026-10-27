// ZOOSTERS_OPTIONS UI kit (ui.h). Layout is on the game's 640x480 grid and copies the pause menu's frame: menu_top (centre
// (320,48), 135 % wide), menu_mid strips of 16 from its lower edge on, MENU_BOTTOM below; rows 26 apart. Textures are the ones
// every level already has loaded.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "../bridge/options_bridge.h"
#include "../strings.h"

using namespace opt_bridge;

namespace opt_ui {
namespace {
const int kMaxText = 40, kMaxQuad = 50;
const int kBackdropQuad = 0, kFrameQuad0 = 1, kFrameQuads = 20, kBarQuad = 21, kWidgetQuad0 = 22;   // quad pool: backdrop, frame strips, the focus bar, widget quads (fixed roles: draw order = creation order)
char g_textNames[kMaxText][16], g_quadNames[kMaxQuad][16];
Sprite g_text[kMaxText], g_quad[kMaxQuad];
int g_textUsed, g_frameUsed, g_barUsed, g_widgetUsed;     // used this frame
bool g_pools, g_backdrop;

// Text width: texts that must fit a column are scaled by (room / width at scale 1). The width is measured from the sprite's box of the
// previous frame (SpriteBounds), per string; until then an estimate.
struct Fit { char s[40]; float w1; };
Fit g_fit[96];
int g_nFit;
struct Pending { Sprite sp; char s[40]; float scale; } g_pend[40];
int g_nPend;
float width1(const char *s)
{
	for (int i = 0; i < g_nFit; i++) if (strcmp(g_fit[i].s, s) == 0) return g_fit[i].w1;
	return 14.0f * (float)strlen(s);
}
float fitScale(const char *s, float room, float maxScale)
{
	float k = room / width1(s);
	return k < maxScale ? (k < 0.5f ? 0.5f : k) : maxScale;
}
void measure()
{
	for (int i = 0; i < g_nPend; i++) {
		float x, y, w, h;
		if (!SpriteBounds(g_pend[i].sp, &x, &y, &w, &h) || w <= 1.0f) continue;
		int k = 0;
		while (k < g_nFit && strcmp(g_fit[k].s, g_pend[i].s) != 0) k++;
		if (k == g_nFit) { if (g_nFit >= 96) continue; g_nFit++; snprintf(g_fit[k].s, sizeof g_fit[k].s, "%s", g_pend[i].s); }
		g_fit[k].w1 = w / g_pend[i].scale;
	}
	g_nPend = 0;
}

struct State {
	int focus, rows, rowsLast;
	bool back;
	MouseState mouse;
} g;

// geometry
const float kCx = 320.0f, kRow0 = 120.0f, kRowH = 26.0f, kFrameScale = 1.6f;   // the pause menu's frame is 1.35 wide: the rows need more room
// The frame's inner area is x 156 .. 488 of the grid (the strips are 160 % wide, the border art takes the rest): the focus bar, labels,
// arrows and numbers all stay inside it.
const float kBarL = 156.0f, kBarW = 328.0f;                 // focus bar / click area
const float kLabelX = 164.0f, kLabelScale = 0.9f, kLabelR = 326.0f;   // labels shrink to fit before the value column
const float kValL = 340.0f, kValR = 470.0f, kValC = 405.0f;  // arrows and the centre of the value
const float kValW = 104.0f;                      // room for a value text between the arrows
const float kNumR = 480.0f;                      // slider number, right edge
const float kTrackL = 340.0f, kTrackR = 424.0f;   // the slider track (the game's SLIDERMARKS strip) and its marker travel
const float kArrow = 20.0f;                      // our arrow_left / arrow_right quads

const unsigned kWhite = 0xFFFFFFFF, kDim = 0xFFC0C0C0, kEnd = 0xFF606060, kTitle = 0xFF426B8A;   // the pause menu: unselected 0xC0 grey, selected white (colours here are what is shown; see half)
const int kLayerFrame = 9, kLayerBar = 8, kLayerItem = 7, kLayerKnob = 6;

unsigned half(unsigned argb) { return (argb & 0xFF000000u) | ((argb >> 1) & 0x7F7F7Fu); }   // both modulate 2x: a tint of 0x80 is full, 0xFF brightens past white

// our arrow textures (Mod\Levels\options.stream) and the game's slider art (every level's dictionary); each missing piece falls back to
// text arrows / the plain bar
bool haveArrows() { static int v = -1; if (v < 0) { float w, h; v = TextureSize("arrow_left", &w, &h) && TextureSize("arrow_right", &w, &h); Log("arrow textures: %s", v ? "found" : "missing, text arrows"); } return v > 0; }
bool haveSliderArt() { static int v = -1; if (v < 0) { float w, h; v = TextureSize("SLIDERMARKS", &w, &h) && TextureSize("SLIDERMARKER", &w, &h); Log("slider art: %s", v ? "found" : "missing, plain bar"); } return v > 0; }
bool haveSliderPlate() { static int v = -1; if (v < 0) { float w, h; v = TextureSize("SLIDER_BKDND", &w, &h); } return v > 0; }

float rowY(int r) { return kRow0 + kRowH * (float)r; }

Sprite text(const char *s, float x, float y, int anchor, unsigned argb, float scale, bool fitted = false)
{
	if (g_textUsed >= kMaxText) return 0;
	int i = g_textUsed++;
	if (argb != kTitle) argb = half(argb);   // kTitle is given as the tint itself
	Sprite sp = g_text[i];
	SpriteSetText(sp, s);
	SpriteSetAnchor(sp, anchor);
	SpriteSetPos(sp, x, y);
	SpriteSetScale(sp, scale, scale);
	SpriteSetTint(sp, argb);
	SpriteSetVisible(sp, true);
	if (fitted && g_nPend < 40) { Pending &p = g_pend[g_nPend++]; p.sp = sp; snprintf(p.s, sizeof p.s, "%s", s); p.scale = scale; }
	return sp;
}

Sprite quadAt(int i, const char *tex, float x, float y, float w, float h, int anchor, unsigned argb)
{
	Sprite sp = g_quad[i];
	SpriteSetQuad(sp, tex);
	SpriteSetAnchor(sp, anchor);
	SpriteSetPos(sp, x, y);
	if (w > 0) SpriteSetSize(sp, w, h); else SpriteSetScale(sp, -w, h);   // w <= 0: scale factors (-w, h) instead of a size
	SpriteSetTint(sp, argb);
	SpriteSetVisible(sp, true);
	return sp;
}

Sprite quad(const char *tex, float x, float y, float w, float h, int anchor, unsigned argb, int layer)
{
	int i;
	if (layer == kLayerFrame) { if (g_frameUsed >= kFrameQuads) return 0; i = kFrameQuad0 + g_frameUsed++; }
	else if (layer == kLayerBar) { i = kBarQuad; g_barUsed = 1; }
	else { if (kWidgetQuad0 + g_widgetUsed >= kMaxQuad) return 0; i = kWidgetQuad0 + g_widgetUsed++; }
	return quadAt(i, tex, x, y, w, h, anchor, argb);
}

unsigned rowColor(int r) { return g.focus == r ? kWhite : kDim; }
// both arrows on every selector; an end of the list dims its arrow
void arrow(bool left, float x, float y, int r, bool enabled)
{
	unsigned c = enabled ? rowColor(r) : kEnd;
	if (haveArrows()) quad(left ? "arrow_left" : "arrow_right", x, y, kArrow, kArrow, ANCHOR_CENTRE, half(c), kLayerItem);
	else text(left ? "<" : ">", x, y, ANCHOR_CENTRE, c, 1.0f);
}

bool inRow(int r) { return g.mouse.valid && g.mouse.x >= kBarL && g.mouse.x <= kBarL + kBarW && g.mouse.y >= rowY(r) - kRowH * 0.5f && g.mouse.y < rowY(r) + kRowH * 0.5f; }
bool clicked(int r) { if (!g.mouse.pressed || !inRow(r)) return false; g.focus = r; return true; }

void label(int r, const char *s)
{
	float sc = fitScale(s, kLabelR - kLabelX, kLabelScale);
	text(s, kLabelX, rowY(r), ANCHOR_MIDDLE_LEFT, rowColor(r), sc, true);
}

// Sprites are created once, in drawing order (equal priority: the sprite made later is drawn later; changing a run-time sprite's priority
// breaks SpriteManager::Update): frame strips, focus bar, widget quads, then the texts.
void createPools()
{
	for (int i = 0; i < kMaxQuad; i++) { sprintf(g_quadNames[i], "port_opt_q%02d", i); g_quad[i] = SpriteCreate(g_quadNames[i]); }
	for (int i = 0; i < kMaxText; i++) { sprintf(g_textNames[i], "port_opt_t%02d", i); g_text[i] = SpriteCreate(g_textNames[i]); }
	g_pools = true;
}
}  // namespace

void ResetFocus() { g.focus = 0; g.rowsLast = 0; }
void SetFocus(int row) { g.focus = row; }
void SetBackdrop(bool on) { g_backdrop = on; }

void Begin(const char *title)
{
	g.rows = 0;
	g.back = false;
	if (!g_pools) createPools();
	g_textUsed = g_frameUsed = g_barUsed = g_widgetUsed = 0;
	measure();
	InputPoll();
	g.mouse = Mouse();
	int n = g.rowsLast;
	if (n > 0) {
		if (InputPressed(NAV_DOWN)) { g.focus = (g.focus + 1) % n; PlaySound(0); }
		if (InputPressed(NAV_UP)) { g.focus = (g.focus + n - 1) % n; PlaySound(0); }
		if (g.mouse.moved)
			for (int r = 0; r < n; r++)
				if (inRow(r) && g.focus != r) { g.focus = r; PlaySound(0); break; }
	}
	if (g.focus >= n && n > 0) g.focus = n - 1;
	g.back = InputPressed(NAV_BACK);

	// over a busy background (the title's 3D scene): the screen dimmed behind the panel
	if (g_backdrop) quadAt(kBackdropQuad, "col", kCx, 240.0f, 4000.0f, 640.0f, ANCHOR_CENTRE, 0x60404040);
	// frame: top, as many mids as the rows need, bottom (same strips and 135 % width as the pause menu's)
	float topW = 0, topH = 0, midW = 0, midH = 0;
	if (TextureSize("menu_top", &topW, &topH) && TextureSize("menu_mid", &midW, &midH) && midH > 0) {
		float midTop = 48.0f + topH * 0.5f;   // the strips start where menu_top ends (they started at 112: a strip-high gap behind the title)
		float need = rowY(n > 0 ? n - 1 : 4) + kRowH * 0.5f + 8.0f - midTop;
		int mids = (int)(need / midH + 0.999f);
		if (mids < 4) mids = 4;
		if (mids > 18) mids = 18;
		quad("menu_top", kCx, 48.0f, -kFrameScale, 1.0f, ANCHOR_CENTRE, 0xFF808080, kLayerFrame);
		for (int i = 0; i < mids; i++)
			quad("menu_mid", kCx, midTop + midH * (float)i, -kFrameScale, 1.0f, ANCHOR_TOP_CENTRE, 0xFF808080, kLayerFrame);
		quad("MENU_BOTTOM", kCx, midTop + midH * (float)mids, -kFrameScale, 1.0f, ANCHOR_TOP_CENTRE, 0xFF808080, kLayerFrame);
	}
	text(title, kCx, 83.0f, ANCHOR_CENTRE, kTitle, 1.4f);
	// the focus bar (the pause menu's "Selection Indicator": the solid 'col' texture, orange at 25 %)
	if (n > 0) quad("col", kBarL, rowY(g.focus), kBarW, 24.0f, ANCHOR_MIDDLE_LEFT, 0x40802D00, kLayerBar);
}

bool Slider(const char *lab, int *v, int lo, int hi, int step)
{
	int r = g.rows++;
	label(r, lab);
	int old = *v;
	if (g.focus == r) {
		if (InputPressed(NAV_LEFT)) *v -= step;
		if (InputPressed(NAV_RIGHT)) *v += step;
	}
	if (g.mouse.valid && g.mouse.down && inRow(r) && g.mouse.x >= kTrackL - 10 && g.mouse.x <= kTrackR + 10 && (g.mouse.pressed || g.focus == r)) {
		g.focus = r;
		float t = (g.mouse.x - kTrackL) / (kTrackR - kTrackL);
		if (t < 0) t = 0; if (t > 1) t = 1;
		int steps = (int)((t * (float)(hi - lo)) / (float)step + 0.5f);
		*v = lo + steps * step;
	}
	if (*v < lo) *v = lo;
	if (*v > hi) *v = hi;
	float frac = hi > lo ? (float)(*v - lo) / (float)(hi - lo) : 0.0f;
	float y = rowY(r);
	if (haveSliderArt()) {
		// the game's own slider art as the pause menu's slider page uses it: the SLIDERMARKS strip as the track, SLIDERMARKER (a
		// pointer, tip down) on it (the SLIDER_BKDND picture is that page's backdrop: here a small plate behind the strip)
		if (haveSliderPlate()) quad("SLIDER_BKDND", (kTrackL + kTrackR) * 0.5f, y, kTrackR - kTrackL + 20.0f, 24.0f, ANCHOR_CENTRE, 0xC0808080, kLayerItem);
		quad("SLIDERMARKS", kTrackL, y + 3.0f, kTrackR - kTrackL, 8.0f, ANCHOR_MIDDLE_LEFT, 0xFF808080, kLayerItem);
		quad("SLIDERMARKER", kTrackL + (kTrackR - kTrackL) * frac, y - 4.0f, 18.0f, 18.0f, ANCHOR_CENTRE, 0xFF808080, kLayerKnob);
	} else {
		quad("col", kTrackL, y, kTrackR - kTrackL, 6.0f, ANCHOR_MIDDLE_LEFT, 0xC0202020, kLayerItem);
		if (frac > 0) quad("col", kTrackL, y, (kTrackR - kTrackL) * frac, 6.0f, ANCHOR_MIDDLE_LEFT, 0xFFE0902A, kLayerItem);
		quad("SLIDERMARKER", kTrackL + (kTrackR - kTrackL) * frac, y, 14.0f, 18.0f, ANCHOR_CENTRE, 0xFF808080, kLayerKnob);
	}
	char num[16]; sprintf(num, "%d", *v);
	text(num, kNumR, y, ANCHOR_MIDDLE_RIGHT, rowColor(r), 1.0f);
	clicked(r);
	return *v != old;
}

bool Selector(const char *lab, int *idx, const char *const *items, int count)
{
	int r = g.rows++;
	label(r, lab);
	int old = *idx;
	float y = rowY(r);
	bool click = clicked(r);
	if (g.focus == r) {
		if (InputPressed(NAV_LEFT)) (*idx)--;
		if (InputPressed(NAV_RIGHT)) (*idx)++;
		if (click) {
			if (g.mouse.x < kValC - 25.0f) (*idx)--;
			else if (g.mouse.x > kValC + 25.0f) (*idx)++;
		}
	}
	if (*idx < 0) *idx = 0;
	if (*idx > count - 1) *idx = count - 1;
	arrow(true, kValL, y, r, *idx > 0);
	arrow(false, kValR, y, r, *idx < count - 1);
	text(items[*idx], kValC, y, ANCHOR_CENTRE, rowColor(r), fitScale(items[*idx], kValW, 1.0f), true);
	if (*idx != old) PlaySound(1);
	return *idx != old;
}

bool Toggle(const char *lab, bool *v)
{
	int r = g.rows++;
	label(r, lab);
	bool old = *v;
	float y = rowY(r);
	bool click = clicked(r);
	if (g.focus == r && (InputPressed(NAV_LEFT) || InputPressed(NAV_RIGHT) || InputPressed(NAV_ACCEPT) || click)) *v = !*v;
	arrow(true, kValL, y, r, true);
	arrow(false, kValR, y, r, true);
	text(*v ? opt_strings::Str("OPT_ON", "On") : opt_strings::Str("OPT_OFF", "Off"), kValC, y, ANCHOR_CENTRE, rowColor(r), 1.0f);
	if (*v != old) PlaySound(1);
	return *v != old;
}

bool Button(const char *lab)
{
	int r = g.rows++;
	float y = rowY(r);
	bool click = clicked(r);
	text(lab, kCx, y, ANCHOR_CENTRE, rowColor(r), 1.0f);
	return (g.focus == r && InputPressed(NAV_ACCEPT)) || click;
}

bool BackRequested() { return g.back; }

void End()
{
	g.rowsLast = g.rows;
	if (g_pools) {
		for (int i = g_textUsed; i < kMaxText; i++) SpriteSetVisible(g_text[i], false);
		for (int i = g_frameUsed; i < kFrameQuads; i++) SpriteSetVisible(g_quad[kFrameQuad0 + i], false);
		if (!g_backdrop) SpriteSetVisible(g_quad[kBackdropQuad], false);
		if (!g_barUsed) SpriteSetVisible(g_quad[kBarQuad], false);
		for (int i = kWidgetQuad0 + g_widgetUsed; i < kMaxQuad; i++) SpriteSetVisible(g_quad[i], false);
	}
	SpritesFlush();
}

// Closes the screen: removes every sprite of the kit. The sprite manager's active set is rebuilt on every level load: a pool made in the title would dangle in the level.
void Destroy()
{
	g_nPend = 0;
	g_textUsed = g_frameUsed = g_barUsed = g_widgetUsed = 0;
	if (!g_pools) return;
	for (int i = 0; i < kMaxQuad; i++) { SpriteDestroy(g_quad[i]); g_quad[i] = 0; }
	for (int i = 0; i < kMaxText; i++) { SpriteDestroy(g_text[i]); g_text[i] = 0; }
	g_pools = false;
	SpritesFlush();
}

}  // namespace opt_ui
