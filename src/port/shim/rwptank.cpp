// RpPTank on librw: the game's FX (FxPtank 0x40a340, ParticlePool) create PTank atomics and write particles through RpPTankAtomicLock.
// The RW atomic extension keeps its real layout because the game pokes it via g_ptankPluginOffset (0x6308e8): maxPCount +0, actPCount
// +4, instFlags +0x40. Data is array form (one buffer per cluster, cluster index = bit index of the data flag). Rendering:
// camera-facing quads through Im3D (ptankRender).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <rw.h>
#include "rwlayouts.h"
#include "../compat/rw_null_device.h"   // headless build: device vertex types (no-op on d3d9)
extern "C" FILE *port_log;
#define CLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)

int g_ptankPluginOffset;            // ?g_ptankPluginOffset@@3HA: the game's name for _rpPTankAtomicDataOffset


static const int kClusterStride[9] = { 12, 4, 8, 64, 12, 4, 16, 16, 32 };   // position color size matrix normal 2drotate vtxcolor vtx2tex vtx4tex
#define PTANK_ID 0x130              // rwID_PTANKPLUGIN

static RpPTankAtomicExtPrv *&extOf(rw::Atomic *a) { return *(RpPTankAtomicExtPrv **)((char *)a + g_ptankPluginOffset); }

static void *ptankCtor(void *object, int offset, int) { *(void **)((char *)object + offset) = 0; return object; }
static void *ptankDtor(void *object, int offset, int)
{
	RpPTankAtomicExtPrv *e = *(RpPTankAtomicExtPrv **)((char *)object + offset);
	if (e) {
		free(e->rawdata);
		free(e);
		*(void **)((char *)object + offset) = 0;
	}
	return object;
}

// Quads as RW's rpPTankD3D8 render makes them: right = -camLTM.right (RW's right points to screen left), up = camLTM.up; per particle
// P + (-R-U, +R-U, +R+U, -R+U) with R = right * sizeX / 2, U = up * sizeY / 2 (rpPTankGENInsPosNCPSNR 0x57e410), texcoords (u0,v1)
// (u1,v1) (u1,v0) (u0,v0) from the top-left / bottom-right pair of cluster 7 (0x57fac0), indices (0,1,2) (0,2,3): counter-clockwise,
// so the draw survives the world pass's cull mode. The render callback 0x57c520 sets ALPHAREF 1, the material texture,
// VERTEXALPHAENABLE = vertexAlphaBlend and, if set, SRCBLEND / DESTBLEND (restored after); no cull, z or fog state. 2D rotation (flag
// 0x20) and matrices (0x8) are not implemented: the only creator, FxPtank::Construct, uses flags 0x87.
extern "C" int RwD3D8SetRenderState(int state, unsigned value);
extern "C" int RwD3D8SetTexture(void *tex, int stage);
extern "C" int RwRenderStateSet(int state, void *value);
extern "C" int RwRenderStateGet(int state, void *value);
static int g_ptankRenderLogged;
// PORT_FXLOG=<dir>: log every tank draw (state, raster format, colour / size ranges) and dump each particle raster once as
// <dir>/ptank_<texture>.png.
extern "C" int port_frames_presented(void);
#ifdef RW_GL3
extern "C" unsigned char (*glad_glIsEnabled)(unsigned);
extern "C" void (*glad_glGetIntegerv)(unsigned, int *);
#endif
static void ptankFxLog(rw::Atomic *a, RpPTankAtomicExtPrv *e, rw::Texture *tex, rw::Raster *ras)
{
	static const char *dir = getenv("PORT_FXLOG");
	if (!dir) return;
	RpPTankData *pd = &e->publicData;
	unsigned zw = rw::GetRenderState(rw::ZWRITEENABLE), aref = rw::GetRenderState(rw::ALPHATESTREF), afn = rw::GetRenderState(rw::ALPHATESTFUNC);
	int glBlend = -1, glSrc = -1, glDst = -1;
#ifdef RW_GL3
	glBlend = glad_glIsEnabled(0x0BE2 /* GL_BLEND */); glad_glGetIntegerv(0x80C9 /* GL_BLEND_SRC_RGB */, &glSrc); glad_glGetIntegerv(0x80C8 /* GL_BLEND_DST_RGB */, &glDst);
#endif
	int amin = 255, amax = 0; float smin = 1e9f, smax = 0; unsigned rgbOr = 0, rgbAnd = 0xffffff;
	for (int p = 0; p < e->actPCount; p++) {
		if (pd->clusters[1].data) { const unsigned char *c = pd->clusters[1].data + p * pd->clusters[1].stride; unsigned rgb = c[0] << 16 | c[1] << 8 | c[2]; rgbOr |= rgb; rgbAnd &= rgb; int al = c[3]; if (al < amin) amin = al; if (al > amax) amax = al; }
		if (pd->clusters[2].data) { float s = *(const float *)(pd->clusters[2].data + p * pd->clusters[2].stride); if (s < smin) smin = s; if (s > smax) smax = s; }
	}
	const char *name = tex ? tex->name : "(none)";
	CLOG("fxlog: ptank frame %d tex '%s' ras %dx%d fmt %x type %d filter %d addr %d/%d | blend %u/%u valpha %u zwrite %u atest func %u ref %u | GL blend %d %x/%x va %d | n %d alpha %d..%d size %g..%g rgb and %06x or %06x atomic %p\n",
	     port_frames_presented(), name, ras ? ras->width : 0, ras ? ras->height : 0, ras ? ras->format : 0, ras ? ras->type : -1,
	     tex ? (int)tex->getFilter() : -1, tex ? (int)tex->getAddressU() : -1, tex ? (int)tex->getAddressV() : -1,
	     pd->srcBlend, pd->dstBlend, (unsigned)pd->vertexAlphaBlend, zw, afn, aref, glBlend, glSrc, glDst, (int)rw::GetRenderState(rw::VERTEXALPHA), e->actPCount, amin, amax, smin, smax, rgbAnd, rgbOr, (void *)a);
	static void *dumped[256]; static int nd;
	for (int i = 0; i < nd; i++) if (dumped[i] == ras) return;
	if (!ras || nd >= 256) return;
	dumped[nd++] = ras;
	rw::Image *img = ras->toImage();
	if (!img) return;
	char path[1024]; snprintf(path, sizeof path, "%s/ptank_%s.png", dir, name);
	if (img->depth <= 8) img->unpalettize(true);
	if (img->depth < 32) img->convertTo32();
	rw::writePNG(img, path);
	img->destroy();
	(void)a;
}
static void ptankRender(rw::Atomic *a)
{
	RpPTankAtomicExtPrv *e = extOf(a);
	if (!e || e->actPCount <= 0) return;
	rw::Camera *cam = rw::engine->currentCamera;
	if (!cam) return;
	RpPTankData *pd = &e->publicData;
	const unsigned char *pos = pd->clusters[0].data, *col = pd->clusters[1].data, *siz = pd->clusters[2].data, *uv = pd->clusters[7].data;
	if (!pos) return;
	rw::Matrix *cm = cam->getFrame()->getLTM();
	rw::V3d right = { -cm->right.x, -cm->right.y, -cm->right.z }, up = cm->up;
	rw::Material *m = (rw::Material *)pd->userData;
	rw::Texture *tex = m ? m->texture : 0;
	rw::Raster *ras = tex ? tex->raster : 0;
	// rpPTankD3D8RenderCallBack 0x57c520
	RwD3D8SetRenderState(24 /* D3DRS_ALPHAREF */, 1);
	RwD3D8SetTexture(tex, 0);
	unsigned oldSrc = 0, oldDst = 0;
	if (pd->vertexAlphaBlend) {
		RwRenderStateGet(10, &oldSrc); RwRenderStateSet(10, (void *)(uintptr_t)pd->srcBlend);
		RwRenderStateGet(11, &oldDst); RwRenderStateSet(11, (void *)(uintptr_t)pd->dstBlend);
		RwRenderStateSet(12 /* rwRENDERSTATEVERTEXALPHAENABLE */, (void *)1);
	} else
		RwRenderStateSet(12, (void *)0);
	enum { BATCH = 256 };
	static rw::RWDEVICE::Im3DVertex verts[BATCH * 4];
	static rw::uint16 idx[BATCH * 6];
	int n = e->actPCount, done = 0;
	while (done < n) {
		int cnt = n - done < BATCH ? n - done : BATCH;
		for (int i = 0; i < cnt; i++) {
			int p = done + i;
			const float *P = (const float *)(pos + p * pd->clusters[0].stride);
			float sx = pd->cSize.x, sy = pd->cSize.y;
			if (siz) { const float *S = (const float *)(siz + p * pd->clusters[2].stride); sx = S[0]; sy = S[1]; }
			rw::RGBA c = pd->cColor;
			if (col) c = *(const rw::RGBA *)(col + p * pd->clusters[1].stride);
			float u0 = pd->cUV[0].u, v0 = pd->cUV[0].v, u1 = pd->cUV[1].u, v1 = pd->cUV[1].v;
			if (uv) { const float *U = (const float *)(uv + p * pd->clusters[7].stride); u0 = U[0]; v0 = U[1]; u1 = U[2]; v1 = U[3]; }
			if (!uv && u0 == 0.0f && v0 == 0.0f && u1 == 0.0f && v1 == 0.0f) { u1 = 1.0f; v1 = 1.0f; }
			float hx = sx * 0.5f, hy = sy * 0.5f;
			rw::RWDEVICE::Im3DVertex *v = &verts[i * 4];
			static const float sgn[4][2] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };    // corner offsets in units of (R, U)
			for (int k = 0; k < 4; k++) {
				v[k].setX(P[0] + right.x * hx * sgn[k][0] + up.x * hy * sgn[k][1]);
				v[k].setY(P[1] + right.y * hx * sgn[k][0] + up.y * hy * sgn[k][1]);
				v[k].setZ(P[2] + right.z * hx * sgn[k][0] + up.z * hy * sgn[k][1]);
				v[k].setColor(c.red, c.green, c.blue, c.alpha);
				v[k].setU(sgn[k][0] < 0 ? u0 : u1);
				v[k].setV(sgn[k][1] < 0 ? v1 : v0);
			}
			rw::uint16 *t = &idx[i * 6]; rw::uint16 b = (rw::uint16)(i * 4);
			t[0] = b; t[1] = b + 1; t[2] = b + 2; t[3] = b; t[4] = b + 2; t[5] = b + 3;
		}
		rw::im3d::Transform(verts, cnt * 4, nil, rw::im3d::VERTEXUV);
		rw::im3d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, idx, cnt * 6);
		rw::im3d::End();
		done += cnt;
	}
	if (pd->vertexAlphaBlend) { RwRenderStateSet(10, (void *)(uintptr_t)oldSrc); RwRenderStateSet(11, (void *)(uintptr_t)oldDst); }
	ptankFxLog(a, e, tex, ras);
	if (g_ptankRenderLogged < 4) { g_ptankRenderLogged++; CLOG("ptank: drew %d/%d particles (raster %p)\n", n, e->maxPCount, (void *)ras); }
}

extern "C" int RpPTankPluginAttach(void)
{
	g_ptankPluginOffset = rw::Atomic::registerPlugin(sizeof(void *), PTANK_ID, ptankCtor, ptankDtor, 0);
	CLOG("rwcore: RpPTankPluginAttach (offset %d)\n", g_ptankPluginOffset);
	return 1;
}

extern "C" rw::Atomic *RpPTankAtomicCreate(int maxPCount, int dataFlags, int platFlags)
{
	rw::Atomic *a = rw::Atomic::create();
	if (!a) return 0;
	RpPTankAtomicExtPrv *e = (RpPTankAtomicExtPrv *)calloc(1, sizeof(RpPTankAtomicExtPrv));
	extOf(a) = e;
	e->maxPCount = maxPCount; e->actPCount = 0; e->isAStructure = 0;
	e->platFlags = platFlags;
	e->publicData.format.dataFlags = dataFlags;
	e->publicData.srcBlend = 5; e->publicData.dstBlend = 6;      // rwBLENDSRCALPHA / rwBLENDINVSRCALPHA
	size_t total = 0; int n = 0;
	for (int i = 0; i < 9; i++) if (dataFlags & (1 << i)) { total += (size_t)maxPCount * kClusterStride[i] + 16; n++; }
	e->publicData.format.numClusters = n;
	unsigned char *buf = (unsigned char *)calloc(1, total + 16);
	e->rawdata = buf;
	unsigned char *p = (unsigned char *)(((uintptr_t)buf + 15) & ~(uintptr_t)15);
	e->publicData.data = p;
	for (int i = 0; i < 9; i++) if (dataFlags & (1 << i)) {
		e->publicData.clusters[i].data = p; e->publicData.clusters[i].stride = kClusterStride[i];
		p += (((size_t)maxPCount * kClusterStride[i]) + 15) & ~(size_t)15;
	}
// RpPTankAtomicCreate gives the atomic an empty geometry with one material; the game reaches it via atomic+0x18 -> +0x20
// (TexPartLike::FUN_0044cb00)
	rw::Geometry *g = rw::Geometry::create(0, 0, 0);
	rw::Material *m = rw::Material::create();
	g->matList.appendMaterial(m);
	a->setGeometry(g, 0);
	g->destroy();                                       // the atomic holds the reference now
	e->publicData.userData = m;
	a->setRenderCB(ptankRender);
	CLOG("ptank: RpPTankAtomicCreate(%d, 0x%x, 0x%x) = %p\n", maxPCount, dataFlags, platFlags, (void *)a);
	return a;
}

extern "C" int RpAtomicDestroy(rw::Atomic *a);                 // rwcore.cpp: removes the world sector ties first
// 0x57ba80: goes through the shim's RpAtomicDestroy, which removes the sector ties of an atomic added with RpWorldAddAtomic (the pool
// Tank's destructor does not remove it).
extern "C" int RpPTankAtomicDestroy(rw::Atomic *a)
{
	if (!a) return 0;
	return RpAtomicDestroy(a);
}

// RpPTankAtomicLock (0x57bb10): a data flag the tank lacks zeroes dst->data / stride and returns 0. Otherwise dst is the flag's
// cluster, stride is overridden by format.stride (publicData+0x54) when non-zero, and a write lock (0x40000000) or's lockFlag |
// dataFlags into ext+0x3C; RpPTankAtomicUnlock (0x57bdd0) moves that into ext+0x40 (instFlags). The cluster is picked by lowest set
// bit; the game passes single flags, so this matches its switch.
extern "C" int RpPTankAtomicLock(rw::Atomic *a, RpPTankLockStruct *dst, unsigned dataFlags, int lockFlag)
{
	RpPTankAtomicExtPrv *e = a ? extOf(a) : 0;
	if (!e || !dst) return 0;
	if (!(e->publicData.format.dataFlags & dataFlags)) { dst->data = 0; dst->stride = 0; return 0; }
	for (int i = 0; i < 9; i++) if (dataFlags & (1 << i)) { *dst = e->publicData.clusters[i]; break; }
	if (e->publicData.format.stride) dst->stride = e->publicData.format.stride;
	if (dst->data && (lockFlag & 0x40000000)) e->lockFlags |= (unsigned)lockFlag | dataFlags;
	return 1;
}
extern "C" int rpptank_static_57bb10(rw::Atomic *a, RpPTankLockStruct *dst, int dataFlags, int lockFlag) { return RpPTankAtomicLock(a, dst, (unsigned)dataFlags, lockFlag); }
extern "C" rw::Atomic *RpPTankAtomicUnlock(rw::Atomic *a) { RpPTankAtomicExtPrv *e = a ? extOf(a) : 0; if (e) { e->instFlags |= e->lockFlags; e->lockFlags = 0; } return a; }
extern "C" rw::Atomic *rpptank_static_57bdd0(rw::Atomic *a) { return RpPTankAtomicUnlock(a); }

extern "C" rw::Atomic *RpPTankAtomicSetTexture(rw::Atomic *a, rw::Texture *t)
{
	RpPTankAtomicExtPrv *e = a ? extOf(a) : 0;
	if (e && e->publicData.userData) ((rw::Material *)e->publicData.userData)->setTexture(t);
	return a;
}
extern "C" rw::Atomic *RpPTankAtomicSetBlendModes(rw::Atomic *a, unsigned src, unsigned dst)
{
	RpPTankAtomicExtPrv *e = a ? extOf(a) : 0;
	if (e) { e->publicData.srcBlend = src; e->publicData.dstBlend = dst; e->instFlags |= 0x10000000; }
	return a;
}
extern "C" rw::Atomic *RpPTankAtomicSetVertexAlpha(rw::Atomic *a, int on)
{
	RpPTankAtomicExtPrv *e = a ? extOf(a) : 0;
	if (e) e->publicData.vertexAlphaBlend = on;
	return a;
}
extern "C" int RpPTankAtomicGetActiveParticlesCount(rw::Atomic *a) { RpPTankAtomicExtPrv *e = a ? extOf(a) : 0; return e ? e->actPCount : 0; }
extern "C" int RpPTankAtomicGetMaximumParticlesCount(rw::Atomic *a) { RpPTankAtomicExtPrv *e = a ? extOf(a) : 0; return e ? e->maxPCount : 0; }
extern "C" void RpPTankAtomicSetActiveParticlesCount(rw::Atomic *a, int n) { RpPTankAtomicExtPrv *e = a ? extOf(a) : 0; if (e) { e->actPCount = n; e->instFlags |= 0x800000; } }
