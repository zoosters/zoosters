// Rendering entry points over librw: render states (RW 3.6 ids -> librw, RwGlobals device pointers), RwIm2D / RwIm3D, the game's
// direct RwD3D8* calls (no-ops headless) and D3DXCreateTextureFromFileA, an IDirect3DTexture8 facade over a librw image
// (Hud2D::LoadTexture uses GetLevelDesc / LockRect / UnlockRect / Release).
#include <stdio.h>
#include "../debugui/debugui.h"   // PORT_DBG_HIDDEN (0 unless ZOOSTERS_DEBUGUI)
#include <stdlib.h>
#include <string.h>
#ifdef RW_D3D9
#include <windows.h>
#include <d3d9.h>          // before rw.h: its d3d/rwd3d.h then declares the Direct3D side (EngineOpenParams, d3d::d3ddevice)
#endif
#include <malloc.h>
#include <rw.h>
#include "rwlayouts.h"
#include "rw/d3dtexture8.h"   // the game's interface class (src/game/include/rw): PortTexture8 is one
extern "C" int port_frames_presented(void);   // rwcore.cpp (all platforms)
#define PORT_HEAPCHK(where) do { static int bad; static const bool on = getenv("PORT_HEAPCHK") != 0; if (on && !bad && _heapchk() != _HEAPOK) { bad = 1; if (port_log) { fprintf(port_log, "HEAP CHECK FAILED in %s\n", where); fflush(port_log); } } } while (0)

extern "C" FILE *port_log;
#define RLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)

// ---------------------------------------------------------------- render states
enum {   // RenderWare 3.6 RwRenderState (rwplcore.h)
	rwRS_TEXTURERASTER = 1, rwRS_TEXTUREADDRESS, rwRS_TEXTUREADDRESSU, rwRS_TEXTUREADDRESSV, rwRS_TEXTUREPERSPECTIVE, rwRS_ZTESTENABLE,
	rwRS_SHADEMODE, rwRS_ZWRITEENABLE, rwRS_TEXTUREFILTER, rwRS_SRCBLEND, rwRS_DESTBLEND, rwRS_VERTEXALPHAENABLE, rwRS_BORDERCOLOR,
	rwRS_FOGENABLE, rwRS_FOGCOLOR, rwRS_FOGTYPE, rwRS_FOGDENSITY, rwRS_CULLMODE = 20 /* 18, 19: FOGTABLE, ALPHAPRIMITIVEBUFFER (rwplcore.h) */, rwRS_STENCILENABLE, rwRS_STENCILFAIL, rwRS_STENCILZFAIL,
	rwRS_STENCILPASS, rwRS_STENCILFUNCTION, rwRS_STENCILFUNCTIONREF, rwRS_STENCILFUNCTIONMASK, rwRS_STENCILFUNCTIONWRITEMASK,
	rwRS_ALPHATESTFUNCTION, rwRS_ALPHATESTFUNCTIONREF
};
static int mapState(int state)
{
	switch (state) {
	case rwRS_TEXTURERASTER: return rw::TEXTURERASTER;
	case rwRS_TEXTUREADDRESS: return rw::TEXTUREADDRESS;
	case rwRS_TEXTUREADDRESSU: return rw::TEXTUREADDRESSU;
	case rwRS_TEXTUREADDRESSV: return rw::TEXTUREADDRESSV;
	case rwRS_ZTESTENABLE: return rw::ZTESTENABLE;
	case rwRS_ZWRITEENABLE: return rw::ZWRITEENABLE;
	case rwRS_TEXTUREFILTER: return rw::TEXTUREFILTER;
	case rwRS_SRCBLEND: return rw::SRCBLEND;
	case rwRS_DESTBLEND: return rw::DESTBLEND;
	case rwRS_VERTEXALPHAENABLE: return rw::VERTEXALPHA;
	case rwRS_FOGENABLE: return rw::FOGENABLE;
	case rwRS_FOGCOLOR: return rw::FOGCOLOR;
	case rwRS_CULLMODE: return rw::CULLMODE;
	case rwRS_STENCILENABLE: return rw::STENCILENABLE;
	case rwRS_STENCILFAIL: return rw::STENCILFAIL;
	case rwRS_STENCILZFAIL: return rw::STENCILZFAIL;
	case rwRS_STENCILPASS: return rw::STENCILPASS;
	case rwRS_STENCILFUNCTION: return rw::STENCILFUNCTION;
	case rwRS_STENCILFUNCTIONREF: return rw::STENCILFUNCTIONREF;
	case rwRS_STENCILFUNCTIONMASK: return rw::STENCILFUNCTIONMASK;
	case rwRS_STENCILFUNCTIONWRITEMASK: return rw::STENCILFUNCTIONWRITEMASK;
	case rwRS_ALPHATESTFUNCTION: return rw::ALPHATESTFUNC;
	case rwRS_ALPHATESTFUNCTIONREF: return rw::ALPHATESTREF;
	default: return -1;      // TEXTUREPERSPECTIVE, SHADEMODE, BORDERCOLOR, FOGTYPE, FOGDENSITY: no librw equivalent
	}
}
static unsigned mapAlphaFunc(unsigned f)
{
	switch (f) {      // rwALPHATESTFUNCTIONNEVER=1 LESS EQUAL LESSEQUAL GREATER NOTEQUAL GREATEREQUAL ALWAYS
	case 2: case 4: return rw::ALPHALESS;
	case 5: case 7: return rw::ALPHAGREATEREQUAL;
	default: return rw::ALPHAALWAYS;
	}
}
extern "C" int port_trace_on(void);
extern "C" int port_d3dcap_ndraw(void);
#ifndef RW_D3D9
extern "C" int port_d3dcap_ndraw(void) { return -1; }   // headless build: no draw-call capture (d3dcap.cpp is d3d9-only)
#endif
#if defined(RW_D3D9) || defined(RW_GL3)
// D3D8 fog range: RwRenderStateSet(FOGTYPE, linear) sets FOGSTART = fogPlane, FOGEND = farPlane, then the game overrides them via
// RwD3D8SetRenderState(0x24 / 0x25) (pixel fog, FUN_0041aca0). The state survives camera updates in D3D8; librw's BeginUpdate reloads
// fogPlane / farPlane into the shader constants, so the stored range is applied again after every BeginUpdate (port_fog_after_begin,
// camBeginInner) and when it changes.
static bool g_fogGot; static float g_fogS, g_fogE;
static void fogApply()
{
	if (!g_fogGot) return;
	float s = g_fogS, e = g_fogE;
#ifdef RW_D3D9
	rw::d3d::d3dShaderState.fogData.start = s; rw::d3d::d3dShaderState.fogData.end = e;
	rw::d3d::d3dShaderState.fogData.range = s != e ? 1.0f / (s - e) : 0.0f;
	rw::d3d::d3dShaderState.fogDirty = true;
#else
	rw::gl3::setFogRange(s, e);     // librw patch 0018 (gl3's BeginUpdate loads fogPlane / farPlane the same way)
#endif
}
extern "C" void port_fog_after_begin(void) { fogApply(); }
extern "C" int port_frames_presented(void);
static void fogSet(float s, float e) { static int n; if ((s != g_fogS || e != g_fogE) && (n++ < 40 || port_trace_on())) RLOG("rwrender: fog range %g .. %g (frame %d)\n", s, e, port_frames_presented()); g_fogGot = true; g_fogS = s; g_fogE = e; fogApply(); }
#else
extern "C" void port_fog_after_begin(void) {}
#endif
extern "C" int port_RenderStateSet(int state, void *value)
{ PORT_HEAPCHK("port_RenderStateSet");
#if defined(RW_D3D9) || defined(RW_GL3)
	if (state == rwRS_FOGTYPE && (unsigned)(uintptr_t)value == 1 && rw::engine->currentCamera) fogSet(rw::engine->currentCamera->fogPlane, rw::engine->currentCamera->farPlane);
#endif
	int ls = mapState(state);
	if (ls < 0) return 1;
	if ((state == rwRS_CULLMODE || state == rwRS_ZTESTENABLE || state == rwRS_ZWRITEENABLE) && port_trace_on()) {   // PORT_TRACEFRAME: who changes cull / z, before which draw
		RLOG("trace: RenderStateSet %d = %u caller %p before draw #%d\n", state, (unsigned)(uintptr_t)value, PORT_RETURN_ADDRESS(), port_d3dcap_ndraw());
	}
	if (ls == rw::TEXTURERASTER) rw::SetRenderStatePtr(ls, value);
#ifdef RW_D3D9
	else if (ls == rw::ALPHATESTFUNC) rw::d3d::setAlphaTestFuncRW((unsigned)(uintptr_t)value);      // RW's eight functions (librw patch 0006)
#elif defined(RW_GL3)
	else if (ls == rw::ALPHATESTFUNC) rw::gl3::setAlphaTestFuncRW((unsigned)(uintptr_t)value);      // RW's eight functions (librw patch 0018)
#endif
	else if (ls == rw::ALPHATESTFUNC) rw::SetRenderState(ls, mapAlphaFunc((unsigned)(uintptr_t)value));
	else if (ls == rw::FOGCOLOR) {     // RW packs RWRGBALONG = a<<24 | r<<16 | g<<8 | b; librw wants r | g<<8 | b<<16 | a<<24
		unsigned v = (unsigned)(uintptr_t)value;
		rw::SetRenderState(ls, ((v >> 16) & 0xff) | (v & 0xff00) | ((v & 0xff) << 16) | (v & 0xff000000));
		{ static int n; static unsigned last; if (v != last && n++ < 50) RLOG("rwrender: fog colour %08x (RW ARGB, frame %d)\n", v, port_frames_presented()); last = v; }
	}
	else rw::SetRenderState(ls, (rw::uint32)(uintptr_t)value);
	return 1;
}
extern "C" int port_RenderStateGet(int state, void *value)
{ PORT_HEAPCHK("port_RenderStateGet");
	int ls = mapState(state);
	if (ls < 0) { *(unsigned *)value = 0; return 1; }
	if (ls == rw::TEXTURERASTER) *(void **)value = rw::GetRenderStatePtr(ls);
#ifdef RW_D3D9
	else if (ls == rw::ALPHATESTFUNC) *(unsigned *)value = rw::d3d::getAlphaTestFuncRW();
#elif defined(RW_GL3)
	else if (ls == rw::ALPHATESTFUNC) *(unsigned *)value = rw::gl3::getAlphaTestFuncRW();
#endif
	else *(unsigned *)value = rw::GetRenderState(ls);
	return 1;
}
// The game sets a global material callback (FUN_004c80d0; TFBAtomic::RenderCallback, TFBSector_Render install 0x45d930 =
// TFBRenderState::Apply) that RW calls with each mesh's material before drawing it. librw runs rw::d3d::meshMaterialHook the same way
// (patch 0006).
static void *(*g_materialCB)(void *);
#if defined(RW_D3D9) || defined(RW_GL3)
static void materialHook(rw::Material *m)
{
	rw::RGBA before = m->color;
	if (g_materialCB) g_materialCB(m);
	static const char *tm = getenv("PORT_TRACEMAT") ? getenv("PORT_TRACEMAT") : "water";
	if (port_trace_on() && m->texture && strstr(m->texture->name, tm))   // PORT_TRACEFRAME: the game's per-mesh callback on PORT_TRACEMAT (default water) materials
		RLOG("trace: material %p %s colour %02x%02x%02x%02x -> %02x%02x%02x%02x after the game's material callback\n", (void *)m, m->texture->name,
		        before.red, before.green, before.blue, before.alpha, m->color.red, m->color.green, m->color.blue, m->color.alpha);
}
#endif
extern "C" void FUN_004c80d0(void *cb)
{
	g_materialCB = (void *(*)(void *))cb;
#ifdef RW_D3D9
	rw::d3d::meshMaterialHook = g_materialCB ? materialHook : 0;
#elif defined(RW_GL3)
	rw::gl3::meshMaterialHook = g_materialCB ? materialHook : 0;     // librw patch 0018
#endif
}
extern "C" int RwRenderStateSet(int state, void *value) { PORT_HEAPCHK("RwRenderStateSet"); return port_RenderStateSet(state, value); }
extern "C" int RwRenderStateGet(int state, void *value) { PORT_HEAPCHK("RwRenderStateGet"); return port_RenderStateGet(state, value); }

// ---------------------------------------------------------------- immediate mode (RW's d3d8 vertex layouts == librw's d3d ones)
extern "C" { int port_draw_im2d, port_draw_im3d, port_draw_d3d, port_draw_world, port_draw_atomic; }   // per presented frame (rwcore.cpp prints and clears them)
extern "C" int port_trace_on(void);
extern "C" int port_frames_presented(void);
extern "C" int port_diag_raster(rw::Raster *r, char *buf, int n);
// rxD3D8Im2DRenderFlush turns D3DRS_CLIPPING off for every Im2D draw. The HUD quads (Hud2DObject::DrawQuad) carry z = 20.2 (camera
// space, not 0..1), so with clipping on D3D9 clips them away (the mouse pointer was never visible). RW restores nothing; the port
// restores the previous state after the draw.
struct Im2DClipOff {
#ifdef RW_D3D9
	unsigned long old; int on;
	Im2DClipOff() : old(1), on(0) { IDirect3DDevice9 *d = rw::d3d::d3ddevice; if (d) { d->GetRenderState(D3DRS_CLIPPING, &old); d->SetRenderState(D3DRS_CLIPPING, 0); on = 1; } }
	~Im2DClipOff() { if (on) rw::d3d::d3ddevice->SetRenderState(D3DRS_CLIPPING, old); }
#endif
};
// D3D8 lets pre-transformed vertices with z outside 0..1 through once clipping is off; librw's d3d9 Im2D shader outputs z * w with w =
// rhw, so the hardware clips them anyway. Clamp z to 0..1 for the HUD quads (depth test is off for them; D3D8 clamps the depth the
// same way).
struct Im2DZClamp {
	unsigned char *buf;
	Im2DZClamp(void *verts, int n) : buf(0)
	{
#ifdef RW_GL3
		// gl3: the game's vertices are RenderWare's D3D8 layout (x y z rhw, colour D3DCOLOR = bytes B G R A, u v; z in D3D's 0..1, which
		// RwIm2DGetNear/FarScreenZ report below); librw's gl3 Im2DVertex is x y z w(= camera z), bytes R G B A, u v with z in GL's -1..1
		buf = (unsigned char *)malloc((size_t)n * 28);
		memcpy(buf, verts, (size_t)n * 28);
		for (int i = 0; i < n; i++) {
			float *v = (float *)(buf + i * 28);
			float z = v[2] < 0.0f ? 0.0f : v[2] > 1.0f ? 1.0f : v[2];
			v[2] = 2.0f * z - 1.0f;
			v[3] = v[3] > 0.0f ? 1.0f / v[3] : 1.0f;
			unsigned char *c = buf + i * 28 + 16, t = c[0]; c[0] = c[2]; c[2] = t;
		}
		return;
#endif
		const float *f = (const float *)verts;
		int bad = 0;
		for (int i = 0; i < n && !bad; i++) { float z = f[i * 7 + 2]; if (z < 0.0f || z > 1.0f) bad = 1; }
		if (!bad) return;
		buf = (unsigned char *)malloc((size_t)n * 28);
		memcpy(buf, verts, (size_t)n * 28);
		for (int i = 0; i < n; i++) { float *z = (float *)buf + i * 7 + 2; if (*z < 0.0f) *z = 0.0f; else if (*z > 1.0f) *z = 1.0f; }
	}
	~Im2DZClamp() { free(buf); }
	void *get(void *orig) const { return buf ? buf : orig; }
};
extern "C" void *port_im2d_clip_off(void) { return new Im2DClipOff; }
extern "C" void port_im2d_clip_restore(void *p) { delete (Im2DClipOff *)p; }
extern "C" int RwIm2DRenderPrimitive(int prim, void *verts, int n) { PORT_HEAPCHK("RwIm2DRenderPrimitive"); port_draw_im2d++; if (port_trace_on()) { const float *f = (const float *)verts; RLOG("trace: Im2D prim %d n %d from %p: v1 %g %g v2 %g %g v3 %g %g v0 %g %g z %g rhw %g col %08x uv %g %g; texraster %p zwrite %u ztest %u\n", prim, n, PORT_RETURN_ADDRESS(), f[7], f[8], f[14], f[15], f[21], f[22], f[0], f[1], f[2], f[3], ((const unsigned *)verts)[4], f[5], f[6], rw::GetRenderStatePtr(rw::TEXTURERASTER), rw::GetRenderState(rw::ZWRITEENABLE), rw::GetRenderState(rw::ZTESTENABLE)); { char rd[200]; port_diag_raster((rw::Raster *)rw::GetRenderStatePtr(rw::TEXTURERASTER), rd, sizeof rd); RLOG("trace:   raster %s\n", rd); } } { Im2DClipOff co; Im2DZClamp zc(verts, n); if (!PORT_DBG_HIDDEN(PORT_DBG_IM2D)) rw::im2d::RenderPrimitive((rw::PrimitiveType)prim, zc.get(verts), n); } return 1; }
extern "C" int RwIm2DRenderIndexedPrimitive(int prim, void *verts, int n, void *idx, int ni) { PORT_HEAPCHK("RwIm2DRenderIndexedPrimitive"); port_draw_im2d++; { Im2DClipOff co; Im2DZClamp zc(verts, n); if (!PORT_DBG_HIDDEN(PORT_DBG_IM2D)) rw::im2d::RenderIndexedPrimitive((rw::PrimitiveType)prim, zc.get(verts), n, idx, ni); } return 1; }
extern "C" int RwIm2DRenderLine(void *verts, int n, int a, int b) { PORT_HEAPCHK("RwIm2DRenderLine"); { Im2DClipOff co; Im2DZClamp zc(verts, n); rw::im2d::RenderLine(zc.get(verts), n, a, b); } return 1; }
extern "C" int RwIm2DRenderTriangle(void *verts, int n, int a, int b, int c) { PORT_HEAPCHK("RwIm2DRenderTriangle"); { Im2DClipOff co; Im2DZClamp zc(verts, n); rw::im2d::RenderTriangle(zc.get(verts), n, a, b, c); } return 1; }
#ifdef RW_GL3   // the game's vertices are in D3D8's screen z range (Im2DZClamp converts)
extern "C" float RwIm2DGetNearScreenZ(void) { PORT_HEAPCHK("RwIm2DGetNearScreenZ"); return 0.0f; }
extern "C" float RwIm2DGetFarScreenZ(void) { PORT_HEAPCHK("RwIm2DGetFarScreenZ"); return 1.0f; }
#else
extern "C" float RwIm2DGetNearScreenZ(void) { PORT_HEAPCHK("RwIm2DGetNearScreenZ"); return rw::im2d::GetNearZ(); }
extern "C" float RwIm2DGetFarScreenZ(void) { PORT_HEAPCHK("RwIm2DGetFarScreenZ"); return rw::im2d::GetFarZ(); }
#endif
// RwIm3DTransform (0x4e0940): NULL when n > 0x10000, flags |= VERTEXXYZ | VERTEXRGBA, returns pVerts. RW's D3D8 submit node sets
// vertex alpha (off for rwIM3D_ALLOPAQUE) and D3DRS_CLIPPING (off for rwIM3D_NOCLIP) per call; librw ignores both, so they are applied
// here. Deviation: librw drops the bound texture when VERTEXUV is unset.
extern "C" void *RwIm3DTransform(void *verts, unsigned n, rw::Matrix *ltm, unsigned flags)
{ PORT_HEAPCHK("RwIm3DTransform");
	if (n > 0x10000) return 0;
	flags |= 0x18;
	rw::SetRenderState(rw::VERTEXALPHA, (flags & 2) ? 0 : 1);
#ifdef RW_D3D9
	if (rw::d3d::d3ddevice) rw::d3d::d3ddevice->SetRenderState(D3DRS_CLIPPING, (flags & 4) ? 0 : 1);
#endif
	rw::im3d::Transform(verts, (rw::int32)n, ltm, flags);
	return verts;
}
extern "C" int RwIm3DEnd(void) { PORT_HEAPCHK("RwIm3DEnd"); rw::im3d::End(); return 1; }
extern "C" int RwIm3DRenderPrimitive(int prim) { PORT_HEAPCHK("RwIm3DRenderPrimitive"); port_draw_im3d++; if (!PORT_DBG_HIDDEN(PORT_DBG_IM3D)) rw::im3d::RenderPrimitive((rw::PrimitiveType)prim); return 1; }
extern "C" int RwIm3DRenderIndexedPrimitive(int prim, void *idx, int n) { PORT_HEAPCHK("RwIm3DRenderIndexedPrimitive"); port_draw_im3d++; if (!PORT_DBG_HIDDEN(PORT_DBG_IM3D)) rw::im3d::RenderIndexedPrimitive((rw::PrimitiveType)prim, idx, n); return 1; }
extern "C" int RwIm3DRenderLine(int a, int b) { PORT_HEAPCHK("RwIm3DRenderLine"); rw::im3d::RenderLine(a, b); return 1; }
extern "C" int RwIm3DRenderTriangle(int a, int b, int c) { PORT_HEAPCHK("RwIm3DRenderTriangle"); rw::im3d::RenderTriangle(a, b, c); return 1; }

// ---------------------------------------------------------------- RwD3D8*: the game's direct Direct3D 8 calls
#ifdef RW_D3D9
static IDirect3DDevice9 *dev(void) { return rw::d3d::d3ddevice; }
// d3dcap (port/shim/d3dcap.cpp): the device, and the name of the RW texture behind a d3d9 texture pointer
extern "C" void *port_d3d_device(void) { return rw::d3d::d3ddevice; }
extern "C++" void port_for_each_texture(void (*cb)(rw::Texture *, void *), void *ud);
struct D3dCapFind { void *d3d; const char *name; };
static void d3dcapFindCb(rw::Texture *t, void *ud)
{
	D3dCapFind *f = (D3dCapFind *)ud;
	if (f->name || !t || !t->raster || !rw::d3d::nativeRasterOffset) return;
	if (t->raster->platform != rw::platform) return;
	if (PLUGINOFFSET(rw::d3d::D3dRaster, t->raster, rw::d3d::nativeRasterOffset)->texture == f->d3d) f->name = t->name;
}
extern "C" const char *port_d3dcap_texname(void *d3dtex, char *buf, int n)
{
	D3dCapFind f = { d3dtex, 0 };
	port_for_each_texture(d3dcapFindCb, &f);
	if (!f.name) return 0;
	_snprintf(buf, n, "%s", f.name); buf[n - 1] = 0;
	return buf;
}
// Frame capture: render target -> system memory (GetRenderTargetData) -> PNG via rw::Image (librw's Raster::toImage yields a 1-bit
// empty image for a d3d9 camera raster).
extern "C" int port_capture_backbuffer(const char *path)
{
	IDirect3DDevice9 *d = dev();
	if (!d) return 0;
	IDirect3DSurface9 *rt = 0, *off = 0;
	if (FAILED(d->GetRenderTarget(0, &rt)) || !rt) return 0;
	D3DSURFACE_DESC desc; rt->GetDesc(&desc);
	int ok = 0;
	if (SUCCEEDED(d->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &off, 0)) && off) {
		HRESULT hr = d->GetRenderTargetData(rt, off);
		D3DLOCKED_RECT lr;
		if (SUCCEEDED(hr) && SUCCEEDED(off->LockRect(&lr, 0, D3DLOCK_READONLY))) {
			rw::Image *img = rw::Image::create((rw::int32)desc.Width, (rw::int32)desc.Height, 32);
			img->allocate();
			int bpp = (desc.Format == D3DFMT_R5G6B5 || desc.Format == D3DFMT_X1R5G5B5 || desc.Format == D3DFMT_A1R5G5B5) ? 2 : 4;
			for (unsigned y = 0; y < desc.Height; y++) {
				const unsigned char *src = (const unsigned char *)lr.pBits + y * lr.Pitch;
				unsigned char *dst = img->pixels + y * img->stride;
				for (unsigned x = 0; x < desc.Width; x++, dst += 4) {
					if (bpp == 4) { dst[0] = src[x * 4 + 2]; dst[1] = src[x * 4 + 1]; dst[2] = src[x * 4]; dst[3] = 255; }
					else { unsigned v = src[x * 2] | (src[x * 2 + 1] << 8); dst[0] = (v >> 11) << 3; dst[1] = ((v >> 5) & 63) << 2; dst[2] = (v & 31) << 3; dst[3] = 255; }
				}
			}
			rw::writePNG(img, path);
			img->destroy();
			off->UnlockRect();
			ok = 1;
		} else if (port_log) fprintf(port_log, "rwrender: capture: GetRenderTargetData/LockRect failed (%08lx, format %d)\n", (unsigned long)hr, (int)desc.Format);
		off->Release();
	}
	rt->Release();
	return ok;
}
extern "C" void *RwD3D8GetCurrentD3DDevice(void) { PORT_HEAPCHK("RwD3D8GetCurrentD3DDevice"); return dev(); }
extern "C" int RwD3D8SetRenderState(int state, unsigned value)
{ PORT_HEAPCHK("RwD3D8SetRenderState");
	if (state == 47 /* D3DRS_ZBIAS (D3D8) */ || !dev()) return 1;
	if (state == 36 || state == 37) {   // D3DRS_FOGSTART / FOGEND: the shader path reads its own constants (see fogSet)
		float f = *(float *)&value, s = state == 36 ? f : g_fogS, e = state == 37 ? f : g_fogE;
		fogSet(s, e); return 1;
	}
	rw::d3d::setRenderState(state, value);          // through librw's state cache: a direct device call leaves it stale
	return 1;
}
extern "C" int RwD3D8GetRenderState(int state, unsigned *value) { PORT_HEAPCHK("RwD3D8GetRenderState"); DWORD v = 0; if (dev()) dev()->GetRenderState((D3DRENDERSTATETYPE)state, &v); *value = v; return 1; }
extern "C" int RwD3D8SetTextureStageState(int stage, int type, unsigned value)
{ PORT_HEAPCHK("RwD3D8SetTextureStageState");
	if (!dev()) return 1;
	{   // log each distinct (stage, type, value) the game sets once
		static unsigned seen[256]; static int ns;
		unsigned key = ((unsigned)stage << 28) | ((unsigned)type << 20) | (value & 0xfffff);
		int k = 0; while (k < ns && seen[k] != key) k++;
		if (k == ns && ns < 256) { seen[ns++] = key; RLOG("tss: stage %d type %d value %x (first use, frame %d)\n", stage, type, value, port_frames_presented()); }
	}
	int samp = 0;
	switch (type) {         // D3D8 texture stage states that became D3D9 sampler states
	case 13: samp = D3DSAMP_ADDRESSU; break; case 14: samp = D3DSAMP_ADDRESSV; break; case 15: samp = D3DSAMP_BORDERCOLOR; break;
	case 16: samp = D3DSAMP_MAGFILTER; break; case 17: samp = D3DSAMP_MINFILTER; break; case 18: samp = D3DSAMP_MIPFILTER; break;
	case 19: samp = D3DSAMP_MIPMAPLODBIAS; break; case 20: samp = D3DSAMP_MAXMIPLEVEL; break; case 21: samp = D3DSAMP_MAXANISOTROPY; break;
	case 25: samp = D3DSAMP_ADDRESSW; break;
	}
	if (samp) { rw::d3d::setSamplerState(stage, samp, value); return 1; }
	rw::d3d::setTextureStageState(stage, type, value);
	return 1;
}
extern "C" int RwD3D8SetTexture(rw::Texture *tex, int stage) { PORT_HEAPCHK("RwD3D8SetTexture"); rw::d3d::setTexture(stage, tex); return 1; }
// D3D8 "vertex shader" = an FVF code: clear librw's cached shader / declaration so its next draw sets its own (a direct device call
// would leave the caches stale)
extern "C" int RwD3D8SetVertexShader(unsigned fvf) { PORT_HEAPCHK("RwD3D8SetVertexShader"); if (!dev()) return 0; rw::d3d::setVertexShader(nil); rw::d3d::setVertexDeclaration(nil); return dev()->SetFVF(fvf) == D3D_OK; }
extern "C" int RwD3D8SetPixelShader(unsigned h) { PORT_HEAPCHK("RwD3D8SetPixelShader"); (void)h; if (!dev()) return 0; rw::d3d::setPixelShader(nil); return 1; }
// _RwHasStencilBuffer: the depth-stencil surface's format carries stencil bits (cached once the device exists)
extern "C" int port_has_stencil(void)
{
	static int has = -1;
	if (has < 0 && dev()) {
		IDirect3DSurface9 *ds = 0; D3DSURFACE_DESC dd;
		has = 0;
		if (dev()->GetDepthStencilSurface(&ds) == D3D_OK && ds) {
			if (ds->GetDesc(&dd) == D3D_OK) has = (dd.Format == D3DFMT_D24S8 || dd.Format == D3DFMT_D24X4S4 || dd.Format == D3DFMT_D15S1 || dd.Format == D3DFMT_D24FS8) ? 1 : 0;
			ds->Release();
		}
	}
	return has > 0;
}
// matrix == NULL means the identity matrix (the D3D device would reject a NULL pointer)
extern "C" int RwD3D8SetTransform(int state, const void *m)
{ PORT_HEAPCHK("RwD3D8SetTransform");
	static const float ident[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	if (!m) m = ident;
	return dev() && dev()->SetTransform((D3DTRANSFORMSTATETYPE)state, (const D3DMATRIX *)m) == D3D_OK;
}
extern "C" int RwD3D8SetStreamSource(unsigned n, IDirect3DVertexBuffer9 *vb, unsigned stride) { PORT_HEAPCHK("RwD3D8SetStreamSource"); if (!dev()) return 0; rw::d3d::setStreamSource(n, vb, 0, stride); return 1; }
static unsigned g_ibBase;      // RwD3D8SetIndices(ib, base): D3D8's BaseVertexIndex, applied by the next RwD3D8DrawIndexedPrimitive
extern "C" int RwD3D8SetIndices(IDirect3DIndexBuffer9 *ib, unsigned base) { PORT_HEAPCHK("RwD3D8SetIndices"); g_ibBase = base; if (!dev()) return 0; rw::d3d::setIndices(ib); return 1; }
// RwD3D8DrawPrimitive / DrawIndexedPrimitive take a vertex / index COUNT; primitive count by type: point n, line list n/2, line strip n-1,
// triangle list n/3, strip and fan n-2; any other type 0 (the draw fails)
static unsigned d3d8PrimCount(int type, unsigned n)
{
	switch (type) {
	case D3DPT_POINTLIST:     return n;
	case D3DPT_LINELIST:      return n / 2;
	case D3DPT_LINESTRIP:     return n - 1;
	case D3DPT_TRIANGLELIST:  return n / 3;
	case D3DPT_TRIANGLESTRIP:
	case D3DPT_TRIANGLEFAN:   return n - 2;
	}
	return 0;
}
extern "C" int RwD3D8DrawPrimitive(int type, int start, int n)
{ PORT_HEAPCHK("RwD3D8DrawPrimitive"); port_draw_d3d++; if (PORT_DBG_HIDDEN(PORT_DBG_D3D)) return 1; if (dev()) rw::d3d::flushCache();
	return dev() && dev()->DrawPrimitive((D3DPRIMITIVETYPE)type, start, d3d8PrimCount(type, (unsigned)n)) == D3D_OK;
}
extern "C" int RwD3D8DrawIndexedPrimitive(int type, int minIdx, int nVerts, int start, int numIndices)
{ PORT_HEAPCHK("RwD3D8DrawIndexedPrimitive"); port_draw_d3d++; if (PORT_DBG_HIDDEN(PORT_DBG_D3D)) return 1; if (dev()) rw::d3d::flushCache();
	return dev() && dev()->DrawIndexedPrimitive((D3DPRIMITIVETYPE)type, (INT)g_ibBase, minIdx, nVerts, start, d3d8PrimCount(type, (unsigned)numIndices)) == D3D_OK;
}
// The bbox ({sup, inf} pair, world space) is fully inside the frustum when, for each of the six planes, the corner furthest along the
// normal (bbox[1 - closest*]) is not beyond it. The caller (FUN_00444770) turns D3DRS_CLIPPING off for fully inside sectors.
extern "C" int RwD3D8CameraIsBBoxFullyInsideFrustum(const void *cam, const void *bbox)
{ PORT_HEAPCHK("RwD3D8CameraIsBBoxFullyInsideFrustum");
	const rw::Camera *c = (const rw::Camera *)cam; const rw::V3d *bb = (const rw::V3d *)bbox;
	if (!c || !bb) return 1;
	for (int i = 0; i < 6; i++) {
		const rw::FrustumPlane *p = &c->frustumPlanes[i];
		float d = bb[1 - p->closestX].x * p->plane.normal.x + bb[1 - p->closestY].y * p->plane.normal.y + bb[1 - p->closestZ].z * p->plane.normal.z;
		if (d > p->plane.distance) return 0;
	}
	return 1;
}
extern "C" int RwD3D8CreateVertexBuffer(unsigned stride, unsigned size, IDirect3DVertexBuffer9 **vb, unsigned *off)
{ PORT_HEAPCHK("RwD3D8CreateVertexBuffer");
	(void)stride; if (off) *off = 0; *vb = 0;
	return dev() && dev()->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, vb, 0) == D3D_OK;
}
extern "C" void RwD3D8DestroyVertexBuffer(unsigned a, unsigned b, IDirect3DVertexBuffer9 *vb, unsigned off) { PORT_HEAPCHK("RwD3D8DestroyVertexBuffer"); (void)a; (void)b; (void)off; if (vb) vb->Release(); }
extern "C" int RwD3D8IndexBufferCreate(unsigned n, IDirect3DIndexBuffer9 **ib)
{ PORT_HEAPCHK("RwD3D8IndexBufferCreate");
	*ib = 0;
	return dev() && dev()->CreateIndexBuffer(n * 2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, ib, 0) == D3D_OK;
}
extern "C" void RwD3D8SetVertexBufferToNull(void) { PORT_HEAPCHK("RwD3D8SetVertexBufferToNull"); if (dev()) rw::d3d::setStreamSource(0, 0, 0, 0); }
#else
#ifdef RW_GL3
// gl3: the D3D8 state the game sets directly is recorded for its one raw draw path, the shadow receiver pass (ShadowPipelineRender
// 0x444770: texture matrix, camera-space texcoords, TFACTOR colour, texture or complemented texture alpha * TFACTOR alpha). The pass's
// buffers are CPU entries (port_pipe_run_custom); a draw reproduces the fixed-function result through Im3D: uv = camera-space position
// * texture matrix (x, y), colour = TFACTOR, alpha complement via librw patch 0019, alpha test off.
struct PortCpuVB { unsigned magic; rw::Geometry *geo; };
struct PortCpuIB { rw::uint16 *idx; int n; };
enum { PORT_CPUVB_MAGIC = 0x43505642 };
static struct { float texMat[16]; unsigned tci, ttff, colorArg1, alphaArg1, tfactor; PortCpuVB *vb; PortCpuIB *ib; } g_d8 = { { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, 0, 0, 2, 2, 0xffffffffu, 0, 0 };
static void d8DrawReceiver(int type, int numIndices)
{
	PortCpuVB *vb = g_d8.vb; PortCpuIB *ib = g_d8.ib;
	rw::Camera *cam = rw::engine->currentCamera;
	if (!vb || vb->magic != PORT_CPUVB_MAGIC || !ib || !ib->idx || !cam || g_d8.tci != 0x20000 /* D3DTSS_TCI_CAMERASPACEPOSITION */) return;
	rw::Geometry *g = vb->geo;
	int nv = g->numVertices;
	const rw::V3d *pos = g->morphTargets[0].vertices;
	if (nv <= 0 || !pos) return;
	static rw::RWDEVICE::Im3DVertex *verts; static int cap;
	if (nv > cap) { free(verts); cap = nv; verts = (rw::RWDEVICE::Im3DVertex *)malloc(sizeof(*verts) * cap); }
	rw::Matrix inv;
	rw::Matrix::invert(&inv, cam->getFrame()->getLTM());
	const float *m = g_d8.texMat;
	unsigned tf = g_d8.tfactor;
	rw::uint8 a = (rw::uint8)(tf >> 24), r = 255, gr = 255, b = 255;
	if ((g_d8.colorArg1 & 0xf) == 3) { r = (rw::uint8)(tf >> 16); gr = (rw::uint8)(tf >> 8); b = (rw::uint8)tf; }   // D3DTA_TFACTOR
	for (int i = 0; i < nv; i++) {
		const rw::V3d &w = pos[i];   // sector atomics: world-space vertices
		// RW's D3D8 view (_rwD3D8CameraBeginUpdate): inverse camera LTM with the x column negated
		float cx = -(w.x * inv.right.x + w.y * inv.up.x + w.z * inv.at.x + inv.pos.x);
		float cy = w.x * inv.right.y + w.y * inv.up.y + w.z * inv.at.y + inv.pos.y;
		float cz = w.x * inv.right.z + w.y * inv.up.z + w.z * inv.at.z + inv.pos.z;
		verts[i].setX(w.x); verts[i].setY(w.y); verts[i].setZ(w.z);
		verts[i].setColor(r, gr, b, a);
		verts[i].setU(cx * m[0] + cy * m[4] + cz * m[8] + m[12]);
		verts[i].setV(cx * m[1] + cy * m[5] + cz * m[9] + m[13]);
	}
	{ static int dbg; if (dbg++ < 3) { rw::Raster *ras = (rw::Raster *)rw::GetRenderStatePtr(rw::TEXTURERASTER);
		RLOG("rwrender: gl3 receiver draw: raster %p %dx%d type %d, addr %u %u, tci %x ttff %u arg1 %x/%x tfactor %08x uv0 %g %g\n", (void *)ras, ras ? ras->width : 0, ras ? ras->height : 0, ras ? ras->type : -1,
		     rw::GetRenderState(rw::TEXTUREADDRESSU), rw::GetRenderState(rw::TEXTUREADDRESSV), g_d8.tci, g_d8.ttff, g_d8.colorArg1, g_d8.alphaArg1, g_d8.tfactor, verts[0].getU(), verts[0].getV()); } }
	{ static const char *dump = getenv("PORT_SHADOW_DUMP"); static int done;   // diagnostic: the shadow raster at that presented frame -> <dir>\shadow_raster.png
		rw::Raster *ras = (rw::Raster *)rw::GetRenderStatePtr(rw::TEXTURERASTER);
		if (dump && !done && ras && port_frames_presented() >= atoi(dump)) { done = 1; rw::Image *img = ras->toImage(); if (img) { rw::writePNG(img, "shadow_raster.png"); img->destroy(); RLOG("rwrender: shadow raster dumped\n"); } } }
	unsigned oldFunc = rw::gl3::getAlphaTestFuncRW();
	void *oldVA = 0; port_RenderStateGet(12, &oldVA);
	rw::gl3::setAlphaTestFuncRW(8);                          // D3DRS_ALPHATESTENABLE 0
	rw::SetRenderState(rw::VERTEXALPHA, 1);                  // the shadow raster has alpha: RW's D3D8 driver blends
	rw::gl3::setAlphaComplement((g_d8.alphaArg1 & 0x1f) == 0x12);   // D3DTA_TEXTURE | D3DTA_COMPLEMENT
	rw::im3d::Transform(verts, nv, nil, rw::im3d::VERTEXUV);
	rw::im3d::RenderIndexedPrimitive(type == 5 ? rw::PRIMTYPETRISTRIP : rw::PRIMTYPETRILIST, ib->idx, numIndices < ib->n ? numIndices : ib->n);
	rw::im3d::End();
	rw::gl3::setAlphaComplement(0);
	if (oldFunc) rw::gl3::setAlphaTestFuncRW(oldFunc);
	rw::SetRenderState(rw::VERTEXALPHA, (rw::uint32)(uintptr_t)oldVA);
}
#endif
extern "C" void *RwD3D8GetCurrentD3DDevice(void) { PORT_HEAPCHK("RwD3D8GetCurrentD3DDevice"); return 0; }
extern "C" int port_capture_backbuffer(const char *) { return 0; }
extern "C" int RwD3D8SetRenderState(int state, unsigned value)
{ PORT_HEAPCHK("RwD3D8SetRenderState");
#ifdef RW_GL3
	if (state == 60) g_d8.tfactor = value;                   // D3DRS_TEXTUREFACTOR
	else if (state == 19) rw::SetRenderState(rw::SRCBLEND, value);    // D3DRS_SRCBLEND / DESTBLEND: D3DBLEND_* = RwBlendFunction values
	else if (state == 20) rw::SetRenderState(rw::DESTBLEND, value);
	else if (state == 36 || state == 37) {   // D3DRS_FOGSTART / FOGEND: the game's pixel fog range (fogSet above)
		float f = *(float *)&value, s = state == 36 ? f : g_fogS, e = state == 37 ? f : g_fogE;
		fogSet(s, e);
	} else if (state == 24)             // D3DRS_ALPHAREF (rpPTankD3D8RenderCallBack: 1)
		rw::SetRenderState(rw::ALPHATESTREF, value);
	// other D3D8 states: no equivalent on gl3 outside the shadow pass (TODO: D3D8 fixed-function emulation)
#endif
	(void)state; (void)value; return 1;
}
extern "C" int RwD3D8GetRenderState(int state, unsigned *value) { PORT_HEAPCHK("RwD3D8GetRenderState"); (void)state; *value = 0; return 1; }
extern "C" int RwD3D8SetTextureStageState(int stage, int type, unsigned value)
{ PORT_HEAPCHK("RwD3D8SetTextureStageState");
#ifdef RW_GL3
	if (stage == 0 && (type == 16 || type == 17))   // D3D8 D3DTSS_MAGFILTER / MINFILTER (Hud2DObject::DrawQuad: 2 = linear) -> stage 0's filter
		rw::SetRenderState(rw::TEXTUREFILTER, value >= 2 ? rw::Texture::LINEAR : rw::Texture::NEAREST);
	else if (stage == 0 && type == 2) g_d8.colorArg1 = value;      // D3DTSS_COLORARG1
	else if (stage == 0 && type == 5) g_d8.alphaArg1 = value;      // D3DTSS_ALPHAARG1
	else if (stage == 0 && type == 11) g_d8.tci = value;           // D3DTSS_TEXCOORDINDEX
	else if (stage == 0 && type == 24) g_d8.ttff = value;          // D3DTSS_TEXTURETRANSFORMFLAGS
#endif
	(void)stage; (void)type; (void)value; return 1;
}
extern "C" int RwD3D8SetTexture(void *tex, int stage)
{ PORT_HEAPCHK("RwD3D8SetTexture");
#ifdef RW_GL3
	rw::gl3::setTexture(stage, (rw::Texture *)tex);   // as d3d9's rw::d3d::setTexture: the texture's raster, filter and addressing (particles: rwptank.cpp)
#endif
	(void)tex; (void)stage; return 1;
}
extern "C" int RwD3D8SetVertexShader(unsigned h) { PORT_HEAPCHK("RwD3D8SetVertexShader"); (void)h; return 1; }
extern "C" int RwD3D8SetPixelShader(unsigned h) { PORT_HEAPCHK("RwD3D8SetPixelShader"); (void)h; return 1; }
#ifdef RW_GL3
extern "C" int RwD3D8SetTransform(int state, const void *m) { PORT_HEAPCHK("RwD3D8SetTransform"); if (state == 16 /* D3DTS_TEXTURE0 */ && m) memcpy(g_d8.texMat, m, sizeof g_d8.texMat); return 1; }
extern "C" int RwD3D8SetStreamSource(unsigned n, void *vb, unsigned stride) { PORT_HEAPCHK("RwD3D8SetStreamSource"); (void)stride; if (n == 0) g_d8.vb = (PortCpuVB *)vb; return 1; }
extern "C" int RwD3D8SetIndices(void *ib, unsigned base) { PORT_HEAPCHK("RwD3D8SetIndices"); (void)base; g_d8.ib = (PortCpuIB *)ib; return 1; }
extern "C" int RwD3D8DrawPrimitive(int type, int start, int n) { PORT_HEAPCHK("RwD3D8DrawPrimitive"); (void)type; (void)start; (void)n; port_draw_d3d++; return 1; }
extern "C" int RwD3D8DrawIndexedPrimitive(int type, int minIdx, int nVerts, int start, int numIndices)
{ PORT_HEAPCHK("RwD3D8DrawIndexedPrimitive");
	(void)minIdx; (void)nVerts; (void)start;
	port_draw_d3d++;
	if (!PORT_DBG_HIDDEN(PORT_DBG_D3D)) d8DrawReceiver(type, numIndices);
	return 1;
}
#else
extern "C" int RwD3D8SetTransform(int state, const void *m) { PORT_HEAPCHK("RwD3D8SetTransform"); (void)state; (void)m; return 1; }
extern "C" int RwD3D8SetStreamSource(unsigned n, void *vb, unsigned stride) { PORT_HEAPCHK("RwD3D8SetStreamSource"); (void)n; (void)vb; (void)stride; return 1; }
extern "C" int RwD3D8SetIndices(void *ib, unsigned base) { PORT_HEAPCHK("RwD3D8SetIndices"); (void)ib; (void)base; return 1; }
extern "C" int RwD3D8DrawPrimitive(int type, int start, int n) { PORT_HEAPCHK("RwD3D8DrawPrimitive"); (void)type; (void)start; (void)n; return 1; }
extern "C" int RwD3D8DrawIndexedPrimitive(int type, int a, int b, int c, int d) { PORT_HEAPCHK("RwD3D8DrawIndexedPrimitive"); (void)type; (void)a; (void)b; (void)c; (void)d; return 1; }
#endif
extern "C" int RwD3D8CameraIsBBoxFullyInsideFrustum(const void *cam, const void *bbox) { PORT_HEAPCHK("RwD3D8CameraIsBBoxFullyInsideFrustum"); (void)cam; (void)bbox; return 1; }
extern "C" int RwD3D8CreateVertexBuffer(unsigned a, unsigned b, void **vb, unsigned *off) { PORT_HEAPCHK("RwD3D8CreateVertexBuffer"); (void)a; (void)b; *vb = 0; if (off) *off = 0; return 0; }
extern "C" void RwD3D8DestroyVertexBuffer(unsigned a, unsigned b, void *vb, unsigned off) { PORT_HEAPCHK("RwD3D8DestroyVertexBuffer"); (void)a; (void)b; (void)vb; (void)off; }
extern "C" int RwD3D8IndexBufferCreate(unsigned n, void **ib) { PORT_HEAPCHK("RwD3D8IndexBufferCreate"); (void)n; *ib = 0; return 0; }
extern "C" void RwD3D8SetVertexBufferToNull(void) { PORT_HEAPCHK("RwD3D8SetVertexBufferToNull");}
#endif

// D3DCAPS8 = the first 0xd4 bytes of D3DCAPS9. The game reads only MaxSimultaneousTextures (+0x98): > 1 creates the shadow pipeline
// and g_shadowTexture (FUN_00444a50), a path the stock game always takes. Headless: zeroed.
static char g_d3dCaps[0x400];
extern "C" const void *RwD3D8GetCaps(void)
{ PORT_HEAPCHK("RwD3D8GetCaps");
#ifdef RW_D3D9
	static int filled;
	if (!filled && dev()) {
		D3DCAPS9 c;
		if (dev()->GetDeviceCaps(&c) == D3D_OK) { memcpy(g_d3dCaps, &c, 0xd4); filled = 1; }
		RLOG("rwrender: RwD3D8GetCaps: from the d3d9 device (MaxSimultaneousTextures %u)\n", *(unsigned *)(g_d3dCaps + 0x98));
	}
#elif defined(RW_GL3)
	*(unsigned *)(g_d3dCaps + 0x98) = 8;     // GL 3.3 guarantees 16 texture units; D3D8 hardware of the time reported 2..8
#endif
	return g_d3dCaps;
}

// diagnostics: pixels that pass the depth/stencil tests between begin and end (a D3D9 occlusion query); -1 = unavailable
#ifdef RW_D3D9
static IDirect3DQuery9 *g_occ;
extern "C" void port_occ_begin(void) { if (!dev()) return; if (!g_occ) dev()->CreateQuery(D3DQUERYTYPE_OCCLUSION, &g_occ); if (g_occ) g_occ->Issue(D3DISSUE_BEGIN); }
extern "C" int port_occ_end(void)
{
	if (!g_occ) return -1;
	g_occ->Issue(D3DISSUE_END);
	DWORD n = 0;
	for (int i = 0; i < 100000 && g_occ->GetData(&n, sizeof(n), D3DGETDATA_FLUSH) == S_FALSE; i++) {}
	return (int)n;
}
#else
extern "C" void port_occ_begin(void) {}
extern "C" int port_occ_end(void) { return -1; }
#endif

// ValidateDevice failure bisect: states recorded after the device came up (port_record_default_states) vs now; each differing state is
// restored alone and the device re-validated
static unsigned long g_rsEarly[256]; static unsigned char g_rsHave[256]; static int g_rsRecorded;
extern "C" void port_record_default_states(void)
{
#ifdef RW_D3D9
	if (g_rsRecorded || !dev()) return;
	for (int s = 0; s < 256; s++) { DWORD v; if (SUCCEEDED(dev()->GetRenderState((D3DRENDERSTATETYPE)s, &v))) { g_rsEarly[s] = v; g_rsHave[s] = 1; } }
	g_rsRecorded = 1;
	if (port_log) {
		DWORD np = 0; HRESULT hv = dev()->ValidateDevice(&np);
		fprintf(port_log, "validate: at the first present ValidateDevice %08lx; render states:", (unsigned long)hv);
		for (int s = 0; s < 256; s++) if (g_rsHave[s]) fprintf(port_log, " %d=%lx", s, (unsigned long)g_rsEarly[s]);
		fprintf(port_log, "\n");
	}
#endif
}
#ifdef RW_D3D9
static void port_validate_bisect(IDirect3DDevice9 *d)
{
	static int done; if (done++) return;
	DWORD np;
	for (int s = 0; s < 256; s++) {
		if (!g_rsHave[s]) continue;
		DWORD v; if (FAILED(d->GetRenderState((D3DRENDERSTATETYPE)s, &v)) || v == g_rsEarly[s]) continue;
		d->SetRenderState((D3DRENDERSTATETYPE)s, g_rsEarly[s]);
		HRESULT h = d->ValidateDevice(&np);
		RLOG("validate: render state %d = %lx (early %lx): resetting it alone -> %08lx\n", s, (unsigned long)v, (unsigned long)g_rsEarly[s], (unsigned long)h);
		d->SetRenderState((D3DRENDERSTATETYPE)s, v);
	}
	{   // the documented D3D9 defaults of the states that can conflict
		static const DWORD defs[][2] = { { D3DRS_SRCBLEND, 2 }, { D3DRS_DESTBLEND, 1 }, { D3DRS_BLENDOP, 1 }, { D3DRS_ZFUNC, 4 }, { D3DRS_ALPHAFUNC, 8 },
			{ D3DRS_FOGTABLEMODE, 0 }, { D3DRS_FOGVERTEXMODE, 0 }, { D3DRS_STENCILFUNC, 8 }, { D3DRS_COLORWRITEENABLE, 0xf }, { D3DRS_SRGBWRITEENABLE, 0 },
			{ D3DRS_SEPARATEALPHABLENDENABLE, 0 }, { D3DRS_SRCBLENDALPHA, 2 }, { D3DRS_DESTBLENDALPHA, 1 }, { D3DRS_BLENDOPALPHA, 1 }, { D3DRS_SHADEMODE, 2 },
			{ D3DRS_FILLMODE, 3 }, { D3DRS_WRAP0, 0 }, { D3DRS_WRAP1, 0 }, { D3DRS_VERTEXBLEND, 0 }, { D3DRS_INDEXEDVERTEXBLENDENABLE, 0 }, { D3DRS_POINTSPRITEENABLE, 0 },
			{ D3DRS_MULTISAMPLEANTIALIAS, 1 }, { D3DRS_PATCHEDGESTYLE, 0 }, { D3DRS_POSITIONDEGREE, 3 }, { D3DRS_NORMALDEGREE, 1 }, { D3DRS_ADAPTIVETESS_Y, 0 },
			{ D3DRS_ENABLEADAPTIVETESSELLATION, 0 }, { D3DRS_TWOSIDEDSTENCILMODE, 0 }, { D3DRS_CCW_STENCILFUNC, 8 }, { D3DRS_DIFFUSEMATERIALSOURCE, 1 },
			{ D3DRS_SPECULARMATERIALSOURCE, 2 }, { D3DRS_AMBIENTMATERIALSOURCE, 0 }, { D3DRS_EMISSIVEMATERIALSOURCE, 0 }, { D3DRS_COLORVERTEX, 1 },
			{ D3DRS_SPECULARENABLE, 0 }, { D3DRS_DITHERENABLE, 0 }, { D3DRS_ZENABLE, 1 }, { D3DRS_ALPHATESTENABLE, 0 }, { D3DRS_ALPHABLENDENABLE, 0 } };
		for (unsigned i = 0; i < sizeof defs / sizeof defs[0]; i++) {
			DWORD v; d->GetRenderState((D3DRENDERSTATETYPE)defs[i][0], &v);
			if (v == defs[i][1]) continue;
			d->SetRenderState((D3DRENDERSTATETYPE)defs[i][0], defs[i][1]);
			HRESULT h = d->ValidateDevice(&np);
			RLOG("validate: state %lu = %lx (default %lx): default alone -> %08lx\n", (unsigned long)defs[i][0], (unsigned long)v, (unsigned long)defs[i][1], (unsigned long)h);
			d->SetRenderState((D3DRENDERSTATETYPE)defs[i][0], v);
		}
	}
	{   // texture stage states 1..32 of stages 0..7: reset each to its D3D9 default alone
		for (int st = 0; st < 8; st++) for (int t = 1; t <= 32; t++) {
			DWORD def;
			switch (t) {
			case D3DTSS_COLOROP: def = st ? D3DTOP_DISABLE : D3DTOP_MODULATE; break;
			case D3DTSS_ALPHAOP: def = st ? D3DTOP_DISABLE : D3DTOP_SELECTARG1; break;
			case D3DTSS_COLORARG1: case D3DTSS_ALPHAARG1: def = D3DTA_TEXTURE; break;
			case D3DTSS_COLORARG2: case D3DTSS_ALPHAARG2: def = D3DTA_CURRENT; break;
			case D3DTSS_TEXCOORDINDEX: def = (DWORD)st; break;
			case D3DTSS_TEXTURETRANSFORMFLAGS: def = 0; break;
			case D3DTSS_COLORARG0: case D3DTSS_ALPHAARG0: case D3DTSS_RESULTARG: def = D3DTA_CURRENT; break;
			case D3DTSS_BUMPENVMAT00: case D3DTSS_BUMPENVMAT01: case D3DTSS_BUMPENVMAT10: case D3DTSS_BUMPENVMAT11: case D3DTSS_BUMPENVLSCALE: case D3DTSS_BUMPENVLOFFSET: case D3DTSS_CONSTANT: def = 0; break;
			default: continue;
			}
			DWORD v; if (FAILED(d->GetTextureStageState(st, (D3DTEXTURESTAGESTATETYPE)t, &v)) || v == def) continue;
			d->SetTextureStageState(st, (D3DTEXTURESTAGESTATETYPE)t, def);
			HRESULT h = d->ValidateDevice(&np);
			RLOG("validate: stage %d tss %d = %lx (default %lx): default alone -> %08lx\n", st, t, (unsigned long)v, (unsigned long)def, (unsigned long)h);
			d->SetTextureStageState(st, (D3DTEXTURESTAGESTATETYPE)t, v);
		}
		for (int st = 0; st < 8; st++) for (int t = 1; t <= 13; t++) {
			DWORD v; if (FAILED(d->GetSamplerState(st, (D3DSAMPLERSTATETYPE)t, &v))) continue;
			RLOG("validate: sampler %d state %d = %lx\n", st, t, (unsigned long)v);
		}
	}
	for (int st = 0; st < 8; st++) {
		DWORD op = 0, a1 = 0, a2 = 0, aop = 0; d->GetTextureStageState(st, D3DTSS_COLOROP, &op); d->GetTextureStageState(st, D3DTSS_COLORARG1, &a1); d->GetTextureStageState(st, D3DTSS_COLORARG2, &a2); d->GetTextureStageState(st, D3DTSS_ALPHAOP, &aop);
		DWORD mn = 0, mg = 0, mp = 0; d->GetSamplerState(st, D3DSAMP_MINFILTER, &mn); d->GetSamplerState(st, D3DSAMP_MAGFILTER, &mg); d->GetSamplerState(st, D3DSAMP_MIPFILTER, &mp);
		IDirect3DBaseTexture9 *t = 0; d->GetTexture(st, &t);
		RLOG("validate: stage %d colorop %lu args %lx %lx alphaop %lu min %lu mag %lu mip %lu texture %p\n", st, op, a1, a2, aop, mn, mg, mp, (void *)t);
		if (t) t->Release();
	}
}
#endif

// PORT_RAWQUADS=<tex1>,<tex2>: before a present, draw two screen quads with raw D3D9 calls using tex1 and tex2.
extern "C++" rw::Texture *port_find_texture(const char *name);
#ifdef RW_D3D9
static IDirect3DSurface9 *g_rawSavedDS;
#endif
extern "C" void port_raw_quads(void)
{
#ifdef RW_D3D9
	static const char *e = getenv("PORT_RAWQUADS");
	if (!e || !dev()) return;
	char a[64] = "", b[64] = ""; sscanf(e, "%63[^,],%63s", a, b);
	rw::Texture *ta = port_find_texture(a), *tb = port_find_texture(b);
	struct V { float x, y, z, rhw; float u, v; };
	IDirect3DDevice9 *d = dev();
	HRESULT hs = d->BeginScene();
	{   // full device state at frames 5..9 (diff a working and a broken frame)
		int f = port_frames_presented();
		if (f >= 5 && f <= 9 && port_log) {
			fprintf(port_log, "statedump frame %d RS:", f);
			for (int k = 0; k < 210; k++) { DWORD v; if (SUCCEEDED(d->GetRenderState((D3DRENDERSTATETYPE)k, &v)) && v != 0xbaadcafe) fprintf(port_log, " %d=%lx", k, (unsigned long)v); }
			for (int st = 0; st < 4; st++) {
				fprintf(port_log, "\nstatedump frame %d TSS%d:", f, st);
				for (int k = 1; k <= 32; k++) { DWORD v; if (SUCCEEDED(d->GetTextureStageState(st, (D3DTEXTURESTAGESTATETYPE)k, &v))) fprintf(port_log, " %d=%lx", k, (unsigned long)v); }
				fprintf(port_log, "\nstatedump frame %d SS%d:", f, st);
				for (int k = 1; k <= 13; k++) { DWORD v; if (SUCCEEDED(d->GetSamplerState(st, (D3DSAMPLERSTATETYPE)k, &v))) fprintf(port_log, " %d=%lx", k, (unsigned long)v); }
			}
			for (int st = 0; st < 3; st++) { UINT fr = 0; d->GetStreamSourceFreq(st, &fr); fprintf(port_log, "\nstatedump frame %d streamfreq%d: %x", f, st, fr); }
			DWORD fvf = 0; d->GetFVF(&fvf); IDirect3DVertexDeclaration9 *decl = 0; d->GetVertexDeclaration(&decl); if (decl) decl->Release();
			RECT sc; d->GetScissorRect(&sc); D3DVIEWPORT9 vp; d->GetViewport(&vp); D3DCLIPSTATUS9 cs; d->GetClipStatus(&cs);
			fprintf(port_log, "\nstatedump frame %d fvf %lx decl %p viewport %lu %lu %lu %lu %g %g npatch %g\n", f, (unsigned long)fvf, (void *)decl, vp.X, vp.Y, vp.Width, vp.Height, vp.MinZ, vp.MaxZ, d->GetNPatchMode());
		}
	}
	{ D3DRECT rc = { 500, 300, 620, 460 }; HRESULT hc = d->Clear(1, &rc, D3DCLEAR_TARGET, 0xff00ff00, 1.0f, 0); static int lc; if (lc++ < 2) RLOG("rawquads: green clear rect %08lx, BeginScene %08lx\n", (unsigned long)hc, (unsigned long)hs); }
	{
		D3DVIEWPORT9 vp; d->GetViewport(&vp); RECT sc; d->GetScissorRect(&sc);
		DWORD msm = 0, clip = 0, scis = 0; d->GetRenderState(D3DRS_MULTISAMPLEMASK, &msm); d->GetRenderState(D3DRS_CLIPPING, &clip); d->GetRenderState(D3DRS_SCISSORTESTENABLE, &scis);
		IDirect3DSurface9 *rt = 0; d->GetRenderTarget(0, &rt); D3DSURFACE_DESC rd; memset(&rd, 0, sizeof rd); if (rt) { rt->GetDesc(&rd); rt->Release(); }
		{ IDirect3DSurface9 *ds = 0; d->GetDepthStencilSurface(&ds); D3DSURFACE_DESC dd; memset(&dd, 0, sizeof dd); if (ds) { ds->GetDesc(&dd); ds->Release(); }
		  static int ld; if (ld++ < 12) RLOG("rawquads: frame %d depth-stencil %p %ux%u fmt %d ms %d\n", port_frames_presented(), (void *)ds, dd.Width, dd.Height, (int)dd.Format, (int)dd.MultiSampleType); }
		static int lv; if (lv++ < 2) RLOG("rawquads: viewport %lu,%lu %lux%lu z %g..%g scissor %ld,%ld,%ld,%ld (on %lu) msmask %lx clipping %lu rt %ux%u fmt %d ms %d\n", vp.X, vp.Y, vp.Width, vp.Height, vp.MinZ, vp.MaxZ,
		                                  sc.left, sc.top, sc.right, sc.bottom, scis, msm, clip, rd.Width, rd.Height, (int)rd.Format, (int)rd.MultiSampleType);
		if (getenv("PORT_RAWFIXVP")) { D3DVIEWPORT9 v2 = { 0, 0, 640, 480, 0.0f, 1.0f }; d->SetViewport(&v2); d->SetRenderState(D3DRS_MULTISAMPLEMASK, 0xffffffff); }
		if (getenv("PORT_RAWNODS")) { d->GetDepthStencilSurface(&g_rawSavedDS); d->SetDepthStencilSurface(0); d->SetRenderState(D3DRS_CLIPPING, 0); d->SetRenderState(D3DRS_ZWRITEENABLE, 0); }
	}
	d->SetVertexShader(0); d->SetPixelShader(0); d->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	d->SetRenderState(D3DRS_ZENABLE, 0); d->SetRenderState(D3DRS_ALPHABLENDENABLE, 0); d->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	d->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); d->SetRenderState(D3DRS_LIGHTING, 0); if (getenv("PORT_RAWNOSTENCIL")) d->SetRenderState(D3DRS_STENCILENABLE, 0); { DWORD fe = 0, fm = 0, ft = 0; d->GetRenderState(D3DRS_FOGENABLE, &fe); d->GetRenderState(D3DRS_FOGVERTEXMODE, &fm); d->GetRenderState(D3DRS_FOGTABLEMODE, &ft); static int lg; if (lg++ < 3) RLOG("rawquads: fogenable %lu vertexmode %lu tablemode %lu\n", fe, fm, ft); if (getenv("PORT_RAWNOFOG")) d->SetRenderState(D3DRS_FOGENABLE, 0); }
	d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1); d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	for (int q = 0; q < 3; q++) {
		rw::Texture *t = q == 2 ? 0 : q ? tb : ta;
		if (q == 2) { d->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffff00ff); d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR); d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR); }
		IDirect3DBaseTexture9 *dt = t && t->raster ? (IDirect3DBaseTexture9 *)PLUGINOFFSET(rw::d3d::D3dRaster, t->raster, rw::d3d::nativeRasterOffset)->texture : 0;
		float x0 = 20.0f + q * 220.0f, y0 = 20.0f, s = 200.0f;
		V v[4] = { { x0, y0, 0, 1, 0, 0 }, { x0 + s, y0, 0, 1, 1, 0 }, { x0, y0 + s, 0, 1, 0, 1 }, { x0 + s, y0 + s, 0, 1, 1, 1 } };
		HRESULT h1 = d->SetTexture(0, dt);
		DWORD np = 0; HRESULT hv = d->ValidateDevice(&np);
		if (FAILED(hv) && q == 0) port_validate_bisect(d); { static int lv; if (lv++ < 6) RLOG("rawquads: quad %d ValidateDevice %08lx passes %lu\n", q, (unsigned long)hv, (unsigned long)np); }
		HRESULT h2 = d->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(V));
		static int n; if ((dt || q == 2) && n++ < 6) RLOG("rawquads: quad %d tex %s d3d %p: SetTexture %08lx Draw %08lx\n", q, t ? t->name : "-", (void *)dt, (unsigned long)h1, (unsigned long)h2);
	}
	if (g_rawSavedDS) { d->SetDepthStencilSurface(g_rawSavedDS); g_rawSavedDS->Release(); g_rawSavedDS = 0; }
	if (SUCCEEDED(hs)) d->EndScene();
	// librw's caches now disagree with the device: invalidate what we touched
	rw::d3d::setVertexShader(nil); rw::d3d::setPixelShader(nil); rw::d3d::setVertexDeclaration(nil);
#endif
}

// diagnostics: the D3D9 description of a raster's texture (format, size, levels, pool); 0 = no texture
extern "C" int port_diag_raster(rw::Raster *r, char *buf, int n)
{
#ifdef RW_D3D9
	IDirect3DTexture9 *t = r ? (IDirect3DTexture9 *)PLUGINOFFSET(rw::d3d::D3dRaster, r, rw::d3d::nativeRasterOffset)->texture : 0;
	if (!t) { _snprintf(buf, n, "no d3d texture"); return 0; }
	D3DSURFACE_DESC sd; t->GetLevelDesc(0, &sd);
	D3DLOCKED_RECT lr; HRESULT hl = t->LockRect(0, &lr, 0, D3DLOCK_READONLY);
	unsigned px0 = 0;
	if (SUCCEEDED(hl)) { px0 = *(unsigned *)lr.pBits; t->UnlockRect(0); }
	_snprintf(buf, n, "d3d fmt %d %ux%u levels %lu pool %d usage %lx hasAlpha %d lock %08lx texel0 %08x", (int)sd.Format, sd.Width, sd.Height, (unsigned long)t->GetLevelCount(), (int)sd.Pool,
	          (unsigned long)sd.Usage, (int)PLUGINOFFSET(rw::d3d::D3dRaster, r, rw::d3d::nativeRasterOffset)->hasAlpha, (unsigned long)hl, px0);
	return 1;
#else
	_snprintf(buf, n, "-"); return 0;
#endif
}

// diagnostics: what librw is about to draw for an atomic (geometry flags, first meshes' textures and their d3d9 objects)
extern "C" void port_diag_atomic(rw::Atomic *a, const char *what)
{
	if (!a || !a->geometry) return;
	rw::Geometry *g = a->geometry;
	RLOG("diag: %s atomic %p geo flags %x texsets %d mats %d meshes %d inst %p\n", what, (void *)a, g->flags, g->numTexCoordSets,
	     g->matList.numMaterials, g->meshHeader ? g->meshHeader->numMeshes : -1, (void *)g->instData);
	for (int i = 0; i < 3 && i < g->numVertices; i++) {
		rw::V3d *v = &g->morphTargets[0].vertices[i];
		rw::TexCoords *t = g->texCoords[0] ? &g->texCoords[0][i] : 0;
		rw::RGBA *c = g->colors ? &g->colors[i] : 0;
		RLOG("diag:   v%d pos %.1f %.1f %.1f uv %g %g col %02x%02x%02x%02x\n", i, v->x, v->y, v->z, t ? t->u : -1.0f, t ? t->v : -1.0f,
		     c ? c->red : 0, c ? c->green : 0, c ? c->blue : 0, c ? c->alpha : 0);
	}
#ifdef RW_D3D9
	if (dev()) {
		static const int st[] = { D3DRS_ZENABLE, D3DRS_ZWRITEENABLE, D3DRS_ZFUNC, D3DRS_ALPHABLENDENABLE, D3DRS_SRCBLEND, D3DRS_DESTBLEND,
		                          D3DRS_ALPHATESTENABLE, D3DRS_ALPHAFUNC, D3DRS_ALPHAREF, D3DRS_CULLMODE, D3DRS_FOGENABLE, D3DRS_LIGHTING };
		static const char *nm[] = { "z", "zwrite", "zfunc", "blend", "src", "dst", "atest", "afunc", "aref", "cull", "fog", "light" };
		char buf[256]; int o = 0;
		for (int i = 0; i < 12; i++) { DWORD v = 0; dev()->GetRenderState((D3DRENDERSTATETYPE)st[i], &v); o += _snprintf(buf + o, sizeof(buf) - o, " %s=%lu", nm[i], (unsigned long)v); }
		{
			static const int ss[] = { D3DSAMP_ADDRESSU, D3DSAMP_ADDRESSV, D3DSAMP_MAGFILTER, D3DSAMP_MINFILTER, D3DSAMP_MIPFILTER, D3DSAMP_MAXMIPLEVEL, D3DSAMP_MIPMAPLODBIAS, D3DSAMP_BORDERCOLOR };
			static const char *sn[] = { "au", "av", "mag", "min", "mip", "maxmip", "lodbias", "border" };
			char sb[256]; int so = 0;
			for (int i = 0; i < 8; i++) { DWORD v = 0; dev()->GetSamplerState(0, (D3DSAMPLERSTATETYPE)ss[i], &v); so += _snprintf(sb + so, sizeof(sb) - so, " %s=%lx", sn[i], (unsigned long)v); }
			IDirect3DBaseTexture9 *bt = 0; dev()->GetTexture(0, &bt);
			IDirect3DPixelShader9 *ps = 0; dev()->GetPixelShader(&ps);
			IDirect3DVertexShader9 *vs = 0; dev()->GetVertexShader(&vs);
			RLOG("diag:   sampler0%s texture %p ps %p vs %p\n", sb, (void *)bt, (void *)ps, (void *)vs);
			if (bt && bt->GetType() == D3DRTYPE_TEXTURE) {
				D3DSURFACE_DESC sd; ((IDirect3DTexture9 *)bt)->GetLevelDesc(0, &sd);
				DWORD maxaniso = 0; dev()->GetSamplerState(0, D3DSAMP_MAXANISOTROPY, &maxaniso);
				RLOG("diag:   texture0 fmt %d type %d usage %lx pool %d %ux%u levels %lu maxaniso %lu\n", (int)sd.Format, (int)sd.Type, (unsigned long)sd.Usage, (int)sd.Pool, sd.Width, sd.Height, (unsigned long)bt->GetLevelCount(), (unsigned long)maxaniso);
			}
			if (bt) bt->Release(); if (ps) ps->Release(); if (vs) vs->Release();
		}
		D3DVIEWPORT9 vp; dev()->GetViewport(&vp);
		RLOG("diag:   device%s viewport %lu,%lu %lux%lu z %.2f-%.2f\n", buf, vp.X, vp.Y, vp.Width, vp.Height, vp.MinZ, vp.MaxZ);
	}
#endif
	if (g->colors) {
		int lo = 0, mid = 0, hi = 0;
		for (int i = 0; i < g->numVertices; i++) { int al = g->colors[i].alpha; if (al < 2) lo++; else if (al < 255) mid++; else hi++; }
		RLOG("diag:   prelit alpha: %d vertices < 2, %d partial, %d opaque (of %d)\n", lo, mid, hi, g->numVertices);
	}
	for (int i = 0; i < g->matList.numMaterials && i < 12; i++) {
		rw::Material *mm = g->matList.materials[i];
		RLOG("diag:   material %d: color %02x%02x%02x%02x surf %g %g %g\n", i, mm->color.red, mm->color.green, mm->color.blue, mm->color.alpha,
		     mm->surfaceProps.ambient, mm->surfaceProps.specular, mm->surfaceProps.diffuse);
	}
	for (int i = 0; g->meshHeader && i < g->meshHeader->numMeshes && i < 3; i++) {
		rw::Mesh *m = &g->meshHeader->getMeshes()[i];
		rw::Texture *t = m->material ? m->material->texture : 0;
		rw::Raster *r = t ? t->raster : 0;
#ifdef RW_D3D9
		void *tex = r ? PLUGINOFFSET(rw::d3d::D3dRaster, r, rw::d3d::nativeRasterOffset)->texture : 0;
#else
		void *tex = 0;
#endif
		RLOG("diag:   mesh %d: %u idx, mat %p tex %s raster %p (platform %d type %d fmt %x %dx%d) d3dtex %p\n", i, m->numIndices, (void *)m->material,
		     t ? t->name : "-", (void *)r, r ? r->platform : -1, r ? r->type : -1, r ? r->format : 0, r ? r->width : 0, r ? r->height : 0, tex);
	}
}

// FUN_00444a50 (shadow receivers) builds a D3D8 world-sector pipeline with its own render callback (FUN_00444770). The port records
// (name, render callback); RxPipelineExecute (rwshim.cpp) skips sectors through such a pipeline (port_pipe_custom_cb != 0).
struct PortNodeDef { const char *name; };
struct PortPipe { unsigned magic; const PortNodeDef *def; void *renderCB; };
static const PortNodeDef g_sectorAllInOne = { "nodeD3D8WorldSectorAllInOne.csl" };
enum { PORT_PIPE_MAGIC = 0x50495045 };
extern "C" const PortNodeDef *RxNodeDefinitionGetD3D8WorldSectorAllInOne(void) { return &g_sectorAllInOne; }
extern "C" PortPipe *RxPipelineCreate(void) { PortPipe *p = (PortPipe *)calloc(1, sizeof(PortPipe)); p->magic = PORT_PIPE_MAGIC; RLOG("rwrender: RxPipelineCreate -> %p\n", (void *)p); return p; }
extern "C" PortPipe *RxPipelineLock(PortPipe *p) { return p; }
extern "C" PortPipe *RxLockedPipeAddFragment(PortPipe *p, unsigned *firstIndex, const PortNodeDef *def, ...) { (void)firstIndex; if (p && !p->def) p->def = def; return p; }
extern "C" PortPipe *RxLockedPipeUnlock(PortPipe *p) { return p; }
extern "C" PortPipe *RxPipelineFindNodeByName(PortPipe *p, const char *name, void *start, int *index) { (void)start; (void)index; return p && p->def && name && !strcmp(p->def->name, name) ? p : 0; }
extern "C" PortPipe *RxD3D8AllInOneSetRenderCallBack(PortPipe *node, void *cb) { if (node) node->renderCB = cb; RLOG("rwrender: RxD3D8AllInOneSetRenderCallBack(%p, %p)\n", (void *)node, cb); return node; }
extern "C" void _rxPipelineDestroy(PortPipe *p) { if (p && p->magic == PORT_PIPE_MAGIC) { p->magic = 0; free(p); } }
extern "C" void *port_pipe_custom_cb(const void *pipe)
{
	const PortPipe *p = (const PortPipe *)pipe;
	return p && p->magic == PORT_PIPE_MAGIC ? p->renderCB : 0;
}

// Shadow receivers: the callback FUN_00444770 (0x444770) reads RW 3.6's RxD3D8ResEntryHeader ("entry"): +0x1a numMeshes, 0x2c-byte
// mesh records from +0x20 {+0 stride, +4 numVertices, +8 numIndices (converted to a primitive count by the draw), +0xc material,
// +0x14 primType, +0x18 index buffer, +0x1c vertex buffer, +0x20 base vertex index}, and draws each mesh with RwD3D8* calls (fixed
// function, FVF 0x142). The port builds the entry from the sector's d3d9 instance data (one shared vertex stream, one index buffer
// with rebased indices + baseIndex): per mesh a private index buffer (startIndex 0 as in D3D8), the shared vertex buffer, baseIndex
// as the base vertex index. Device transforms are set from the camera first (librw's shaders ignore them; the D3D8 driver set them
// in BeginUpdate).
#ifdef RW_D3D9
struct PortRecv { rw::Atomic *atomic; unsigned char *entry; int n; IDirect3DIndexBuffer9 *ib[64]; };
static PortRecv g_recv[512]; static int g_nRecv;
// A destroyed sector atomic (rwshim.cpp sectorDestroy) drops its cached entry. The cache is keyed by address: a later level's atomic
// at the same address picked up the stale entry with its freed geometry (intermittent crash at level start).
extern "C" void port_recv_forget(void *atomic)
{
	for (int i = 0; i < g_nRecv; i++)
		if (g_recv[i].atomic == atomic) {
			for (int k = 0; k < 64; k++) if (g_recv[i].ib[k]) g_recv[i].ib[k]->Release();
			free(g_recv[i].entry);
			g_recv[i] = g_recv[--g_nRecv];
			return;
		}
}
static PortRecv *recvEntry(rw::Atomic *a)
{
	for (int i = 0; i < g_nRecv; i++) if (g_recv[i].atomic == a) return g_recv[i].entry ? &g_recv[i] : 0;
	if (g_nRecv >= 512 || !dev()) return 0;
	rw::Geometry *g = a->geometry;
	rw::d3d9::InstanceDataHeader *h = g ? (rw::d3d9::InstanceDataHeader *)g->instData : 0;
	if (!h || !g->meshHeader || h->numMeshes > 64 || !h->vertexStream[0].vertexBuffer) return 0;      // not instanced yet: next frame
	PortRecv *r = &g_recv[g_nRecv++]; memset(r, 0, sizeof *r); r->atomic = a; r->n = (int)h->numMeshes;
	PortRecvEntry *en = (PortRecvEntry *)calloc(1, offsetof(PortRecvEntry, meshes) + sizeof(PortRecvMesh) * (r->n ? r->n : 1));
	r->entry = (unsigned char *)en;
	en->numMeshes = (unsigned short)r->n;
	rw::Mesh *meshes = g->meshHeader->getMeshes();
	for (int i = 0; i < r->n; i++) {
		rw::d3d9::InstanceData *in = &h->inst[i]; PortRecvMesh *m = &en->meshes[i];
		IDirect3DIndexBuffer9 *ib = 0; unsigned short *dst;
		if (in->numIndex && dev()->CreateIndexBuffer(in->numIndex * 2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, 0) == D3D_OK && ib->Lock(0, 0, (void **)&dst, 0) == D3D_OK) {
			for (unsigned j = 0; j < in->numIndex; j++) dst[j] = (unsigned short)(meshes[i].indices[j] - in->minVert);
			ib->Unlock();
		}
		r->ib[i] = ib;
		m->stride = (int)h->vertexStream[0].stride; m->numVertices = (int)in->numVertices; m->numIndices = (int)in->numIndex;
		m->material = in->material; m->vertexShader = 0x142;
		m->primType = (unsigned)h->primType; m->indexBuffer = ib; m->vertexBuffer = h->vertexStream[0].vertexBuffer; m->baseIndex = in->baseIndex;
	}
	RLOG("rwrender: receiver entry for atomic %p: %d meshes, stride %u, primType %u\n", (void *)a, r->n, (unsigned)h->vertexStream[0].stride, (unsigned)h->primType);
	return r;
}
extern "C" void port_pipe_run_custom(const void *pipe, void *sector, rw::Atomic *atomic)
{
	void *cb = port_pipe_custom_cb(pipe); if (!cb || !atomic || !dev()) return;
	PortRecv *r = recvEntry(atomic); if (!r) return;
	rw::Camera *cam = rw::engine->currentCamera; if (!cam) return;
	static const float ident[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	dev()->SetRenderState(D3DRS_LIGHTING, 0); dev()->SetRenderState(D3DRS_CLIPPING, 1); dev()->SetRenderState(D3DRS_DITHERENABLE, 1);   // the D3D8 device state RW left (stock capture); librw's shader path never uses them
	dev()->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)ident);
	dev()->SetTransform(D3DTS_VIEW, (const D3DMATRIX *)&cam->devView);
	dev()->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX *)&cam->devProj);
	((void (*)(void *, void *, unsigned char, unsigned))cb)(r->entry, sector, 1, 0);
	static int n; if (n++ < 3) RLOG("rwrender: receiver callback %p ran for sector %p (%d meshes)\n", cb, sector, r->n);
}
#elif defined(RW_GL3)
// gl3: the same entry with CPU "buffers": vertex buffer = the sector geometry (PortCpuVB), each mesh's index buffer its own list
// (absolute indices, base 0); the game's callback then drives the recording RwD3D8* calls above. A destroyed sector atomic drops its
// entry, as in the d3d9 cache (else SIGSEGV in d8DrawReceiver at level start).
struct PortRecvGL { rw::Atomic *atomic; PortRecvEntry *entry; PortCpuVB vb; PortCpuIB ib[64]; };
static PortRecvGL g_recvGL[512]; static int g_nRecvGL;
extern "C" void port_recv_forget(void *atomic)
{
	for (int i = 0; i < g_nRecvGL; i++)
		if (g_recvGL[i].atomic == atomic) {
			free(g_recvGL[i].entry);
			if (i != g_nRecvGL - 1) {   // the moved entry's meshes point into its own ib / vb: re-point them
				g_recvGL[i] = g_recvGL[g_nRecvGL - 1];
				PortRecvGL *r = &g_recvGL[i];
				for (int k = 0; r->entry && k < r->entry->numMeshes; k++) { r->entry->meshes[k].indexBuffer = &r->ib[k]; r->entry->meshes[k].vertexBuffer = &r->vb; }
			}
			g_nRecvGL--;
			if (g_d8.vb == &g_recvGL[g_nRecvGL].vb) g_d8.vb = 0;
			return;
		}
}
static PortRecvGL *recvEntryGL(rw::Atomic *a)
{
	for (int i = 0; i < g_nRecvGL; i++) if (g_recvGL[i].atomic == a) return g_recvGL[i].entry ? &g_recvGL[i] : 0;
	rw::Geometry *g = a->geometry;
	if (g_nRecvGL >= 512 || !g || !g->meshHeader || g->meshHeader->numMeshes > 64 || !g->morphTargets || !g->morphTargets[0].vertices) return 0;
	PortRecvGL *r = &g_recvGL[g_nRecvGL++]; memset(r, 0, sizeof *r); r->atomic = a;
	int n = (int)g->meshHeader->numMeshes;
	PortRecvEntry *en = (PortRecvEntry *)calloc(1, offsetof(PortRecvEntry, meshes) + sizeof(PortRecvMesh) * (n ? n : 1));
	r->entry = en; en->numMeshes = (unsigned short)n;
	r->vb.magic = PORT_CPUVB_MAGIC; r->vb.geo = g;
	rw::Mesh *meshes = g->meshHeader->getMeshes();
	unsigned prim = (g->meshHeader->flags & rw::MeshHeader::TRISTRIP) ? 5 : 4;   // D3DPT_TRIANGLESTRIP / TRIANGLELIST
	for (int i = 0; i < n; i++) {
		PortRecvMesh *m = &en->meshes[i];
		r->ib[i].idx = meshes[i].indices; r->ib[i].n = (int)meshes[i].numIndices;
		m->stride = 24; m->numVertices = g->numVertices; m->numIndices = (int)meshes[i].numIndices;
		m->material = meshes[i].material; m->vertexShader = 0x142; m->primType = prim;
		m->indexBuffer = &r->ib[i]; m->vertexBuffer = &r->vb; m->baseIndex = 0;
	}
	RLOG("rwrender: receiver entry (gl3, CPU) for atomic %p: %d meshes, %d vertices, primType %u\n", (void *)a, n, g->numVertices, prim);
	return r;
}
extern "C" void port_pipe_run_custom(const void *pipe, void *sector, rw::Atomic *atomic)
{
	void *cb = port_pipe_custom_cb(pipe); if (!cb || !atomic) return;
	PortRecvGL *r = recvEntryGL(atomic); if (!r) return;
	((void (*)(void *, void *, unsigned char, unsigned))cb)(r->entry, sector, 1, 0);
	static int n; if (n++ < 3) RLOG("rwrender: receiver callback %p ran for sector %p (%d meshes, gl3)\n", cb, sector, (int)r->entry->numMeshes);
}
#else
extern "C" void port_pipe_run_custom(const void *, void *, void *) {}
extern "C" void port_recv_forget(void *) {}
#endif

// ---------------------------------------------------------------- D3DX: IDirect3DTexture8 facade over a librw image
struct PortTexture8 : D3DTexture8 {       // IDirect3DTexture8 vtable order (rw/d3dtexture8.h), COM __stdcall; only the slots the font/HUD loaders use do anything
	long __stdcall QueryInterface(const void *, void **) override { return (long)(int)0x80004002; }
	unsigned long __stdcall AddRef() override { return ++refs; }
	unsigned long __stdcall Release() override { if (--refs == 0) { free(bgra); delete this; return 0; } return refs; }
	long __stdcall GetDevice(void **) override { return 0; }
	long __stdcall SetPrivateData(const void *, const void *, unsigned, unsigned) override { return 0; }
	long __stdcall GetPrivateData(const void *, void *, unsigned *) override { return 0; }
	long __stdcall FreePrivateData(const void *) override { return 0; }
	unsigned long __stdcall SetPriority(unsigned long) override { return 0; }
	unsigned long __stdcall GetPriority() override { return 0; }
	void __stdcall PreLoad() override {}
	int __stdcall GetType() override { return 3; }
	unsigned long __stdcall SetLOD(unsigned long) override { return 0; }
	unsigned long __stdcall GetLOD() override { return 0; }
	unsigned long __stdcall GetLevelCount() override { return 1; }
	long __stdcall GetLevelDesc(unsigned level, D3DSurfaceDesc *d) override
	{
		(void)level;
		d->format = 21; d->type = 3; d->usage = 0; d->pool = 1; d->size = width * height * 4; d->msType = 0; d->width = width; d->height = height;
		return 0;
	}
	long __stdcall GetSurfaceLevel(unsigned, void **) override { return (long)(int)0x80004001; }
	long __stdcall LockRect(unsigned level, D3DLockedRect *r, const void *area, unsigned flags) override { (void)level; (void)area; (void)flags; r->pitch = width * 4; r->bits = bgra; return 0; }
	long __stdcall UnlockRect(unsigned) override { return 0; }
	long __stdcall AddDirtyRect(const void *) override { return 0; }
	unsigned refs; unsigned width, height; unsigned char *bgra;
};
extern "C" long __stdcall D3DXCreateTextureFromFileA(void *device, const char *path, D3DTexture8 **out)
{ PORT_HEAPCHK("D3DXCreateTextureFromFileA");
	(void)device;
	*out = 0;
	char base[512];                                   // librw's Image::read appends the registered extensions itself
	strncpy(base, path ? path : "", sizeof(base) - 1); base[sizeof(base) - 1] = 0;
	{ char *dot = strrchr(base, '.'); char *sl = strrchr(base, '\\'); if (dot && (!sl || dot > sl)) *dot = 0; }
	rw::Image *img = rw::Image::read(base);
	if (!img) { RLOG("rwrender: D3DXCreateTextureFromFileA(%s): no image\n", path ? path : ""); return (long)(int)0x88760B59; }   // D3DXERR_INVALIDDATA
	img->convertTo32();
	PortTexture8 *t = new PortTexture8;
	t->refs = 1; t->width = img->width; t->height = img->height;
	t->bgra = (unsigned char *)malloc(t->width * t->height * 4);
	for (unsigned y = 0; y < t->height; y++) {
		const unsigned char *src = img->pixels + y * img->stride;
		unsigned char *dst = t->bgra + y * t->width * 4;
		for (unsigned x = 0; x < t->width; x++, src += 4, dst += 4) { dst[0] = src[2]; dst[1] = src[1]; dst[2] = src[0]; dst[3] = src[3]; }   // RGBA -> D3D ARGB (BGRA bytes)
	}
	img->destroy();
	RLOG("rwrender: D3DXCreateTextureFromFileA(%s) -> %ux%u\n", path, t->width, t->height);
	*out = t;
	return 0;
}

// diagnostics hook for temporary instrumentation in the build copy of librw (port/build/librw_src): the port log
extern "C" FILE *port_log;
FILE *port_log_cpp() { return port_log; }

extern "C" { int port_diag_draws; }   // DIAG: librw build-copy instrumentation logs the device state of the next N draws
extern "C" void port_ud_log(void *o, int off, int n, void *a)   // DIAG: librw build copy destroyUserData with arrays
{
	static int k; if (k++ < 5000 && port_log) fprintf(port_log, "userdata destroy: object %p offset %d numArrays %d arrays %p (type byte %d)\n", o, off, n, a, *(unsigned char *)o);
}
