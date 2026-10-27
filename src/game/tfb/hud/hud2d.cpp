// Hud2D: font cache, frame begin / end, and the drawing of one Hud2DObject (text, textured quad, billboard).
#include "tfb/hud/hud2d.h"
#include "framework/core/cresourcemanager.h"
#include "rw/rpcollis.h"
#include "rw/rwd3d8.h"
#include "modules/tfb/camera_views.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "tfb/math/boundingsphere.h"
#include "rw/rttoolkits.h"
#include "tfb/render/tfbrenderstate.h"
#include "tfb/render/atomicflags.h"
#include "tfb/render/tfbclump.h"
#ifdef ZOOSTERS_OPTIONS
#include "tfb/level/resourcehandlers_views.h"
#endif

// GLOBAL: 0x464ef0 ??1Hud2DObject@@QAE@XZ
// GLOBAL: 0x629e60 g_fontCache
// GLOBAL: 0x629f1c g_fontCacheCount
// GLOBAL: 0x629ed8 g_hudBrush
// GLOBAL: 0x629f30 g_hudText
// GLOBAL: 0x629edc g_hudScreenA
// GLOBAL: 0x629ee0 g_hudScreenB
// GLOBAL: 0x629ee4 g_hudScreenC
// GLOBAL: 0x629ee8 g_hudScreenD
// GLOBAL: 0x629eec g_hudScreenE
// GLOBAL: 0x629ef0 g_hudScreenH
// GLOBAL: 0x629ef4 g_hudScreenW
// GLOBAL: 0x629f18 g_hudQuadState
// GLOBAL: 0x629ef8 g_hudQuad
// GLOBAL: 0x62a038 g_hudVerts
// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x5caf40 g_degToRad

int _stricmp(const char *a, const char *b);

extern float g_hudScreenA, g_hudScreenB, g_hudScreenC, g_hudScreenD, g_hudScreenE, g_hudScreenH, g_hudScreenW;
#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_frame(void);                                  // port/fixes/widescreen.cpp
extern "C" void port_wide2d_hud(float W, float H, float *B, float *D);
#endif
extern int g_hudQuadState;
extern int g_hudQuad[8];
extern RwIm2DVertex g_hudVerts[4];
extern const float g_floatZero;
extern const float g_degToRad;



namespace Hud2D {
// 0x450220
RwTexture *LoadSpriteTexture(const char *name)
{
	char path[0x34];
	D3DTexture8 *d3dTex;
	D3DSurfaceDesc desc;
	D3DLockedRect rect;
	RwTexture *texture = 0;
	if (_stricmp(name, "MOUSE_POINTER") == 0)
		strcpy(path, "Data\\Images\\MousePointer.png");
	else if (_stricmp(name, "SPRITE_X_BUTTON") == 0)
		strcpy(path, "Data\\Images\\CrossSmall.png");
	else if (_stricmp(name, "XSPRITE") == 0)
		strcpy(path, "Data\\Images\\Cross.png");
	else if (_stricmp(name, "SPRITE_TRIANGLE_BUTTON") == 0)
		strcpy(path, "Data\\Images\\TriangleSmall.png");
	else if (_stricmp(name, "TRIANGLESPRITE") == 0)
		strcpy(path, "Data\\Images\\Triangle.png");
	else if (_stricmp(name, "SPRITE_SQUARE_BUTTON") == 0)
		strcpy(path, "Data\\Images\\SquareSmall.png");
	else if (_stricmp(name, "SQUARESPRITE") == 0)
		strcpy(path, "Data\\Images\\Square.png");
	else if (_stricmp(name, "SPRITE_O_BUTTON") == 0)
		strcpy(path, "data\\Images\\CircleSmall.png");
	else if (_stricmp(name, "CIRCLESPRITE") == 0)
		strcpy(path, "data\\Images\\Circle.png");
	else
		return RwTextureRead(name, 0);
	if (D3DXCreateTextureFromFileA(RwD3D8GetCurrentD3DDevice(), path, &d3dTex) == 0) {
		d3dTex->GetLevelDesc(0, &desc);
		texture = RwTextureCreate(RwRasterCreate(desc.width, desc.height, 32, 0x504));
		unsigned *dst = (unsigned *)RwRasterLock(((RwTextureView *)texture)->raster, 0, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH);
		if (dst != 0) {
			d3dTex->LockRect(0, &rect, 0, 0x10);
			unsigned count = ((RwRasterView *)((RwTextureView *)texture)->raster)->height * ((RwRasterView *)((RwTextureView *)texture)->raster)->width;
			unsigned *src = (unsigned *)rect.bits;
			for (; count > 0; count--) {
				*dst = *src;
				src++;
				dst++;
			}
			d3dTex->UnlockRect(0);
			RwRasterUnlock(((RwTextureView *)texture)->raster);
		}
		d3dTex->Release();
	}
	return texture;
}

// 0x450490
void DestroyFontCache()
{
	for (int i = 0; i < g_fontCacheCount; i++) {
		if (g_fontCache[i].m_font != 0) {
			Rt2dFontDestroy(g_fontCache[i].m_font);
			g_fontCache[i].m_name = 0;
			g_fontCache[i].m_font = 0;
			g_fontCache[i].m_refCount = 0;
		}
	}
	g_fontCacheCount = 0;
}

// 0x4504e0
void Close()
{
	DestroyFontCache();
	if (g_hudBrush) {
		Rt2dBrushDestroy(g_hudBrush);
	}
	g_hudBrush = 0;
}

// 0x450510
static __declspec(noinline) FontCacheEntry *FindCachedFont(const char *name)
{
	for (int i = 0; i < g_fontCacheCount; i++) {
		if (_stricmp(name, g_fontCache[i].m_name) == 0)
			return &g_fontCache[i];
	}
	return 0;
}

// 0x450560
static __declspec(noinline) bool FindFontResource(FontCacheEntry *e, const char *name, Rt2dFont *font)
{
	e->m_name = name;
	e->m_font = font;
	if (font == 0) {
		char guid[16];
		unsigned size = 0;
		const char *type = 0;
		const void *res = RWS::CResourceManager::FindByName(name, (RWS::RWSGUID *)guid, &type, &size);
		if (res != 0) {
			if (rwstrcmp("rwID_2DFONT", type) == 0)
				e->m_font = (Rt2dFont *)res;
		}
	}
	if (e->m_font) {
		e->m_refCount = 1;
		return true;
	}
	return false;
}

// 0x4505d0
static __declspec(noinline) FontCacheEntry *LoadFontResource(Rt2dFont *font, const char *name)
{
	FontCacheEntry *e = g_fontCache;
	if (font != 0) {
		for (int i = 0; i < g_fontCacheCount; i++, e++) {
			if (e->m_font == font)
				return e;
		}
	}
	if (g_fontCacheCount < 10) {
		if (FindFontResource(&g_fontCache[g_fontCacheCount], name, font))
			g_fontCacheCount++;
		return e;
	}
	return 0;
}

// 0x4506b0
Rt2dFont *GetFont(Rt2dFont *res, const char *name)
{
	FontCacheEntry *e = FindCachedFont(name);
	if (e == 0)
		e = LoadFontResource(res, name);
	if (e != 0) {
		e->m_refCount++;
		return e->m_font;
	}
	return 0;
}

// 0x4506e0
void ReleaseFont(const char *name)
{
	FontCacheEntry *e = FindCachedFont(name);
	if (e)
		e->m_refCount--;
}


// 0x450870
void SetRenderStates()
{
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
	TFBRenderState::Apply(0);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
}

// 0x450900
bool BeginFrame(RwCamera *cam)
{
	if (cam != 0) {
		RwRGBA colour;
		RwCameraSetNearClipPlane(cam, 20.2f);
		RwCameraSetFarClipPlane(cam, 2000.0f);
		CameraView(cam)->fogPlane = 2000.0f;   // RwCameraSetFogDistance
		RwCameraClear(cam, &colour, rwCAMERACLEARZ);
		RwCameraBeginUpdate(cam);
		g_hudScreenA = 0.049504947f;
		const RwRasterView *frameBuffer = (const RwRasterView *)CameraView(cam)->frameBuffer;
		g_hudScreenW = (float)frameBuffer->width;
		g_hudScreenH = (float)frameBuffer->height;
		g_hudScreenB = 0.0f;
		g_hudScreenC = 0.0f;
		g_hudScreenD = g_hudScreenW * 0.0015625f;
		g_hudScreenE = g_hudScreenH * 0.0020833334f;
#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only (port/fixes/widescreen.cpp): the 640 x 480 grid keeps its 4:3 shape
		port_wide2d_frame();
		port_wide2d_hud(g_hudScreenW, g_hudScreenH, &g_hudScreenB, &g_hudScreenD);
#endif
		Rt2dDeviceSetLayerDepth(20.402f);
		SetBlendAndAlphaTest(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, 2);
		SetRenderStates();
		g_hudQuadState = 0;
		return true;
	}
	return false;
}

// 0x4509d0
RwCamera *EndFrame(RwCamera *cam)
{
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
	return RwCameraEndUpdate(cam);
}

#ifdef ZOOSTERS_OPTIONS   // display change at run time: the cached fonts were scaled to the old screen height when loaded (RwResLoader::Read2DFont)
void RescaleFonts(float ratio)
{
	for (int i = 0; i < g_fontCacheCount; i++)
		if (g_fontCache[i].m_font)
			((Rt2dFontView *)g_fontCache[i].m_font)->height *= ratio;
}
#endif

// 0x451470
void Open()
{
	g_hudBrush = Rt2dBrushCreate();
}

}

// 0x450620
void Hud2DObject::Init()
{
	m_posZ = 240.0f;
	m_drawY = 240.0f;
	m_drawScaleX = 1.0f;
	m_drawScaleY = 1.0f;
	m_scaleY = 1.0f;
	m_scaleX = 1.0f;
	m_distanceScale = 1.0f;
	m_drawAlpha = 1.0f;
	m_flags = 0;
	m_texture = 0;
	m_font = 0;
	m_clump = 0;
	m_format = 0;
	m_cullBack = 0;
	m_rotation = 0.0f;
	m_boxWidth = 0.0f;
	m_boxHeight = 0.0f;
	m_value = 0;
	m_posX = 320.0f;
	m_posY = 3.4028235e38f;
	m_tint.alpha = 0xff;
	m_tint.blue = 0xff;
	m_tint.green = 0xff;
	m_tint.red = 0xff;
	m_offsetY = 0.0f;
	m_offsetX = 0.0f;
	m_drawX = 320.0f;
	m_drawRotation = 0.0f;
	m_drawTint.red = 0xff;
	m_drawTint.green = 0xff;
	m_drawTint.blue = 0xff;
	m_drawTint.alpha = 0xff;
}

// 0x450700
void Hud2DObject::SetFormat(const char *fmt)
{
	m_format = fmt;
	if (fmt != 0) {
		m_flags &= ~0x14;
		int n = 0;
		char c = *fmt;
		while (c != 0) {
			if (c == '%') {
				c = *++fmt;
				if (c == 'd') {
					n++;
					m_flags |= 4;
				} else if (c == 't') {
					n++;
					m_flags |= 0x10;
				}
			}
			c = *++fmt;
		}
		if (n != 1)
			m_flags &= ~0x14;
	}
}

// 0x450750
const char *Hud2DObject::FormatTimeHMS()
{
	char tmp[12];
	if (m_flags & 4) {
		sprintf(g_hudText, m_format, m_value);
		return g_hudText;
	}
	if (m_flags & 0x10) {
		int t = m_value;
		int sec = t % 60;
		int mins = (t - sec) / 60;
		int hrs = mins / 60;
		mins = mins % 60;
		if (hrs > 99) {
			sec = 59;
			hrs = 99;
			mins = sec;
		}
		sprintf(tmp, "%02d:%02d:%02d", hrs, mins, sec);
		const char *src = m_format;
		char *dst = g_hudText;
		while (*src != 0) {
			if (src[0] == '%' && src[1] == 't') {
				src += 2;
				const char *q = tmp;
				while (*q != 0)
					*dst++ = *q++;
			} else {
				*dst++ = *src++;
			}
		}
		*dst = 0;
		return g_hudText;
	}
	return m_format;
}

// 0x450850
void Hud2DObject::SetSpriteByName(const char *name)
{
	if (name != 0)
		m_texture = Hud2D::LoadSpriteTexture(name);
}

// 0x413ac0
void Hud2DObject::SetSizeFromTexture()
{
	RwTextureView *tex = (RwTextureView *)m_texture;
	if (tex) {
		m_boxWidth = (float)((RwRasterView *)tex->raster)->width;
		m_boxHeight = (float)((RwRasterView *)tex->raster)->height;
	}
}

// 0x450840
void Hud2DObject::SetTexture(RwTexture *texture)
{
	m_texture = texture;
}

// 0x451460
void Hud2DObject::SetCullBack(unsigned char v)
{
	m_cullBack = v;
}

// 0x4509f0
void Hud2DObject::GetAnchorOffset(float *out, int anchor)
{
	out[1] = 0;
	out[0] = 0;
	switch (anchor) {
	case 1: out[0] = -m_width; break;
	case 2: out[0] = m_width * -0.5f; break;
	case 7: out[0] = -m_width; out[1] = m_height * -0.5f; break;
	case 8: out[0] = m_width * -0.5f;
	case 6: out[1] = m_height * -0.5f; break;
	case 4: out[0] = -m_width; out[1] = -m_height; break;
	case 5: out[0] = m_width * -0.5f;
	case 3: out[1] = -m_height; break;
	}
}

// 0x451110
bool Hud2DObject::ComputeScreenQuad()
{
	float *q = (float *)g_hudQuad;
	float t = -m_drawRotation;
	double a;
	double sn = sin(a = (double)t * g_degToRad);
	double cs = cos(a);
	float h = m_height;
	float w = m_width;
	float X = (m_drawScaleX * m_offsetX * cs - m_drawScaleY * m_offsetY * sn + m_drawX);
	float Y = (m_drawScaleY * m_offsetY * cs + m_drawScaleX * m_offsetX * sn + m_drawY);
	float hs = h * sn;
	float hc;
	q[0] = (X - hs) * g_hudScreenD + g_hudScreenB;
	q[1] = ((hc = h * cs) + Y) * g_hudScreenE + g_hudScreenC;
	float wcx = (w * cs + X);
	q[2] = (wcx - hs) * g_hudScreenD + g_hudScreenB;
	q[3] = ((hc + w * sn) + Y) * g_hudScreenE + g_hudScreenC;
	q[4] = g_hudScreenD * X + g_hudScreenB;
	q[5] = g_hudScreenE * Y + g_hudScreenC;
	q[6] = g_hudScreenD * wcx + g_hudScreenB;
	q[7] = (Y + w * sn) * g_hudScreenE + g_hudScreenC;
	int common = -1;
	for (float *p = q + 1; p < q + 9; p += 2) {
		int outcode = 0;
		if (p[-1] < g_floatZero)
			outcode = 2;
		if (p[-1] > g_hudScreenW)
			outcode |= 8;
		if (p[0] < g_floatZero)
			outcode |= 1;
		if (p[0] > g_hudScreenH)
			outcode |= 4;
		common &= outcode;
		if (common == 0)
			return true;
	}
	return false;
}


// 0x4512b0
void Hud2DObject::DrawQuad()
{
	if (g_hudQuadState != 1) {
		if (g_hudQuadState == 3)
			Hud2D::SetRenderStates();
		int rhw = *(int *)&g_hudScreenA;
		int rhw1 = rhw;
		int rhw2 = rhw;
		g_hudQuadState = 1;
		g_hudVerts[0].z = 20.2f;
		*(int *)&g_hudVerts[0].rhw = rhw;
		g_hudVerts[0].u = 0.0f;
		g_hudVerts[0].v = 1.0f;
		g_hudVerts[1].z = 20.2f;
		*(int *)&g_hudVerts[1].rhw = rhw1;
		g_hudVerts[1].u = 1.0f;
		g_hudVerts[1].v = 1.0f;
		g_hudVerts[2].z = 20.2f;
		*(int *)&g_hudVerts[2].rhw = rhw2;
		g_hudVerts[2].u = 0.0f;
		g_hudVerts[2].v = 0.0f;
		g_hudVerts[3].z = 20.2f;
		*(int *)&g_hudVerts[3].rhw = rhw;
		g_hudVerts[3].u = 1.0f;
		g_hudVerts[3].v = 0.0f;
	}
	unsigned char r = m_drawTint.red;
	unsigned char g = m_drawTint.green;
	unsigned char b = m_drawTint.blue;
	unsigned alpha = (unsigned char)(int)(m_drawAlpha * 255.0f);
	*(int *)&g_hudVerts[0].x = g_hudQuad[0];
	*(int *)&g_hudVerts[0].y = g_hudQuad[1];
	unsigned colour = (((((alpha << 8) | r) << 8) | g) << 8) | b;
	*(int *)&g_hudVerts[1].x = g_hudQuad[2];
	*(int *)&g_hudVerts[1].y = g_hudQuad[3];
	*(int *)&g_hudVerts[2].x = g_hudQuad[4];
	*(int *)&g_hudVerts[2].y = g_hudQuad[5];
	g_hudVerts[0].emissiveColor = colour;
	g_hudVerts[1].emissiveColor = colour;
	g_hudVerts[2].emissiveColor = colour;
	*(int *)&g_hudVerts[3].x = g_hudQuad[6];
	*(int *)&g_hudVerts[3].y = g_hudQuad[7];
	g_hudVerts[3].emissiveColor = colour;
	RwD3D8SetTextureStageState(0, 0x10, 2);
	RwD3D8SetTextureStageState(0, 0x11, 2);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)((RwTextureView *)m_texture)->raster);
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, g_hudVerts, 4);
}


// 0x450ab0
float Hud2DObject::MeasureText(float *outW, float *outH)
{
	const char *text = FormatTimeHMS();
	*outH = Rt2dFontGetHeight(m_font);
	if (!(m_flags & 2)) {
		float w = Rt2dFontGetStringWidth(m_font, text, Rt2dFontGetHeight(m_font));
		*outW = w;
		if ((m_flags & 1) && w > m_boxWidth)
			return m_boxWidth / w;
		return 1.0f;
	}
	float limit = m_boxWidth;
	*outW = 0;
	int lines = 0;
	float cur = 0;
	char *last = 0;
	char *sp = (char *)text;
	do {
		cur = 0;
		sp = (char *)text;
		last = 0;
		for (;;) {
			sp = strchr(sp, 0x20);
			if (sp)
				*sp = 0;
			float w = Rt2dFontGetStringWidth(m_font, text, Rt2dFontGetHeight(m_font));
			if (!(w <= limit)) {
				if (sp)
					*sp = 0x20;
				break;
			}
			cur = w;
			last = sp;
			if (!sp)
				break;
			*sp = 0x20;
			sp++;
		}
		if (cur == g_floatZero)
			break;
		if (cur > *outW)
			*outW = cur;
		lines++;
		text = last + 1;
	} while (sp != 0);
	if (last)
		lines++;
	float t = (float)lines * *outH;
	*outH = t;
	if ((m_flags & 1) && t > m_boxHeight)
		return m_boxHeight / t;
	return 1.0f;
}

// 0x450c30
void Hud2DObject::DrawText()
{
	if (g_hudQuadState == 3)
		Hud2D::SetRenderStates();
	g_hudQuadState = 2;
	Rt2dCTMPush();
	RwRGBA c;
	c.red = m_drawTint.red;
	c.green = m_drawTint.green;
	c.blue = m_drawTint.blue;
	c.alpha = (unsigned char)(m_drawAlpha * 255.0f);
	Rt2dBrushSetRGBA(g_hudBrush, &c, &c, &c, &c);
	Rt2dCTMTranslate(m_drawX, 480.0f - m_drawY);
	Rt2dCTMRotate(m_drawRotation);
	Rt2dCTMScale(m_drawScaleX, m_drawScaleY);
	if (!(m_flags & 2)) {
		RwV2d anchor;
		float h = m_height / m_drawScaleY;
		anchor.x = m_offsetX;
		anchor.y = -(h + m_offsetY);
		Rt2dFontShow(m_font, FormatTimeHMS(), h, &anchor, g_hudBrush);
		Rt2dCTMPop();
		return;
	}
	const char *text = FormatTimeHMS();
	float lineH = Rt2dFontGetHeight(m_font) * m_drawScaleY;
	RwV2d anchor;
	anchor.y = -(lineH + m_offsetY);
	float limit = m_width / m_drawScaleX;
	char *last;
	char *sp;
	float cur;
	do {
		cur = 0;
		sp = (char *)text;
		last = 0;
		for (;;) {
			sp = strchr(sp, 0x20);
			if (sp)
				*sp = 0;
			float w = Rt2dFontGetStringWidth(m_font, text, lineH);
			if (!(w <= m_boxWidth)) {   // the game tests `!(w <= m_boxWidth)` (NaN polarity); `limit` only feeds the alignment
				if (sp)
					*sp = 0x20;
				break;
			}
			cur = w;
			last = sp;
			if (!sp)
				break;
			*sp = 0x20;
			sp++;
		}
		if (cur == g_floatZero)
			break;
		anchor.x = m_offsetX;
		switch (m_anchor - 1) {
		case 0: case 3: case 6:
			anchor.x += limit - cur;
			break;
		case 1: case 4: case 7:
			anchor.x += (limit - cur) * 0.5f;
			break;
		}
		if (last)
			*last = 0;
		Rt2dFontShow(m_font, text, lineH, &anchor, g_hudBrush);
		if (last)
			*last = 0x20;
		anchor.y -= lineH;
		text = last + 1;
	} while (sp != 0 || last != 0);
	Rt2dCTMPop();
}


void ClumpSetAtomicColour(RpClump *clump, RwRGBA *c);                           // 0x44ed00
float *_rt2dCTMGet();                                                      // 0x467bf0
extern const RwV3d DAT_005d54b0;   // rotation axis of the model tilt (x)
extern const RwV3d DAT_005d54c8;   // of the roll (z)
extern const RwV3d DAT_005d54bc;   // of the facing (y)

// 0x450e80
void Hud2DObject::DrawBillboard()
{
	Rt2dDeviceSetLayerDepth(1000.0f);
	Rt2dCTMPush();
	Rt2dCTMTranslate(m_drawX, 480.0f - m_drawY);
	Rt2dCTMRotate(m_drawRotation);
	float sx = m_drawScaleX;
	float sy = m_drawScaleY;
	if (sx == g_floatZero)
		sx = 0.0001f;
	if (sy == g_floatZero)
		sy = 0.0001f;
	Rt2dCTMScale(sx, sy);
	RwFrame *frame = RpClumpGetFrame(m_clump);
	RwMatrix *modelling = &frame->modelling;
	float *ctm = _rt2dCTMGet();
	memcpy(modelling, ctm, 0x40);
	RwV3d offset;   // the model is drawn at its 2D position: the matrix' "at" vector scaled out to the 3D depth
	offset.x = modelling->at.x * 1000.0f;
	offset.y = modelling->at.y * 1000.0f;
	offset.z = modelling->at.z * 1000.0f;
	if (m_tilt != g_floatZero)
		RwFrameRotate(frame, &DAT_005d54b0, m_tilt, rwCOMBINEPRECONCAT);
	if (m_roll != g_floatZero)
		RwFrameRotate(frame, &DAT_005d54c8, m_roll, rwCOMBINEPRECONCAT);
	if (m_facing != g_floatZero)
		RwFrameRotate(frame, &DAT_005d54bc, m_facing, rwCOMBINEPRECONCAT);
	RwFrameTranslate(frame, &offset, rwCOMBINEPOSTCONCAT);
	RwSphere sphere;
	sphere.radius = 0.0f;
	RwLLLink *head = &m_clump->atomicList.link;
	RwLLLink *n = head->next;
	while (n != head) {
		RwLLLink *next = n->next;
		if (!AtomicGrowSphereCB(RpAtomicFromClumpLink(n), &sphere))
			break;
		n = next;
	}
	if (sphere.radius != g_floatZero && RwCameraFrustumTestSphere(RwCameraGetCurrentCamera(), &sphere)) {
		if (g_hudQuadState != 3) {
			g_hudQuadState = 3;
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
		}
		RwRGBA c;
		c.red = m_drawTint.red;
		c.green = m_drawTint.green;
		c.blue = m_drawTint.blue;
		c.alpha = (unsigned char)(int)(m_drawAlpha * 255.0f);
		ClumpSetAtomicColour(m_clump, &c);
		if (m_cullBack)
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
		else
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLFRONT);
		TFBClump_Render(m_clump);
	}
	Rt2dCTMPop();
	Rt2dDeviceSetLayerDepth(20.402f);
}
