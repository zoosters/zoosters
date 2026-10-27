// Bink movie playback: open / per-frame step / close and the PlayVideo loop.
#include "zoosters/base.h"
#include "tfb/hud/hud2d.h"
#include "modules/audio/audioglobalmixer.h"
#include "framework/toolkits/world/framehelper.h"
#include <stdio.h>
#include "framework/mainloop/mainloop.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/platform.h"
#include "modules/tfb/camera_views.h"
#include "rw/rttoolkits.h"
#include "script/types/scriptsystem.h"
#include "tfb/level/bink.h"
using namespace Script;


// GLOBAL: 0x5ca280 _imp__BinkOpenDirectSound
// GLOBAL: 0x5ca284 _imp__BinkSetSoundSystem
// GLOBAL: 0x5ca288 _imp__BinkWait
// GLOBAL: 0x5ca28c _imp__BinkDoFrame
// GLOBAL: 0x5ca290 _imp__BinkCopyToBuffer
// GLOBAL: 0x5ca294 _imp__BinkClose
// GLOBAL: 0x5ca298 _imp__BinkOpen
// GLOBAL: 0x5ca29c _imp__BinkNextFrame
#include "zoosters/os.h"
static inline void Sleep(unsigned ms) { zoosters::os::SleepMs(ms); }

// GLOBAL: 0x606c3c RWS::MainLoop::Logic::Rate
void MemCardSendRunningPostTick();
ZOOSTERS_NOINLINE char *GetVideoDir();   // 0x460c10 (noinline: Video_Open calls it as a function)
void Video_Close();
float GetPlayerInput(int input, float threshold, int player);
void CtlRefresh();
unsigned GetTimeMs();

extern BINK *g_bink;                // GLOBAL: 0x62a150 g_bink
extern RwRaster *g_binkRaster;     // GLOBAL: 0x62a14c g_binkRaster
extern RwCamera *g_binkSavedRtCam;      // GLOBAL: 0x62a144 g_binkSavedRtCam
extern RwCamera *g_binkCam;        // GLOBAL: 0x62a148 g_binkCam
extern const char *g_binkName;      // GLOBAL: 0x62a138 g_binkName
extern int g_binkClearCount;        // GLOBAL: 0x62a13c g_binkClearCount
extern int g_binkSavedFlip;         // GLOBAL: 0x62a140 g_binkSavedFlip
extern int g_vsyncFlipCount;        // GLOBAL: 0x6221b8 g_vsyncFlipCount

// GLOBAL: 0x4025a0 Script::ScriptSystem::ExposeVSyncFlipCount::SetValue

// 0x460790
void Video_Close()
{
	if (g_bink) {
		BinkClose(g_bink);
		g_bink = 0;
		if (g_binkRaster) {
			RwRasterDestroy(g_binkRaster);
			g_binkRaster = 0;
		}
		Rt2dDeviceSetCamera(g_binkSavedRtCam);
		if (g_binkSavedRtCam)
			Rt2dDeviceSetMetric(0.0f, 0.0f, 640.0f, 480.0f);
		if (RwCameraGetWorld(g_binkCam))
			RpWorldRemoveCamera(RwCameraGetWorld(g_binkCam), g_binkCam);
		RwFrame *frame = CameraView(g_binkCam)->frame;
		_rwObjectHasFrameSetFrame(g_binkCam, 0);
		RWS::FrameHelper::FrameDestroy(frame);
		RwCameraDestroy(g_binkCam);
		if (RWS::MainLoop::Logic::Rate != 60) {
			ScriptSystem::ExposeVSyncFlipCount vsync;
			vsync.SetValue(g_binkSavedFlip, &vsync);
		}
	}
}

// 0x4608a0
bool Video_Open(const char *name)
{
	char path[0x100];
	g_binkName = name;
	sprintf(path, "%s%s", GetVideoDir(), name);
	g_bink = BinkOpen(path, 0);
	if (g_bink) {
		RwCamera *cam = RWS::MainLoop::Render::GetCamera();
		g_binkRaster = RwRasterCreate(g_bink->Width, g_bink->Height, ((RwRasterView *)CameraView(cam)->frameBuffer)->depth, rwRASTERTYPECAMERATEXTURE);
		g_binkSavedRtCam = Rt2dDeviceGetCamera();
		g_binkCam = RwCameraClone(cam);
		Rt2dDeviceSetCamera(g_binkCam);
		Rt2dDeviceSetMetric(0.0f, 0.0f, 640.0f, 480.0f);
		if (RWS::MainLoop::Logic::Rate != 60) {
			ScriptSystem::ExposeVSyncFlipCount vsync;
			g_binkSavedFlip = g_vsyncFlipCount;
			vsync.SetValue(1, &vsync);
		}
		g_binkClearCount = 0;
		return true;
	}
	return false;
}


// 0x4609c0
int Video_Step(Hud2DObject *hud)
{
	if (g_bink && g_bink->FrameNum < g_bink->Frames) {
		if (BinkWait(g_bink)) {
			do {
				Sleep(1);
			} while (BinkWait(g_bink));
		}
		BinkDoFrame(g_bink);
		void *pixels = RwRasterLock(g_binkRaster, 0, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH);
		BinkCopyToBuffer(g_bink, pixels, RasterView(g_binkRaster)->stride, RasterView(g_binkRaster)->height, 0, 0, 0x80000003);
		RwRasterUnlock(g_binkRaster);
		BinkNextFrame(g_bink);
#if ZOOSTERS_TIER >= 2
		if (true) {   // qol: the bars beside the movie are cleared every frame (a swapped-in back buffer may hold anything)
#else
		if (g_binkClearCount < 2) {
#endif
			g_binkClearCount++;
			RwRGBA black;
			black.red = 0;
			black.green = 0;
			black.blue = 0;
			black.alpha = 0;
			RwCameraClear(g_binkCam, &black, rwCAMERACLEARIMAGE);
		}
		RwRasterPushContext(CameraView(g_binkCam)->frameBuffer);
		if (g_bink->Width == (unsigned)RasterView(CameraView(g_binkCam)->frameBuffer)->width && g_bink->Height == (unsigned)RasterView(CameraView(g_binkCam)->frameBuffer)->height) {
			RwRasterRenderFast(g_binkRaster, 0, 0);
		} else {
			RwRect r;
			RwRasterView *fb = RasterView(CameraView(g_binkCam)->frameBuffer);
			r.y = 0;
			r.x = 0;
			r.w = fb->width;
			r.h = fb->height;
#if ZOOSTERS_TIER >= 2
			// qol: the movie keeps its shape (bars left and right, or above and below), not stretched to the window
			if ((long long)r.w * g_bink->Height > (long long)r.h * g_bink->Width) {
				int w = (int)((long long)r.h * g_bink->Width / g_bink->Height);
				r.x = (r.w - w) / 2;
				r.w = w;
			} else {
				int h = (int)((long long)r.w * g_bink->Height / g_bink->Width);
				r.y = (r.h - h) / 2;
				r.h = h;
			}
#endif
			RwRasterRenderScaled(g_binkRaster, &r);
		}
		RwRasterPopContext();
		if (hud && hud->m_font) {
			Hud2D::BeginFrame(g_binkCam);
			hud->DrawText();
			Hud2D::EndFrame(g_binkCam);
		}
		RwCameraShowRaster(g_binkCam, 0, 1);
		MemCardSendRunningPostTick();
		if (g_bink->ReadError) {
			Video_Close();
			return 2;
		}
		return 0;
	}
	return 1;
}

// 0x460b90
int PlayVideo(const char *path, bool skippable)
{
	int rc = 2;
	if (Video_Open(path)) {
		unsigned start = GetTimeMs();
		do {
			rc = Video_Step(0);
			if (rc != 0)
				break;
			CtlRefresh();
		} while (!skippable || GetTimeMs() < start + 1000 || GetPlayerInput(-1, 0.25f, rc) == 0.0f);
		Video_Close();
	}
	return rc;
}

// GLOBAL: 0x5ca280 _imp__BinkOpenDirectSound
// GLOBAL: 0x5ca284 _imp__BinkSetSoundSystem
extern "C" {
void FUN_005074b0();
}

// 0x460c50
int PlayIntroVideo(const char *path)
{
	RWS::Audio::AudioGlobalMixer::StopAllStreams(true);
	FUN_005074b0();
	DSound_Init(g_mainWnd);
	BinkSetSoundSystem((void *)BinkOpenDirectSound, (void *)g_dsoundObj);
	int rc = PlayVideo(path, true);
	DSound_Release();
	FUN_005074b0();
	return rc;
}

extern char DAT_0062a1f8;

// 0x460c10
char *GetVideoDir()
{
	return &DAT_0062a1f8;
}
