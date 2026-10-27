#include "modules/tfb/camera_views.h"
#include "tfb/level/level.h"

// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x617d5c g_pixelFogEnable
// GLOBAL: 0x617d90 g_pixelFogDist
// GLOBAL: 0x617d98 g_pixelCutoff
// GLOBAL: 0x629700 g_fogMode
// GLOBAL: 0x629708 g_vertFogDist
// GLOBAL: 0x62970c g_farClip
// GLOBAL: 0x629710 g_nearClip
// GLOBAL: 0x60297c g_levelFarDefault
// GLOBAL: 0x602ed0 g_pixelFogBlendB
// GLOBAL: 0x602ed4 g_pixelFogScaleA
// GLOBAL: 0x61813c g_pixelFogCacheD
// GLOBAL: 0x618140 g_pixelFogCacheC
// GLOBAL: 0x618144 g_pixelFogCacheB
// GLOBAL: 0x618148 g_pixelFogCacheA
// GLOBAL: 0x61814c g_fogCacheKey
extern const float g_floatZero;
extern unsigned char g_pixelFogEnable;   // post-effect state (g/postfx_411.cpp)
extern float g_pixelFogDist;
extern int g_pixelCutoff;
extern float g_pixelFogBlendB, g_pixelFogScaleA;
extern float g_pixelFogCacheD, g_pixelFogCacheC, g_pixelFogCacheB, g_pixelFogCacheA;
extern unsigned g_fogCacheKey;
namespace RWS { namespace MainLoop { namespace Render { unsigned GetFrameCount(); } } }  // 0x462710 the frame stamp the cache is keyed on
void SetPixelFogEnd(float distance);   // 0x411320

// depth(z) = offset + scale / z through (zNear, depthNear) and (zFar, depthFar): this is scale, 0x41a200 offset
// 0x41a1a0
float DepthCurveScale(float depthNear, float depthFar, float zNear, float zFar)
{
	if (zNear <= g_floatZero || zFar <= g_floatZero || zNear == zFar)
		return g_floatZero;
	return (depthFar - depthNear) / (1.0f / zFar - 1.0f / zNear);
}

// 0x41a200
float DepthCurveOffset(float depthNear, float depthFar, float zNear, float zFar)
{
	float s = depthNear + depthFar;
	return (s - DepthCurveScale(depthNear, depthFar, zNear, zFar) * (1.0f / zNear + 1.0f / zFar)) * 0.5f;
}

// 0x41a250
float PixelFogDepthHigh(float zNear, float zFar, float depth, float fogDist)
{
	float ia = 1.0f / zNear;
	float p = 1.0f / (1.0f - zFar * ia);
	float ib = 1.0f / zFar;
	float q = 1.0f / (zNear * ib - 1.0f);
	float s = 1.0f / ((ib - ia) * fogDist);
	return (((1.0f - p - q) * 0.5f + s) * depth - 32767.0f) / (s - (q + p + 1.0f) * 0.5f);
}

// 0x41a2e0
float PixelFogDepthLow(float zNear, float zFar, float depth, float fogDist)
{
	float ia = 1.0f / zNear;
	float p = 1.0f / (1.0f - zFar * ia);
	float ib = 1.0f / zFar;
	float q = 1.0f / (zNear * ib - 1.0f);
	float s = 1.0f / ((ib - ia) * fogDist);
	return ((s - (q + p + 1.0f) * 0.5f) * depth + 32767.0f) / ((1.0f - p - q) * 0.5f + s);
}

// Near-match of 0x41a370: the game reads the timer twice (cache test, then key) and pushes esi / edi after the cache test; cl merges the
// cache-hit copy-out with the identical end copy.
void GetPixelFogParams(RwCamera *camera, float *a, float *b, float *c, float *d)
{
	if (g_fogCacheKey == RWS::MainLoop::Render::GetFrameCount()) {   // cache hit: the game reads the timer twice (here and below)
		*a = g_pixelFogCacheA;
		*b = g_pixelFogCacheB;
		*c = g_pixelFogCacheC;
		*d = g_pixelFogCacheD;
		return;
	}
	{
	g_fogCacheKey = RWS::MainLoop::Render::GetFrameCount();
	float zFar = g_farClip;
	float zNear = g_nearClip;
	float v;
	if (g_pixelFogDist <= g_floatZero) {
		v = g_floatZero;
	} else {
		float x = DepthCurveScale(16777215.0f, 0.0f, zNear, zFar);
		v = (16777215.0f - (1.0f / g_nearClip + 1.0f / g_farClip) * x) * 0.5f + (1.0f / g_pixelFogDist) * x;
	}
	float lo, hi;
	if (g_pixelFogDist <= g_nearClip) {
		hi = 65535.1f;
		lo = 0.0f;
	} else if (g_pixelFogDist > g_farClip) {
		hi = 16777215.0f;
		lo = 65535.1f;
	} else if (v < 32767.0f) {
		hi = 16777215.0f;
		lo = PixelFogDepthLow(zNear, zFar, 16777215.0f, g_pixelFogDist);
	} else {
		lo = 0.0f;
		hi = PixelFogDepthHigh(zNear, zFar, 0.0f, g_pixelFogDist);
	}
	int cutoff = (unsigned char)g_pixelCutoff;
	float t = DepthCurveScale(hi, lo, zNear, zFar);
	float top = lo + 255.0f;
	float w = t / (top - DepthCurveOffset(hi, lo, zNear, zFar));
	g_pixelFogCacheD = w;
	g_pixelFogCacheC = (w - g_pixelFogDist) * ((float)cutoff * 0.01f) + g_pixelFogDist;
	g_pixelFogCacheA = g_pixelFogScaleA * g_pixelFogDist;
	g_pixelFogCacheB = (g_levelFarDefault - g_pixelFogCacheC) * g_pixelFogBlendB + g_pixelFogCacheC;
	}
	*a = g_pixelFogCacheA;
	*b = g_pixelFogCacheB;
	*c = g_pixelFogCacheC;
	*d = g_pixelFogCacheD;
}

// Sets the camera's fog plane (and the pixel fog end) for the next render: from the pixel fog distances when the pixel fog is on,
// else from the vertex fog distance of the level.
// 0x41ac20
void ApplyCameraFog(RwCamera *camera)
{
	float v[4];
	if (g_pixelFogEnable == 1) {
		GetPixelFogParams(camera, &v[0], &v[1], &v[3], &v[2]);
		CameraView(camera)->fogPlane = v[0];
		SetPixelFogEnd(v[1]);
	} else if (g_fogMode != 0) {
		CameraView(camera)->fogPlane = g_vertFogDist;
		SetPixelFogEnd(g_farClip);
	}
}
