// Level-load screen: the random leaf textures and the progress sprites.
#include <stdlib.h>
#include "framework/core/loadhooks.h"
#include "framework/loadscreentex.h"
#include "framework/mainloop/mainloop.h"
#include "framework/startup/startup.h"
#include "modules/tfb/camera_views.h"

unsigned GetTimeMs();

extern LoadScreenTex g_loadTexA[8];   // GLOBAL: 0x606b80 g_loadTexA
extern LoadScreenTex g_loadTexB[8];   // GLOBAL: 0x606bc0 g_loadTexB
extern unsigned g_loadStartTime;      // GLOBAL: 0x62a1e0 g_loadStartTime
extern unsigned g_loadDuration;       // GLOBAL: 0x62a1e4 g_loadDuration
extern float g_loadOfsX;              // GLOBAL: 0x62a1e8 g_loadOfsX
extern float g_loadOfsY;              // GLOBAL: 0x62a1ec g_loadOfsY
extern int g_loadSpriteAge[15];       // GLOBAL: 0x62a168 g_loadSpriteAge
extern RwTexture *g_loadSpriteTex[15];   // GLOBAL: 0x62a1a4 g_loadSpriteTex
extern unsigned g_loadLastDraw;       // GLOBAL: 0x62a308 g_loadLastDraw
extern float g_loadSpriteDy[15];      // GLOBAL: 0x606c00 g_loadSpriteDy

// 0x460ca0
void LoadScreen_Begin(unsigned duration)
{
	g_loadStartTime = GetTimeMs();
	g_loadDuration = duration;
	for (int i = 0; i < 8; i++) {
		if (!g_loadTexA[i].tex)
			g_loadTexA[i].tex = RwTextureRead(g_loadTexA[i].name, 0);
		if (!g_loadTexB[i].tex)
			g_loadTexB[i].tex = RwTextureRead(g_loadTexB[i].name, 0);
	}
	for (int k = 0; k < 15; k++) {
		g_loadSpriteAge[k] = 0;
		int texIndex = rand() & 7;
		if (k & 1)
			g_loadSpriteTex[k] = g_loadTexB[texIndex].tex;
		else
			g_loadSpriteTex[k] = g_loadTexA[texIndex].tex;
	}
}

#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_hud(float W, float H, float *B, float *D);   // port/fixes/widescreen.cpp
#endif
// 0x460da0
void TFBLoadingProgress()
{
	if (g_loadDuration <= 0)
		return;
	unsigned now = GetTimeMs();
	if (now - g_loadLastDraw < 100 && g_loadStartTime > 0)
		return;
	g_loadLastDraw = now;
	RwCamera *cam = RWS::MainLoop::Render::GetCamera();
	LoadScreen_Draw(cam);
	if (!RwCameraBeginUpdate(cam))
		return;
	unsigned spritesDue = (now - g_loadStartTime) * 15 / g_loadDuration;
	if (spritesDue > 15)
		spritesDue = 15;
	RwIm2DVertex vtx[4];
	RwIm2DVertex *vp = vtx;
	for (int k = 4; k != 0; k--, vp++) {
		vp->z = 20.2f;
		vp->rhw = 1.0f / 20.2f;
		vp->emissiveColor = 0xffffffff;
	}
	vtx[0].u = 0;
	vtx[0].v = 1.0f;
	vtx[1].u = 1.0f;
	vtx[1].v = 1.0f;
	vtx[2].u = 0;
	vtx[2].v = 0;
	vtx[3].u = 1.0f;
	vtx[3].v = 0;
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
	float sx = RasterView(CameraView(cam)->frameBuffer)->width * 0.0015625f;
#ifdef ZOOSTERS_WIDESCREEN   // leaves on the picture's centred 4:3 grid
	{
		float B;
		port_wide2d_hud((float)RasterView(CameraView(cam)->frameBuffer)->width, (float)RasterView(CameraView(cam)->frameBuffer)->height, &B, &sx);
	}
#endif
	float sy = RasterView(CameraView(cam)->frameBuffer)->height * 0.0020833334f;
	float stepX = sx * 16.0f;
	float stepY = sy * 24.0f;
	for (int i = 0; i < (int)spritesDue; i++) {
		RwTexture *tex = g_loadSpriteTex[i];
		if (tex) {
			int age = g_loadSpriteAge[i];
			g_loadSpriteAge[i] = age + 1;
			if (age < 6) {
				RwRasterView *r = RasterView(TextureView(tex)->raster);
				float w = r->width * sx * 0.8f;
				float h = r->height * sy * 0.8f;
				float x = i * stepX + g_loadOfsX;
				float y = sy * g_loadSpriteDy[i] + (g_loadOfsY - h);
				if (i & 1)
					y += stepY;
				vtx[0].x = x;
				vtx[0].y = y + h;
				vtx[1].x = x + w;
				vtx[1].y = y + h;
				vtx[2].x = x;
				vtx[2].y = y;
				vtx[3].x = x + w;
				vtx[3].y = y;
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, TextureView(tex)->raster);
				RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, vtx, 4);
			}
		}
		g_loadSpriteAge[i] = 0;
	}
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	RwCameraEndUpdate(cam);
	RwCameraShowRaster(cam, 0, 1);
}

// 0x460d50
void LoadScreen_FreeTextures()
{
	for (int i = 0; i < 8; i++) {
		if (g_loadTexA[i].tex) {
			RwTextureDestroy(g_loadTexA[i].tex);
			g_loadTexA[i].tex = 0;
		}
		if (g_loadTexB[i].tex) {
			RwTextureDestroy(g_loadTexB[i].tex);
			g_loadTexB[i].tex = 0;
		}
	}
}
