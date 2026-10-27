// Minimal Rt2d toolkit: CTM, brushes and bitmap fonts drawn through librw's Im2D (rt2dFontShowBitmap 0x468860, Rt2dFontGetHeight 0x469a80).
// rwID_2DFONT (0x199) chunk body, no struct sub-chunk (title fonts t10 / tsa16 / fbigtxt):
//   +0x00 u32 0x02000001   +0x04 u32 0   +0x08 f32 height in texels (27 for tsa16)   +0x0c f32 0.185   +0x10..+0x1b 0
//   +0x1c u32 glyph count (138)   +0x20 u32 0   +0x24 u16 map[256]: char code -> glyph index, 0xffff = none
//   +0x224 glyphs, 21 bytes each: f32 u0, v0, u1, v1 (v0 at the top), f32 width (advance / height), u8 flags
//   then u32 1 (texture count) and char[32] texture name (looked up in the current texture dictionary)
// The game reads only +4 (a float it rescales to the screen) and +0x14 (the rw::Texture) of the font object, so those stay put.
// Device space is the metric rectangle (the game always sets 0,0,640,480), y up; the CTM is a 2x3 affine matrix applied before the metric.
#include <stdio.h>
#include <vector>
#include "../debugui/debugui.h"   // PORT_DBG_HIDDEN (0 unless ZOOSTERS_DEBUGUI)
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef RW_D3D9
#include <windows.h>
#include <d3d9.h>
#endif
#include <rw.h>
#include "rwlayouts.h"
#include "../compat/rw_null_device.h"   // headless build: device vertex types (no-op on d3d9)

extern "C" FILE *port_log;
#define TLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)
extern "C++" rw::Texture *port_find_texture(const char *name);       // rwshim.cpp
extern "C" void port_realign_asset(void *stream, const char *who);   // rwshim.cpp
extern "C" int port_draw_im2d;
extern "C" void *port_im2d_clip_off(void);          // rwrender.cpp: clipping off for the draw
extern "C" void port_im2d_clip_restore(void *);
extern "C++" rw::Stream *port_stream_of(void *stream);   // rwshim.cpp

// ---- call counters (port_rt2d_report)
enum { C_OPEN, C_CLOSE, C_SETCAM, C_GETCAM, C_METRIC, C_LAYER, C_PUSH, C_POP, C_TRANS, C_SCALE, C_ROT, C_BCREATE, C_BDESTROY, C_BRGBA,
       C_FREAD, C_FSREAD, C_FDESTROY, C_FHEIGHT, C_FWIDTH, C_FSHOW, C_FTXDSET, C_FTXDGET, C_FPATH, C_N };
static const char *const g_cname[C_N] = { "Rt2dOpen", "Rt2dClose", "Rt2dDeviceSetCamera", "Rt2dDeviceGetCamera", "Rt2dDeviceSetMetric",
	"Rt2dDeviceSetLayerDepth", "Rt2dCTMPush", "Rt2dCTMPop", "Rt2dCTMTranslate", "Rt2dCTMScale", "Rt2dCTMRotate", "Rt2dBrushCreate",
	"Rt2dBrushDestroy", "Rt2dBrushSetRGBA", "Rt2dFontRead", "Rt2dFontStreamRead", "Rt2dFontDestroy", "Rt2dFontGetHeight",
	"Rt2dFontGetStringWidth", "Rt2dFontShow", "Rt2dFontTexDictionarySet", "Rt2dFontTexDictionaryGet", "Rt2dFontSetPath" };
static unsigned g_calls[C_N];
extern "C" void port_rt2d_report(void)
{
	if (!port_log) return;
	fprintf(port_log, "rt2d: call counts:");
	for (int i = 0; i < C_N; i++) if (g_calls[i]) fprintf(port_log, " %s=%u", g_cname[i], g_calls[i]);
	fprintf(port_log, "\n");
}

// ---- device
static rw::Camera *g_cam;
static float g_metric[4] = { 0, 0, 640, 480 };
static float g_layerDepth = 1.0f;
extern "C" int Rt2dOpen(void *cam) { g_calls[C_OPEN]++; g_cam = (rw::Camera *)cam; TLOG("rt2d: Rt2dOpen(%p)\n", cam); return 1; }
extern "C" void Rt2dClose(void) { g_calls[C_CLOSE]++; }
extern "C" int Rt2dDeviceSetCamera(void *cam) { g_calls[C_SETCAM]++; g_cam = (rw::Camera *)cam; return 1; }
extern "C" void *Rt2dDeviceGetCamera(void) { g_calls[C_GETCAM]++; return g_cam; }
extern "C" int Rt2dDeviceSetMetric(float x, float y, float w, float h) { g_calls[C_METRIC]++; g_metric[0] = x; g_metric[1] = y; g_metric[2] = w; g_metric[3] = h; return 1; }
extern "C" int Rt2dDeviceSetLayerDepth(float d) { g_calls[C_LAYER]++; g_layerDepth = d; return 1; }

// ---- current transformation matrix: x' = a*x + c*y + e, y' = b*x + d*y + f
struct Ctm { float a, b, c, d, e, f; };
static Ctm g_ctm[32] = { { 1, 0, 0, 1, 0, 0 } };
static int g_sp;
extern "C" int Rt2dCTMPush(void) { g_calls[C_PUSH]++; if (g_sp < 31) { g_ctm[g_sp + 1] = g_ctm[g_sp]; g_sp++; } return 1; }
extern "C" int Rt2dCTMPop(void) { g_calls[C_POP]++; if (g_sp > 0) g_sp--; return 1; }
static void ctmMul(const Ctm &m)        // current = current * m (m applied first)
{
	Ctm &t = g_ctm[g_sp];
	Ctm r;
	r.a = t.a * m.a + t.c * m.b; r.b = t.b * m.a + t.d * m.b;
	r.c = t.a * m.c + t.c * m.d; r.d = t.b * m.c + t.d * m.d;
	r.e = t.a * m.e + t.c * m.f + t.e; r.f = t.b * m.e + t.d * m.f + t.f;
	t = r;
}
extern "C" int Rt2dCTMTranslate(float x, float y) { g_calls[C_TRANS]++; Ctm m = { 1, 0, 0, 1, x, y }; ctmMul(m); return 1; }
extern "C" int Rt2dCTMScale(float x, float y) { g_calls[C_SCALE]++; Ctm m = { x, 0, 0, y, 0, 0 }; ctmMul(m); return 1; }
extern "C" int Rt2dCTMRotate(float deg) { g_calls[C_ROT]++; float r = deg * 3.14159265f / 180.0f; Ctm m = { cosf(r), sinf(r), -sinf(r), cosf(r), 0, 0 }; ctmMul(m); return 1; }

// 0x467bf0 _rt2dCTMGet: the 2D CTM as an RwMatrix in world space: CTM, then S(layer), S(-mindim, mindim), T(vw/2 * layer) as
// postconcats, then the camera LTM. vw = 2 * view window, mindim = min(vw.x, vw.y). The -mindim is real: Rt2d x is mirrored against
// the camera (RW's +x is left). The game folds the metric in as the innermost transform; this port keeps it in the model -> metric Ctm
// and applies it here. Hud2DObject::DrawBillboard (0x450e80) copies the result into a 3D object's frame and adds `at * 1000`.
static void metricX(float *m0, float *m2);
static void metricY(float *m1, float *m3);
extern "C" float *_rt2dCTMGet(void)
{
	static float m[16];
	const Ctm &t = g_ctm[g_sp];
	float g_metric[4];   // x through metricX (the game's metric; the widescreen layout in a ZOOSTERS_WIDESCREEN build)
	metricX(&g_metric[0], &g_metric[2]); metricY(&g_metric[1], &g_metric[3]);
	float vwx = g_cam ? g_cam->viewWindow.x * 2.0f : 2.0f, vwy = g_cam ? g_cam->viewWindow.y * 2.0f : 1.5f;
	float md = vwx < vwy ? vwx : vwy;
	float layer = (g_cam && g_cam->projection == rw::Camera::PARALLEL) ? 1.0f : g_layerDepth;
	float xl = vwx / (g_metric[2] * md), yl = vwy / (g_metric[3] * md);
	float sx = xl * layer * -md, sy = yl * layer * md;           // metric fold-in, S(layer), S(-mindim, mindim) on the x / y columns
	float rx = t.a * sx, ry = t.b * sy, ux = t.c * sx, uy = t.d * sy;                     // right, up (vectors)
	float px = (t.e - g_metric[0]) * sx + vwx * 0.5f * layer, py = (t.f - g_metric[1]) * sy - vwy * 0.5f * layer;   // pos
	const rw::Matrix *L = g_cam ? g_cam->getFrame()->getLTM() : 0;
	memset(m, 0, sizeof m);
	if (L) {
		m[0] = rx * L->right.x + ry * L->up.x;  m[1] = rx * L->right.y + ry * L->up.y;  m[2] = rx * L->right.z + ry * L->up.z;
		m[4] = ux * L->right.x + uy * L->up.x;  m[5] = ux * L->right.y + uy * L->up.y;  m[6] = ux * L->right.z + uy * L->up.z;
		m[8] = L->at.x; m[9] = L->at.y; m[10] = L->at.z;
		m[12] = px * L->right.x + py * L->up.x + L->pos.x; m[13] = px * L->right.y + py * L->up.y + L->pos.y; m[14] = px * L->right.z + py * L->up.z + L->pos.z;
	} else { m[0] = rx; m[1] = ry; m[4] = ux; m[5] = uy; m[10] = 1.0f; m[12] = px; m[13] = py; }
	m[15] = 1.0f;
	{ static int n; if (n++ < 3) TLOG("rt2d: _rt2dCTMGet -> right %g %g up %g %g pos %g %g %g (layer %g, ltm %d)\n", m[0], m[1], m[4], m[5], m[12], m[13], m[14], layer, L != 0); }
	return m;
}

// ---- brushes
// Rt2dBrushSetRGBA : bottom = col0, dbottom = col1 - col0, dtop = col2 - col3, top = col3. A glyph's
// bottom vertices are bottom + dbottom * t and its top vertices top + dtop * t, t = position along the string / string width.
struct Brush { float bottom[4], dbottom[4], top[4], dtop[4]; };
extern "C" void *Rt2dBrushCreate(void)
{
	g_calls[C_BCREATE]++;
	Brush *b = (Brush *)calloc(1, 0x200);
	for (int i = 0; i < 4; i++) b->bottom[i] = b->top[i] = 255.0f;
	return b;
}
extern "C" int Rt2dBrushDestroy(void *b) { g_calls[C_BDESTROY]++; free(b); return 1; }
static void rgbaF(float *o, const rw::RGBA *c) { o[0] = c->red; o[1] = c->green; o[2] = c->blue; o[3] = c->alpha; }
extern "C" void *Rt2dBrushSetRGBA(void *b, const rw::RGBA *c0, const rw::RGBA *c1, const rw::RGBA *c2, const rw::RGBA *c3)
{
	g_calls[C_BRGBA]++;
	Brush *br = (Brush *)b;
	if (br) {
		float f0[4], f1[4], f2[4], f3[4];
		rgbaF(f0, c0); rgbaF(f1, c1); rgbaF(f2, c2); rgbaF(f3, c3);
		for (int i = 0; i < 4; i++) { br->top[i] = f3[i]; br->dtop[i] = f2[i] - f3[i]; br->bottom[i] = f0[i]; br->dbottom[i] = f1[i] - f0[i]; }
	}
	return b;
}

// ---- fonts
static rw::TexDictionary *g_fontTxd;
extern "C" void *Rt2dFontTexDictionarySet(void *txd) { g_calls[C_FTXDSET]++; g_fontTxd = (rw::TexDictionary *)txd; return txd; }
extern "C" void *Rt2dFontTexDictionaryGet(void) { g_calls[C_FTXDGET]++; return g_fontTxd; }
extern "C" int Rt2dFontSetPath(const char *p) { g_calls[C_FPATH]++; (void)p; return 1; }
static Font *emptyFont(void)
{
	Font *f = (Font *)calloc(1, sizeof(Font));
	f->height = 1.0f;
	memset(f->map, 0xff, sizeof f->map);
	rw::Raster *r = rw::Raster::create(1, 1, 32, rw::Raster::C8888 | rw::Raster::TEXTURE);
	f->tex = rw::Texture::create(r);
	return f;
}
extern "C" void *Rt2dFontRead(const char *name) { g_calls[C_FREAD]++; TLOG("rt2d: Rt2dFontRead(%s) -> empty font\n", name ? name : ""); return emptyFont(); }
extern "C" void *Rt2dFontStreamRead(void *stream)
{
	g_calls[C_FSREAD]++;
	// the game has found the rwID_2DFONT chunk: the stream is at its body (no struct chunk inside)
	rw::Stream *ls = port_stream_of(stream);
	Font *f = 0;
	unsigned char head[0x224];
	if (ls && ls->read8(head, sizeof head) == sizeof head) {
		int ng; memcpy(&ng, head + 0x1c, 4);
		rw::uint32 len = 0x224 + (ng > 0 && ng <= 1024 ? ng : 0) * 21 + 4 + 32;
		unsigned char *buf = (unsigned char *)malloc(len);
		memcpy(buf, head, sizeof head);
		if (ls->read8(buf + sizeof head, len - sizeof head) == len - sizeof head) {
			f = (Font *)calloc(1, sizeof(Font));
			memcpy(&f->height, buf + 0x08, 4);
			f->height0 = f->height;
			memcpy(&f->baseline, buf + 0x0c, 4);
			memcpy(&f->nglyphs, buf + 0x1c, 4);
			memcpy(f->map, buf + 0x24, 512);
			if (f->nglyphs < 0 || f->nglyphs > 1024 || 0x224 + f->nglyphs * 21 + 4 > (int)len) f->nglyphs = 0;
			f->glyphs = (Glyph *)calloc(f->nglyphs ? f->nglyphs : 1, sizeof(Glyph));
			memcpy(f->glyphs, buf + 0x224, f->nglyphs * sizeof(Glyph));
			unsigned tail = 0x224 + f->nglyphs * 21;
			if (tail + 4 + 32 <= len) { memcpy(f->texname, buf + tail + 4, 32); f->texname[32] = 0; }
			rw::Texture *t = 0;
			if (g_fontTxd && f->texname[0]) t = g_fontTxd->find(f->texname);
			if (!t && f->texname[0]) t = port_find_texture(f->texname);
			if (t) { t->refCount++; f->tex = t; }
			else { rw::Raster *r = rw::Raster::create(1, 1, 32, rw::Raster::C8888 | rw::Raster::TEXTURE); f->tex = rw::Texture::create(r); }
			TLOG("rt2d: font: height %g baseline %g %d glyphs, texture %s (%s, %dx%d)\n", f->height, f->baseline, f->nglyphs, f->texname,
			     t ? "found" : "missing", f->tex->raster ? f->tex->raster->width : 0, f->tex->raster ? f->tex->raster->height : 0);
		}
		free(buf);
	}
	port_realign_asset(stream, "Rt2dFontStreamRead");
	return f ? f : emptyFont();
}
extern "C" int Rt2dFontDestroy(void *p) { g_calls[C_FDESTROY]++; Font *f = (Font *)p; if (f) { free(f->glyphs); free(f); } return 1; }
// Height is font->height * |ystep| (0x469a80). One pixel is metric height / viewport height units, 0.8 at 800x600 with metric 640x480,
// which cancels the game's rescale by screen height / 480 (cover_c.cpp) so text keeps its 640x480 size.
static float viewportH(void) { return g_cam && g_cam->frameBuffer ? (float)g_cam->frameBuffer->height : 480.0f; }
// Metric x origin and width: the game's, or the widescreen layout of port/fixes/widescreen.cpp (ZOOSTERS_WIDESCREEN):
// pixel = W (0.5 + (u - 0.5) k + s) for u = (x - x0) / w.
#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_map(float W, float H, float *k, float *s);
#endif
static void metricX(float *m0, float *m2)
{
#ifdef ZOOSTERS_WIDESCREEN
	float W = g_cam && g_cam->frameBuffer ? (float)g_cam->frameBuffer->width : 640.0f, k, s;
	port_wide2d_map(W, viewportH(), &k, &s);
	*m2 = g_metric[2] / k;
	*m0 = g_metric[0] - *m2 * (0.5f * (1.0f - k) + s);
#else
	*m0 = g_metric[0];
	*m2 = g_metric[2];
#endif
}
// Metric y origin and height: the game's, or with the qol HUD scale (port_wide2d_map_y: pixel = H (v ky + cy)) the metric that puts
// the element there: t ky + cu with cu = 1 - ky - cy, i.e. m3' = m3 / ky, m1' = m1 - cu m3'.
#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_map_y(float H, float *ky, float *cy);
#endif
static void metricY(float *m1, float *m3)
{
	*m1 = g_metric[1];
	*m3 = g_metric[3];
#ifdef ZOOSTERS_WIDESCREEN
	float ky, cy;
	port_wide2d_map_y(viewportH(), &ky, &cy);
	if (ky != 1.0f || cy != 0.0f) {
		float cu = 1.0f - ky - cy;
		*m3 = g_metric[3] / ky;
		*m1 = g_metric[1] - cu * *m3;
	}
#endif
}
static float pixelStepY(void)
{
	const Ctm &m = g_ctm[g_sp];
	float det = m.a * m.d - m.b * m.c;
	if (det == 0.0f) return 0.0f;
	float py = g_metric[3] / viewportH();
	// inverse of the linear part applied to (0, py)
	float sx = -m.c * py / det, sy = m.a * py / det;
	return sqrtf(sx * sx + sy * sy);
}
extern "C" float Rt2dFontGetHeight(void *p) { g_calls[C_FHEIGHT]++; Font *f = (Font *)p; return f ? f->height * pixelStepY() : 0.0f; }
// Rt2dFontGetStringWidth: sum of (glyph width + intergap) * height; glyph widths are fractions of the font height
static float stringWidth(const Font *f, const char *s, float h)
{
	float w = 0;
	for (const unsigned char *c = (const unsigned char *)s; c && *c; c++) {
		unsigned g = f->map[*c];
		if (g < (unsigned)f->nglyphs) w += f->glyphs[g].w;
	}
	return w * h;
}
extern "C" float Rt2dFontGetStringWidth(void *p, const char *s, float h) { g_calls[C_FWIDTH]++; Font *f = (Font *)p; return f ? stringWidth(f, s, h) : 0.0f; }

static void toScreen(float x, float y, float *sx, float *sy)
{
	const Ctm &m = g_ctm[g_sp];
	float dx = m.a * x + m.c * y + m.e, dy = m.b * x + m.d * y + m.f;
	float W = g_cam && g_cam->frameBuffer ? (float)g_cam->frameBuffer->width : 640.0f;
	float H = viewportH();
	float m0, m2, m1, m3;
	metricX(&m0, &m2);
	metricY(&m1, &m3);
	*sx = (dx - m0) / m2 * W;
	*sy = (1.0f - (dy - m1) / m3) * H;           // Rt2d y up, screen y down
}
static rw::uint8 clampByte(float v) { return v <= 0.0f ? 0 : v >= 255.0f ? 255 : (rw::uint8)(int)v; }
// rt2dFontShowBitmap: CTM pushed, translated to the anchor, scaled by the height, translated by (0, -baseline). A glyph is the quad
// x..x+width, y..y+1 with uv0 = (u0, v1) at the bottom left, coloured by the brush gradient along the string. The anchor's x advances
// by the string width.
#ifdef ZOOSTERS_FIX_BUGS
// Fixes tier: many glyph rectangles in the game's font pages are a column too wide and hold a strip of the next letter (tsa16: 34 of
// 138 glyphs), drawn as black marks at the letter's edge. Once per page, at the font's first draw: each opaque ink pixel is assigned
// to the glyph with the nearest fill (breadth-first inside its own rectangle); an ink pixel inside glyph g's rectangle that belongs to
// another glyph o and lies outside o's rectangle is made transparent. A glyph's own rectangle is never touched.
static void cleanFontPage(Font *f)
{
	rw::Raster *r = f->tex ? f->tex->raster : 0;
	if (!r || f->nglyphs <= 0)
		return;
	static rw::Raster *done[32];
	static int ndone;
	for (int i = 0; i < ndone; i++)
		if (done[i] == r) return;
	if (ndone < 32) done[ndone++] = r;
	rw::Image *img = r->toImage();
	if (!img)
		return;
	img->unpalettize();
	if (img->depth != 32) img->convertTo32();
	const int W = img->width, H = img->height;
	auto px = [&](int x, int y) { return img->pixels + y * img->stride + x * 4; };
	std::vector<int> owner((size_t)W * H, -1), queue;
	std::vector<int> rx0(f->nglyphs), ry0(f->nglyphs), rx1(f->nglyphs), ry1(f->nglyphs);
	for (int g = 0; g < f->nglyphs; g++) {
		const Glyph &gl = f->glyphs[g];
		rx0[g] = (int)lroundf((gl.u0 < gl.u1 ? gl.u0 : gl.u1) * W); rx1[g] = (int)lroundf((gl.u0 < gl.u1 ? gl.u1 : gl.u0) * W);
		ry0[g] = (int)lroundf((gl.v0 < gl.v1 ? gl.v0 : gl.v1) * H); ry1[g] = (int)lroundf((gl.v0 < gl.v1 ? gl.v1 : gl.v0) * H);
		for (int y = ry0[g] < 0 ? 0 : ry0[g]; y < ry1[g] && y < H; y++)
			for (int x = rx0[g] + 2 < 0 ? 0 : rx0[g] + 2; x < rx1[g] - 2 && x < W; x++) {     // the rectangle's inner part: the glyph's own fill
				const unsigned char *p = px(x, y);
				int i = y * W + x;
				if (p[3] >= 128 && p[0] + p[1] + p[2] >= 3 * 128 && owner[i] < 0) { owner[i] = g; queue.push_back(i); }
			}
	}
	for (size_t q = 0; q < queue.size(); q++) {          // breadth-first over the ink: each ink pixel to the nearest fill's glyph
		int i = queue[q], x = i % W, y = i / W;
		const int nx[4] = { x + 1, x - 1, x, x }, ny[4] = { y, y, y + 1, y - 1 };
		for (int k = 0; k < 4; k++) {
			if (nx[k] < 0 || ny[k] < 0 || nx[k] >= W || ny[k] >= H) continue;
			int j = ny[k] * W + nx[k];
			if (owner[j] < 0 && px(nx[k], ny[k])[3] >= 128) { owner[j] = owner[i]; queue.push_back(j); }
		}
	}
	int cleared = 0, glyphs = 0;
	for (int g = 0; g < f->nglyphs; g++) {
		int before = cleared;
		for (int y = ry0[g] < 0 ? 0 : ry0[g]; y < ry1[g] && y < H; y++)
			for (int x = rx0[g] < 0 ? 0 : rx0[g]; x < rx1[g] && x < W; x++) {
				int o = owner[y * W + x];
				if (o < 0 || o == g) continue;
				if (x >= rx0[o] && x < rx1[o] && y >= ry0[o] && y < ry1[o]) continue;   // also inside its owner's rectangle: needed there
				unsigned char *p = px(x, y);
				p[0] = p[1] = p[2] = p[3] = 0;
				cleared++;
			}
		glyphs += cleared > before;
	}
	if (cleared) {
		rw::int32 w, h, d, fmt;
		rw::Raster::imageFindRasterFormat(img, rw::Raster::TEXTURE, &w, &h, &d, &fmt);
		rw::Raster *nr = rw::Raster::create(w, h, d, fmt);
		if (nr) {
			nr->setFromImage(img);
			r->destroy();
			f->tex->raster = nr;
			done[ndone - 1] = nr;
		}
	}
	img->destroy();
	TLOG("rt2d: font page %s: %d stray pixels of neighbouring letters cleared in %d glyph rectangles\n", f->texname, cleared, glyphs);
}
#endif
extern "C" void *Rt2dFontShow(void *p, const char *s, float h, float *anchor, void *brush)
{
	g_calls[C_FSHOW]++;
	Font *f = (Font *)p;
	if (!f || !s || !f->nglyphs || !g_cam || !anchor) return p;
#ifdef ZOOSTERS_FIX_BUGS
	cleanFontPage(f);
#endif
	{ static int n; if (n++ < 12) TLOG("rt2d: Rt2dFontShow(\"%s\", h %g, anchor %g %g)\n", s, h, anchor[0], anchor[1]); }
	Brush *br = (Brush *)brush;
	Brush white; for (int i = 0; i < 4; i++) { white.bottom[i] = white.top[i] = 255.0f; white.dbottom[i] = white.dtop[i] = 0.0f; }
	if (!br) br = &white;
	float total = stringWidth(f, s, 1.0f);
	float oobase = total != 0.0f ? 1.0f / total : 0.0f;
	int n = (int)strlen(s);
	rw::RWDEVICE::Im2DVertex *v = (rw::RWDEVICE::Im2DVertex *)malloc(sizeof(*v) * 6 * (n ? n : 1));
	int nv = 0;
	float x = 0, baseu = 0;
	float z = rw::im2d::GetNearZ(), rz = g_cam->nearPlane;
	for (const unsigned char *c = (const unsigned char *)s; *c; c++) {
		unsigned g = f->map[*c];
		if (g >= (unsigned)f->nglyphs) continue;
		const Glyph &gl = f->glyphs[g];
		float w = gl.w;
		float gx[4] = { x, x + w, x, x + w }, gy[4] = { 1, 1, 0, 0 };      // top-left, top-right, bottom-left, bottom-right
		float t[4] = { baseu * oobase, (baseu + w) * oobase, baseu * oobase, (baseu + w) * oobase };
		float u0 = gl.u0, v0 = gl.v0, u1 = gl.u1, v1 = gl.v1;
#ifdef ZOOSTERS_FIX_BUGS
// Fixes tier: inset the glyph's texture rectangle by half a texel. Nearest sampling plus the D3D pixel convention put edge samples on the
// border, showing a texel of the neighbouring glyph (white dashes under "y"); a full pixel row on large displays.
		if (f->tex && f->tex->raster && f->tex->raster->width > 0 && f->tex->raster->height > 0) {
			float iu = 0.5f / f->tex->raster->width, iv = 0.5f / f->tex->raster->height;
			u0 += u1 >= u0 ? iu : -iu; u1 -= u1 >= u0 ? iu : -iu;
			v0 += v1 >= v0 ? iv : -iv; v1 -= v1 >= v0 ? iv : -iv;
		}
#endif
		float uv[4][2] = { { u0, v0 }, { u1, v0 }, { u0, v1 }, { u1, v1 } };
		static const int tri[6] = { 0, 1, 2, 2, 1, 3 };
		for (int k = 0; k < 6; k++) {
			int q = tri[k];
			float sx, sy; toScreen(anchor[0] + gx[q] * h, anchor[1] + (gy[q] - f->baseline) * h, &sx, &sy);
			const float *base = gy[q] ? br->top : br->bottom, *dl = gy[q] ? br->dtop : br->dbottom;
			float cc[4]; for (int i = 0; i < 4; i++) cc[i] = base[i] + dl[i] * t[q];
			rw::RWDEVICE::Im2DVertex &o = v[nv++];
			o.setScreenX(sx); o.setScreenY(sy); o.setScreenZ(z); o.setCameraZ(rz); o.setRecipCameraZ(1.0f / rz);
			o.setColor(clampByte(cc[0]), clampByte(cc[1]), clampByte(cc[2]), clampByte(cc[3])); o.setU(uv[q][0], 1.0f / rz); o.setV(uv[q][1], 1.0f / rz);
		}
		x += w;
		baseu += w;
	}
	if (nv) {
// Like rt2dFontShowBitmap, on a texture change set TEXTUREFILTER = filterAddressing & 0xff and TEXTURERASTER = raster;
// the fonts' filter is NEAREST.
		if (f->tex) rw::SetRenderState(rw::TEXTUREFILTER, f->tex->filterAddressing & 0xff);
		rw::SetRenderStatePtr(rw::TEXTURERASTER, f->tex ? f->tex->raster : 0);
		rw::SetRenderState(rw::VERTEXALPHA, 1);
		rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
		rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
		rw::SetRenderState(rw::ZTESTENABLE, 0);
		rw::SetRenderState(rw::ZWRITEENABLE, 0);
		rw::uint32 fog = rw::GetRenderState(rw::FOGENABLE);
		rw::SetRenderState(rw::FOGENABLE, 0);                   // 2D text is not fogged (the scene's fog state is still set)
		void *clip = port_im2d_clip_off();
		if (!PORT_DBG_HIDDEN(PORT_DBG_IM2D)) rw::im2d::RenderPrimitive(rw::PRIMTYPETRILIST, v, nv);
		port_im2d_clip_restore(clip);
		rw::SetRenderState(rw::FOGENABLE, fog);
		port_draw_im2d++;
	}
	free(v);
	anchor[0] += x * h;
	return p;
}
