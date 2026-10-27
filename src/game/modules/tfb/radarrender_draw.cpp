#include <math.h>
#include "modules/tfb/radarrender.h"
#include "modules/tfb/camera_views.h"

// GLOBAL: 0x6223ac g_radarHeight
// GLOBAL: 0x6223b0 g_radarWidth
// GLOBAL: 0x6034c4 g_radarSize
// GLOBAL: 0x5caea4 g_radarAspect
// GLOBAL: 0x629710 g_nearClip
// GLOBAL: 0x6223c8 g_rsSrcBlend
// GLOBAL: 0x6223c4 g_rsDestBlend
// GLOBAL: 0x6223c0 g_rsVertexAlpha
// GLOBAL: 0x6223bc g_radarZWrite
// GLOBAL: 0x6223b8 g_radarZTest
// GLOBAL: 0x6223cc g_rsCullMode
// GLOBAL: 0x6223b4 g_rsFog

extern float g_nearClip;   // 0x629710 the main camera's near plane (the radar overlay's reciprocal depth)

// Render states saved by SaveRenderStates and put back by RestoreRenderStates; the game's variables are in the order of the saves.
extern int g_rsSrcBlend;
extern int g_rsDestBlend;
extern int g_rsVertexAlpha;
extern int g_radarZWrite;
extern int g_radarZTest;
extern int g_rsCullMode;
extern int g_rsFog;

static unsigned PackARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
	return (((((a << 8) | r) << 8) | g) << 8) | b;
}

// 0x4418f0
void SaveRenderStates()
{
	RwRenderStateGet(rwRENDERSTATESRCBLEND, (void *)&g_rsSrcBlend);
	RwRenderStateGet(rwRENDERSTATEDESTBLEND, (void *)&g_rsDestBlend);
	RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)&g_rsVertexAlpha);
	RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, (void *)&g_radarZWrite);
	RwRenderStateGet(rwRENDERSTATEZTESTENABLE, (void *)&g_radarZTest);
	RwRenderStateGet(rwRENDERSTATECULLMODE, (void *)&g_rsCullMode);
	RwRenderStateGet(rwRENDERSTATEFOGENABLE, (void *)&g_rsFog);
}

// 0x441970
void SetRadarRenderStates()
{
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONALWAYS);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
}

// 0x4419e0
void RestoreRenderStates()
{
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)(intptr_t)g_rsSrcBlend);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)(intptr_t)g_rsDestBlend);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(intptr_t)g_rsVertexAlpha);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(intptr_t)g_radarZWrite);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)(intptr_t)g_radarZTest);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)(intptr_t)g_rsCullMode);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)(intptr_t)g_rsFog);
}

// An opaque textured quad (triangle strip) with the texture origin at `uv`; depth and reciprocal depth as given. The game takes the
// alpha (0xFF) in eax, which C++ cannot express: it is the constant here.
// 0x441a60
int RadarDrawTexturedQuad(float left, float top, float right, float bottom, float screenZ, float recipZ, float uv)
{
	RwIm2DVertex v[4];
	unsigned alpha = 0xff;
	unsigned col = (alpha << 24) | 0xffffff;
	float uv1 = uv + 1.0f;
	v[0].x = left;   v[0].y = top;     v[0].z = screenZ;  v[0].rhw = recipZ;  v[0].emissiveColor = col;  v[0].u = uv;   v[0].v = uv;
	v[1].x = left;   v[1].y = bottom;  v[1].z = screenZ;  v[1].rhw = recipZ;  v[1].emissiveColor = col;  v[1].u = uv;   v[1].v = uv1;
	v[2].x = right;  v[2].y = top;     v[2].z = screenZ;  v[2].rhw = recipZ;  v[2].emissiveColor = col;  v[2].u = uv1;  v[2].v = uv;
	v[3].x = right;  v[3].y = bottom;  v[3].z = screenZ;  v[3].rhw = recipZ;  v[3].emissiveColor = col;  v[3].u = uv1;  v[3].v = uv1;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	return 1;
}

// An untextured quad in one colour (`rgba`: packed, red in the low byte): the radar background and the dot blips.
// 0x441b80
int RadarDrawRect(float left, float top, float right, float bottom, int rgba)
{
	RwIm2DVertex v[4];
	RwRGBA in = *(RwRGBA *)&rgba;
	unsigned col = PackARGB(in.alpha, in.red, in.green, in.blue);
	v[0].x = left;
	v[0].y = top;
	v[0].z = 0.0f;
	v[0].rhw = 1.0f;
	v[0].emissiveColor = col;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[1].x = left;
	v[1].y = bottom;
	v[1].z = 0.0f;
	v[1].rhw = 1.0f;
	v[1].emissiveColor = col;
	v[1].u = 0.0f;
	v[1].v = 1.0f;
	v[2].x = right;
	v[2].y = top;
	v[2].z = 0.0f;
	v[2].rhw = 1.0f;
	v[2].emissiveColor = col;
	v[2].u = 1.0f;
	v[2].v = 0.0f;
	v[3].x = right;
	v[3].y = bottom;
	v[3].z = 0.0f;
	v[3].rhw = 1.0f;
	v[3].emissiveColor = col;
	v[3].u = 1.0f;
	v[3].v = 1.0f;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	return 1;
}

// The quad of a view cone: the apex (a0, a1), two edge points (a2, a3), (a6, a7) and the tip (a4, a5) in 0..1 radar coordinates,
// the texture coordinates of the BIGCIRCLE texture cut by the cone's angle (degrees), in colour `rgba`.
// 0x441cb0
int RadarDrawCone(float a0, float a1, float a2, float a3, float a4, float a5, float a6, float a7, float angle, int rgba)
{
	RwIm2DVertex v[4];
	float t = angle * 0.011111111f;
	angle = angle * 0.017453292f;
	v[2].u = (float)sin(angle);
	unsigned char *b = (unsigned char *)&rgba;
	unsigned col = PackARGB(b[3], b[0], b[1], b[2]);
	v[0].emissiveColor = col;
	v[1].emissiveColor = col;
	v[2].emissiveColor = col;
	v[3].emissiveColor = col;
	v[0].z = 0.0f;  v[0].rhw = 1.0f;  v[0].u = 0.0f;  v[0].v = 1.0f;
	v[1].z = 0.0f;  v[1].rhw = 1.0f;  v[1].u = 0.0f;  v[1].v = 0.0f;
	v[2].z = 0.0f;  v[2].rhw = 1.0f;
	v[3].z = 0.0f;  v[3].rhw = 1.0f;  v[3].v = 0.0f;
	v[0].x = g_radarWidth * a0;
	v[0].y = g_radarHeight * a1;
	v[1].x = g_radarWidth * a2;
	v[1].y = g_radarHeight * a3;
	v[2].x = g_radarWidth * a6;
	v[2].y = g_radarHeight * a7;
	v[2].v = 1.0f - (float)cos(angle);
	v[3].x = g_radarWidth * a4;
	v[3].y = g_radarHeight * a5;
	v[3].u = t;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	return 1;
}

// Draws `target` (the radar's own raster) as a textured quad over the screen: from `position` (fractions of the screen, x then y)
// to that plus (size * aspect, size), clamped to the screen, in the camera `camera`. The 4th argument is ignored by the game.
#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_rect(float W, float H, float *x, float *w);   // port/fixes/widescreen.cpp
#endif
// The callee 0x441a60 takes the alpha 0xFF in eax, which C++ cannot express (passed nowhere here).
void RadarRenderToCamera(RwRaster *target, float *position, RwCamera *camera, float alpha)
{
	if (RwCameraBeginUpdate(camera)) {
		RwRasterView *frameBuffer = RasterView(CameraView(camera)->frameBuffer);
		g_radarWidth = (float)frameBuffer->width;
		g_radarHeight = (float)frameBuffer->height;
		float gy, gx, fy, fx, inv;
		inv = 1.0f / g_nearClip;
		fx = position[0];
		fy = position[1];
		gx = g_radarAspect * g_radarSize + fx;
		if (gx > 1.0f)
			gx = 1.0f;
		gy = g_radarSize + fy;
		if (gy > 1.0f)
			gy = 1.0f;
#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only (port/fixes/widescreen.cpp): square on any screen, at its side edge
		{
			float x = g_radarWidth * fx, rw = g_radarWidth * (gx - fx);
			port_wide2d_rect(g_radarWidth, g_radarHeight, &x, &rw);
			fx = x / g_radarWidth;
			gx = (x + rw) / g_radarWidth;
		}
#endif
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, target);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
		RadarDrawTexturedQuad(g_radarWidth * fx, g_radarHeight * fy, g_radarWidth * gx, g_radarHeight * gy, RwIm2DGetNearScreenZ(), inv, 0.0f);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
		RwCameraEndUpdate(camera);
	}
}
