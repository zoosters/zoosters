#include "rw/rwcore.h"
#include "tfb/render/tfbatomic.h"
#include "tfb/render/tfbclump.h"
#include "tfb/render/tfbgeometry.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbrenderstate.h"
#include "tfb/render/tfbsector.h"
#include "framework/mainloop/gameclock.h"

float CameraGetValueD0();           // 0x41e450
int *CutScene_StackSlot(int delta); // cut-scene record; its first word is the state
namespace RWS { namespace MainLoop { namespace Render { unsigned GetFrameCount(); } } }  // 0x462710

enum {
	kFadeOpaqueBits = 0x3F800000,   // 1.0f
	kFadeUnsetBits = 0x7F7FFFFF     // FLT_MAX: the smoothed fade has no value yet
};

// A colour as a word and as bytes (the game fades the alpha byte of the word it read).
union TFBColorWord {
	RwUInt32 bits;
	RwRGBA rgba;
};

// 0x45bce0
RpAtomic *TFBAtomic::RenderCallback(RpAtomic *atomic)
{
	RpGeometry *geometry = RpAtomicGetGeometry(atomic);
	if (!geometry)
		return atomic;
	TFBAtomicData *d;
	d = TFBAtomicGetData(atomic);
	unsigned geometryFlags = RpGeometryGetFlags(geometry) | rpGEOMETRYMODULATEMATERIALCOLOR;
	if (d->m_flags & TFB_ATOMIC_LIT)
		geometryFlags |= rpGEOMETRYLIGHT;
	else
		geometryFlags &= ~(unsigned)rpGEOMETRYLIGHT;
	RpGeometrySetFlags(geometry, geometryFlags);
	TFBColorWord color;
	color.bits = d->m_colorBits;
	if (d->m_fade < 1.0f)
		color.rgba.alpha = (unsigned char)(int)((int)(color.bits >> 24) * d->m_fade);
	if (!(color.rgba.alpha > 0))
		return atomic;
	float time = (float)(TFBClock_GetRawTime() - d->m_createTime) * 0.001f;
	FUN_004c80d0(TFBRenderStateApplyCB);
	int numMaterials = RpGeometryGetNumMaterials(geometry);
	for (int i = 0; i < numMaterials; i++) {
		RpMaterial *material = RpGeometryGetMaterial(geometry, i);
		if (material) {
			unsigned char *materialFlagByte = &TFBMaterialGetData(material)->m_flagBytes[0];
			if (*materialFlagByte & TFB_MATERIAL_SECTOR_FADE) {
				float fade = (d->m_viewDistance - g_secFadeStart) / g_secFadeRange;
				if (fade > 1.0f)
					fade = 1.0f;
				else if (fade < 0.0f)
					fade = 0.0f;
				color.rgba.alpha = (unsigned char)(int)((1.0f - fade) * color.rgba.alpha);
				unsigned materialFlags = TFBMaterialGetData(material)->m_flags;
				if ((float)color.rgba.alpha > 0.0f)
					materialFlags &= ~(unsigned)TFB_MATERIAL_HIDDEN;
				else
					materialFlags |= TFB_MATERIAL_HIDDEN;
				TFBMaterialGetData(material)->m_flags = materialFlags;
			}
			TFBMaterial::UpdateAnimation(material, time, 0, &color.rgba);
		}
	}
	if ((geometryFlags & rpGEOMETRYPRELIT) && (geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR))
		TFBGeometry_RelockPrelight(geometry);
	if (d->m_flags & TFB_ATOMIC_SORT_TRIANGLES)
		TFBGeometry_ResortTriangles(geometry, time);
	else
		TFBRenderState::Apply(0);
	int zTestEnabled = 0;
	if (d->m_flags & TFB_ATOMIC_NO_ZTEST) {
		RwRenderStateGet(rwRENDERSTATEZTESTENABLE, (void *)&zTestEnabled);
		if (zTestEnabled)
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
	}
	RxPipeline *pipeline;
	RpAtomicGetPipeline(atomic, &pipeline);
	if (!pipeline)
		pipeline = RpAtomicGetDefaultPipeline();
	RxPipelineExecute(pipeline, atomic, 1);
	FUN_004c80d0(0);
	if (zTestEnabled)
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)(intptr_t)zTestEnabled);
	return atomic;
}

// 0x45ce10
int TFBAtomic::Update(RpAtomic *atomic, RwCamera *camera, float viewDistance)
{
	if (g_tfbFdOfs > 0) {
		TFBAtomicData *d = TFBAtomicGetData(atomic);
		d->m_viewDistance = viewDistance;
		if (d->m_billboardFlags & TFB_BILLBOARD_ON)
			UpdateBillboardFrame(atomic, camera);
		float fade = 1.0f;
		if ((d->m_flags & TFB_ATOMIC_FADE_RANGE) && viewDistance > d->m_fadeFrom) {
			float progress = (viewDistance - d->m_fadeFrom) / (d->m_fadeTo - d->m_fadeFrom);
			if (progress < 1.0f)
				fade = 1.0f - progress;
			else
				fade = 0.0f;
		}
		if ((d->m_flags & TFB_ATOMIC_NO_DISTANCE_DIM) == 0) {
			if (viewDistance + 40.0f < CameraGetValueD0())
				fade = fade * 0.3f;
		}
		if (fade < 1.0f || d->m_fadeBits != kFadeOpaqueBits) {
			switch (*CutScene_StackSlot(0)) {
			case 2:
				if (d->m_fadeBits == kFadeUnsetBits) {
					d->m_fade = fade;
				} else {
					d->m_fade = fade * 0.19999999f + d->m_fade * 0.8f;
					float difference = d->m_fade - fade;
					if (difference < 0.01f && difference > -0.01f)
						d->m_fade = fade;
				}
				break;
			case 3:
				break;
			default:
				d->m_fadeBits = kFadeOpaqueBits;
				break;
			}
		}
		RpClump *clump = RpAtomicGetClump(atomic);
		if (clump)
			TFBPlugin_B1_SetRenderFrame(clump, RWS::MainLoop::Render::GetFrameCount());
		if ((d->m_flags & TFB_ATOMIC_FLAG_08) == 0 && d->m_color.alpha == 0xff && !(d->m_fade < 1.0f))
			return 0;
		return 1;
	}
	return 0;
}
