// Loading screen: a full-screen image drawn before a level streams in.
#include <stdio.h>
#include "framework/startup/startup.h"
#include "modules/tfb/camera_views.h"
#define BS "\\"

#include "framework/startup/loadscreen_views.h"

int GetLanguage();
const char *GetLanguageCode(int language);

extern RwRaster *g_loadRaster;   // GLOBAL: 0x62a57c g_loadRaster

// 0x4628d0
bool LoadScreen_LoadImage(const char *name)
{
	char path[0x100];
	if (name) {
		sprintf(path, "Images" BS "%s" BS "%s", GetLanguageCode(GetLanguage()), name);
		RwImage *image = RwImageRead(path);
		if (image) {
			const RwImageView *view = (const RwImageView *)image;
			RwRaster *raster = RwRasterCreate(view->width, view->height, view->depth, 0);
			if (raster)
				RwRasterSetFromImage(raster, image);
			RwImageDestroy(image);
			g_loadRaster = raster;
			return true;
		}
	}
	g_loadRaster = 0;
	return true;
}

#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_hud(float W, float H, float *B, float *D);   // port/fixes/widescreen.cpp
#endif
// 0x4629a0
void LoadScreen_Draw(RwCamera *camera)
{
	RwRasterPushContext(RwCameraGetRaster(camera));
	if (!g_loadRaster) {
		RwRasterClear(0);
		RwRasterPopContext();
		return;
	}
	if (RwRasterGetWidth(RwCameraGetRaster(camera)) == RwRasterGetWidth(g_loadRaster)
	    && RwRasterGetHeight(RwCameraGetRaster(camera)) == RwRasterGetHeight(g_loadRaster)) {
		RwRasterClear(0);
		RwRasterRenderFast(g_loadRaster, 0, 0);
	} else {
		RwRect rect;
		rect.y = 0;
		rect.x = 0;
		rect.h = RwRasterGetHeight(RwCameraGetRaster(camera));
		rect.w = RwRasterGetWidth(RwCameraGetRaster(camera));
#ifdef ZOOSTERS_WIDESCREEN   // the 4:3 picture centred between black bars
		{
			float B, D;
			port_wide2d_hud((float)rect.w, (float)rect.h, &B, &D);
			if (B > 0.0f) {
				RwRasterClear(0);
				rect.x = (int)B;
				rect.w = (int)(D * 640.0f);
			}
		}
#endif
		RwRasterRenderScaled(g_loadRaster, &rect);
	}
	RwRasterPopContext();
}

// 0x462970
void LoadScreen_DestroyRaster()
{
	if (g_loadRaster) {
		RwRasterDestroy(g_loadRaster);
		g_loadRaster = 0;
	}
	RwFreeListPurgeAllFreeLists();
}

// 0x462a50
void LoadScreen_Show(RwCamera *camera, const char *name)
{
	if (LoadScreen_LoadImage(name)) {
		for (int i = 2; i != 0; i--)
			LoadScreen_Draw(camera);
		if (g_loadRaster) {
			RwRasterDestroy(g_loadRaster);
			g_loadRaster = 0;
		}
		RwFreeListPurgeAllFreeLists();
	}
}
