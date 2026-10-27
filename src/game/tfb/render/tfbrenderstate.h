// TFBRenderState: the blend and alpha-test settings of a material (a 0x14-byte block of its plugin data, or the game's default block)
// and the cached render-state calls that apply them; plus the stencil and blend helpers the world and sprite drawing share.
#ifndef ZOOSTERS_TFB_RENDER_TFBRENDERSTATE_H
#define ZOOSTERS_TFB_RENDER_TFBRENDERSTATE_H

#include "zoosters/base.h"
#include "rw/rpworld.h"

struct TFBRenderState {
	int m_srcBlend;     // RwBlendFunction for rwRENDERSTATESRCBLEND
	int m_destBlend;    // RwBlendFunction for rwRENDERSTATEDESTBLEND
	int m_alphaRef;     // rwRENDERSTATEALPHATESTFUNCTIONREF
	int m_alphaFunc;    // rwRENDERSTATEALPHATESTFUNCTION (RwAlphaTestFunction)
	unsigned m_flags;   // bit 1: write the stencil (pass REPLACE, reference 255)

	// Sets whatever of the block differs from the cached state (g_rs*); with no material the default block is used. Returns false (and sets nothing) for a hidden material (TFB_MATERIAL_HIDDEN).
	static bool Apply(RpMaterial *material);
};
ZOOSTERS_ASSERT_SIZE(TFBRenderState, 0x14);

extern const TFBRenderState g_tfbMatDefault;  // the default block (a material's block starts as a copy of it)
extern int g_rsBlendSrc;  // cache of the last values Apply / SetBlendAndAlphaTest set
extern int g_rsBlendDst;
extern int g_rsAlphaRef;
extern int g_rsAlphaFunc;
extern int g_rsCull;  // (SetBlendAndAlphaTest stores 2 = rwCULLMODECULLBACK)
extern int g_rsReset;  // when set, Apply ends by restoring ONE / ZERO and ALWAYS

// The per-material callback of the pipeline instance step (the global at 0x631d20; 0 clears it); TFBRenderStateApplyCB is what the
// atomic and sector renders install: it applies the material's render state and skips a hidden material.
typedef RpMaterial *(*TFBMaterialCallBack)(RpMaterial *material);
extern "C" {
void FUN_004c80d0(TFBMaterialCallBack callback); // 0x4c80d0
}
RpMaterial *TFBRenderStateApplyCB(RpMaterial *material);  // the material, or 0 when Apply refuses it

// With `on` the stencil is written (pass REPLACE, reference 0xFF), else its pass op is ZERO.
void StencilWrite(bool on);
// Enables or disables the stencil test (function ALWAYS).
void StencilEnable(unsigned char enable);
// Blend `srcBlend` / `destBlend`, alpha test GREATER with `alphaRef`, stencil pass ZERO, vertex alpha on; updates the cache.
void SetBlendAndAlphaTest(RwBlendFunction srcBlend, RwBlendFunction destBlend, int alphaRef);

#endif
