// Scenery for the widescreen view (ZOOSTERS_WIDESCREEN). Sets that end just outside the original 4:3 frame show the clear colour at
// wider aspects. CTFBWorld::RenderFrame draws the level's own worlds once more under the real pass, zoomed in; this decides when.
#include <stdio.h>

extern "C" FILE *port_log;
extern "C" void *RwCameraGetRaster(const void *cam);
extern "C" int RwRasterGetWidth(const void *raster);
extern "C" int RwRasterGetHeight(const void *raster);
extern int g_currentLevel;   // GLOBAL: 0x606a38 g_currentLevel

enum { LEVEL_RAVE = 5, LEVEL_BANQUET = 7, LEVEL_RESCUE = 10, LEVEL_MAP = 14 };

// Zoom of the underlay pass, 0 for none. Levels: map; rave (AirplaneCam) and banquet (SimonIntroCam01) have
// cameras that see past their set at wider aspects. The 4:3 frame is covered in every view, so zooming by (W / H) / (4 / 3) (+ a margin) puts
// it over the whole screen. Map: the ground, clouds and sky fade out or end beyond the frame the original showed.
extern "C" float port_wide_underlay_zoom(void *camera)
{
	if (g_currentLevel != LEVEL_MAP && g_currentLevel != LEVEL_RAVE && g_currentLevel != LEVEL_BANQUET) return 0.0f;
	const void *r = RwCameraGetRaster(camera);
	if (!r) return 0.0f;
	int w = RwRasterGetWidth(r), h = RwRasterGetHeight(r);
	if (w < 400 || h < 300) return 0.0f;   // the shadow and radar cameras
	float z = (3.0f * w) / (4.0f * h) * 1.15f;
	return z > 1.15f ? z : 0.0f;
}

// Rescue: the daytime sky card is a flat quad that ends inside the wider frame; clearing to its colour at the edge hides it
extern "C" void port_wide_clear_color(void *camera, unsigned char *rgba)
{
	if (g_currentLevel != LEVEL_RESCUE) return;
	const void *r = RwCameraGetRaster(camera);
	if (!r) return;
	int w = RwRasterGetWidth(r), h = RwRasterGetHeight(r);
	if (w < 400 || h < 300 || 3 * w <= 4 * h * 1.02f) return;   // the shadow and radar cameras, 4:3
	rgba[0] = 110; rgba[1] = 147; rgba[2] = 202;
}
