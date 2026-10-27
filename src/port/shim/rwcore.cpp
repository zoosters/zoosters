// RenderWare 3.6 core over librw: engine, files, plugin attach table (g_pluginList 0x5d5cf0, g_imageFormats 0x5d5d6c), frames,
// cameras, rasters, images, clumps, HAnim, RtAnim. RW 3.6 RwFrame is librw's rw::Frame (same layout); cameras and rasters are RW 3.6
// layout structs (the game reads camera->frameBuffer at +0x60, raster->width at +0xc). The linker routes every C++ spelling of these
// names in the game to the C functions here.
#include <stdio.h>
#include "../debugui/debugui.h"   // PORT_DBG_HIDDEN (0 unless ZOOSTERS_DEBUGUI)
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <float.h>
#include "../os/native_path.h"   // PORT_NATIVE_PATH
#if defined(__aarch64__) || defined(_M_ARM64)
#include <stdint.h>
#else
#include <xmmintrin.h>
#endif
#define OFF(s, m) __builtin_offsetof(s, m)
#include <malloc.h>
#include <rw.h>
#include "../compat/rw_null_device.h"   // headless build: device vertex types (no-op on d3d9)
#include "../../dev/common/mathlog_core.h"
#include "rwlayouts.h"   // RwGlobalsShim: the engine globals the game reads (renderFrame, curCamera, curWorld by member, not +8 / +4 offsets)
static void gameHanimInterpCB(void *vout, void *vin1, void *vin2, float t, void *custom);   // below (the game's HAnim key frame interpolation)
#define PORT_HEAPCHK(where) do { static int bad; static const bool on = getenv("PORT_HEAPCHK") != 0; if (on && !bad && _heapchk() != _HEAPOK) { bad = 1; if (port_log) { fprintf(port_log, "HEAP CHECK FAILED in %s\n", where); fflush(port_log); } } } while (0)

extern "C" FILE *port_log;
extern "C++" rw::Texture *port_find_texture(const char *name);
extern "C" void port_dump_classmap(void);
static void testTriangle(rw::Camera *cam);
#include "../provisional/game_access.h"   // the game's globals behind accessors (real headers in game_access.cpp)
#define g_shadowCamera (PortGame_ShadowCamera())
#define g_sm_pOpaqueWorld (PortGame_OpaqueWorld())
#define g_worldSlot29728 (PortGame_TranslucentWorld())
#define g_worldSlot29734 (PortGame_SkyWorld())
#define g_worldSlot29738 (PortGame_FogWorld())
#define g_planeList413 (PortGame_PlaneList())
#define g_gameCamDst (PortGame_CamDst())
#define g_shadowRender (PortGame_ShadowRender())
extern bool g_detailedShadows;     // 0x606cc4
extern unsigned g_renderQueueCount;   // 0x618118
extern float g_frameDt, g_frameRate;   // 0x618318, 0x618314
extern bool g_gameCamValid;        // 0x6182d4
extern "C" int port_draw_im2d, port_draw_im3d, port_draw_d3d, port_draw_world, port_draw_atomic;
extern "C" int port_capture_backbuffer(const char *path);
#ifdef ZOOSTERS_DEBUGUI
static char g_pendingShot[640];   // a PNG path to save with this frame (backends where port_capture_backbuffer has no render target to read)
extern "C" void port_capture_this_frame(const char *path) { snprintf(g_pendingShot, sizeof g_pendingShot, "%s", path); }
#else
static char g_pendingShot[1];
#endif
#define CLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)

extern "C" void *port_engine_open(void);
extern "C" void *RwEngineInstance;      // src/game/include/rw/rwcore.h declares it untyped with C linkage; defined in rwshim.cpp
static void *g_globals;
static int g_vidW = 800, g_vidH = 600;
extern "C" void port_set_video_mode(int w, int h) { PORT_HEAPCHK("port_set_video_mode"); g_vidW = w; g_vidH = h; }

// ---------------------------------------------------------------- RW 3.6 layouts = librw's objects (patch 0002)
struct RwV2dS { float x, y; };
struct RwRectS { int x, y, w, h; };
struct RwRGBAS { unsigned char r, g, b, a; };
struct RwVideoModeS { int width, height, depth, flags, refRate, format; };
typedef rw::Raster RwRasterS;
typedef rw::Camera RwCameraS;
typedef rw::Image RwImageS;
static_assert(sizeof(void *) != 4 || (OFF(rw::Raster, width) == 0xc && OFF(rw::Raster, cType) == 0x20 && OFF(rw::Raster, originalPixels) == 0x24), "RwRaster layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Camera, frameBuffer) == 0x60 && OFF(rw::Camera, nearPlane) == 0x80 && OFF(rw::Camera, frustumCorners) == 0x124), "RwCamera layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Atomic, geometry) == 0x18 && OFF(rw::Atomic, clump) == 0x3c && OFF(rw::Atomic, renderCB) == 0x48), "RpAtomic layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Clump, lights) == 0x10 && OFF(rw::Clump, inWorld) == 0x20), "RpClump layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Geometry, numTriangles) == 0x10 && OFF(rw::Geometry, matList) == 0x20 && OFF(rw::Geometry, triangles) == 0x2c && OFF(rw::Geometry, morphTargets) == 0x5c), "RpGeometry layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Material, pipeline) == 8 && OFF(rw::Material, surfaceProps) == 0xc), "RpMaterial layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Light, radius) == 0x14 && OFF(rw::Light, color) == 0x18), "RpLight layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Frame, child) == 0x98 && OFF(rw::Frame, ltm) == 0x50), "RwFrame layout");
static_assert(sizeof(void *) != 4 || (OFF(rw::Texture, name) == 0x10 && OFF(rw::Texture, filterAddressing) == 0x50), "RwTexture layout");
// the camera's six frustum planes for the game's yon-box walk (src/game/tfb/render/yonbox.cpp; RW 3.6 x86 reads them at +0x94)
static_assert(sizeof(rw::FrustumPlane) == 0x14, "RwFrustumPlane");
extern "C" const void *port_RwCameraFrustumPlanes(const void *camera) { return ((const rw::Camera *)camera)->frustumPlanes; }
static inline void rasterSyncBytes(rw::Raster *r)      // the RW byte fields the game reads, from librw's int fields
{
	if (!r) return;
	r->cType = (rw::uint8)r->type; r->cFlags = (rw::uint8)(r->flags & 0xf8); r->cPrivateFlags = (rw::uint8)r->privateFlags; r->cFormat = (rw::uint8)(r->format >> 8);
}

// ---------------------------------------------------------------- headless rasters (librw NULL driver has none)
static rw::Raster *nullRasterCreate(rw::Raster *r)
{
	if (r->depth == 0) r->depth = 32;
	r->stride = r->width * ((r->depth + 7) / 8);
	r->pixels = 0;
	return r;
}
static rw::uint8 *nullRasterLock(rw::Raster *r, rw::int32 level, rw::int32 mode)
{
	(void)level; (void)mode;
	if (!r->pixels && r->width > 0 && r->height > 0) { r->pixels = (rw::uint8 *)calloc(r->height, r->stride); r->originalPixels = r->pixels; }
	return r->pixels;
}
static void nullRasterUnlock(rw::Raster *r, rw::int32 level) { (void)r; (void)level; }
static rw::int32 nullRasterNumLevels(rw::Raster *r) { (void)r; return 1; }
static rw::bool32 nullRasterFromImage(rw::Raster *r, rw::Image *img) { (void)r; (void)img; return 1; }
static rw::Image *nullRasterToImage(rw::Raster *r) { (void)r; return 0; }

// ---------------------------------------------------------------- engine
static int g_engineState;   // 0 none, 1 init, 2 open, 3 started
extern "C" int RwEngineInit(void *memfuncs, int flags, int arena)
{ PORT_HEAPCHK("RwEngineInit");
	CLOG("rwcore: RwEngineInit(memfuncs=%p flags=%x arena=%d)\n", memfuncs, flags, arena);
	if (g_engineState) return 1;
#ifdef RW_D3D9
	rw::platform = rw::PLATFORM_D3D9;
#elif defined(RW_GL3)
	rw::platform = rw::PLATFORM_GL3;
#else
	rw::platform = rw::PLATFORM_NULL;
#endif
	rw::bool32 ok = rw::Engine::init();
	CLOG("rwcore: Engine::init -> %d, engine %p, platform %d\n", ok, rw::engine, rw::platform);
	if (!ok) return 0;
	rw::registerMeshPlugin();
	rw::registerNativeDataPlugin();
	rw::registerAtomicRightsPlugin();
	rw::registerMaterialRightsPlugin();
	rw::registerSkinPlugin();
	rw::registerUserDataPlugin();
	rw::registerHAnimPlugin();
	if (rw::AnimInterpolatorInfo *hi = rw::AnimInterpolatorInfo::find(1)) hi->interpCB = gameHanimInterpCB;   // RpHAnimKeyFrameInterpolate 0x557000
	rw::registerMatFXPlugin();
	rw::registerUVAnimPlugin();
	g_engineState = 1;
	return 1;
}
#ifdef RW_GL3
extern "C" SDL_Window *port_sdl_window(void);     // port/provisional/window_sdl.cpp
#endif
struct PortEngineOpenParams { void *window; };          // librw d3d9: EngineOpenParams { HWND window; }
extern "C" int RwEngineOpen(void **params)
{ PORT_HEAPCHK("RwEngineOpen");
	CLOG("rwcore: RwEngineOpen(displayID=%p)\n", params ? params[0] : 0);
	if (g_engineState >= 2) return 1;
#ifdef RW_GL3
	// librw gl3 on SDL3: the GL context goes on the game's SDL window (librw patch 0016); without one librw opens its own at Engine::start
	static SDL_Window *s_sdlWindow;
	s_sdlWindow = port_sdl_window();
	rw::EngineOpenParams op = {};
	op.window = &s_sdlWindow; op.fullscreen = 0; op.width = g_vidW; op.height = g_vidH; op.windowtitle = "Madagascar (port, OpenGL)";
	if (!rw::Engine::open(&op)) { CLOG("rwcore: Engine::open failed\n"); return 0; }
#else
	PortEngineOpenParams op = { params ? params[0] : 0 };
	if (!rw::Engine::open(rw::platform == rw::PLATFORM_NULL ? nil : (rw::EngineOpenParams *)&op)) { CLOG("rwcore: Engine::open failed\n"); return 0; }
#endif
	// Image formats are registered in RwEngineStart: librw's image module data exists only once Engine::start has opened the plugins
#ifdef RW_NULL
	// librw's NULL driver has no rasters at all (rasterCreate asserts): headless rasters are plain memory, pixels on demand
	rw::engine->driver[rw::PLATFORM_NULL]->rasterCreate = nullRasterCreate;
	rw::engine->driver[rw::PLATFORM_NULL]->rasterLock = nullRasterLock;
	rw::engine->driver[rw::PLATFORM_NULL]->rasterUnlock = nullRasterUnlock;
	rw::engine->driver[rw::PLATFORM_NULL]->rasterNumLevels = nullRasterNumLevels;
	rw::engine->driver[rw::PLATFORM_NULL]->rasterFromImage = nullRasterFromImage;
	rw::engine->driver[rw::PLATFORM_NULL]->rasterToImage = nullRasterToImage;
#endif
	g_engineState = 2;
	return 1;
}
extern "C" int RwEngineStart(void)
{ PORT_HEAPCHK("RwEngineStart");
	CLOG("rwcore: RwEngineStart\n");
	if (g_engineState >= 3) return 1;
	if (!rw::Engine::start()) { CLOG("rwcore: Engine::start failed\n"); return 0; }
	// RwImageRead / RwTextureRead: librw's readers. Register after Engine::start: its plugin constructor (imageOpen) clears the format
	// table, and before it the table is uninitialised memory (registering in RwEngineOpen crashed on a garbage count). The find
	// callback stays librw's default = TextureDefaultFind (0x4de110, patch 0009).
	rw::Image::registerFileFormat("tga", rw::readTGA, rw::writeTGA);
	rw::Image::registerFileFormat("bmp", rw::readBMP, rw::writeBMP);
	rw::Image::registerFileFormat("png", rw::readPNG, rw::writePNG);
	// _rwProcessorInitialize (device start) puts the x87 into single precision and, with SSE, sets MXCSR flush-to-zero (0x8000); the
	// headless build has no D3D device to do it. arm64: FPCR.FZ (bit 24) also flushes denormal
	// inputs (x86 would need DAZ, which RW does not set); only denormals not from a flushed operation differ.
#if defined(__aarch64__)
	uint64_t fpcr;
	__asm__ volatile("mrs %0, fpcr" : "=r"(fpcr));
	fpcr |= (uint64_t)1 << 24;
	__asm__ volatile("msr fpcr, %0" : : "r"(fpcr));
	CLOG("rwcore: FPU as RW's device start: arm64 flush-to-zero (fpcr %08llx)\n", (unsigned long long)fpcr);
#elif defined(_WIN32)
	_control87(_PC_24, _MCW_PC);
	_mm_setcsr(_mm_getcsr() | 0x8000);
	CLOG("rwcore: FPU as RW's device start: x87 single precision, SSE flush-to-zero (control %08x, mxcsr %08x)\n", _control87(0, 0), _mm_getcsr());
#else
	_mm_setcsr(_mm_getcsr() | 0x8000);   // no _control87 here; x86-64 floats are SSE
	CLOG("rwcore: FPU as RW's device start: SSE flush-to-zero (mxcsr %08x)\n", _mm_getcsr());
#endif
	g_globals = port_engine_open();
	((RwGlobalsShim *)g_globals)->renderFrame = 1;               // RW world module open: renderFrame = 1
	g_engineState = 3;
	return 1;
}
extern "C" int RwEngineStop(void) { PORT_HEAPCHK("RwEngineStop"); CLOG("rwcore: RwEngineStop\n"); return 1; }
extern "C" int RwEngineClose(void) { PORT_HEAPCHK("RwEngineClose"); CLOG("rwcore: RwEngineClose\n"); return 1; }
extern "C" int RwEngineTerm(void) { PORT_HEAPCHK("RwEngineTerm"); CLOG("rwcore: RwEngineTerm\n"); return 1; }
// Video modes: headless = one made-up mode; d3d9 = librw's mode 0 (windowed current mode) reported with the window size and the
// exclusive flag so the game's SelectVideoMode (score: flags == 1, size match) picks it; the back buffer takes the window's client
// size. PORT_FULLSCREEN=1 exposes librw's real mode list.
static bool realModes(void) { const char *fs = getenv("PORT_FULLSCREEN"); return rw::platform != rw::PLATFORM_NULL && fs && fs[0] == '1'; }
extern "C" int RwEngineGetNumSubSystems(void) { PORT_HEAPCHK("RwEngineGetNumSubSystems"); return rw::platform == rw::PLATFORM_NULL ? 1 : rw::Engine::getNumSubSystems(); }
extern "C" int RwEngineGetCurrentSubSystem(void) { PORT_HEAPCHK("RwEngineGetCurrentSubSystem"); return rw::platform == rw::PLATFORM_NULL ? 0 : rw::Engine::getCurrentSubSystem(); }
extern "C" int RwEngineSetSubSystem(int n) { PORT_HEAPCHK("RwEngineSetSubSystem"); return rw::platform == rw::PLATFORM_NULL ? 1 : rw::Engine::setSubSystem(n); }
extern "C" void *RwEngineGetSubSystemInfo(char *info, int n)
{ PORT_HEAPCHK("RwEngineGetSubSystemInfo");
	if (rw::platform == rw::PLATFORM_NULL) { (void)n; strcpy(info, "port headless"); return info; }
	return rw::Engine::getSubSystemInfo((rw::SubSystemInfo *)info, n);
}
extern "C" int RwEngineGetNumVideoModes(void) { PORT_HEAPCHK("RwEngineGetNumVideoModes"); return realModes() ? rw::Engine::getNumVideoModes() : 1; }
extern "C" int RwEngineGetCurrentVideoMode(void) { PORT_HEAPCHK("RwEngineGetCurrentVideoMode"); return rw::platform == rw::PLATFORM_NULL ? 0 : rw::Engine::getCurrentVideoMode(); }
extern "C" int RwEngineSetVideoMode(int n)
{ PORT_HEAPCHK("RwEngineSetVideoMode");
	CLOG("rwcore: RwEngineSetVideoMode(%d)\n", n);
	if (rw::platform == rw::PLATFORM_NULL) return 1;
	return rw::Engine::setVideoMode(n);
}
extern "C" RwVideoModeS *RwEngineGetVideoModeInfo(RwVideoModeS *info, int n)
{ PORT_HEAPCHK("RwEngineGetVideoModeInfo");
	if (realModes()) {
		rw::VideoMode vm;
		rw::Engine::getVideoModeInfo(&vm, n);
		info->width = vm.width; info->height = vm.height; info->depth = vm.depth; info->flags = vm.flags; info->refRate = 0; info->format = 0;
		return info;
	}
	(void)n;
	info->width = g_vidW; info->height = g_vidH; info->depth = 32; info->flags = 1; info->refRate = 60; info->format = 0;
	return info;
}
static int g_presented;
extern "C" void (*port_present_hook)(void) = 0;         // port_run.cpp: the fixed clock restarts its per-frame read counter here
extern "C" const char *port_input_shot(int frame);   // port/provisional/game_stubs.cpp (PORT_INPUT shot events)
extern "C" int port_frames_presented(void) { PORT_HEAPCHK("port_frames_presented"); return g_presented; }
// PORT_TRACEFRAME=<n>: log every camera / clear / draw call of presented frame n (drawn while g_presented == n - 1)
extern "C" int port_trace_on(void)
{
	static int f = -2;
	if (f == -2) { const char *e = getenv("PORT_TRACEFRAME"); f = e ? atoi(e) : -1; }
	return f >= 0 && g_presented == f - 1;
}
// Levels < 2 select 1 (off), else min(levels, adapter maximum), used when RwEngineStart creates the device (d3d9Globals.msLevel).
// librw only offers the highest supported level, so the request is clamped to it (an in-between level the adapter rejects would fail
// CreateDevice; the game's retry with level 0 in StartUp::Open recovers). RW returns void; the game declares int and ignores it.
extern "C" int RwD3D8EngineSetMultiSamplingLevels(int n)
{ PORT_HEAPCHK("RwD3D8EngineSetMultiSamplingLevels");
	unsigned lv = n < 2 ? 1u : (unsigned)n;
#ifdef RW_D3D9
	if (lv > 1) { unsigned mx = rw::Engine::getMaxMultiSamplingLevels(); if (lv > mx) lv = mx; }
	rw::Engine::setMultiSamplingLevels(lv);
#endif
	CLOG("rwcore: RwD3D8EngineSetMultiSamplingLevels(%d) -> %u\n", n, lv);
	return 1;
}

// ---------------------------------------------------------------- textures (librw; its Texture has the RW 3.6 layout, its Raster does not)

// debugging aid: log when the shadow globals change
extern "C" void port_watch_globals(const char *where)
{
	static void *lastR = (void *)-1, *lastC = (void *)-1;
	if (g_shadowRender != lastR) { CLOG("watch: g_shadowRender %p -> %p at %s\n", lastR, g_shadowRender, where); lastR = g_shadowRender; }
	if (g_shadowCamera != lastC) { CLOG("watch: g_shadowCamera %p -> %p at %s\n", lastC, g_shadowCamera, where); lastC = g_shadowCamera; }
}
extern "C" void *RwTextureRead(const char *name, const char *mask)
{ PORT_HEAPCHK("RwTextureRead");
	port_watch_globals(name ? name : "RwTextureRead");
	rw::Texture *t = rw::Texture::read(name, mask);
	CLOG("rwcore: RwTextureRead(%s, %s) -> %p%s\n", name ? name : "", mask ? mask : "", t, t && t->raster ? "" : " (no raster)");
	return t;
}
extern "C" int RwTextureDestroy(rw::Texture *t) { PORT_HEAPCHK("RwTextureDestroy"); if (t) t->destroy(); return 1; }
extern "C" void *RwTextureCreate(void *raster) { PORT_HEAPCHK("RwTextureCreate"); rw::Texture *t = rw::Texture::create(nil); t->raster = (rw::Raster *)raster; return t; }
extern "C" void *RwTextureSetRaster(rw::Texture *t, void *raster) { PORT_HEAPCHK("RwTextureSetRaster"); t->raster = (rw::Raster *)raster; return t; }
extern "C" int RwTextureSetMipmapping(int on) { PORT_HEAPCHK("RwTextureSetMipmapping"); (void)on; return 1; }
extern "C" int RwTextureSetAutoMipmapping(int on) { PORT_HEAPCHK("RwTextureSetAutoMipmapping"); (void)on; return 1; }

// ---------------------------------------------------------------- file system
// RtWinFSystemInit (0x466460) allocates per call and fails only on a duplicate name or no memory; the game calls it once per logical
// drive (up to 26), so slots are never reused.
static char g_fsDummy[32][0x100];
static char g_fsNames[32][0x100];
static int g_nFs;
extern "C" int RtFSManagerOpen(int n) { PORT_HEAPCHK("RtFSManagerOpen"); CLOG("rwcore: RtFSManagerOpen(%d)\n", n); return 1; }
extern "C" void RtFSManagerClose(void) { PORT_HEAPCHK("RtFSManagerClose"); g_nFs = 0; }      // the manager drops its registry: names are free again
extern "C" void *RtWinFSystemInit(int maxFiles, const char *path, const char *name)
{ PORT_HEAPCHK("RtWinFSystemInit");
	CLOG("rwcore: RtWinFSystemInit(%d, %s, %s)\n", maxFiles, path ? path : "", name ? name : "");
	if (g_nFs >= 32) return 0;
	for (int i = 0; i < g_nFs; i++) if (name && !strcmp(g_fsNames[i], name)) return 0;      // duplicate name: RtFSManager rejects it
	if (name) { strncpy(g_fsNames[g_nFs], name, 0xff); g_fsNames[g_nFs][0xff] = 0; }
	return g_fsDummy[g_nFs++];
}
extern "C" int RtFSManagerRegister(void *fs) { PORT_HEAPCHK("RtFSManagerRegister"); (void)fs; return 1; }
extern "C" void RtFSManagerSetDefaultFileSystem(void *fs) { PORT_HEAPCHK("RtFSManagerSetDefaultFileSystem"); (void)fs; }
extern "C" void RtFSManagerSyncAllFiles(void) { PORT_HEAPCHK("RtFSManagerSyncAllFiles");}
extern "C" int RwFexist(const char *name)
{ PORT_HEAPCHK("RwFexist");
	char native[1024];
	FILE *f = name ? fopen(PORT_NATIVE_PATH(name, native), "rb") : 0;
	CLOG("rwcore: RwFexist(%s) = %d\n", name ? name : "", f != 0);
	if (f) fclose(f);
	return f != 0;
}

// ---------------------------------------------------------------- free lists (plain malloc)
struct FreeListS { int entrySize; };
extern "C" void RwFreeListPurgeAllFreeLists(void) { PORT_HEAPCHK("RwFreeListPurgeAllFreeLists");}
extern "C" FreeListS *RwFreeListCreate(int entrySize, int entriesPerBlock, int alignment, unsigned hint)
{ PORT_HEAPCHK("RwFreeListCreate");
	(void)entriesPerBlock; (void)alignment; (void)hint;
	FreeListS *fl = (FreeListS *)malloc(sizeof(FreeListS));
	fl->entrySize = entrySize;
	return fl;
}
extern "C" int RwFreeListDestroy(FreeListS *fl) { PORT_HEAPCHK("RwFreeListDestroy"); free(fl); return 1; }
extern "C" void *RwFreeListAlloc(FreeListS *fl, unsigned hint) { PORT_HEAPCHK("RwFreeListAlloc"); (void)hint; void *p = malloc(fl->entrySize); memset(p, 0, fl->entrySize); return p; }
extern "C" FreeListS *RwFreeListFree(FreeListS *fl, void *p) { PORT_HEAPCHK("RwFreeListFree"); free(p); return fl; }
extern "C" void RwFree(void *p) { PORT_HEAPCHK("RwFree"); free(p); }

// ---------------------------------------------------------------- plugin attaches (RW library ones are no-ops: librw registered its own)
extern "C" int RpPTankPluginAttach(void);   // port/shim/rwptank.cpp
// _rwSqrt / _rwInvSqrt (0x4d77f0 / 0x4d7850): RW's table square roots (11 bits of mantissa, IEEE-754 bit tricks on 2 x 2048 entries
// built at module open: sqrtf; the inverse table as InvSqrtTableCreate 0x4d7ce0: sqrt and divide in double, rounded once to float). 0
// -> 0. Results differ from sqrtf by up to ~2^-11 relative, which camera and collision code sees.
static unsigned g_rwSqrtTab[4096], g_rwInvSqrtTab[4096];
static void port_rwSqrtTabInit(void)
{
	static int done; if (done) return; done = 1;
	union { float f; unsigned u; } in, out;
	in.f = 1.0f;
	for (int i = 0; i < 2048; i++) { out.f = sqrtf(in.f); g_rwSqrtTab[2048 + i] = out.u + 0x00400000u - 0x20000000u; out.f = (float)(1.0 / sqrt((double)in.f)); g_rwInvSqrtTab[2048 + i] = out.u - 0x20000000u; in.u += (1u << 12); }
	for (int i = 0; i < 2048; i++) { out.f = sqrtf(in.f); g_rwSqrtTab[i] = out.u - 0x20000000u; out.f = (float)(1.0 / sqrt((double)in.f)); g_rwInvSqrtTab[i] = out.u + 0x00400000u - 0x20000000u; in.u += (1u << 12); }
}
extern "C" float _rwSqrt(float x)
{
	port_rwSqrtTabInit();
	union { float f; unsigned u; } r; r.f = x;
	if (r.u) { r.u += (1u << 11); r.u = g_rwSqrtTab[(r.u & 0x00ffffffu) >> 12] + ((0x7f800000u & r.u) >> 1); }
	return r.f;
}
extern "C" float _rwInvSqrt(float x)
{
	port_rwSqrtTabInit();
	union { float f; unsigned u; } r; r.f = x;
	if (r.u) { r.u += (1u << 11); r.u = g_rwInvSqrtTab[(r.u & 0x00ffffffu) >> 12] + ((0x7f800000u & ~r.u) >> 1); }
	return r.f;
}
#define ATTACH(Name) extern "C" int Name(void) { PORT_HEAPCHK("Name"); CLOG("rwcore: " #Name "\n"); return 1; }
extern "C" int RpWorldPluginAttach(void);   // port/shim/rwshim.cpp (camera extension)
extern "C" int RpCollisionPluginAttach(void);   // port/shim/rwcollis.inc (sector collision trees)
extern "C" void port_atom_selftest(void);   // port/shim/rwcollis.inc (PORT_ATOMSELFTEST)
ATTACH(RpPVSPluginAttach) ATTACH(RpMorphPluginAttach)
ATTACH(RpDMorphPluginAttach) ATTACH(RpRandomPluginAttach) ATTACH(RpPatchPluginAttach)
ATTACH(RpLtMapPluginAttach) ATTACH(RpLODAtomicPluginAttach) ATTACH(RpSplinePluginAttach)
ATTACH(RtAnimInitialize)
extern "C" int RpMatFXPluginAttach(void);
extern "C" int RpSkinPluginAttach(void);
extern "C" int RpHAnimPluginAttach(void);
extern "C" int RpUserDataPluginAttach(void);
// The game's own attach functions (C++ spellings as cl 13.10 mangles them; clang's MSVC mangling matches)
int TFBPlugins_Attach();                                                              // 0x45f9a0 (src/game tfb/render/tfbplugins.h)
#define FUN_0045f9a0 TFBPlugins_Attach
namespace RWS {
namespace RpAtomicToEventHandler { int PluginAttach(void); }                          // 0x5503b0
namespace RpAtomicCollisionProperties { int PluginAttach(void); }                     // 0x550cd0
namespace MaterialHelper { class CMaterialEvent { public: static int MaterialEventPluginAttach(void); }; }   // 0x44dd70
}
static int attachTFB(void) { CLOG("rwcore: TFBPlugins_Attach (FUN_0045f9a0)\n"); int r = FUN_0045f9a0(); CLOG("rwcore:  -> %d\n", r); return r; }
static int attachEv(void) { CLOG("rwcore: RpAtomicToEventHandler::PluginAttach\n"); return RWS::RpAtomicToEventHandler::PluginAttach(); }
static int attachCol(void) { CLOG("rwcore: RpAtomicCollisionProperties::PluginAttach\n"); return RWS::RpAtomicCollisionProperties::PluginAttach(); }
static int attachMat(void) { CLOG("rwcore: CMaterialEvent::MaterialEventPluginAttach\n"); return RWS::MaterialHelper::CMaterialEvent::MaterialEventPluginAttach(); }
typedef int (__cdecl *PluginAttachFn)();
// GLOBAL: 0x5d5cf0 g_pluginList (19 entries; RWS::StartUp::AttachPlugins 0x4627b0 walks it)
extern const PluginAttachFn g_pluginList[19] = {
	RpWorldPluginAttach, RpCollisionPluginAttach, RpPVSPluginAttach, RpMorphPluginAttach, RpDMorphPluginAttach,
	RpSkinPluginAttach, RpHAnimPluginAttach, RpRandomPluginAttach, RpPatchPluginAttach, RpMatFXPluginAttach,
	RpPTankPluginAttach, RpLtMapPluginAttach, RpLODAtomicPluginAttach, RpSplinePluginAttach, RpUserDataPluginAttach,
	attachTFB, attachEv, attachCol, attachMat,
};
struct ImageFormat { void *rd; void *wr; const char *name; };
// GLOBAL: 0x5d5d6c g_imageFormats (png, bmp, ras, tif; readers are not wired headless)
extern const ImageFormat g_imageFormats[4] = { { 0, 0, "png" }, { 0, 0, "bmp" }, { 0, 0, "ras" }, { 0, 0, "tif" } };
extern "C" int RwImageRegisterImageFormat(const char *ext, void *rd, void *wr) { PORT_HEAPCHK("RwImageRegisterImageFormat"); CLOG("rwcore: RwImageRegisterImageFormat(%s, %p, %p)\n", ext ? ext : "", rd, wr); return 1; }

// ---------------------------------------------------------------- frames and matrices (librw)
extern "C" rw::Frame *RwFrameCreate(void) { PORT_HEAPCHK("RwFrameCreate"); return rw::Frame::create(); }
extern "C" int RwFrameDestroy(rw::Frame *f) { PORT_HEAPCHK("RwFrameDestroy"); f->destroy(); return 1; }
extern "C" rw::Frame *RwFrameSetIdentity(rw::Frame *f) { PORT_HEAPCHK("RwFrameSetIdentity"); f->matrix.setIdentity(); f->updateObjects(); return f; }
extern "C" rw::Matrix *RwFrameGetMatrix(rw::Frame *f) { PORT_HEAPCHK("RwFrameGetMatrix"); return &f->matrix; }
extern "C" rw::Matrix *RwFrameGetMatrixM(rw::Frame *f) { PORT_HEAPCHK("RwFrameGetMatrixM"); return &f->matrix; }
static rw::Matrix *mlimpl_RwFrameGetLTM(rw::Frame *f) { PORT_HEAPCHK("RwFrameGetLTM"); return f->getLTM(); }
extern "C" rw::Frame *RwFrameGetParent(rw::Frame *f) { PORT_HEAPCHK("RwFrameGetParent"); return f->getParent(); }
extern "C" rw::Frame *RwFrameGetRoot(rw::Frame *f) { PORT_HEAPCHK("RwFrameGetRoot"); return f->root; }
extern "C" rw::Frame *RwFrameAddChild(rw::Frame *parent, rw::Frame *child) { PORT_HEAPCHK("RwFrameAddChild"); parent->addChild(child); return parent; }
extern "C" rw::Frame *RwFrameRemoveChild(rw::Frame *child) { PORT_HEAPCHK("RwFrameRemoveChild"); child->removeChild(); return child; }
extern "C" rw::Frame *RwFrameUpdateObjects(rw::Frame *f) { PORT_HEAPCHK("RwFrameUpdateObjects"); f->updateObjects(); return f; }
extern "C" rw::Frame *RwFrameForAllChildren(rw::Frame *f, rw::Frame::Callback cb, void *data) { PORT_HEAPCHK("RwFrameForAllChildren"); return f->forAllChildren(cb, data); }
extern "C" rw::Frame *RwFrameForAllObjects(rw::Frame *f, void *(*cb)(void *, void *), void *data)
{ PORT_HEAPCHK("RwFrameForAllObjects");
	FORLIST(lnk, f->objectList)
		if (cb(rw::ObjectWithFrame::fromFrame(lnk), data) == 0) break;
	return f;
}
static rw::Frame *mlimpl_RwFrameTranslate(rw::Frame *f, const rw::V3d *v, int op) { PORT_HEAPCHK("RwFrameTranslate"); f->translate(v, (rw::CombineOp)op); return f; }
static int port_animlog(void) { static int on = -1; if (on < 0) on = getenv("PORT_ANIMLOG") ? 1 : 0; return on; }
static rw::Matrix *port_matrixRotate(rw::Matrix *m, const rw::V3d *axis, float angle, int op);   // below, RwMatrixRotate (RW's formula)
static rw::Frame *mlimpl_RwFrameRotate(rw::Frame *f, const rw::V3d *axis, float angle, int op) { PORT_HEAPCHK("RwFrameRotate"); { static int n; if (port_animlog() && (n++ % 50) == 0) CLOG("animlog: RwFrameRotate #%d frame %p axis %g %g %g angle %g op %d\n", n, (void *)f, axis->x, axis->y, axis->z, angle, op); } port_matrixRotate(&f->matrix, axis, angle, op); f->updateObjects(); return f; }   // RwFrameRotate = RwMatrixRotate on the modelling matrix + RwFrameUpdateObjects
static rw::Matrix *port_matrixScale(rw::Matrix *m, const rw::V3d *v, int op);    // below, RwMatrixScale
static rw::Matrix *port_matrixOrthoNormalize(rw::Matrix *dst, const rw::Matrix *src);
// RwFrameScale: RwMatrixScale on the modelling matrix (clears IDENTITY and the type bits), then RwFrameUpdateObjects
extern "C" rw::Frame *RwFrameScale(rw::Frame *f, const rw::V3d *v, int op) { PORT_HEAPCHK("RwFrameScale"); port_matrixScale(&f->matrix, v, op); f->updateObjects(); return f; }
static rw::Frame *mlimpl_RwFrameTransform(rw::Frame *f, const rw::Matrix *m, int op) { PORT_HEAPCHK("RwFrameTransform"); f->transform(m, (rw::CombineOp)op); return f; }
// RwFrameOrthoNormalize (game 0x4d9630): RwMatrixOrthoNormalize(&modelling, &modelling), then RwFrameUpdateObjects
extern "C" rw::Frame *RwFrameOrthoNormalize(rw::Frame *f) { PORT_HEAPCHK("RwFrameOrthoNormalize"); port_matrixOrthoNormalize(&f->matrix, &f->matrix); f->updateObjects(); return f; }
extern "C" void _rwObjectHasFrameSetFrame(rw::ObjectWithFrame *obj, rw::Frame *f) { PORT_HEAPCHK("_rwObjectHasFrameSetFrame"); obj->setFrame(f); }
extern "C" int RwFrameRegisterPlugin(int size, unsigned id, void *ctor, void *dtor, void *copy)
{ PORT_HEAPCHK("RwFrameRegisterPlugin"); return rw::Frame::registerPlugin(size, id, (rw::Constructor)ctor, (rw::Destructor)dtor, (rw::CopyConstructor)copy); }

extern "C" rw::Matrix *RwMatrixCreate(void) { PORT_HEAPCHK("RwMatrixCreate"); return rw::Matrix::create(); }
extern "C" int RwMatrixDestroy(rw::Matrix *m) { PORT_HEAPCHK("RwMatrixDestroy"); m->destroy(); return 1; }
extern "C" rw::Matrix *RwMatrixSetIdentity(rw::Matrix *m) { PORT_HEAPCHK("RwMatrixSetIdentity"); m->setIdentity(); return m; }
extern "C" rw::Matrix *RwMatrixCopy(rw::Matrix *dst, const rw::Matrix *src) { PORT_HEAPCHK("RwMatrixCopy"); *dst = *src; return dst; }
static rw::Matrix *mlimpl_RwMatrixMultiply(rw::Matrix *dst, const rw::Matrix *a, const rw::Matrix *b) { PORT_HEAPCHK("RwMatrixMultiply"); return rw::Matrix::mult(dst, a, b); }
// RwMatrixInvertMacro: identity -> copy, orthonormal -> transpose, else MatrixInvertGeneric, which ends with rwMatrixSetFlags(dst, 0)
// (0x4da9f0 `mov [edx+0Ch],0`); librw's invertGeneral only clears IDENTITY and keeps stale type bits.
extern "C" rw::Matrix *RwMatrixInvert(rw::Matrix *dst, const rw::Matrix *src)
{ PORT_HEAPCHK("RwMatrixInvert");
	bool general = !(src->flags & rw::Matrix::IDENTITY) && (src->flags & rw::Matrix::TYPEMASK) != rw::Matrix::TYPEORTHONORMAL;
	rw::Matrix::invert(dst, src);
	if (general) dst->flags = 0;
	return dst;
}
// RwMatrixRotate (0x4da7a0): RW's own float formula, not librw's. The diagonal is 1 - (1 - a^2) * (1 - cos) with 1 - cos rounded to
// float first, so a quarter turn gives exactly 0 where librw keeps cos = -4.37e-8 (visible as follow.vel.z -6.9e-6 against stock 0).
// Axis normalised with the _rwInvSqrt table, radians = angle * (float)(pi / 180), CRT sin / cos in double.
extern "C" float _rwInvSqrt(float x);
static rw::Matrix *port_matrixRotateOMCS(rw::Matrix *m, const rw::V3d *a, float omc, float sn, int op)
{
	rw::V3d lead, crossed, scaled;
	lead.x = (1.0f - a->x * a->x) * omc; lead.y = (1.0f - a->y * a->y) * omc; lead.z = (1.0f - a->z * a->z) * omc;
	crossed.x = (a->y * a->z) * omc; crossed.y = (a->z * a->x) * omc; crossed.z = (a->x * a->y) * omc;
	scaled.x = a->x * sn; scaled.y = a->y * sn; scaled.z = a->z * sn;
	rw::Matrix r;
	r.right.x = 1.0f - lead.x;          r.right.y = crossed.z + scaled.z;  r.right.z = crossed.y - scaled.y;
	r.up.x = crossed.z - scaled.z;      r.up.y = 1.0f - lead.y;            r.up.z = crossed.x + scaled.x;
	r.at.x = crossed.y + scaled.y;      r.at.y = crossed.x - scaled.x;     r.at.z = 1.0f - lead.z;
	r.pos.x = 0.0f; r.pos.y = 0.0f; r.pos.z = 0.0f;
	r.flags = rw::Matrix::TYPEORTHONORMAL;
	switch (op) {
	case rw::COMBINEREPLACE: *m = r; break;
	case rw::COMBINEPRECONCAT: { rw::Matrix t; rw::Matrix::mult(&t, &r, m); *m = t; break; }
	case rw::COMBINEPOSTCONCAT: { rw::Matrix t; rw::Matrix::mult(&t, m, &r); *m = t; break; }
	default: return 0;
	}
	return m;
}
static rw::Matrix *port_matrixRotate(rw::Matrix *m, const rw::V3d *axis, float angle, int op)
{
	float radians = angle * (float)(3.1415926535897932384626433832795 / 180.0);
	float dot = axis->x * axis->x + axis->y * axis->y + axis->z * axis->z;
	float recip = _rwInvSqrt(dot);
	rw::V3d unit; unit.x = axis->x * recip; unit.y = axis->y * recip; unit.z = axis->z * recip;
	float sn = (float)sin((double)radians);
	float omc = 1.0f - (float)cos((double)radians);
	return port_matrixRotateOMCS(m, &unit, omc, sn, op);
}
static rw::Matrix *mlimpl_RwMatrixRotate(rw::Matrix *m, const rw::V3d *axis, float angle, int op) { PORT_HEAPCHK("RwMatrixRotate"); return port_matrixRotate(m, axis, angle, op); }
extern "C" rw::Matrix *RwMatrixTranslate(rw::Matrix *m, const rw::V3d *v, int op) { PORT_HEAPCHK("RwMatrixTranslate"); return m->translate(v, (rw::CombineOp)op); }
// RwMatrixScale (0x4dac50): REPLACE = identity with the scale on the diagonal, PRE = right/up/at rows scaled by x/y/z (pos untouched),
// POST = every row (pos too); afterwards IDENTITY and both type bits are cleared (and 0xFFFDFFFC).
static rw::Matrix *port_matrixScale(rw::Matrix *m, const rw::V3d *v, int op)
{
	switch (op) {
	case rw::COMBINEREPLACE:
		m->right.x = v->x; m->right.y = 0.0f; m->right.z = 0.0f;
		m->up.x = 0.0f; m->up.y = v->y; m->up.z = 0.0f;
		m->at.x = 0.0f; m->at.y = 0.0f; m->at.z = v->z;
		m->pos.x = 0.0f; m->pos.y = 0.0f; m->pos.z = 0.0f;
		break;
	case rw::COMBINEPRECONCAT:
		m->right.x *= v->x; m->right.y *= v->x; m->right.z *= v->x;
		m->up.x *= v->y; m->up.y *= v->y; m->up.z *= v->y;
		m->at.x *= v->z; m->at.y *= v->z; m->at.z *= v->z;
		break;
	case rw::COMBINEPOSTCONCAT:
		m->right.x *= v->x; m->right.y *= v->y; m->right.z *= v->z;
		m->up.x *= v->x; m->up.y *= v->y; m->up.z *= v->z;
		m->at.x *= v->x; m->at.y *= v->y; m->at.z *= v->z;
		m->pos.x *= v->x; m->pos.y *= v->y; m->pos.z *= v->z;
		break;
	default:
		return NULL;   // RW: RWERROR(E_RW_BADPARAM) and matrix = NULL; the game never passes another op
	}
	m->flags &= ~(unsigned)(rw::Matrix::IDENTITY | rw::Matrix::TYPEMASK);
	return m;
}
extern "C" rw::Matrix *RwMatrixScale(rw::Matrix *m, const rw::V3d *v, int op) { PORT_HEAPCHK("RwMatrixScale"); return port_matrixScale(m, v, op); }

// MatrixOrthoNormalize as the game has it (0x4da190): row factors are _rwInvSqrt table values and pick the pivot pair (x <= 0:
// up,at,right; else y <= 0: at,right,up; else right,up,at; no "most right-angular" search), W = U x V and V = W x U, each renormalised
// through the table. x87 53-bit arithmetic (doubles), one float store per result.
static inline float port_dot3d(const rw::V3d &a) { return (float)((double)a.x * a.x + (double)a.y * a.y + (double)a.z * a.z); }
static inline void port_scale3(rw::V3d &a, float k) { a.x *= k; a.y *= k; a.z *= k; }
static inline void port_cross3(rw::V3d &o, const rw::V3d &a, const rw::V3d &b)
{
	o.x = (float)((double)a.y * b.z - (double)a.z * b.y);
	o.y = (float)((double)a.z * b.x - (double)a.x * b.z);
	o.z = (float)((double)a.x * b.y - (double)a.y * b.x);
}
static rw::Matrix *port_matrixOrthoNormalize(rw::Matrix *dst, const rw::Matrix *src)
{
	rw::V3d right = src->right, up = src->up, at = src->at, pos = src->pos;
	float ix = _rwInvSqrt(port_dot3d(right)); port_scale3(right, ix);
	float iy = _rwInvSqrt(port_dot3d(up));    port_scale3(up, iy);
	float iz = _rwInvSqrt(port_dot3d(at));    port_scale3(at, iz);
	(void)iz;   // the game tests it too, but all three outcomes of its branch pick the same triple (right, up, at)
	rw::V3d *U, *V, *W;
	if (!(ix > 0.0f))      { U = &up;    V = &at;    W = &right; }
	else if (!(iy > 0.0f)) { U = &at;    V = &right; W = &up; }
	else                   { U = &right; V = &up;    W = &at; }
	port_cross3(*W, *U, *V);
	port_scale3(*W, _rwInvSqrt(port_dot3d(*W)));
	port_cross3(*V, *W, *U);
	port_scale3(*V, _rwInvSqrt(port_dot3d(*V)));
	dst->right = right; dst->up = up; dst->at = at; dst->pos = pos;
	dst->flags = (dst->flags | (unsigned)rw::Matrix::TYPEORTHONORMAL) & ~(unsigned)rw::Matrix::IDENTITY;
	return dst;
}
static rw::Matrix *mlimpl_RwMatrixTransform(rw::Matrix *m, const rw::Matrix *t, int op) { PORT_HEAPCHK("RwMatrixTransform"); return m->transform(t, (rw::CombineOp)op); }
extern "C" rw::Matrix *RwMatrixUpdate(rw::Matrix *m) { PORT_HEAPCHK("RwMatrixUpdate"); m->update(); return m; }
extern "C" rw::Matrix *RwMatrixOptimize(rw::Matrix *m, void *tol) { PORT_HEAPCHK("RwMatrixOptimize"); m->optimize((rw::Matrix::Tolerance *)tol); return m; }
// RW's argument order (matrix, unitAxis, oneMinusCosine, sine, combineOp)
extern "C" rw::Matrix *RwMatrixRotateOneMinusCosineSine(rw::Matrix *m, const rw::V3d *axis, float omc, float s, int op)
{ PORT_HEAPCHK("RwMatrixRotateOneMinusCosineSine");
	return port_matrixRotateOMCS(m, axis, omc, s, op);
}
extern "C" rw::V3d *RwV3dTransformPoints(rw::V3d *out, const rw::V3d *in, int n, const rw::Matrix *m)
{ PORT_HEAPCHK("RwV3dTransformPoints");
	rw::V3d::transformPoints(out, in, n, m);
	return out;
}
extern "C" rw::V3d *RwV3dTransformPoint(rw::V3d *out, const rw::V3d *in, const rw::Matrix *m) { PORT_HEAPCHK("RwV3dTransformPoint"); rw::V3d::transformPoints(out, in, 1, m); return out; }
static rw::V3d *mlimpl_RwV3dTransformVectors(rw::V3d *out, const rw::V3d *in, int n, const rw::Matrix *m) { PORT_HEAPCHK("RwV3dTransformVectors"); rw::V3d::transformVectors(out, in, n, m); return out; }
// (0x4d76d0 / 0x4d77a0): dot product in x87 53-bit precision rounded to float, then the table roots: Length = _rwSqrt(dot); Normalize
// returns _rwSqrt(dot) and scales the input by _rwInvSqrt(dot) (zero vector -> (0,0,0) and 0).
extern "C" float RwV3dLength(const rw::V3d *v) { PORT_HEAPCHK("RwV3dLength"); return _rwSqrt(port_dot3d(*v)); }
static float mlimpl_RwV3dNormalize(rw::V3d *out, const rw::V3d *in)
{ PORT_HEAPCHK("RwV3dNormalize");
	float d = port_dot3d(*in), l = _rwSqrt(d), r = _rwInvSqrt(d);
	rw::V3d i = *in; out->x = i.x * r; out->y = i.y * r; out->z = i.z * r;
	return l;
}

// PORT_MATHLOG=<file> PORT_MATHLOG_FRAMES=A-B logs these RW math calls with argument/result bits in trace frames A..B (needs
// PORT_GTRACE) (mathlog_core.h).
extern "C" int port_gtrace_trace_frame(void);   // gtrace_port.cpp
static mathlog::Window g_ml;
static int mlOn(int *tf)
{
	static int init, on;
	if (!init) { init = 1; on = g_ml.Parse(getenv("PORT_MATHLOG"), getenv("PORT_MATHLOG_FRAMES")); }
	if (!on) return 0;
	*tf = port_gtrace_trace_frame();
	return g_ml.On(*tf);
}
static const void *mlModelling(rw::Frame *f) { return (const char *)f + 0x10; }   // RW 3.6 frame layout (port patch 0001)
extern "C" rw::Frame *RwFrameRotate(rw::Frame *f, const rw::V3d *axis, float angle, int op)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwFrameRotate(f, axis, angle, op);
	unsigned in[18], out[13]; mathlog::Mat(in, mlModelling(f)); memcpy(in + 13, axis, 12); memcpy(in + 16, &angle, 4); in[17] = op;
	rw::Frame *r = mlimpl_RwFrameRotate(f, axis, angle, op);
	mathlog::Mat(out, mlModelling(f)); mathlog::Line(g_ml, tf, "RwFrameRotate", in, 18, out, 13); return r;
}
extern "C" rw::Matrix *RwMatrixRotate(rw::Matrix *m, const rw::V3d *axis, float angle, int op)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwMatrixRotate(m, axis, angle, op);
	unsigned in[18], out[13]; mathlog::Mat(in, m); memcpy(in + 13, axis, 12); memcpy(in + 16, &angle, 4); in[17] = op;
	rw::Matrix *r = mlimpl_RwMatrixRotate(m, axis, angle, op);
	mathlog::Mat(out, m); mathlog::Line(g_ml, tf, "RwMatrixRotate", in, 18, out, 13); return r;
}
extern "C" rw::Frame *RwFrameTranslate(rw::Frame *f, const rw::V3d *v, int op)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwFrameTranslate(f, v, op);
	unsigned in[17], out[13]; mathlog::Mat(in, mlModelling(f)); memcpy(in + 13, v, 12); in[16] = op;
	rw::Frame *r = mlimpl_RwFrameTranslate(f, v, op);
	mathlog::Mat(out, mlModelling(f)); mathlog::Line(g_ml, tf, "RwFrameTranslate", in, 17, out, 13); return r;
}
extern "C" rw::Frame *RwFrameTransform(rw::Frame *f, const rw::Matrix *m, int op)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwFrameTransform(f, m, op);
	unsigned in[27], out[13]; mathlog::Mat(in, mlModelling(f)); mathlog::Mat(in + 13, m); in[26] = op;
	rw::Frame *r = mlimpl_RwFrameTransform(f, m, op);
	mathlog::Mat(out, mlModelling(f)); mathlog::Line(g_ml, tf, "RwFrameTransform", in, 27, out, 13); return r;
}
extern "C" rw::Matrix *RwMatrixTransform(rw::Matrix *d, const rw::Matrix *m, int op)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwMatrixTransform(d, m, op);
	unsigned in[27], out[13]; mathlog::Mat(in, d); mathlog::Mat(in + 13, m); in[26] = op;
	rw::Matrix *r = mlimpl_RwMatrixTransform(d, m, op);
	mathlog::Mat(out, d); mathlog::Line(g_ml, tf, "RwMatrixTransform", in, 27, out, 13); return r;
}
extern "C" rw::Matrix *RwMatrixMultiply(rw::Matrix *d, const rw::Matrix *a, const rw::Matrix *b)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwMatrixMultiply(d, a, b);
	unsigned in[26], out[13]; mathlog::Mat(in, a); mathlog::Mat(in + 13, b);
	rw::Matrix *r = mlimpl_RwMatrixMultiply(d, a, b);
	mathlog::Mat(out, d); mathlog::Line(g_ml, tf, "RwMatrixMultiply", in, 26, out, 13); return r;
}
extern "C" float RwV3dNormalize(rw::V3d *o, const rw::V3d *i)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwV3dNormalize(o, i);
	unsigned in[3], out[4]; memcpy(in, i, 12);
	float r = mlimpl_RwV3dNormalize(o, i);
	memcpy(out, o, 12); memcpy(out + 3, &r, 4); mathlog::Line(g_ml, tf, "RwV3dNormalize", in, 3, out, 4); return r;
}
extern "C" rw::Matrix *RwFrameGetLTM(rw::Frame *f)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwFrameGetLTM(f);
	unsigned in[13], out[13 + 26 * 6]; memset(out, 0, sizeof out); mathlog::Mat(in, mlModelling(f));
	rw::Matrix *r = mlimpl_RwFrameGetLTM(f);
	mathlog::Mat(out, r);
	rw::Frame *cur = f;   // ancestors: modelling + LTM of up to 6 (object.parent at +4, the root points to itself), as on the stock side
	for (int k = 0; k < 6; k++) {
		rw::Frame *par = *(rw::Frame **)((char *)cur + 4);
		if (!par || par == cur) break;
		mathlog::Mat(out + 13 + 26 * k, mlModelling(par)); mathlog::Mat(out + 26 + 26 * k, (const char *)par + 0x50);
		cur = par;
	}
	mathlog::Line(g_ml, tf, "RwFrameGetLTM", in, 13, out, 13 + 26 * 6); return r;
}
extern "C" rw::V3d *RwV3dTransformVectors(rw::V3d *o, const rw::V3d *i, int n, const rw::Matrix *m)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RwV3dTransformVectors(o, i, n, m);
	unsigned in[17], out[3]; memcpy(in, i, 12); in[3] = n; mathlog::Mat(in + 4, m);
	rw::V3d *r = mlimpl_RwV3dTransformVectors(o, i, n, m);
	memcpy(out, o, 12); mathlog::Line(g_ml, tf, "RwV3dTransformVectors", in, 17, out, 3); return r;
}

// ---------------------------------------------------------------- rasters (librw)
extern "C" RwRasterS *RwRasterCreate(int w, int h, int depth, int flags)
{ PORT_HEAPCHK("RwRasterCreate");
	// No format given (rwRASTERFORMATDEFAULT): RW's driver picks one from the depth, librw gl3 returned 0 (LoadScreen_LoadImage's
	// RwRasterCreate(640, 480, 24, 0)). Such a NORMAL raster is made a TEXTURE one: the D3D8 driver keeps both as textures and gl3's
	// rasterFromImage takes only TEXTURE (else a black loading screen) (librw patch 0022).
	if ((flags & 0xF00) == 0 && ((flags & 7) == rw::Raster::NORMAL || (flags & 7) == rw::Raster::TEXTURE)) {
		flags |= depth == 32 ? rw::Raster::C8888 : depth == 24 ? rw::Raster::C888 : depth == 16 ? rw::Raster::C1555 : 0;
		flags = (flags & ~7) | rw::Raster::TEXTURE;
	}
	rw::Raster *r = rw::Raster::create(w, h, depth, flags);
	rasterSyncBytes(r);
	CLOG("rwcore: RwRasterCreate(%d, %d, %d, %x) = %p\n", w, h, depth, flags, r);
	return r;
}
extern "C" int RwRasterDestroy(RwRasterS *r) { PORT_HEAPCHK("RwRasterDestroy"); if (r) r->destroy(); return 1; }
// RwRasterSubRaster: NULL when the sub raster owns pixels; the device makes it inherit stride, depth, type and format of the parent
// (librw's subRaster does neither). rwOffsetX/Y are RW's int16 nOffsetX/Y at +0x1c.
extern "C" RwRasterS *RwRasterSubRaster(RwRasterS *sub, RwRasterS *parent, const RwRectS *rect)
{ PORT_HEAPCHK("RwRasterSubRaster");
	if (!(sub->flags & rw::Raster::DONTALLOCATE)) return 0;
	rw::Rect rc = { rect->x, rect->y, rect->w, rect->h };
	sub->subRaster(parent, &rc);
	sub->stride = parent->stride; sub->depth = parent->depth; sub->type = parent->type; sub->format = parent->format;
	sub->pixels = 0;
	sub->rwOffsetX = (rw::int16)sub->offsetX; sub->rwOffsetY = (rw::int16)sub->offsetY;
	rasterSyncBytes(sub);
	return sub;
}
#ifdef RW_GL3
// gl3: librw keeps locked pixels bottom-up (GL row order), the game reads and writes them top-down as on D3D8 (the font atlases came
// out upside down). While the game holds a lock raster->pixels points at a top-down copy; unlock writes it back flipped. The pixel
// layout differs too: D3D8 locks are A8R8G8B8 = bytes B G R A (R8G8B8 / X8R8G8B8: B G R), A1R5G5B5; librw gl3 stores R G B A / R G B
// and R5G5B5A1. The copy is converted both ways (red and blue were swapped otherwise).
struct PortFlipLock { rw::Raster *r; rw::uint8 *orig, *copy; int rows, stride, fmt; };
static PortFlipLock g_flipLocks[16];
static void flipRow(rw::uint8 *dst, const rw::uint8 *src, int stride, int fmt, int toGame)
{
	if (fmt == rw::Raster::C8888 || fmt == rw::Raster::C888) {
		int bpp = fmt == rw::Raster::C8888 ? 4 : 3;
		for (int x = 0; x + bpp <= stride; x += bpp) { dst[x] = src[x + 2]; dst[x + 1] = src[x + 1]; dst[x + 2] = src[x]; if (bpp == 4) dst[x + 3] = src[x + 3]; }
	} else if (fmt == rw::Raster::C1555) {
		const rw::uint16 *s16 = (const rw::uint16 *)src; rw::uint16 *d16 = (rw::uint16 *)dst;
		for (int x = 0; x < stride / 2; x++) {
			rw::uint16 v = s16[x];
			d16[x] = toGame ? (rw::uint16)((v & 1) << 15 | (v >> 1)) : (rw::uint16)((v & 0x7fff) << 1 | (v >> 15));   // RGBA5551 <-> ARGB1555
		}
	} else
		memcpy(dst, src, stride);
}
extern "C" unsigned char *RwRasterLock(RwRasterS *r, unsigned char level, int mode)
{ PORT_HEAPCHK("RwRasterLock");
	rw::uint8 *p = r->lock(level, mode);
	if (!p || r->stride <= 0 || r->height <= 0) return p;
	for (int i = 0; i < 16; i++) {
		PortFlipLock &l = g_flipLocks[i];
		if (l.r) continue;
		int h = r->height >> level; if (h < 1) h = 1;   // librw sets stride for the locked level; RW 3.6 rasters keep the level-0 height
		l.r = r; l.orig = p; l.rows = h; l.stride = r->stride; l.fmt = r->format & 0xf00;
		if (!l.fmt && r->width > 0)   // default format (the movie's camera texture): librw gl3 makes it R G B, or R G B A
			l.fmt = r->stride == r->width * 3 ? rw::Raster::C888 : r->stride == r->width * 4 ? rw::Raster::C8888 : 0;
		l.copy = (rw::uint8 *)malloc((size_t)h * l.stride);
		for (int y = 0; y < h; y++) flipRow(l.copy + (size_t)y * l.stride, p + (size_t)(h - 1 - y) * l.stride, l.stride, l.fmt, 1);
		r->pixels = l.copy;
		return l.copy;
	}
	return p;   // more than 16 nested locks: no flip
}
extern "C" RwRasterS *RwRasterUnlock(RwRasterS *r)
{ PORT_HEAPCHK("RwRasterUnlock");
	for (int i = 0; i < 16; i++) {
		PortFlipLock &l = g_flipLocks[i];
		if (l.r != r) continue;
		for (int y = 0; y < l.rows; y++) flipRow(l.orig + (size_t)(l.rows - 1 - y) * l.stride, l.copy + (size_t)y * l.stride, l.stride, l.fmt, 0);
		r->pixels = l.orig;
		free(l.copy);
		l.r = 0;
		break;
	}
	r->unlock(0);
	return r;
}
#else
extern "C" unsigned char *RwRasterLock(RwRasterS *r, unsigned char level, int mode) { PORT_HEAPCHK("RwRasterLock"); return r->lock(level, mode); }
extern "C" RwRasterS *RwRasterUnlock(RwRasterS *r) { PORT_HEAPCHK("RwRasterUnlock"); r->unlock(0); return r; }
#endif
extern "C" RwRasterS *RwRasterPushContext(RwRasterS *r) { PORT_HEAPCHK("RwRasterPushContext"); return rw::Raster::pushContext(r); }
extern "C" RwRasterS *RwRasterPopContext(void) { PORT_HEAPCHK("RwRasterPopContext"); return rw::Raster::popContext(); }
extern "C" RwRasterS *RwRasterGetCurrentContext(void) { PORT_HEAPCHK("RwRasterGetCurrentContext"); return rw::Raster::getCurrentContext(); }
// Raster context operations (_rwD3D8RasterClear, RasterRenderFast = no alpha, RasterRender = alpha, RasterRenderScaled): draw into the
// raster given to RwRasterPushContext (librw patch 0013)
extern "C" int RwRasterClear(int pixel) { PORT_HEAPCHK("RwRasterClear"); return rw::Raster::contextClear((rw::uint32)pixel) ? 1 : 0; }
extern "C" RwRasterS *RwRasterRenderFast(RwRasterS *r, int x, int y)
{ PORT_HEAPCHK("RwRasterRenderFast"); rw::Rect rc = { x, y, 0, 0 }; r->contextRender(&rc, 0, 0); return r; }
extern "C" RwRasterS *RwRasterRenderScaled(RwRasterS *r, const RwRectS *rect)
{ PORT_HEAPCHK("RwRasterRenderScaled"); rw::Rect rc = { rect->x, rect->y, rect->w, rect->h }; r->contextRender(&rc, 1, 1); return r; }
extern "C" RwRasterS *RwRasterRender(RwRasterS *r, int x, int y)
{ PORT_HEAPCHK("RwRasterRender"); rw::Rect rc = { x, y, 0, 0 }; r->contextRender(&rc, 0, 1); return r; }
// RW: NULL when the device fails (librw patch 0013 lets NORMAL rasters take an image and resamples a different size)
extern "C" RwRasterS *RwRasterSetFromImage(RwRasterS *r, RwImageS *img)
{ PORT_HEAPCHK("RwRasterSetFromImage"); if (!r || !img) return 0; rw::Raster *ok = r->setFromImage(img); rasterSyncBytes(r); return ok; }
extern "C" RwRasterS *RwRasterShowRaster(RwRasterS *r, void *hwnd, unsigned flags) { PORT_HEAPCHK("RwRasterShowRaster"); (void)hwnd; r->show(flags); return r; }
extern "C" int RwRasterRegisterPlugin(int size, unsigned id, void *ctor, void *dtor, void *copy)
{ PORT_HEAPCHK("RwRasterRegisterPlugin"); return rw::Raster::registerPlugin(size, id, (rw::Constructor)ctor, (rw::Destructor)dtor, (rw::CopyConstructor)copy); }

// ---------------------------------------------------------------- images (librw)
// RwImageRead: the name carries its extension ("demo.png"), which selects the reader; a name without a registered extension falls back
// to librw's probing (RW returns NULL there).
extern "C" RwImageS *RwImageRead(const char *name)
{ PORT_HEAPCHK("RwImageRead");
	rw::Image *img = rw::Image::readFile(name);
	if (!img) img = rw::Image::read(name);
	CLOG("rwcore: RwImageRead(%s) -> %p\n", name ? name : "", img);
	return img;
}
extern "C" RwImageS *RwImageCreate(int w, int h, int depth) { PORT_HEAPCHK("RwImageCreate"); return rw::Image::create(w, h, depth); }
extern "C" RwImageS *RwImageAllocatePixels(RwImageS *img) { PORT_HEAPCHK("RwImageAllocatePixels"); img->allocate(); return img; }
extern "C" int RwImageDestroy(RwImageS *img) { PORT_HEAPCHK("RwImageDestroy"); if (img) img->destroy(); return 1; }
// RwImageSetFromRaster: the raster is read-locked and converted into the 32-bit image. Only 16/24/32 bit formats; NULL when unreadable.
extern "C" RwImageS *RwImageSetFromRaster(RwImageS *img, RwRasterS *r)
{ PORT_HEAPCHK("RwImageSetFromRaster");
	if (!img || !r || img->depth != 32 || !img->pixels) return 0;
	bool locked = !(r->privateFlags & (rw::Raster::PRIVATELOCK_READ | rw::Raster::PRIVATELOCK_WRITE));
	const rw::uint8 *pix = locked ? r->lock(0, rw::Raster::LOCKREAD) : r->pixels;
	if (!pix) return 0;
	int w = r->width < img->width ? r->width : img->width, h = r->height < img->height ? r->height : img->height;
	int bpp = r->depth / 8; if (bpp < 2) bpp = 4;
	int fmt = r->format & 0xf00;
	for (int y = 0; y < h; y++) {
		const rw::uint8 *s = pix + (size_t)y * r->stride; rw::uint8 *d = img->pixels + (size_t)y * img->stride;
		for (int x = 0; x < w; x++, s += bpp, d += 4) {
			unsigned a = 255, rr, g, b;
			if (bpp == 4) { b = s[0]; g = s[1]; rr = s[2]; if (fmt != rw::Raster::C888) a = s[3]; }   // D3DFMT_A8R8G8B8 / X8R8G8B8
			else {
				unsigned p = s[0] | (s[1] << 8);
				if (fmt == rw::Raster::C565) { rr = (p >> 11 & 31) * 255 / 31; g = (p >> 5 & 63) * 255 / 63; b = (p & 31) * 255 / 31; }
				else if (fmt == rw::Raster::C4444) { a = (p >> 12 & 15) * 17; rr = (p >> 8 & 15) * 17; g = (p >> 4 & 15) * 17; b = (p & 15) * 17; }
				else { if (fmt == rw::Raster::C1555) a = (p >> 15 & 1) ? 255 : 0; rr = (p >> 10 & 31) * 255 / 31; g = (p >> 5 & 31) * 255 / 31; b = (p & 31) * 255 / 31; }
			}
			d[0] = (rw::uint8)rr; d[1] = (rw::uint8)g; d[2] = (rw::uint8)b; d[3] = (rw::uint8)a;
		}
	}
	if (locked) r->unlock(0);
	return img;
}
// RwImageWrite: the extension picks the registered writer (png, bmp, tga), NULL when unknown or unwritable
extern "C" RwImageS *RwImageWrite(RwImageS *img, const char *name)
{ PORT_HEAPCHK("RwImageWrite");
	if (!img || !name) return 0;
	bool ok = img->writeFile(name);
	CLOG("rwcore: RwImageWrite(%s) = %d\n", name, ok ? 1 : 0);
	return ok ? img : 0;
}

// ---------------------------------------------------------------- cameras (librw)
extern "C" void port_camera_sync_sectors(rw::Camera *cam);
extern "C" void *port_world_of(rw::World *rw);
// RwGlobals: curCamera +0 and curWorld +4 (the game reads both: CTFBWorld's sector walk compares curWorld with its worlds and takes
// the default sector pipeline from curWorld+0x6c)
extern "C" int port_trace_on(void);
extern "C" void port_raw_quads(void);   // rwrender.cpp
extern "C" void port_record_default_states(void);   // rwrender.cpp
extern "C" void port_rt2d_report(void);   // rt2d.cpp
#include "zoosters/os.h"
static unsigned long GetCurrentThreadId(void) { return zoosters::os::ThreadId(); }
extern "C" int port_tie_count(void); extern "C" int port_tie_atomics(void);   // rwshim.cpp
extern "C" unsigned short port_render_frame(void) { return g_globals ? ((RwGlobalsShim *)g_globals)->renderFrame : 1; }
extern "C" void port_fog_after_begin(void);
static RwCameraS *camBeginInner(RwCameraS *cam);
static RwCameraS *camBegin(RwCameraS *cam)
{
	{   // once per camera: what it renders into (render-to-texture cameras must not draw into the back buffer)
		static void *seen[32]; static int n;
		int k = 0; while (k < n && seen[k] != cam) k++;
		if (k == n && n < 32) {
			seen[n++] = cam;
			rw::Raster *fb = cam->frameBuffer, *zb = cam->zBuffer;
			CLOG("rwcore: camera %p first BeginUpdate: frameBuffer %p (%dx%d type %d fmt %x parent %p) zBuffer %p (%dx%d type %d) near %g far %g\n", (void *)cam,
			     (void *)fb, fb ? fb->width : 0, fb ? fb->height : 0, fb ? fb->type : -1, fb ? fb->format : 0, fb ? (void *)fb->parent : 0,
			     (void *)zb, zb ? zb->width : 0, zb ? zb->height : 0, zb ? zb->type : -1, (double)cam->nearPlane, (double)cam->farPlane);
		}
	}
#ifdef ZOOSTERS_DEBUGUI
	port_debugui_camera_begin(cam);                               // free camera: the game camera's frame is swapped for this render
#endif
	return camBeginInner(cam);
}
static RwCameraS *camBeginInner(RwCameraS *cam) { if (port_trace_on()) { int np = 0; for (void **n = (void **)g_planeList413; n && np < 1000; n = (void **)*n) np++; CLOG("trace: BeginUpdate cam %p; game occluder planes %d, part pool %d\n", (void *)cam, np, g_renderQueueCount); } cam->beginUpdate(); port_fog_after_begin(); port_camera_sync_sectors(cam); if (g_globals) ((RwGlobalsShim *)g_globals)->renderFrame++; /* WorldCameraBeginUpdate: renderFrame++ (RwGlobals +8) */ if (g_globals) ((RwGlobalsShim *)g_globals)->curWorld = cam->world ? port_world_of(cam->world) : 0; if (g_globals) ((RwGlobalsShim *)g_globals)->curCamera = cam; return cam; }
static RwCameraS *camEnd(RwCameraS *cam) { if (port_trace_on()) CLOG("trace: EndUpdate cam %p\n", (void *)cam); cam->endUpdate();
#ifdef ZOOSTERS_DEBUGUI
	port_debugui_camera_end(cam);                                 // free camera: the game camera's own frame back after each update
#endif
	if (g_globals) { ((RwGlobalsShim *)g_globals)->curCamera = 0; ((RwGlobalsShim *)g_globals)->curWorld = 0; } return cam; }
extern "C" RwCameraS *RwCameraCreate(void)
{ PORT_HEAPCHK("RwCameraCreate");
	rw::Camera *cam = rw::Camera::create();
	if (!cam) return 0;
	cam->recipViewWindow.x = cam->viewWindow.x ? 1.0f / cam->viewWindow.x : 0.0f;
	cam->recipViewWindow.y = cam->viewWindow.y ? 1.0f / cam->viewWindow.y : 0.0f;
	cam->setNearPlane(cam->nearPlane);     // CameraUpdateZShiftScale: zScale / zShift (+0x8c / +0x90) from near / far (librw computes them only in setNear / setFar)
	CLOG("rwcore: RwCameraCreate = %p\n", cam);
	return cam;
}
extern "C" int RwCameraDestroy(RwCameraS *cam) { PORT_HEAPCHK("RwCameraDestroy"); if (cam) cam->destroy(); return 1; }
// RwCameraClone: copies viewWindow AND recipViewWindow, near / far / fog, rasters, projection and recomputes zScale / zShift (librw's
// clone leaves recipViewWindow uninitialised)
extern "C" RwCameraS *RwCameraClone(RwCameraS *cam)
{ PORT_HEAPCHK("RwCameraClone");
	rw::Camera *c = cam->clone();
	if (!c) return 0;
	c->recipViewWindow = cam->recipViewWindow;
	c->setNearPlane(c->nearPlane);
	return c;
}
extern "C" RwCameraS *RwCameraSetFrame(RwCameraS *cam, rw::Frame *f) { PORT_HEAPCHK("RwCameraSetFrame"); cam->setFrame(f); return cam; }
extern "C" rw::Frame *RwCameraGetFrame(const RwCameraS *cam) { PORT_HEAPCHK("RwCameraGetFrame"); return (rw::Frame *)cam->object.object.parent; }
// only rwPERSPECTIVE (1) / rwPARALLEL (2) are accepted (else NULL); zScale / zShift depend on the projection and are recomputed
extern "C" RwCameraS *RwCameraSetProjection(RwCameraS *cam, int p)
{ PORT_HEAPCHK("RwCameraSetProjection");
	if (p != rw::Camera::PERSPECTIVE && p != rw::Camera::PARALLEL) return 0;
	cam->setProjection(p); cam->setNearPlane(cam->nearPlane);
	return cam;
}
extern "C" RwCameraS *RwCameraSetNearClipPlane(RwCameraS *cam, float v) { PORT_HEAPCHK("RwCameraSetNearClipPlane"); { static int n; if (n++ < 6) CLOG("rwcore: RwCameraSetNearClipPlane(%p, %g)\n", (void *)cam, (double)v); } cam->setNearPlane(v); return cam; }
extern "C" RwCameraS *RwCameraSetFarClipPlane(RwCameraS *cam, float v) { PORT_HEAPCHK("RwCameraSetFarClipPlane"); cam->setFarPlane(v); return cam; }
extern "C" RwCameraS *RwCameraSetFogDistance(RwCameraS *cam, float v) { PORT_HEAPCHK("RwCameraSetFogDistance"); cam->fogPlane = v; return cam; }
extern "C" RwCameraS *RwCameraSetViewWindow(RwCameraS *cam, const RwV2dS *vw)
{ PORT_HEAPCHK("RwCameraSetViewWindow");
	rw::V2d v = { vw->x, vw->y };
	cam->setViewWindow(&v);
	cam->recipViewWindow.x = vw->x ? 1.0f / vw->x : 0.0f; cam->recipViewWindow.y = vw->y ? 1.0f / vw->y : 0.0f;
	return cam;
}
extern "C" RwCameraS *RwCameraSetViewOffset(RwCameraS *cam, const RwV2dS *v) { PORT_HEAPCHK("RwCameraSetViewOffset"); rw::V2d o = { v->x, v->y }; cam->setViewOffset(&o); return cam; }
extern "C" RwCameraS *RwCameraSetRaster(RwCameraS *cam, RwRasterS *r) { PORT_HEAPCHK("RwCameraSetRaster"); cam->frameBuffer = r; return cam; }
extern "C" RwCameraS *RwCameraSetZRaster(RwCameraS *cam, RwRasterS *r) { PORT_HEAPCHK("RwCameraSetZRaster"); cam->zBuffer = r; return cam; }
extern "C" RwRasterS *RwCameraGetRaster(const RwCameraS *cam) { PORT_HEAPCHK("RwCameraGetRaster"); return cam->frameBuffer; }
extern "C" RwRasterS *RwCameraGetZRaster(const RwCameraS *cam) { PORT_HEAPCHK("RwCameraGetZRaster"); return cam->zBuffer; }
extern "C" int RwRasterGetWidth(const RwRasterS *r) { return r->width; }
extern "C" int RwRasterGetHeight(const RwRasterS *r) { return r->height; }
extern "C" int RwRasterGetDepth(const RwRasterS *r) { return r->depth; }
#ifdef RW_GL3
// Display change at run time (port/options/display.cpp): the camera's frame and Z rasters take the new window size, in place (other
// cameras share them: the sprite manager's). A gl3 CAMERA raster is the default framebuffer, so its size is only bookkeeping (the
// viewport comes from the window); the Z raster's texture keeps its old size and is not used with the default framebuffer.
extern "C" int port_camera_resize(RwCameraS *cam, int w, int h)
{
	if (!cam || !cam->frameBuffer || w <= 0 || h <= 0) return 0;
	rw::Raster *rs[2] = { cam->frameBuffer, cam->zBuffer };
	for (int i = 0; i < 2; i++)
		if (rs[i] && rs[i]->parent == rs[i]) { rs[i]->width = rs[i]->originalWidth = w; rs[i]->height = rs[i]->originalHeight = h; }
	g_vidW = w; g_vidH = h;
	CLOG("rwcore: camera %p rasters resized to %dx%d\n", (void *)cam, w, h);
	return 1;
}
#else
extern "C" int port_camera_resize(RwCameraS *, int, int) { return 0; }
#endif
#ifdef RW_D3D9
extern "C" int port_has_stencil(void);       // rwrender.cpp: the device's depth format has stencil bits
#endif
extern "C" RwCameraS *RwCameraClear(RwCameraS *cam, const RwRGBAS *c, int flags)
{ PORT_HEAPCHK("RwCameraClear");
	rw::RGBA col = { (rw::uint8)(c ? c->r : 0), (rw::uint8)(c ? c->g : 0), (rw::uint8)(c ? c->b : 0), (rw::uint8)(c ? c->a : 0) };
#ifdef ZOOSTERS_DEBUGUI
	extern int port_dbg_clear_key;         // debugui_camtour.cpp: the camera tour clears to magenta (pixels nothing draws over are countable)
	if (port_dbg_clear_key && (flags & 1)) { col.red = 255; col.green = 0; col.blue = 255; }
#endif
	{ static int n; if (n++ < 10) CLOG("rwcore: RwCameraClear(%p, %d %d %d %d, %d)\n", (void *)cam, col.red, col.green, col.blue, col.alpha, flags); }
	if (port_trace_on()) CLOG("trace: Clear cam %p %d %d %d %d flags %d\n", (void *)cam, col.red, col.green, col.blue, col.alpha, flags);
	// _rwD3D8CameraClear: on a camera TEXTURE the flags clear exactly what is asked, but Z and stencil are masked when the camera has
	// no Z raster (D3D9 Clear fails for a depth flag without a depth surface); on the main raster CLEARZ also clears stencil
	// (_RwHasStencilBuffer), and CLEARSTENCIL only does when the depth format has stencil bits.
	rw::uint32 f = (rw::uint32)flags & 7;
#ifdef RW_D3D9
	if (cam->frameBuffer && (cam->frameBuffer->type & 0x7f) == rw::Raster::CAMERATEXTURE) {
		if (!cam->zBuffer) f &= ~(rw::Camera::CLEARZ | rw::Camera::CLEARSTENCIL);
	} else {
		if (!port_has_stencil()) f &= ~rw::Camera::CLEARSTENCIL;
		else if (f & rw::Camera::CLEARZ) f |= rw::Camera::CLEARSTENCIL;
	}
#endif
	cam->clear(&col, f);
	return cam;
}
// PORT_STACKFILL=<hex byte>: fill 128 KB of the unused stack below this call with that byte at every present (exposes reads of
// uninitialised locals)
static __declspec(noinline) void port_stack_fill(int byte)
{
	volatile unsigned char buf[128 * 1024];
	memset((void *)buf, byte, sizeof(buf));
}
#ifdef RW_D3D9
extern "C" void port_d3dcap_after_present(int presented);
#endif
extern "C" RwCameraS *RwCameraShowRaster(RwCameraS *cam, void *hwnd, unsigned flags)
{ PORT_HEAPCHK("RwCameraShowRaster");
	{ static int fill = -2; if (fill == -2) { const char *e = getenv("PORT_STACKFILL"); fill = e ? (int)strtoul(e, 0, 16) : -1; } if (fill >= 0) port_stack_fill(fill); }
	(void)hwnd;
	g_presented++;
	port_atom_selftest();
	if (g_presented <= 8 || (g_presented % 50) == 0)
		CLOG("rwcore: frame %d presented: draws world %d atomic %d im2d %d im3d %d d3d %d; shadowCam %p detailedShadows %d; worlds opaque %p 28 %p 34 %p 38 %p pool %d dt %g fps %g camDst %p valid %d shadowRender %p thread %lu ties %d atomicsInWorlds %d\n", g_presented, port_draw_world, port_draw_atomic, port_draw_im2d, port_draw_im3d, port_draw_d3d, g_shadowCamera, g_detailedShadows, g_sm_pOpaqueWorld, g_worldSlot29728, g_worldSlot29734, g_worldSlot29738, g_renderQueueCount, g_frameDt, g_frameRate, g_gameCamDst, g_gameCamValid, g_shadowRender, (unsigned long)GetCurrentThreadId(), port_tie_count(), port_tie_atomics());
#ifdef ZOOSTERS_DEBUGUI
	port_debugui_present(cam);                                     // the overlay reads this frame's counters, draws on top, before the present
#endif
	port_draw_world = port_draw_atomic = port_draw_im2d = port_draw_im3d = port_draw_d3d = 0;
	port_watch_globals("present");
	if (g_presented == 2) port_dump_classmap();
	testTriangle(cam);
	const char *cap = getenv("PORT_CAPTURE");                      // frame numbers to save as PNG (before presenting), comma-separated
	int capThis = 0;
	for (const char *p = cap; p && *p; ) { if (atoi(p) == g_presented) capThis = 1; p = strchr(p, ','); if (p) p++; }
	port_record_default_states();
	port_raw_quads();
	if (g_presented % 250 == 0) port_rt2d_report();
	const char *shotName = port_input_shot(g_presented - 1);       // PORT_INPUT `<frame> shot <name>` (gtrace): frame N = the image of present N + 1
	if ((capThis || shotName || g_pendingShot[0]) && cam->frameBuffer) {
		char path[512], native[1024];
		const char *dir = getenv("PORT_USER_DIR");
		if (g_pendingShot[0]) {
			_snprintf(path, sizeof(path), "%s", g_pendingShot);
			g_pendingShot[0] = 0;
		} else if (shotName) {
			const char *od = getenv("PORT_GTRACE_OUT");
			if (od) _snprintf(path, sizeof(path), "%s\\%s.png", od, shotName); else _snprintf(path, sizeof(path), "%s\\..\\%s.png", dir ? dir : ".", shotName);
		} else
		_snprintf(path, sizeof(path), "%s\\..\\frame_%d.png", dir ? dir : ".", g_presented);
		if (port_capture_backbuffer(path)) {
			CLOG("rwcore: frame %d saved to %s (render target)\n", g_presented, path);
		} else {
			rw::Image *img = cam->frameBuffer->toImage();
			if (img) { rw::writePNG(img, PORT_NATIVE_PATH(path, native)); img->destroy(); CLOG("rwcore: frame %d saved to %s (toImage)\n", g_presented, path); }
			else CLOG("rwcore: frame %d: capture failed\n", g_presented);
		}
	}
	{   // PORT_SHOTTEST=<frame>: the game's screenshot sequence (h_w2d_poll.cpp) -> <user>\..\shottest_<frame>.bmp / .png
		const char *st = getenv("PORT_SHOTTEST");
		if (st && atoi(st) == g_presented && cam->frameBuffer) {
			const char *dir = getenv("PORT_USER_DIR");
			rw::Raster *fb = cam->frameBuffer;
			for (int k = 0; k < 2; k++) {
				char path[512];
				_snprintf(path, sizeof(path), "%s\\..\\shottest_%d.%s", dir ? dir : ".", g_presented, k ? "png" : "bmp");
				RwImageS *img = RwImageCreate(fb->width, fb->height, 32);
				if (img) { RwImageAllocatePixels(img); RwImageSetFromRaster(img, fb); RwImageS *w = RwImageWrite(img, path); CLOG("rwcore: PORT_SHOTTEST %s -> %p\n", path, (void *)w); RwImageDestroy(img); }
			}
		}
	}
	cam->showRaster(flags);
	if (port_present_hook) port_present_hook();
#ifdef RW_D3D9
	port_d3dcap_after_present(g_presented);                        // PORT_D3DCAP=<frame>:<path> (port/shim/d3dcap.cpp)
#endif
	return cam;
}
// PORT_TESTTRI=1: a magenta Im2D triangle in the top-left corner before each present (does librw's d3d9 backend draw at all?)
static void testTriangle(RwCameraS *cam)
{
	static int on = -1;
	if (on < 0) on = getenv("PORT_TESTTRI") ? 1 : 0;
	if (!on) return;
	rw::RWDEVICE::Im2DVertex v[3];
	float z = cam->zBuffer ? 0.5f : 0.0f; (void)z;
	float xy[3][2] = { { 10, 10 }, { 200, 10 }, { 10, 200 } };
	for (int i = 0; i < 3; i++) {
		v[i].setScreenX(xy[i][0]); v[i].setScreenY(xy[i][1]); v[i].setScreenZ(rw::im2d::GetNearZ());
		v[i].setCameraZ(cam->nearPlane); v[i].setRecipCameraZ(1.0f / cam->nearPlane);
		v[i].setColor(255, 0, 255, 255); v[i].setU(0, 1.0f); v[i].setV(0, 1.0f);
	}
	cam->beginUpdate();                                   // render states and Im2D need a current camera
	rw::SetRenderState(rw::TEXTURERASTER, 0);
	rw::SetRenderState(rw::VERTEXALPHA, 0);
	rw::SetRenderState(rw::ZTESTENABLE, 0);
	rw::im2d::RenderPrimitive(rw::PRIMTYPETRILIST, v, 3);
	rw::SetRenderState(rw::ZTESTENABLE, 1);
	cam->endUpdate();
}
extern "C" RwCameraS *RwCameraBeginUpdate(RwCameraS *cam) { PORT_HEAPCHK("RwCameraBeginUpdate"); return camBegin(cam); }
extern "C" RwCameraS *RwCameraEndUpdate(RwCameraS *cam) { PORT_HEAPCHK("RwCameraEndUpdate"); return camEnd(cam); }
extern "C" int RwCameraFrustumTestSphere(const RwCameraS *cam, const rw::Sphere *s) { PORT_HEAPCHK("RwCameraFrustumTestSphere"); return cam->frustumTestSphere(s); }
extern "C" void *port_world_of(rw::World *w);
extern "C" void *RwCameraGetWorld(const RwCameraS *cam) { PORT_HEAPCHK("RwCameraGetWorld"); void *w = port_world_of(cam->world); static int n; if (!w && n < 6) { n++; CLOG("rwcore: RwCameraGetWorld(%p) -> 0 (rw world %p)\n", (const void *)cam, (void *)cam->world); } return w; }
extern "C" int RwCameraRegisterPlugin(int size, unsigned id, void *ctor, void *dtor, void *copy)
{ PORT_HEAPCHK("RwCameraRegisterPlugin"); return rw::Camera::registerPlugin(size, id, (rw::Constructor)ctor, (rw::Destructor)dtor, (rw::CopyConstructor)copy); }

extern "C" void port_realign(void *stream, const char *who);   // rwshim.cpp
extern "C" void port_realign_asset(void *stream, const char *who);   // rwshim.cpp: skip to the end of the chunk / asset payload the game found for this reader
// Rt2d: port/shim/rt2d.cpp (stage 3c)

// RWA audio: port/shim/rwaudio.cpp The game's atexit (0x558aed) as the game calls it for function-local statics with hard-coded
// destructor thunk addresses (GameAtExit((AtExitFn)0x5c7830)): those thunks are game code the port does not have, so registrations are
// logged and dropped.
extern "C" int GameAtExit(void *fn) { PORT_HEAPCHK("GameAtExit"); CLOG("rwcore: GameAtExit(%p) ignored\n", fn); return 0; }
// ---------------------------------------------------------------- material effects (librw MatFX on librw materials)
extern "C" void *RpMatFXMaterialSetEffects(rw::Material *m, int fx) { PORT_HEAPCHK("RpMatFXMaterialSetEffects"); rw::MatFX::setEffects(m, (rw::uint32)fx); return m; }
extern "C" int RpMatFXMaterialGetEffects(const rw::Material *m) { PORT_HEAPCHK("RpMatFXMaterialGetEffects"); return (int)rw::MatFX::getEffects(m); }
extern "C" void *RpMatFXMaterialSetEnvMapFrame(rw::Material *m, rw::Frame *f) { PORT_HEAPCHK("RpMatFXMaterialSetEnvMapFrame"); rw::MatFX *x = rw::MatFX::get(m); if (x) x->setEnvFrame(f); return m; }
extern "C" void *RpMatFXMaterialSetEnvMapTexture(rw::Material *m, rw::Texture *t) { PORT_HEAPCHK("RpMatFXMaterialSetEnvMapTexture"); rw::MatFX *x = rw::MatFX::get(m); if (x) x->setEnvTexture(t); return m; }
extern "C" void *RpMatFXMaterialSetEnvMapCoefficient(rw::Material *m, float c) { PORT_HEAPCHK("RpMatFXMaterialSetEnvMapCoefficient"); rw::MatFX *x = rw::MatFX::get(m); if (x) x->setEnvCoefficient(c); return m; }
extern "C" void *RpMatFXMaterialSetEnvMapFrameBufferAlpha(rw::Material *m, int a) { PORT_HEAPCHK("RpMatFXMaterialSetEnvMapFrameBufferAlpha"); rw::MatFX *x = rw::MatFX::get(m); if (x) x->setEnvFBAlpha(a); return m; }
extern "C" void *RpMatFXMaterialSetUVTransformMatrices(rw::Material *m, rw::Matrix *base, rw::Matrix *dual) { PORT_HEAPCHK("RpMatFXMaterialSetUVTransformMatrices"); rw::MatFX *x = rw::MatFX::get(m); if (x) x->setUVTransformMatrices(base, dual); return m; }
extern "C" void *RpMatFXAtomicEnableEffects(rw::Atomic *a) { PORT_HEAPCHK("RpMatFXAtomicEnableEffects"); rw::MatFX::enableEffects(a); return a; }
extern "C" int RpMatFXAtomicQueryEffects(rw::Atomic *a) { PORT_HEAPCHK("RpMatFXAtomicQueryEffects"); return rw::MatFX::getEffects(a); }
// ---------------------------------------------------------------- clumps / atomics (librw)
extern "C" rw::Clump *RpClumpForAllAtomics(rw::Clump *c, void *(*cb)(rw::Atomic *, void *), void *data)
{ PORT_HEAPCHK("RpClumpForAllAtomics");
	FORLIST(lnk, c->atomics)
		if (cb(rw::Atomic::fromClump(lnk), data) == 0) break;
	return c;
}
extern "C" int RpClumpGetNumAtomics(rw::Clump *c) { PORT_HEAPCHK("RpClumpGetNumAtomics"); return c->countAtomics(); }
extern "C" rw::Frame *RpClumpGetFrame(rw::Clump *c) { PORT_HEAPCHK("RpClumpGetFrame"); return c->getFrame(); }
extern "C" rw::Clump *RpClumpSetFrame(rw::Clump *c, rw::Frame *f) { PORT_HEAPCHK("RpClumpSetFrame"); c->setFrame(f); return c; }
extern "C" rw::Frame *RpAtomicGetFrame(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicGetFrame"); return a->getFrame(); }
extern "C" rw::Atomic *RpAtomicSetFrame(rw::Atomic *a, rw::Frame *f) { PORT_HEAPCHK("RpAtomicSetFrame"); a->setFrame(f); return a; }
extern "C" rw::Geometry *RpAtomicGetGeometry(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicGetGeometry"); return a->geometry; }
extern "C" rw::Clump *RpAtomicGetClump(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicGetClump"); return a->clump; }
// RpAtomicGetWorldBoundingSphere: centre = local centre through the LTM, radius = local radius times the largest LTM axis length
// (librw leaves the radius unscaled). The HUD billboards (Hud2DObject::DrawBillboard) build their cull sphere from it; the unscaled
// radius made RwCameraFrustumTestSphere cull balloon_white and alex_piggy5_128. RW scales only when the LTM is not flagged
// orthonormal; an orthonormal matrix scales by 1, so it is always applied.
extern "C" rw::Sphere *port_atomic_world_sphere(rw::Atomic *a)
{
	rw::Sphere *s = &a->worldBoundingSphere;
	if (!a->getFrame()->dirty() && (a->object.object.privateFlags & rw::Atomic::WORLDBOUNDDIRTY) == 0)
		return s;
	rw::Matrix *ltm = a->getFrame()->getLTM();
	rw::V3d::transformPoints(&s->center, &a->boundingSphere.center, 1, ltm);
	float x2 = rw::dot(ltm->right, ltm->right), y2 = rw::dot(ltm->up, ltm->up), z2 = rw::dot(ltm->at, ltm->at);
	float m2 = x2 > y2 ? x2 : y2; if (z2 > m2) m2 = z2;
	s->radius = a->boundingSphere.radius * (m2 == 1.0f ? 1.0f : sqrtf(m2));
	a->object.object.privateFlags &= ~rw::Atomic::WORLDBOUNDDIRTY;
	return s;
}
extern "C" rw::Sphere *RpAtomicGetWorldBoundingSphere(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicGetWorldBoundingSphere"); return port_atomic_world_sphere(a); }
extern "C" rw::Atomic *RpAtomicSetFlags(rw::Atomic *a, unsigned f) { PORT_HEAPCHK("RpAtomicSetFlags"); a->object.object.flags = (rw::uint8)f; return a; }
extern "C" unsigned RpAtomicGetFlags(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicGetFlags"); return a->object.object.flags; }
extern "C" rw::Atomic *RpAtomicRender(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicRender"); port_draw_atomic++; a->render(); return a; }
extern "C" rw::Clump *RpClumpRender(rw::Clump *c) { PORT_HEAPCHK("RpClumpRender"); c->render(); return c; }
extern "C" rw::Clump *RpClumpAddAtomic(rw::Clump *c, rw::Atomic *a) { PORT_HEAPCHK("RpClumpAddAtomic"); c->addAtomic(a); return c; }
extern "C" rw::Clump *RpClumpRemoveAtomic(rw::Clump *c, rw::Atomic *a) { PORT_HEAPCHK("RpClumpRemoveAtomic"); c->removeAtomic(a); return c; }
extern "C" rw::Clump *RpClumpClone(rw::Clump *c) { PORT_HEAPCHK("RpClumpClone"); return c->clone(); }
// RpClumpDestroy destroys atomics with RpAtomicDestroy (WorldDeInitAtomicExt -> AtomicDestroyTies) and lights with RpLightDestroy: the
// shim's fixups run first, librw's Clump::destroy then only unlinks and frees
extern "C" void port_atomic_destroy_ties(rw::Atomic *a);   // rwshim.cpp
extern "C" void port_light_unlink_direct_frame(rw::Light *l);   // rwshim.cpp
extern "C" int RpClumpDestroy(rw::Clump *c)
{ PORT_HEAPCHK("RpClumpDestroy");
	FORLIST(lnk, c->atomics) port_atomic_destroy_ties(rw::Atomic::fromClump(lnk));
	FORLIST(lnk, c->lights) port_light_unlink_direct_frame(rw::Light::fromClump(lnk));
	c->destroy();
	return 1;
}
extern "C" rw::Atomic *RpAtomicClone(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicClone"); return a->clone(); }
extern "C" void _rwFrameSyncDirty(void) { PORT_HEAPCHK("_rwFrameSyncDirty"); static int n; if (n++ < 3) CLOG("rwcore: _rwFrameSyncDirty\n"); rw::Frame::syncDirty(); }   // 0x4bfd30-ish: the game syncs dirty frames before its own LTM reads
extern "C" rw::Camera *RpLODAtomicSetCamera(rw::Camera *c) { return c; }   // LOD plugin: the port renders every atomic at LOD 0
extern "C" rw::Material *RpMaterialSetTexture(rw::Material *m, rw::Texture *t) { PORT_HEAPCHK("RpMaterialSetTexture"); if (m) m->setTexture(t); return m; }
extern "C" rw::Texture *RpMaterialGetTexture(rw::Material *m) { return m ? m->texture : 0; }
extern "C" void port_atomic_destroy_ties(rw::Atomic *a);   // rwshim.cpp
extern "C" int RpAtomicDestroy(rw::Atomic *a) { PORT_HEAPCHK("RpAtomicDestroy"); port_atomic_destroy_ties(a); a->destroy(); return 1; }
extern "C" rw::Clump *RpClumpCreate(void) { PORT_HEAPCHK("RpClumpCreate"); return rw::Clump::create(); }
// ---------------------------------------------------------------- user data arrays (librw userdata plugin)
// RpGeometry lock/unlock (librw re-instances a geometry locked since its last instancing), material / mesh iterators
extern "C" rw::Geometry *RpGeometryLock(rw::Geometry *g, int flags) { if (g) g->lock(flags); return g; }
extern "C" rw::Geometry *RpGeometryUnlock(rw::Geometry *g) { if (g) g->unlock(); return g; }
extern "C" rw::Geometry *RpGeometryForAllMaterials(rw::Geometry *g, rw::Material *(*cb)(rw::Material *, void *), void *data)
{
	for (int i = 0; g && i < g->matList.numMaterials; i++) if (!cb(g->matList.materials[i], data)) break;
	return g;
}
extern "C" rw::Geometry *RpGeometryForAllMeshes(rw::Geometry *g, rw::Mesh *(*cb)(rw::Mesh *, rw::MeshHeader *, void *), void *data)
{
	if (!g || !g->meshHeader) return 0;      // no mesh header -> NULL
	rw::Mesh *m = g->meshHeader->getMeshes();
	for (int i = 0; i < g->meshHeader->numMeshes; i++) if (!cb(&m[i], g->meshHeader, data)) break;
	return g;
}
extern "C" rw::Geometry *RpGeometryTriangleGetVertexIndices(rw::Geometry *g, const rw::Triangle *t, unsigned short *a, unsigned short *b, unsigned short *c)
{
	if (a) *a = t->v[0]; if (b) *b = t->v[1]; if (c) *c = t->v[2];
	return g;
}
extern "C" int RpMaterialDestroy(rw::Material *m) { if (m) m->destroy(); return 1; }

// RpRandom (0x5724d0 RpRandomSeed, 0x5725a0 RpRandom, 0x572770 RandomOpen seeds 0x9a319039): additive lagged-Fibonacci over 31 words,
// front pointer 3 ahead of the rear, 310 warm-up draws
static unsigned g_rnd[31], *g_rndF = g_rnd + 3, *g_rndR = g_rnd;
static int g_rndSeeded;
extern "C" unsigned RpRandom(void);
extern "C" unsigned port_rnglog_seed(unsigned seed);               // gtrace_port.cpp: PORT_SRAND forces the seed
extern "C" void port_rnglog_rp(unsigned value, void *caller);      // gtrace_port.cpp: PORT_RNGLOG draw log
extern "C" void RpRandomSeed(unsigned seed)
{
	seed = port_rnglog_seed(seed);
	g_rndSeeded = 1;
	g_rnd[0] = seed;
	for (int i = 1; i < 31; i++) g_rnd[i] = g_rnd[i - 1] * 0x41c64e6du + 0x3039u;
	g_rndF = g_rnd + 3; g_rndR = g_rnd;
	for (int i = 0; i < 310; i++) RpRandom();
}
extern "C" unsigned RpRandom(void)
{
	if (!g_rndSeeded) RpRandomSeed(0x9a319039u);       // RandomOpen at engine start
	*g_rndF += *g_rndR;
	unsigned r = (*g_rndF >> 1) & 0x7fffffff;
	if (++g_rndF >= g_rnd + 31) { g_rndF = g_rnd; ++g_rndR; }
	else if (++g_rndR >= g_rnd + 31) g_rndR = g_rnd;
	port_rnglog_rp(r, PORT_RETURN_ADDRESS());
	return r;
}

extern "C" int RpGeometryGetUserDataArrayCount(rw::Geometry *g) { PORT_HEAPCHK("RpGeometryGetUserDataArrayCount"); return rw::UserDataArray::geometryGetCount(g); }
extern "C" rw::UserDataArray *RpGeometryGetUserDataArray(rw::Geometry *g, int n) { PORT_HEAPCHK("RpGeometryGetUserDataArray"); return rw::UserDataArray::geometryGet(g, n); }
extern "C" int RwFrameGetUserDataArrayCount(rw::Frame *f) { PORT_HEAPCHK("RwFrameGetUserDataArrayCount"); return rw::UserDataArray::frameGetCount(f); }
extern "C" rw::UserDataArray *RwFrameGetUserDataArray(rw::Frame *f, int n) { PORT_HEAPCHK("RwFrameGetUserDataArray"); return rw::UserDataArray::frameGet(f, n); }
extern "C" int RpMaterialGetUserDataArrayCount(rw::Material *m) { PORT_HEAPCHK("RpMaterialGetUserDataArrayCount"); return rw::UserDataArray::materialGetCount(m); }
extern "C" rw::UserDataArray *RpMaterialGetUserDataArray(rw::Material *m, int n) { PORT_HEAPCHK("RpMaterialGetUserDataArray"); return rw::UserDataArray::materialGet(m, n); }
extern "C" int RpUserDataArrayGetNumElements(rw::UserDataArray *a) { PORT_HEAPCHK("RpUserDataArrayGetNumElements"); return a->numElements; }
extern "C" int RpUserDataArrayGetFormat(rw::UserDataArray *a) { PORT_HEAPCHK("RpUserDataArrayGetFormat"); return (int)a->datatype; }
extern "C" const char *RpUserDataArrayGetName(rw::UserDataArray *a) { PORT_HEAPCHK("RpUserDataArrayGetName"); return a->name; }
extern "C" int RpUserDataArrayGetInt(rw::UserDataArray *a, int n) { PORT_HEAPCHK("RpUserDataArrayGetInt"); return a->getInt(n); }
extern "C" float RpUserDataArrayGetReal(rw::UserDataArray *a, int n) { PORT_HEAPCHK("RpUserDataArrayGetReal"); return a->getFloat(n); }
extern "C" const char *RpUserDataArrayGetString(rw::UserDataArray *a, int n) { PORT_HEAPCHK("RpUserDataArrayGetString"); return a->getString(n); }
// ---------------------------------------------------------------- PVS (no visibility data: everything is visible)
extern "C" int RpPVSQuery(void *world, const float *pos) { (void)world; (void)pos; return 0; }   // "has PVS data": no, so CTFBWorld walks every sector (FUN_0041b5f0) instead of the PVS walk (FUN_0041b150, rppvs_static_4b0e00)
extern "C" void *RpPVSHook(void *world) { return world; }
extern "C" void *RpPVSUnhook(void *world) { return world; }
extern "C" void *RpPVSSetViewPosition(void *world, const float *pos) { (void)pos; return world; }
extern "C" int RpPVSAtomicVisible(void *atomic) { (void)atomic; return 1; }
extern "C" int RpPVSWorldSectorVisible(void *sector) { (void)sector; return 1; }
extern "C" int RpPVSStatisticsGet(void *world, int *a, int *b) { (void)world; *a = 0; *b = 0; return 1; }

// ---------------------------------------------------------------- RtCompressedKeyFrame (RtAnim scheme 2)
// RtCompressedKeyFrameRegister 0x599d50 registers interpolated frame 0x24 B (= RpHAnimInterpFrame, so librw's apply is the same), anim
// keyframe 0x18 B, custom data 0x18 B. StreamRead 0x599240: per frame f32 time, 7 x int16 (qx qy qz qr tx ty tz), int32 byte offset of
// the previous frame (/24), then 6 floats of custom data (offset xyz, scalar xyz). Interpolate 0x597fc0: each int16 is a 1-4-11 float
// (sign, exponent bits 11..14 + 0x70, mantissa bits 0..10 << 12), a = (t - t1)/(t2 - t1), q = slerp, t = lerp(t1, t2, a) * scalar + offset.
// The game's quaternion rule (shared by RpHAnimKeyFrameInterpolate 0x557000, RtCompressedKeyFrameInterpolate 0x597fc0 and the Blend
// callbacks 0x5575d0 / 0x598c90): c = q1 . q2; if c < 0 then q2 = -q2, c = -c; only if c < 0.999: theta = acos(c), w1 = sin(w1 * theta)
// / sin(theta), w2 likewise; q = q1 * w1 + q2 * w2, NOT renormalised. librw takes acosf(c) unconditionally: the compressed quaternions
// are not unit length, so near-equal keys often give c > 1 -> NaN -> slerp returns q1 (slow rotations step). PORT_CMPLOG=1 counts the
// compressed interpolations with dot > 1 and how many librw would have left unturned.
// The acos and sin in that rule are fdlibm's single-precision acosf (RW's _rwSqrt in the |x| >= 0.5 branches) and __kernel_sinf with
// no range reduction. The x87 runs in single precision, so plain float C in the same order gives the same bits; libm acosf/sinf
// differ by 1-2 ulp.
static float gameAcos(float x)
{
	static const float pi = 3.1415925026e+00f /* 0x40490fda */, pio2_hi = 1.5707962513e+00f /* 0x3fc90fda */,
		pio2_lo = 7.5497894159e-08f /* 0x33a22168 */, pS0 = 1.6666667163e-01f, pS1 = -3.2556581497e-01f, pS2 = 2.0121252537e-01f,
		pS3 = -4.0055535734e-02f, pS4 = 7.9153501429e-04f, pS5 = 3.4793309169e-05f, qS1 = -2.4033949375e+00f, qS2 = 2.0209457874e+00f,
		qS3 = -6.8828397989e-01f, qS4 = 7.7038154006e-02f;
	int hx; memcpy(&hx, &x, 4);
	int ix = hx & 0x7fffffff;
	if (ix >= 0x3f800000) { unsigned r = hx > 0 ? 0u : 0x40490fdbu; float f; memcpy(&f, &r, 4); return f; }   // |x| >= 1: 0 or pi
	float z, p, q, r, s, w, c, df;
	if (ix < 0x3f000000) {                                                          // |x| < 0.5
		if (ix <= 0x23000000) { unsigned h = 0x3fc90fdbu; float f; memcpy(&f, &h, 4); return f; }
		z = x * x;
		p = (((((z * pS5 + pS4) * z + pS3) * z + pS2) * z + pS1) * z + pS0) * z;
		q = (((z * qS4 + qS3) * z + qS2) * z + qS1) * z + 1.0f;
		r = p / q;
		return pio2_hi - (x - (pio2_lo - x * r));
	}
	if (hx < 0) {                                                                   // x < -0.5
		z = (1.0f + x) * 0.5f;
		p = (((((z * pS5 + pS4) * z + pS3) * z + pS2) * z + pS1) * z + pS0) * z;
		q = (((z * qS4 + qS3) * z + qS2) * z + qS1) * z + 1.0f;
		s = _rwSqrt(z);
		r = p / q;
		w = r * s - pio2_lo;
		return pi - (s + w) * 2.0f;
	}
	z = (1.0f - x) * 0.5f;                                                          // x > 0.5
	s = _rwSqrt(z);
	int is; memcpy(&is, &s, 4); is &= 0xfffff000; memcpy(&df, &is, 4);
	c = (z - df * df) / (s + df);
	p = (((((z * pS5 + pS4) * z + pS3) * z + pS2) * z + pS1) * z + pS0) * z;
	q = (((z * qS4 + qS3) * z + qS2) * z + qS1) * z + 1.0f;
	r = p / q;
	w = r * s + c;
	return (df + w) * 2.0f;
}
static float gameSinK(float x)
{
	static const float S1 = -1.6666667163e-01f, S2 = 8.3333337680e-03f, S3 = -1.9841270114e-04f, S4 = 2.7557314297e-06f,
		S5 = -2.5050759689e-08f, S6 = 1.5896910177e-10f;
	float z = x * x, v = z * x;
	float r = (((z * S6 + S5) * z + S4) * z + S3) * z + S2;
	return (z * r + S1) * v + x;
}
static rw::Quat gameQuatInterp(const rw::Quat &q1, rw::Quat q2, float a)
{
	float c = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	if (c < 0.0f) { c = -c; q2.x = -q2.x; q2.y = -q2.y; q2.z = -q2.z; q2.w = -q2.w; }
	float w1 = 1.0f - a, w2 = a;
	if (c < 0.999f) {
		float theta = gameAcos(c), inv = 1.0f / gameSinK(theta);
		w1 = w1 * theta; w1 = gameSinK(w1) * inv;
		w2 = a * theta; w2 = gameSinK(w2) * inv;
	}
	rw::Quat q;
	q.x = q1.x * w1 + q2.x * w2; q.y = q1.y * w1 + q2.y * w2;
	q.z = q1.z * w1 + q2.z * w2; q.w = q1.w * w1 + q2.w * w2;
	return q;
}
// RpHAnimKeyFrameInterpolate 0x557000 (scheme 1; librw's hanimInterpCB used its own slerp): a = (t - t1) / (t2 - t1), translation t1 +
// (t2 - t1) * a, quaternion by the rule above. (The game also negates key 2's quaternion in place when the dot is < 0.)
static void gameHanimInterpCB(void *vout, void *vin1, void *vin2, float t, void *)
{
	rw::HAnimInterpFrame *out = (rw::HAnimInterpFrame *)vout;
	const rw::HAnimKeyFrame *k1 = (const rw::HAnimKeyFrame *)vin1, *k2 = (const rw::HAnimKeyFrame *)vin2;
	float a = (t - k1->time) / (k2->time - k1->time);
	out->t.x = k1->t.x + (k2->t.x - k1->t.x) * a;
	out->t.y = k1->t.y + (k2->t.y - k1->t.y) * a;
	out->t.z = k1->t.z + (k2->t.z - k1->t.z) * a;
	out->q = gameQuatInterp(k1->q, k2->q, a);
}
static int port_cmplog(void) { static int on = -1; if (on < 0) { const char *e = getenv("PORT_CMPLOG"); on = e && *e && *e != '0'; } return on; }
struct CmpKeyFrame { CmpKeyFrame *prev; float time; unsigned short q[4]; unsigned short t[3]; unsigned short pad; };
static float cmpHalf(unsigned short h)
{
	unsigned b = (unsigned)(h & 0x8000) << 16;
	if (h & 0x7fff) b |= (((unsigned)(h & 0x7800) << 12) + 0x38000000u) | ((unsigned)(h & 0x7ff) << 12);
	float f; memcpy(&f, &b, 4); return f;
}
static void cmpApplyCB(void *result, void *frame)
{
	rw::Matrix *m = (rw::Matrix *)result;
	rw::HAnimInterpFrame *f = (rw::HAnimInterpFrame *)frame;
	m->rotate(f->q, rw::COMBINEREPLACE);
	m->pos = f->t;
}
static void cmpInterpCB(void *vout, void *vin1, void *vin2, float t, void *custom)
{
	rw::HAnimInterpFrame *out = (rw::HAnimInterpFrame *)vout;
	const CmpKeyFrame *a = (const CmpKeyFrame *)vin1, *b = (const CmpKeyFrame *)vin2;
	const float *c = (const float *)custom;          // offset xyz, scalar xyz
	float d = b->time - a->time, r = d != 0.0f ? (t - a->time) / d : 0.0f;
	rw::Quat q1 = rw::makeQuat(cmpHalf(a->q[3]), cmpHalf(a->q[0]), cmpHalf(a->q[1]), cmpHalf(a->q[2]));
	rw::Quat q2 = rw::makeQuat(cmpHalf(b->q[3]), cmpHalf(b->q[0]), cmpHalf(b->q[1]), cmpHalf(b->q[2]));
	rw::V3d t1 = { cmpHalf(a->t[0]), cmpHalf(a->t[1]), cmpHalf(a->t[2]) }, t2 = { cmpHalf(b->t[0]), cmpHalf(b->t[1]), cmpHalf(b->t[2]) };
	rw::V3d tt = rw::lerp(t1, t2, r);
	out->t.x = tt.x * c[3] + c[0]; out->t.y = tt.y * c[4] + c[1]; out->t.z = tt.z * c[5] + c[2];
	out->q = gameQuatInterp(q1, q2, r);                // the game's rule (0x597fc0), not librw's slerp
	if (port_cmplog()) {
		static unsigned calls, over1, stepped, nanq;
		float c = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
		rw::Quat old = rw::slerp(q1, q2, r);            // what the port used to produce
		calls++;
		if (c > 1.0f || c < -1.0f) over1++;
		if (old.x == q1.x && old.y == q1.y && old.z == q1.z && old.w == q1.w && r > 0.01f && r < 0.99f && (q2.x != q1.x || q2.y != q1.y || q2.z != q1.z || q2.w != q1.w)) stepped++;
		if (out->q.x != out->q.x || out->q.y != out->q.y || out->q.z != out->q.z || out->q.w != out->q.w) nanq++;
		if ((calls % 20000) == 1) CLOG("cmplog: compressed interpolations %u, |dot|>1 %u, old slerp left q1 unturned (0.01<a<0.99) %u, NaN in new result %u\n", calls, over1, stepped, nanq);
	}
}
static void cmpStreamRead(rw::Stream *s, rw::Animation *anim)
{
	CmpKeyFrame *fr = (CmpKeyFrame *)anim->keyframes;
	for (int i = 0; i < anim->numFrames; i++) {
		fr[i].time = s->readF32();
		s->read16(fr[i].q, 8); s->read16(fr[i].t, 6); fr[i].pad = 0;
		int prev = s->readI32() / 24;
		fr[i].prev = &fr[prev];
	}
	s->read32(anim->customData, 24);
}
static void cmpStreamWrite(rw::Stream *s, rw::Animation *anim)
{
	CmpKeyFrame *fr = (CmpKeyFrame *)anim->keyframes;
	for (int i = 0; i < anim->numFrames; i++) {
		s->writeF32(fr[i].time); s->write16(fr[i].q, 8); s->write16(fr[i].t, 6); s->writeI32((int)(fr[i].prev - fr) * 24);
	}
	s->write32(anim->customData, 24);
}
static unsigned cmpStreamGetSize(rw::Animation *anim) { return anim->numFrames * 24 + 24; }
extern "C" int RtCompressedKeyFrameRegister(void)
{
	PORT_HEAPCHK("RtCompressedKeyFrameRegister");
	if (rw::AnimInterpolatorInfo::find(2)) return 1;
	rw::AnimInterpolatorInfo *info = rwMallocT(rw::AnimInterpolatorInfo, 1, rw::MEMDUR_GLOBAL);
	memset(info, 0, sizeof *info);
	info->id = 2; info->interpKeyFrameSize = sizeof(rw::HAnimInterpFrame); info->animKeyFrameSize = sizeof(CmpKeyFrame); info->customDataSize = 24;
	info->applyCB = cmpApplyCB; info->interpCB = cmpInterpCB;
	info->streamRead = cmpStreamRead; info->streamWrite = cmpStreamWrite; info->streamGetSize = cmpStreamGetSize;
	rw::AnimInterpolatorInfo::registerInterp(info);
	CLOG("rwcore: RtCompressedKeyFrameRegister (scheme 2 registered)\n");
	return 1;
}
// FUN_00554ea0 (0x554ea0): stores its two arguments in globals 0x632308 / 0x63230c; CAnimSet::UpdateAnim 0x449f60 calls it first
// thing. Nothing else reads them.
static void *g_cmp632308, *g_cmp63230c;
extern "C" void FUN_00554ea0(void *a, void *b) { g_cmp632308 = a; g_cmp63230c = b; }
// ---------------------------------------------------------------- HAnim / skin / animation (librw; layouts patched to RW 3.6)
extern "C" rw::HAnimHierarchy *RpHAnimFrameGetHierarchy(rw::Frame *f) { return rw::HAnimHierarchy::get(f); }
// RpHAnimFrameSetHierarchy (0x555df0): the old hierarchy of the frame loses its parentFrame (+0x14 = 0), the new one gets parentFrame
// = frame; returns TRUE (not the frame). The cutscene camera (0x41f790: SetHierarchy(frame, h); AttachFrameIndex; UpdateMatrices;
// SetHierarchy(frame, 0)) relies on parentFrame being set.
extern "C" int RpHAnimFrameSetHierarchy(rw::Frame *f, rw::HAnimHierarchy *h)
{
	rw::HAnimData *d = rw::HAnimData::get(f);
	if (d->hierarchy) d->hierarchy->parentFrame = nil;
	d->hierarchy = h;
	if (h) h->parentFrame = f;
	return 1;
}
extern "C" int RpHAnimFrameGetID(rw::Frame *f) { return rw::HAnimData::get(f)->id; }
extern "C" rw::HAnimHierarchy *RpHAnimHierarchyCreate(int numNodes, int *nodeFlags, int *nodeIDs, int flags, int maxKeySize)
{ return rw::HAnimHierarchy::create(numNodes, nodeFlags, nodeIDs, flags, maxKeySize); }
// RpHAnimHierarchyDestroy (0x555850): a SUBHIERARCHY does not free the matrices / nodeInfo it shares with its parent (librw patch
// 0010), and the hierarchy pointer of its parentFrame's HAnim data is cleared.
extern "C" int RpHAnimHierarchyDestroy(rw::HAnimHierarchy *h)
{
	rw::Frame *pf = h->parentFrame;
	h->destroy();
	if (pf) rw::HAnimData::get(pf)->hierarchy = nil;
	return 1;
}
// RpHAnimHierarchyAttach (0x555be0 -> HAnimFramesLinkToHierarchy 0x555c00): visits parentFrame and then ONLY its children
// (recursively, not siblings); every node whose id == RpHAnimFrameGetID(frame) gets that frame (last match in pre-order wins; no
// "already attached" or id >= 0 test) and RwFrameUpdateObjects(frame) is called. AttachFrameIndex (0x555cd0 -> 0x555d10) is the same
// walk for the id of node[idx] only. Both return the hierarchy. librw's attach (first unattached match, siblings searched, no
// updateObjects) differs for duplicate ids.
static void hanimLinkFrames(rw::Frame *f, rw::HAnimHierarchy *h, int onlyId, bool one)
{
	int id = rw::HAnimData::get(f)->id;
	if (!one) { for (int i = 0; i < h->numNodes; i++) if (id == h->nodeInfo[i].id) h->nodeInfo[i].frame = f; }
	else if (id == onlyId) { for (int i = 0; i < h->numNodes; i++) if (h->nodeInfo[i].id == onlyId) h->nodeInfo[i].frame = f; }
	f->updateObjects();
	for (rw::Frame *c = f->child; c; c = c->next) hanimLinkFrames(c, h, onlyId, one);
}
extern "C" rw::HAnimHierarchy *RpHAnimHierarchyAttach(rw::HAnimHierarchy *h) { if (h->parentFrame) hanimLinkFrames(h->parentFrame, h, 0, false); return h; }
extern "C" rw::HAnimHierarchy *RpHAnimHierarchyAttachFrameIndex(rw::HAnimHierarchy *h, int idx)
{
	if (h->parentFrame) hanimLinkFrames(h->parentFrame, h, h->nodeInfo[idx].id, true);
	return h;
}
// RpHAnimHierarchyUpdateMatrices (0x555f30, NOMATRICES variant 0x556820): librw's version only fills skin matrices[] and never touches
// the frames, but the game sets UPDATEMODELLINGMATRICES | UPDATELTMS (0x3000, CAnimSet::SetTarget) on every hierarchy and its rigid
// animated objects (trading cards, gates, balloons) are posed only through the frames.
// Node local matrix = key frame apply; world(i) = local(i) * parentWorld (PUSH/POP flags walk the parent stack); root = the world of
// the parent frame's parent (identity for LOCALSPACEMATRICES; a sub-hierarchy starts at its parent hierarchy's matrix); a callback
// registered with FUN_00554ea0 may replace a node (local copied, parent translation added).
// Frames: UPDATEMODELLINGMATRICES copies the local matrix into the frame (marks its objects dirty unless UPDATELTMS), UPDATELTMS
// writes the frame's LTM (matrices[i] * rootWorld) and clears SUBTREESYNCLTM / sets SUBTREESYNCOBJ.
static rw::HAnimHierarchy *mlimpl_RpHAnimHierarchyUpdateMatrices(rw::HAnimHierarchy *h)
{
	using namespace rw;
	const int32 fl = h->flags;
	const bool noMat = (fl & HAnimHierarchy::NOMATRICES) != 0;
	AnimInterpolator *anim = h->interpolator;
	Matrix identity, rootWorld;
	identity.setIdentity(); rootWorld.setIdentity();
	Matrix *rootPtr = &identity;
	bool computeWorld = false; Frame *worldFrame = nil;
	if (noMat) { computeWorld = true; worldFrame = h->parentFrame; }
	else if ((fl & HAnimHierarchy::SUBHIERARCHY) && h->rwRootParentOffset != -1 && h->parentHierarchy) {
		rootPtr = &h->parentHierarchy->matrices[h->rwRootParentOffset];
		if ((fl & HAnimHierarchy::UPDATELTMS) && (fl & HAnimHierarchy::LOCALSPACEMATRICES)) { computeWorld = true; worldFrame = h->parentHierarchy->parentFrame; }
	} else if (fl & HAnimHierarchy::LOCALSPACEMATRICES) {
		if (fl & HAnimHierarchy::UPDATELTMS) { computeWorld = true; worldFrame = h->parentFrame; }
	} else { computeWorld = true; worldFrame = h->parentFrame; rootPtr = &rootWorld; }
	if (computeWorld && worldFrame && worldFrame->getParent()) rootWorld = *worldFrame->getParent()->getLTM();
	if (noMat) rootPtr = &rootWorld;
	if ((fl & HAnimHierarchy::UPDATELTMS) && h->parentFrame && (h->parentFrame->root->object.privateFlags & Frame::HIERARCHYSYNC) == 0) {
		engine->frameDirtyList.add(&h->parentFrame->root->inDirtyList);          // the root's objects must be re-synchronised
		h->parentFrame->root->object.privateFlags |= Frame::HIERARCHYSYNCOBJ;
	}
	typedef int (*NodeCB)(Matrix *parent, int node, Matrix *local, void *user);
	NodeCB cb = (NodeCB)g_cmp632308; void *cbUser = g_cmp63230c;
	// RW keeps the NOMATRICES world matrices on the stack (alloca); a grow-only static buffer here since the 7.1 CRT lacks
	// __alloca_probe_16. Not re-entrant (callbacks do not update a hierarchy).
	static Matrix *tmpBuf; static int32 tmpCap;
	if (noMat) {
		int32 need = h->numNodes > 0 ? h->numNodes : 1;
		if (need > tmpCap) { free(tmpBuf); tmpBuf = (Matrix *)malloc(sizeof(Matrix) * need); tmpCap = need; }
	}
	Matrix *tmp = noMat ? tmpBuf : nil;
	Matrix *stack[64]; int sp = 0;
	Matrix *parentMat = rootPtr;
	static int logn; if (port_animlog() && logn++ < 6) CLOG("animlog: hanim update h %p flags %x nodes %d noMat %d world %d\n", (void *)h, fl, h->numNodes, (int)noMat, (int)computeWorld);
	for (int32 i = 0; i < h->numNodes; i++) {
		Matrix local;
		anim->applyCB(&local, anim->getInterpFrame(i));
		Matrix *cur = noMat ? &tmp[i] : &h->matrices[i];
		if (cb && cb(parentMat, i, &local, cbUser)) { *cur = local; cur->pos.x += parentMat->pos.x; cur->pos.y += parentMat->pos.y; cur->pos.z += parentMat->pos.z; }
		else Matrix::mult(cur, &local, parentMat);
		HAnimNodeInfo *node = &h->nodeInfo[i];
		Frame *nf = node->frame;
		if (nf) {
			if (fl & HAnimHierarchy::UPDATEMODELLINGMATRICES) {
				nf->matrix = local;
				if (!(fl & HAnimHierarchy::UPDATELTMS)) nf->updateObjects();
			}
			if (fl & HAnimHierarchy::UPDATELTMS) {
				if ((fl & HAnimHierarchy::LOCALSPACEMATRICES) && !noMat) Matrix::mult(&nf->ltm, cur, &rootWorld); else nf->ltm = *cur;
				nf->object.privateFlags = (nf->object.privateFlags & ~Frame::SUBTREESYNCLTM) | Frame::SUBTREESYNCOBJ;
			}
		}
		switch (node->flags & 3) {
		case 0: parentMat = cur; break;
		case 1: if (sp > 0) parentMat = stack[--sp]; break;                 // POP
		case 2: if (sp < 64) stack[sp++] = parentMat; parentMat = cur; break; // PUSH
		}
	}
	return h;
}
// mathlog: one line per node, the interpolated key frame (quaternion + translation) | the node frame's modelling matrix (mathlog.cpp
// HAnimUpdate, same raw RW 3.6 offsets)
extern "C" rw::HAnimHierarchy *RpHAnimHierarchyUpdateMatrices(rw::HAnimHierarchy *h)
{
	int tf; if (!mlOn(&tf)) return mlimpl_RpHAnimHierarchyUpdateMatrices(h);
	rw::HAnimHierarchy *r = mlimpl_RpHAnimHierarchyUpdateMatrices(h);
	char *hc = (char *)h; int n = *(int *)(hc + 4); char *info = *(char **)(hc + 0x10); char *ip = *(char **)(hc + 0x20);
	if (!info || !ip) return r;
	int fsz = *(int *)(ip + 0x24);
	for (int i = 0; i < n && i < 128; i++) {
		unsigned in[7], out[26]; memset(out, 0, sizeof out);
		memcpy(in, ip + 0x4c + i * fsz + 8, 28);
		rw::Frame *fr = *(rw::Frame **)(info + i * 16 + 0xc);
		if (fr) { mathlog::Mat(out, mlModelling(fr)); mathlog::Mat(out + 13, (const char *)fr + 0x50); }   // LTM (RW 3.6 frame +0x50)
		mathlog::Line(g_ml, tf, "HAnimNode", in, 7, out, 26);
	}
	return r;
}
extern "C" rw::HAnimHierarchy *RpHAnimHierarchySetFlags(rw::HAnimHierarchy *h, int flags) { h->flags = flags; return h; }
extern "C" int RpHAnimHierarchyGetFlags(rw::HAnimHierarchy *h) { return h->flags; }
extern "C" int RpHAnimIDGetIndex(rw::HAnimHierarchy *h, int id) { return h->getIndex(id); }
static rw::HAnimHierarchy *hierFrom(rw::HAnimHierarchy *parent, int start, int n, int flags, int maxKeySize)
{
	int *fl = (int *)malloc(n * 4), *ids = (int *)malloc(n * 4);
	for (int i = 0; i < n; i++) { fl[i] = parent->nodeInfo[start + i].flags; ids[i] = parent->nodeInfo[start + i].id; }
	rw::HAnimHierarchy *h = rw::HAnimHierarchy::create(n, fl, ids, flags, maxKeySize < 0 ? (parent->interpolator ? parent->interpolator->maxInterpKeyFrameSize : 36) : maxKeySize);
	free(fl); free(ids);
	return h;
}
// RpHAnimHierarchyCreateSubHierarchy(parent, startNode, flags, maxKeySize) (0x555910, FOUR arguments; FUN_0045aff0 passes (h, idx,
// h->flags, -1)): covers node[start] and its PUSH/POP subtree (POP = 1, PUSH = 2: a node that already has POP is a leaf; else count
// nodes until the matching POP); flags | SUBHIERARCHY; interpolator = RtAnimInterpolatorCreateSubInterpolator(parent->interpolator,
// start, count, maxKeySize); rootParentOffset (+0x1c) = -1 for start 0, else the parent node found by replaying nodes 0..start-1 on
// the PUSH/POP stack; parentFrame = parent->node[start].frame; matrices and nodeInfo are SHARED with the parent
// (parent + start * 64 / start * 16, matricesUnaligned = 0).
extern "C" rw::AnimInterpolator *RtAnimInterpolatorCreateSubInterpolator(rw::AnimInterpolator *parent, int start, int num, int maxKeySize);
extern "C" rw::HAnimHierarchy *RpHAnimHierarchyCreateSubHierarchy(rw::HAnimHierarchy *parent, int startNode, int flags, int maxKeySize)
{
	using rw::HAnimHierarchy;
	rw::HAnimNodeInfo *ni = parent->nodeInfo;
	int count = 1, depth = 0;
	{
		const rw::HAnimNodeInfo *p = &ni[startNode];
		if (!(p->flags & 1)) {
			depth = 1; p++;
			while (depth != 0) { if (p->flags & 2) depth++; if (p->flags & 1) depth--; count++; p++; }
		}
	}
	HAnimHierarchy *h = (HAnimHierarchy *)rwMalloc(sizeof(HAnimHierarchy), rw::MEMDUR_EVENT | rw::ID_HANIM);
	if (!h) return nil;
	memset(h, 0, sizeof *h);
	h->flags = flags | HAnimHierarchy::SUBHIERARCHY;
	h->numNodes = count;
	h->interpolator = RtAnimInterpolatorCreateSubInterpolator(parent->interpolator, startNode, count, maxKeySize);
	if (startNode == 0) h->rwRootParentOffset = -1;
	else {
		int stack[64]; int *sp = stack; int cur = -1;
		for (int i = 0; i < startNode; i++) {
			if (ni[i].flags & 2) *sp++ = cur;
			if (ni[i].flags & 1) cur = *--sp; else cur = i;
		}
		h->rwRootParentOffset = cur;
	}
	h->parentFrame = ni[startNode].frame;
	h->matrices = (rw::Matrix *)((char *)parent->matrices + (size_t)startNode * 64);
	h->matricesUnaligned = nil;
	h->nodeInfo = &ni[startNode];
	h->parentHierarchy = parent;
	return h;
}
extern "C" rw::HAnimHierarchy *RpHAnimHierarchyCreateFromHierarchy(rw::HAnimHierarchy *parent, int flags, int maxKeySize) { return hierFrom(parent, 0, parent->numNodes, flags, maxKeySize); }
extern "C" rw::Atomic *RpSkinAtomicSetHAnimHierarchy(rw::Atomic *a, rw::HAnimHierarchy *h) { rw::Skin::setHierarchy(a, h); return a; }
extern "C" rw::HAnimHierarchy *RpSkinAtomicGetHAnimHierarchy(rw::Atomic *a) { return rw::Skin::getHierarchy(a); }
extern "C" rw::Skin *RpSkinGeometryGetSkin(rw::Geometry *g) { return rw::Skin::get(g); }
extern "C" int RpSkinGetNumBones(rw::Skin *s) { return s->numBones; }
extern "C" const float *RpSkinGetSkinToBoneMatrices(rw::Skin *s) { return s->inverseMatrices; }
extern "C" rw::AnimInterpolator *RtAnimInterpolatorCreate(int numNodes, int maxKeyFrameSize) { return rw::AnimInterpolator::create(numNodes, maxKeyFrameSize); }
extern "C" void RtAnimInterpolatorDestroy(rw::AnimInterpolator *i) { i->destroy(); }
extern "C" int RtAnimInterpolatorSetCurrentAnim(rw::AnimInterpolator *i, rw::Animation *a) { return i->setCurrentAnim(a); }
// RtAnimInterpolatorAddAnimTime (0x5741c0; fields at the RW 3.6 offsets of librw patch 0004): nothing if t <= 0. The time callback
// (+0xc, data +0x10, trigger +0x14) is due when the time crosses the trigger, the loop callback (+0x18, data +0x1c) when it wraps.
// cur += t; while cur > duration, cur -= duration (multi-wrap; the remainder is KEPT: librw's addTime restarts at 0 and loses the
// overshoot, ~1 frame per loop); then SetCurrentAnim(anim) with currentTime restored to the remainder (also when nextFrame == 0,
// which SubAnimTime leaves behind). Key frame pairs are advanced to cur (round-robin search for the node whose key frame 2 expires),
// every node is interpolated, then the callbacks fire (one returning 0 is unregistered).
// Deviations: duration <= 0 and a missing animation do not loop forever / crash.
typedef int (*AnimCB)(rw::AnimInterpolator *, void *);
static void animFireCallbacks(rw::AnimInterpolator *i, bool timeCB, bool loopCB)
{
	if (timeCB && i->rwAnimCallBack) { if (!((AnimCB)i->rwAnimCallBack)(i, i->rwAnimCallBackData)) i->rwAnimCallBack = nil; }
	if (loopCB && i->rwAnimLoopCallBack) { if (!((AnimCB)i->rwAnimLoopCallBack)(i, i->rwAnimLoopCallBackData)) i->rwAnimLoopCallBack = nil; }
}
extern "C" int RtAnimInterpolatorAddAnimTime(rw::AnimInterpolator *i, float t)
{
	using namespace rw;
	if (!(t > 0.0f)) return 1;
	Animation *anim = i->currentAnim;
	if (!anim) return 0;
	float c0 = i->currentTime, dur = anim->duration, trig = i->rwAnimCallBackTime;
	bool timeCB = false, loopCB = false;
	if (i->currentTime < trig && i->currentTime + t >= trig) timeCB = true;
	i->currentTime += t;
	if (i->currentTime > dur) {
		while (i->currentTime > dur) {
			if (!(dur > 0.0f)) { i->currentTime = 0.0f; break; }
			i->currentTime -= dur;
			if (i->currentTime >= trig) timeCB = true;
			loopCB = true;
		}
		float rem = i->currentTime;
		i->setCurrentAnim(anim);
		i->currentTime = rem;
	}
	if (!i->nextFrame) { float rem = i->currentTime; i->setCurrentAnim(anim); i->currentTime = rem; }
	KeyFrameHeader *last = (KeyFrameHeader *)((uint8 *)anim->keyframes + anim->numFrames * i->currentAnimKeyFrameSize);
	int idx = 0;
	while ((uint8 *)i->nextFrame < (uint8 *)last) {
		KeyFrameHeader *next = (KeyFrameHeader *)i->nextFrame, *prev = next->prev;
		if (!(prev->time <= i->currentTime)) break;
		InterpFrameHeader *ifrm = nil; int tries = 0;
		do { ifrm = i->getInterpFrame(idx); if (++idx >= i->numNodes) idx = 0; } while (ifrm->keyFrame2 != prev && ++tries < i->numNodes);
		if (ifrm->keyFrame2 == prev) { ifrm->keyFrame1 = ifrm->keyFrame2; ifrm->keyFrame2 = next; }
		i->nextFrame = (uint8 *)i->nextFrame + i->currentAnimKeyFrameSize;
	}
	for (int k = 0; k < i->numNodes; k++) {
		InterpFrameHeader *ifrm = i->getInterpFrame(k);
		i->interpCB(ifrm, ifrm->keyFrame1, ifrm->keyFrame2, i->currentTime, anim->customData);
	}
	{ static int n; if (port_animlog() && (n++ % 200) == 0) CLOG("animlog: AddAnimTime #%d interp %p nodes %d t %g time %g -> %g anim %p dur %g\n", n, (void *)i, i->numNodes, t, c0, i->currentTime, (void *)anim, dur); }
	animFireCallbacks(i, timeCB, loopCB);
	return 1;
}
// RtAnimInterpolatorSubAnimTime (0x573fa0): cur -= t; a negative time wraps by ADDING the duration until >= 0, then SetCurrentAnim +
// AddAnimTime(wrapped); otherwise each node's key frame pair is walked BACKWARDS to the bracketing pair (kf2 = kf1; kf1 = kf1->prev)
// and re-interpolated, and nextFrame is cleared so the next AddAnimTime re-initialises. librw has no such function.
extern "C" int RtAnimInterpolatorSubAnimTime(rw::AnimInterpolator *i, float t)
{
	using namespace rw;
	Animation *anim = i->currentAnim;
	if (!anim) return 0;
	float trig = i->rwAnimCallBackTime;
	bool timeCB = false, loopCB = false;
	if (i->currentTime > trig && !(i->currentTime - t >= trig)) timeCB = true;
	i->currentTime -= t;
	if (i->currentTime < 0.0f) {
		float dur = anim->duration;
		if (dur > 0.0f) while (i->currentTime < 0.0f) i->currentTime += dur; else i->currentTime = 0.0f;
		float wrapped = i->currentTime;
		i->setCurrentAnim(anim);
		RtAnimInterpolatorAddAnimTime(i, wrapped);
		if (!(i->currentTime >= trig)) timeCB = true;
		loopCB = true;
	} else {
		for (int k = 0; k < i->numNodes; k++) {
			InterpFrameHeader *ifrm = i->getInterpFrame(k);
			while (i->currentTime < ifrm->keyFrame1->time && ifrm->keyFrame1->prev != ifrm->keyFrame1) {
				ifrm->keyFrame2 = ifrm->keyFrame1;
				ifrm->keyFrame1 = ifrm->keyFrame1->prev;
			}
			i->interpCB(ifrm, ifrm->keyFrame1, ifrm->keyFrame2, i->currentTime, anim->customData);
		}
		i->nextFrame = nil;
	}
	animFireCallbacks(i, timeCB, loopCB);
	return 1;
}
// RtAnimInterpolatorSetCurrentTime (game 0x574490): d = t - currentTime; d < 0 -> SubAnimTime(-d), else AddAnimTime(d); returns 1.
extern "C" int RtAnimInterpolatorSetCurrentTime(rw::AnimInterpolator *i, float t)
{
	if (!i->currentAnim) return 0;
	float d = t - i->currentTime;
	if (d < 0.0f) RtAnimInterpolatorSubAnimTime(i, -d); else RtAnimInterpolatorAddAnimTime(i, d);
	return 1;
}
// RtAnimInterpolatorBlend (0x5745f0): for node i in 0 .. out->numNodes-1 calls out->keyFrameBlendCB(out frame i, a frame i, b frame i,
// alpha) (cdecl, alpha pushed first), each interpolator addressing its frames at (this + 0x4c + i * currentInterpKeyFrameSize); alpha
// 0 = a, 1 = b; returns 1; no times or frame headers are touched.
// The HAnim (RpHAnimKeyFrameBlend 0x5575d0) and compressed (0x598c90) callbacks work on the RW 3.6 interpolated frame (header 8 B,
// quaternion x y z w at +8, translation at +0x18): t = t1 + (t2 - t1) * alpha, quaternion by the rule above (w1 = 1 - alpha, w2 = alpha).
// librw leaves blendCB nil, so the shim sets it for every scheme whose frame is the HAnim one (ids 1 and 2); other schemes keep the
// dominant-source copy. PORT_BLENDLOG=1 prints the first calls (alpha, nodes, first node's blended quaternion).
static void hanimBlendFrame(rw::HAnimInterpFrame *out, const rw::HAnimInterpFrame *f1, const rw::HAnimInterpFrame *f2, float alpha)
{
	rw::Quat q1 = f1->q, q2 = f2->q;
	rw::V3d t1 = f1->t, t2 = f2->t;                    // copy first: out may alias f1 (RtAnimInterpolatorBlend(x, x, y, w))
	out->t.x = t1.x + (t2.x - t1.x) * alpha;
	out->t.y = t1.y + (t2.y - t1.y) * alpha;
	out->t.z = t1.z + (t2.z - t1.z) * alpha;
	out->q = gameQuatInterp(q1, q2, alpha);
}
static int port_blendlog(void) { static int v = -1; if (v < 0) { const char *e = getenv("PORT_BLENDLOG"); v = e && *e && *e != '0'; } return v; }
extern "C" int RtAnimInterpolatorBlend(rw::AnimInterpolator *out, rw::AnimInterpolator *a, rw::AnimInterpolator *b, float alpha)
{
	rw::Animation *an = out->currentAnim ? out->currentAnim : (a->currentAnim ? a->currentAnim : b->currentAnim);
	int id = an && an->interpInfo ? an->interpInfo->id : 0;
	if (id == 1 || id == 2) {
		char *fo = (char *)out->getFrames(), *fa = (char *)a->getFrames(), *fb = (char *)b->getFrames();
		for (int i = 0; i < out->numNodes; i++)
			hanimBlendFrame((rw::HAnimInterpFrame *)(fo + i * out->currentInterpKeyFrameSize), (const rw::HAnimInterpFrame *)(fa + i * a->currentInterpKeyFrameSize),
			                (const rw::HAnimInterpFrame *)(fb + i * b->currentInterpKeyFrameSize), alpha);
		if (port_blendlog()) {
			static int n;
			if (n++ < 40 || (n % 500) == 0) {
				float mn = 9.0f, mx = 0.0f; int k = out->numNodes > 1 ? 1 : 0;
				for (int i = 0; i < out->numNodes; i++) {
					const rw::HAnimInterpFrame *f = (const rw::HAnimInterpFrame *)(fo + i * out->currentInterpKeyFrameSize);
					float l = sqrtf(f->q.x * f->q.x + f->q.y * f->q.y + f->q.z * f->q.z + f->q.w * f->q.w);
					if (l < mn) mn = l;
					if (l > mx) mx = l;
				}
				const rw::HAnimInterpFrame *o1 = (const rw::HAnimInterpFrame *)(fo + k * out->currentInterpKeyFrameSize);
				const rw::HAnimInterpFrame *a1 = (const rw::HAnimInterpFrame *)(fa + k * a->currentInterpKeyFrameSize), *b1 = (const rw::HAnimInterpFrame *)(fb + k * b->currentInterpKeyFrameSize);
				CLOG("blendlog: #%d out %p a %p b %p alpha %g nodes %d scheme %d |q| min %g max %g; node%d q a (%g %g %g %g) b (%g %g %g %g) -> (%g %g %g %g) t (%g %g %g)\n", n, (void *)out, (void *)a, (void *)b, alpha,
				     out->numNodes, id, mn, mx, k, a1->q.x, a1->q.y, a1->q.z, a1->q.w, b1->q.x, b1->q.y, b1->q.z, b1->q.w, o1->q.x, o1->q.y, o1->q.z, o1->q.w, o1->t.x, o1->t.y, o1->t.z);
			}
		}
		return 1;
	}
	rw::AnimInterpolator *src = alpha < 0.5f ? a : b;   // unknown scheme: dominant source (no blend callback in librw)
	if (out != src && src->currentInterpKeyFrameSize && out->numNodes == src->numNodes)
		memcpy(out->getFrames(), src->getFrames(), src->numNodes * src->currentInterpKeyFrameSize);
	return 1;
}
extern "C" rw::AnimInterpolator *RtAnimInterpolatorCreateSubInterpolator(rw::AnimInterpolator *parent, int start, int num, int maxKeySize)
{
	rw::AnimInterpolator *i = rw::AnimInterpolator::create(num, maxKeySize < 0 ? parent->maxInterpKeyFrameSize : maxKeySize);
	i->rwIsSubInterpolator = 1; i->rwOffsetInParent = start; i->rwParentAnimation = parent;
	return i;
}
extern "C" int RtAnimAnimationGetNumNodes(rw::Animation *a) { return a->getNumNodes(); }
extern "C" int RtAnimAnimationDestroy(rw::Animation *a) { if (a) a->destroy(); return 1; }
extern "C" float RtAnimAnimationGetDuration(rw::Animation *a) { return a->duration; }
// ---------------------------------------------------------------- diagnostics: the script symbol registry (g_symRegistryY 0x61f878)
struct PortSymSlot { uintptr_t obj, key; };   // obj = ScriptReference* | tag bit 0
struct PortSymRegistry { int m00; PortSymSlot *begin; PortSymSlot *end; };   // a view of the game's vector (MSVC 7.1 layout: allocator, first, last, end)
extern "C" void port_dump_registry(void)
{
	if (!port_log) return;
	PortSymRegistry *r = (PortSymRegistry *)PortGame_SymRegistry();
	if (!r || !r->begin) { fprintf(port_log, "registry: none\n"); return; }
	int n = (int)(r->end - r->begin);
	fprintf(port_log, "registry: %d slots (obj|tag, key, vptr)\n", n);
	for (int i = 0; i < n && i < 400; i++) {
		uintptr_t o = r->begin[i].obj, p = o & ~(uintptr_t)1;
		uintptr_t vt = (p > 0x10000) ? *(uintptr_t *)p : 0;
		fprintf(port_log, "  [%d] %08x %08x vptr %08x\n", i, (unsigned)o, (unsigned)r->begin[i].key, (unsigned)vt);
	}
	fflush(port_log);
}
