// TFBRenderState: applies a material's blend / alpha-test block to the RW render state (with a cache of the last values), and the
// stencil / blend helpers of the world and sprite drawing.
#include "rw/rwcore.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbrenderstate.h"

// GLOBAL: 0x5d57f4 g_tfbMatDefault
// GLOBAL: 0x62a0dc g_rsBlendSrc
// GLOBAL: 0x62a0e0 g_rsBlendDst
// GLOBAL: 0x62a0e4 g_rsAlphaRef
// GLOBAL: 0x62a0e8 g_rsAlphaFunc
// GLOBAL: 0x62a0ec g_rsCull
// GLOBAL: 0x62a0f4 g_rsReset

// 0x45e0f0
void StencilWrite(bool on)
{
	if (on) {
		RwRenderStateSet(rwRENDERSTATESTENCILPASS, (void *)rwSTENCILOPERATIONREPLACE);
		RwRenderStateSet(rwRENDERSTATESTENCILFUNCTIONREF, (void *)0xff);
	} else {
		RwRenderStateSet(rwRENDERSTATESTENCILPASS, (void *)rwSTENCILOPERATIONZERO);
	}
}

// 0x45e130
void StencilEnable(unsigned char enable)
{
	RwRenderStateSet(rwRENDERSTATESTENCILENABLE, (void *)enable);
	RwRenderStateSet(rwRENDERSTATESTENCILFUNCTION, (void *)rwSTENCILFUNCTIONALWAYS);
}

// 0x45e2e0
void SetBlendAndAlphaTest(RwBlendFunction srcBlend, RwBlendFunction destBlend, int alphaRef)
{
	g_rsBlendSrc = srcBlend;
	g_rsBlendDst = destBlend;
	g_rsAlphaRef = alphaRef;
	g_rsAlphaFunc = rwALPHATESTFUNCTIONGREATER;
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)(intptr_t)g_rsBlendDst);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void *)(intptr_t)g_rsAlphaRef);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)(intptr_t)g_rsAlphaFunc);
	g_rsCull = rwCULLMODECULLBACK;
	RwRenderStateSet(rwRENDERSTATESTENCILPASS, (void *)rwSTENCILOPERATIONZERO);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
}

// 0x45e370
bool TFBRenderState::Apply(RpMaterial *material)
{
	const TFBRenderState *f;
	if (material == 0) {
		f = &g_tfbMatDefault;
	} else {
		if (TFBMaterialGetData(material)->m_flagBytes[0] & TFB_MATERIAL_HIDDEN)
			return false;
		f = &TFBMaterialGetData(material)->m_renderState;
	}
	if (f->m_srcBlend != g_rsBlendSrc) {
		g_rsBlendSrc = f->m_srcBlend;
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)(intptr_t)f->m_srcBlend);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(intptr_t)(f->m_srcBlend == rwBLENDONE ? 0 : 1));
	}
	if (f->m_destBlend != g_rsBlendDst) {
		g_rsBlendDst = f->m_destBlend;
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)(intptr_t)f->m_destBlend);
	}
	if (f->m_alphaRef != g_rsAlphaRef) {
		g_rsAlphaRef = f->m_alphaRef;
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void *)(intptr_t)f->m_alphaRef);
	}
	if (f->m_alphaFunc != g_rsAlphaFunc) {
		g_rsAlphaFunc = f->m_alphaFunc;
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)(intptr_t)f->m_alphaFunc);
	}
	if (RwCameraGetProjection(RwCameraGetCurrentCamera()) != rwPARALLEL) {
		if ((f->m_flags >> 1) & 1) {
			RwRenderStateSet(rwRENDERSTATESTENCILPASS, (void *)rwSTENCILOPERATIONREPLACE);
			RwRenderStateSet(rwRENDERSTATESTENCILFUNCTIONREF, (void *)255);
		} else {
			RwRenderStateSet(rwRENDERSTATESTENCILPASS, (void *)rwSTENCILOPERATIONZERO);
		}
		if ((f->m_flags >> 1) & 1) {
			RwRenderStateSet(rwRENDERSTATESTENCILENABLE, (void *)1);
			RwRenderStateSet(rwRENDERSTATESTENCILFUNCTION, (void *)rwSTENCILFUNCTIONALWAYS);
		}
	}
	if (g_rsReset) {
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONALWAYS);
		g_rsBlendSrc = rwBLENDONE;
		g_rsBlendDst = rwBLENDZERO;
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
	}
	return true;
}

// 0x45d930
RpMaterial *TFBRenderStateApplyCB(RpMaterial *material)
{
	return TFBRenderState::Apply(material) ? material : 0;
}
