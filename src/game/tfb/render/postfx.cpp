// Pixel fog, depth-of-field and bloom post effect.

#include "rw/rwcore.h"
#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"
#include "script/types/scriptsets.h"
#include "script/types/scriptprop.h"
#include "modules/tfb/levelhub.h"
#include <stdlib.h>
#include "rw/rwd3d8.h"
#include "tfb/render/tfbrenderstate.h"
#include "tfb/render/camera_stub.h"

extern float g_pixelFogDist;          // GLOBAL: 0x617d90 g_pixelFogDist
extern int g_pixelCutoff;             // GLOBAL: 0x617d98 g_pixelCutoff
extern float g_levelFarDefault;       // GLOBAL: 0x60297c g_levelFarDefault
extern unsigned char g_distClip;    // GLOBAL: 0x62a59c g_distClip
extern int g_distance;               // GLOBAL: 0x62a5a0 g_distance
extern int g_dofBluriness;            // GLOBAL: 0x602980 g_dofBluriness
extern float g_bloomAddFactor;        // GLOBAL: 0x602984 g_bloomAddFactor
extern float g_bloomHaloSize;         // GLOBAL: 0x602988 g_bloomHaloSize
extern unsigned char g_bloomEnable;   // GLOBAL: 0x617d95 g_bloomEnable
extern int g_flag60298c;              // GLOBAL: 0x60298c g_flag60298c
extern float g_f60299c;               // GLOBAL: 0x60299c g_f60299c
extern float g_f6029a0;               // GLOBAL: 0x6029a0 g_f6029a0
extern float g_f602990;               // GLOBAL: 0x602990 g_f602990
extern float g_f602994;               // GLOBAL: 0x602994 g_f602994
extern float g_f617d54;               // GLOBAL: 0x617d54 g_f617d54
extern float g_pixelFogEnd;              // GLOBAL: 0x617d50 g_pixelFogEnd
extern float g_f617d6c;               // GLOBAL: 0x617d6c g_f617d6c
extern int g_i617d74;                 // GLOBAL: 0x617d74 g_i617d74
extern int g_i617d78;                 // GLOBAL: 0x617d78 g_i617d78
extern int g_i617d84;                 // GLOBAL: 0x617d84 g_i617d84
extern int g_i617d88;                 // GLOBAL: 0x617d88 g_i617d88
extern int g_i617d8c;                 // GLOBAL: 0x617d8c g_i617d8c
int GetCurrentLevel();                   // 0x460c20
void DrawBloomOverlay();                   // 0x411720

// the suffix is the RwRenderState (rwplcore.h): 1 TEXTURERASTER, 2 TEXTUREADDRESS, 6 ZTESTENABLE, 7 SHADEMODE, 8 ZWRITEENABLE,
// 9 TEXTUREFILTER, 0xa SRCBLEND, 0xb DESTBLEND, 0xc VERTEXALPHAENABLE, 0xe FOGENABLE, 0x14 CULLMODE
extern void *g_rsSaved1;   // GLOBAL: 0x617cf8 g_rsSaved1 (the saved raster pointer)
extern int g_rsSaved7;     // GLOBAL: 0x617cf4 g_rsSaved7
extern int g_rsSavedC;     // GLOBAL: 0x617cf0 g_rsSavedC
extern int g_rsSaved6;     // GLOBAL: 0x617cec g_rsSaved6
extern int g_rsSaved8;     // GLOBAL: 0x617ce8 g_rsSaved8
extern int g_rsSavedA;     // GLOBAL: 0x617ce4 g_rsSavedA
extern int g_rsSavedB;     // GLOBAL: 0x617ce0 g_rsSavedB
extern int g_rsSaved14;    // GLOBAL: 0x617cdc g_rsSaved14
extern int g_rsSaved2;     // GLOBAL: 0x617cd8 g_rsSaved2
extern int g_rsSaved9;     // GLOBAL: 0x617cd4 g_rsSaved9
extern int g_rsSavedE;     // GLOBAL: 0x617cd0 g_rsSavedE


extern unsigned char g_pixelFogColor[4];   // GLOBAL: 0x617d60 (annotated in gfx/small_e_expose.cpp)
extern int g_pixelCutoff;                  // GLOBAL: 0x617d98 (annotated elsewhere)
void SavePixelFogRenderStates();
void RestorePixelFogRenderStates();

// GLOBAL: 0x617d04 g_d3dDevice411
// GLOBAL: 0x617d1c g_bloomTexture
// GLOBAL: 0x617d40 g_bloomHeight
// GLOBAL: 0x617d44 g_bloomWidth
// GLOBAL: 0x602984 g_bloomAddFactor
struct DevVt { void **vt; };
extern DevVt *g_d3dDevice411;
extern DevVt *g_bloomTexture;
extern unsigned g_bloomHeight, g_bloomWidth;
extern float g_bloomAddFactor;

struct SurfDescB { unsigned fmt, type, usage, pool, size, ms, width, height; };
struct VertB { float x, y, z, rhw, u, v; };
#define DEV_RS(state, value) ((int(__stdcall *)(DevVt *, int, unsigned))g_d3dDevice411->vt[0xC8 / 4])(g_d3dDevice411, (state), (value))
#define DEV_TSS(stage, type, value) ((int(__stdcall *)(DevVt *, int, int, int))g_d3dDevice411->vt[0xFC / 4])(g_d3dDevice411, (stage), (type), (value))
#define DEV_TEX(stage, tex) ((int(__stdcall *)(DevVt *, int, DevVt *))g_d3dDevice411->vt[0xF4 / 4])(g_d3dDevice411, (stage), (tex))

// 0x411200
void SetPixelFogDistance(float v)
{
	g_pixelFogDist = v;
}

// 0x411110
// the pair 0x602990 / 0x602994 is the pixel fog range: 0.995 minus the spread scaled by distance / cutoff distance, and 0.995
int UpdatePixelFogRange()
{
	float a = g_flag60298c ? g_f6029a0 : g_f60299c;
	float v;
	if (g_pixelFogDist > g_f617d54)
		v = 0.995f;
	else
		v = 0.995f - a * (g_pixelFogDist / g_f617d54);
	g_f602994 = 0.995f;
	g_f602990 = v;
	return 0;
}

// 0x4112c0
char UpdatePixelFogCutoff(RwCamera *camera)   // unused
{
	g_f617d54 = g_levelFarDefault * ((101.0f - (float)g_pixelCutoff) * 0.01f);
	return 1;
}

// 0x4112f0
int UpdatePixelFogCutoffAndRange(RwCamera *camera, RwRaster *raster)   // unused
{
	g_f617d54 = g_levelFarDefault * ((101.0f - (float)g_pixelCutoff) * 0.01f);
	return UpdatePixelFogRange();
}

// 0x411180
float GetPixelFogStartDistance()
{
	if (g_distClip && GetCurrentLevel() != 1 && GetCurrentLevel() != 3 && GetCurrentLevel() != 8 && GetCurrentLevel() != 0xb &&
	    GetCurrentLevel() != 0xe && GetCurrentLevel() != 0xd && GetCurrentLevel() != 0xf) {
		float reduced = g_pixelFogDist / (3.0f - (float)g_distance * 0.02f);
		if (reduced < g_pixelFogDist)
			return reduced;
	}
	return g_pixelFogDist;
}

// 0x411320
void SetPixelFogEnd(float v)
{
	if (g_distClip && GetCurrentLevel() != 1 && GetCurrentLevel() != 3 && GetCurrentLevel() != 8 && GetCurrentLevel() != 0xb &&
	    GetCurrentLevel() != 0xe && GetCurrentLevel() != 0xd && GetCurrentLevel() != 0xf) {
		float reduced = v / ((3.0f - (float)g_distance * 0.02f) * 2.0f);
		g_pixelFogEnd = reduced < v ? reduced : v;
		return;
	}
	g_pixelFogEnd = v;
}

// 0x411210
int SetDofBluriness(int v)
{
	if ((float)v < 0.0f || (float)v > 100.0f)
		return g_dofBluriness;
	int sharpness = 100 - v;
	g_dofBluriness = v;
	float scale = (float)sharpness * 0.01f;
	// the blur target size: width scaled from 0x617d78, at least 64; height = width / the aspect 0x617d6c, or 0x617d74 at full size
	int width = (int)(scale * g_i617d78);
	if (width <= 0x40)
		width = 0x40;
	int height;
	if (width > g_i617d78) {
		width = g_i617d78;
		height = g_i617d74;
	} else {
		height = (int)((float)width / g_f617d6c);
	}
	g_i617d84 = height;
	g_i617d88 = width;
	return v;
}

// 0x411b20
void DrawBloom()
{
	if (g_bloomAddFactor != 0.0f) {
		float halo = g_bloomHaloSize * 4.0f;
		if (halo > 1.0f)
			halo = 1.0f;
		g_i617d8c = (int)(halo * 13.0f);
		DrawBloomOverlay();
	}
}

// 0x411b70
void DrawBloomIfEnabled(RwCamera *camera)   // unused
{
	if (g_bloomEnable)
		DrawBloom();
}

// 0x4113b0
void SavePixelFogRenderStates()
{
	RwRenderStateGet(rwRENDERSTATETEXTURERASTER, (void *)&g_rsSaved1);
	RwRenderStateGet(rwRENDERSTATESHADEMODE, (void *)&g_rsSaved7);
	RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)&g_rsSavedC);
	RwRenderStateGet(rwRENDERSTATEZTESTENABLE, (void *)&g_rsSaved6);
	RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, (void *)&g_rsSaved8);
	RwRenderStateGet(rwRENDERSTATESRCBLEND, (void *)&g_rsSavedA);
	RwRenderStateGet(rwRENDERSTATEDESTBLEND, (void *)&g_rsSavedB);
	RwRenderStateGet(rwRENDERSTATECULLMODE, (void *)&g_rsSaved14);
	RwRenderStateGet(rwRENDERSTATETEXTUREADDRESS, (void *)&g_rsSaved2);
	RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, (void *)&g_rsSaved9);
	RwRenderStateGet(rwRENDERSTATEFOGENABLE, (void *)&g_rsSavedE);
}

// 0x411470
void RestorePixelFogRenderStates()
{
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, g_rsSaved1);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)(intptr_t)g_rsSaved7);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(intptr_t)g_rsSavedC);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)(intptr_t)g_rsSaved6);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(intptr_t)g_rsSaved8);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)(intptr_t)g_rsSavedA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)(intptr_t)g_rsSavedB);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)(intptr_t)g_rsSaved14);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)(intptr_t)g_rsSaved2);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)(intptr_t)g_rsSaved9);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)(intptr_t)g_rsSavedE);
}

// Full-screen quad in the pixel fog colour (alpha = the colour's alpha, else the cutoff intensity), between camera begin/end with a fixed 2D render state set. The alpha/colour bytes are read as word [0x617d62] + dword [0x617d60], like the game.
// 0x411540
void DrawPixelFogQuad(RwCamera *camera)
{
	RwCameraStub *cam = (RwCameraStub *)camera;
	RwIm2DVertex v[4];
	RwCameraBeginUpdate((RwCamera *)cam);
	StencilEnable(0);
	float w = (float)cam->m_frameBuffer->m_width;
	float h = (float)cam->m_frameBuffer->m_height;
	float z = RwIm2DGetNearScreenZ();
	float rhw = 1.0f / cam->m_nearPlane;
	unsigned short ba = *(unsigned short *)(g_pixelFogColor + 2);
	unsigned char alpha = (unsigned char)(ba >> 8);
	if (alpha == 0)
		alpha = (unsigned char)(int)((float)g_pixelCutoff * 2.55f);
	unsigned col = (((((unsigned)alpha << 8) | g_pixelFogColor[0]) << 8 | g_pixelFogColor[1]) << 8) | (unsigned char)ba;
	v[0].z = z; v[0].rhw = rhw;
	v[1].z = z; v[1].rhw = rhw;
	v[2].z = z; v[2].rhw = rhw;
	v[3].z = z; v[3].rhw = rhw;
	v[0].x = 0; v[0].y = 0; v[1].x = w; v[1].y = 0; v[2].x = 0; v[2].y = h; v[3].x = w; v[3].y = h;
	v[0].emissiveColor = col; v[1].emissiveColor = col; v[2].emissiveColor = col; v[3].emissiveColor = col;
	v[0].u = 0; v[0].v = 0; v[1].u = 0; v[1].v = 0; v[2].u = 0; v[2].v = 0; v[3].u = 0; v[3].v = 0;
	SavePixelFogRenderStates();
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	RestorePixelFogRenderStates();
	StencilEnable(1);
	RwCameraEndUpdate((RwCamera *)cam);
}

// 0x411720
void DrawBloomOverlay()
{
	if (!g_bloomTexture)
		return;
	SurfDescB desc;
	((int(__stdcall *)(DevVt *, int, SurfDescB *))g_bloomTexture->vt[0x38 / 4])(g_bloomTexture, 0, &desc);
	VertB v[4];
	float right = (float)g_bloomWidth - 0.5f;
	float bottom = (float)g_bloomHeight - 0.5f;
	v[0].x = -0.5f; v[0].y = -0.5f; v[0].z = 1.0f; v[0].rhw = 1.0f; v[0].u = 0.0f; v[0].v = 0.0f;
	v[1].x = right; v[1].y = -0.5f; v[1].z = 1.0f; v[1].rhw = 1.0f; v[1].u = (float)desc.width; v[1].v = 0.0f;
	v[2].x = -0.5f; v[2].y = bottom; v[2].z = 1.0f; v[2].rhw = 1.0f; v[2].u = 0.0f; v[2].v = (float)desc.height;
	v[3].x = right; v[3].y = bottom; v[3].z = 1.0f; v[3].rhw = 1.0f; v[3].u = (float)desc.width; v[3].v = (float)desc.height;
	DEV_TEX(0, g_bloomTexture);
	float alpha = g_bloomAddFactor + g_bloomAddFactor;
	if (alpha > 1.0f)
		alpha = 1.0f;
	unsigned color = (unsigned)(int)(alpha * 255.0f) << 24;
	RwD3D8SetRenderState(0x3c, color);
	DEV_RS(0x3c, color);
	DEV_TSS(0, 1, 2);
	DEV_TSS(0, 2, 2);
	DEV_TSS(0, 4, 2);
	DEV_TSS(0, 5, 3);
	DEV_TSS(0, 0xd, 3);
	DEV_TSS(0, 0xe, 3);
	DEV_TSS(0, 0x11, 2);
	DEV_TSS(0, 0x10, 2);
	DEV_RS(7, 0);
	DEV_RS(0xe, 0);
	DEV_RS(0x16, 1);
	DEV_RS(0xf, 0);
	DEV_RS(0x1b, 1);
	DEV_RS(0xab, 1);
	DEV_RS(0x13, 5);
	DEV_RS(0x14, 2);
	DEV_RS(8, 3);
	((int(__stdcall *)(DevVt *, unsigned))g_d3dDevice411->vt[0x130 / 4])(g_d3dDevice411, 0x104);
	((int(__stdcall *)(DevVt *, int, int, void *, int))g_d3dDevice411->vt[0x120 / 4])(g_d3dDevice411, 5, 2, v, 0x18);
	RwD3D8SetRenderState(0x3c, 0xFF808080);
	DEV_TSS(0, 4, 2);
	DEV_TSS(0, 5, 2);
	DEV_RS(0x1b, 0);
	DEV_RS(0xf, 0);
	DEV_RS(7, 1);
	DEV_TEX(0, 0);
}
