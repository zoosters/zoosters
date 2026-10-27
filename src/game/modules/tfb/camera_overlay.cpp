#include "modules/tfb/gamecamera.h"
#include "modules/tfb/camera_views.h"
#include "rw/rwcore.h"
#include "tfb/render/tfbrenderstate.h"


enum { kStencilPass = 2 };                 // SetBlendAndAlphaTest's third argument (STENCILPASS = 2)
enum { kOverlayFade = 1, kOverlayLetterbox = 2 };

// 0x41f2c0
// Alpha = fade * 255; not drawn while it is below 2.
void CameraDrawFadeQuad(RwCamera *camera)
{
	int alpha = (int)(g_camFade * 255.0f);
	if (alpha > 1) {
		RwIm2DVertex v[4];
		TFBRenderState::Apply(0);
		RwRasterView *raster = (RwRasterView *)CameraView(camera)->frameBuffer;
		float rhw = 1.0f / CameraView(camera)->nearPlane;
		float w = (float)raster->width;
		float h = (float)raster->height;
		v[0].x = 0;
		v[0].y = h;
		v[0].z = RwIm2DGetNearScreenZ();
		v[0].rhw = rhw;
		v[0].emissiveColor = alpha << 24;
		v[1].x = w;
		v[1].y = h;
		v[1].z = RwIm2DGetNearScreenZ();
		v[1].rhw = rhw;
		v[1].emissiveColor = alpha << 24;
		v[2].x = 0;
		v[2].y = 0;
		v[2].z = RwIm2DGetNearScreenZ();
		v[2].rhw = rhw;
		v[2].emissiveColor = alpha << 24;
		v[3].x = w;
		v[3].y = 0;
		v[3].z = RwIm2DGetNearScreenZ();
		v[3].rhw = rhw;
		v[3].emissiveColor = alpha << 24;
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONALWAYS);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
		RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONGREATER);
	}
}

// Near-match of 0x41f3d0: min is written as `a > b ? b : a` (the game's NaN polarity); the game keeps its spills in a different order.
// A square frame (side 1.69704 * the frame buffer width) centred on the frame buffer, drawn as four black bars that close in
// by `fade` (the half-width of each bar is half the frame side times the fade), and in the middle, while fading, the button
// prompt as a grey textured quad. At fade > 0.98 the square is drawn as one black quad.
void CameraDrawLetterbox(RwCamera *camera)
{
	RwIm2DVertex v[4];
	TFBRenderState::Apply(0);
	int width = ((RwRasterView *)CameraView(camera)->frameBuffer)->width;
	int height = ((RwRasterView *)CameraView(camera)->frameBuffer)->height;
	float kx = 1.69704f * (float)width;
	int diff = height - width;
	float hf = (float)height;
	float inv = 1.0f / CameraView(camera)->nearPlane;
	float hx = ((float)width - kx) * 0.5f;
	float hxk = hx + kx;
	float hy = (float)diff * 0.5f + hx;
	float hyk = hy + kx;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	float z = RwIm2DGetNearScreenZ();
	v[0].z = z;
	v[1].z = z;
	v[2].z = z;
	v[3].z = z;
	v[0].rhw = inv;
	v[1].rhw = inv;
	v[2].rhw = inv;
	v[3].rhw = inv;
	v[0].emissiveColor = 0xff000000;
	v[1].emissiveColor = 0xff000000;
	v[2].emissiveColor = 0xff000000;
	v[3].emissiveColor = 0xff000000;
	v[0].x = hx;
	v[2].x = hx;
	v[1].x = hxk;
	v[3].x = hxk;
	if (g_camFade > 0.98f) {
		float top = hyk > hf ? hf : hyk;
		v[0].y = top;
		v[1].y = top;
		v[2].y = hy;
		v[3].y = hy;
		RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
		return;
	}
	float k2 = kx * 0.5f * g_camFade;
	v[2].y = hy;
	v[3].y = hy;
	v[0].y = k2 + hy;
	v[1].y = k2 + hy;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	float top = hyk > hf ? hf : hyk;
	v[0].y = top;
	v[1].y = top;
	float k3 = k2 + hx;
	float k2hy = k2 + hy;
	v[1].x = k3;
	v[3].x = k3;
	v[2].y = k2hy;
	v[3].y = k2hy;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	float m = hxk - k2;
	v[0].x = m;
	v[2].x = m;
	v[1].x = hxk;
	v[3].x = hxk;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	float n = hyk - k2;
	v[0].x = k3;
	v[2].x = k3;
	v[1].x = m;
	v[3].x = m;
	v[2].y = n;
	v[3].y = n;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	// The game rewrites all four positions from its spill slots (three uncleaned Im2D calls leave esp 0x24 below the frame):
	// v0=(k3,k2hy), v1=(k3,n), v2=(m,k2hy), v3=(m,n), the button rectangle as a strip.
	v[0].x = k3;
	v[0].y = k2hy;
	v[1].x = k3;
	v[1].y = n;
	v[2].x = m;
	v[2].y = k2hy;
	v[3].x = m;
	v[3].y = n;
	v[0].emissiveColor = 0xff808080;
	v[1].emissiveColor = 0xff808080;
	v[2].emissiveColor = 0xff808080;
	v[3].emissiveColor = 0xff808080;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[1].u = 0.0f;
	v[1].v = 1.0f;
	v[2].u = 1.0f;
	v[2].v = 0.0f;
	v[3].u = 1.0f;
	v[3].v = 1.0f;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)(((RwTextureView *)g_buttonTexture)->raster));
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
}

// 0x41f900
// Overlay of the running transition (g_camModeAux: 1 fade quad, 2 letterbox) while a transition or a fade is active.
// NOTE: the game passes its argument straight on to the two drawing functions with a jump; the callers pass the camera.
void CameraDrawOverlay(RwCamera *camera)
{
	if (g_camModeAux != 0 && (g_camTimeLeft != g_floatZero || g_camFade > g_floatZero)) {
		SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, kStencilPass);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
		if (g_camModeAux == kOverlayFade)
			CameraDrawFadeQuad(camera);
		else if (g_camModeAux == kOverlayLetterbox)
			CameraDrawLetterbox(camera);
	}
}
