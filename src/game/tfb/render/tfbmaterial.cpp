// TFB material plugin 0x800000F6: per-frame animation (colour and alpha tracks, texture list, UV scroll, tint) and the blend-mode setter.
#include <math.h>
#include <stdlib.h>
#include "rw/rwcore.h"
#include "tfb/anim/kfanimation.h"
#include "tfb/anim/texlist.h"
#include "tfb/render/tfbmaterial.h"

extern RwV3d g_tfbUvScroll;   // GLOBAL: 0x62a0f8 g_tfbUvScroll the translation handed to the UV matrix
extern float g_tfbEnvScale;   // GLOBAL: 0x6068d8 g_tfbEnvScale

// 0x45e230
void TFBMaterial_SetBlendMode(RpMaterial *material, unsigned blendMode)
{
	if (g_tfbMatOfs > 0) {
		TFBMaterialData *d = TFBMaterialGetData(material);
		d->m_blendMode = blendMode;
		switch (blendMode) {
		case 0:
			d->m_renderState.m_srcBlend = rwBLENDONE;
			d->m_renderState.m_destBlend = rwBLENDZERO;
			break;
		case 1:
			d->m_renderState.m_destBlend = rwBLENDONE;
			d->m_renderState.m_srcBlend = rwBLENDSRCALPHA;
			break;
		case 2:
			d->m_renderState.m_srcBlend = rwBLENDINVSRCALPHA;
			d->m_renderState.m_destBlend = rwBLENDSRCALPHA;
			break;
		case 4:
			d->m_renderState.m_srcBlend = rwBLENDSRCALPHA;
			d->m_renderState.m_destBlend = rwBLENDSRCALPHA;
			break;
		default:
			d->m_renderState.m_destBlend = rwBLENDINVSRCALPHA;
			d->m_renderState.m_srcBlend = rwBLENDSRCALPHA;
			break;
		}
	}
}

// UV scroll at `time`: per axis offset = sin(time * frequency + phase) * amplitude + time * speed (fractional part), applied to the material's UV matrix.
// 0x45e4f0
void TFBMaterial_UpdateUVScroll(RpMaterial *material, float time)
{
	if (g_tfbMatOfs > 0) {
		TFBMaterialData *d = TFBMaterialGetData(material);
		float fx = (float)sin(time * d->m_uvFrequency[0] + d->m_uvPhase[0]) * d->m_uvAmplitude[0] + time * d->m_uvSpeed[0];
		float fy = (float)sin(time * d->m_uvFrequency[1] + d->m_uvPhase[1]) * d->m_uvAmplitude[1] + time * d->m_uvSpeed[1];
		d->m_uvOffset[0] = fx - (int)fx;
		d->m_uvOffset[1] = fy - (int)fy;
		g_tfbUvScroll.x = d->m_uvOffset[0];
		g_tfbUvScroll.y = -d->m_uvOffset[1];
		g_tfbUvScroll.z = 0;
		RwMatrix *uvMatrix = d->m_uvMatrix;
		if (uvMatrix)
			RwMatrixTranslate(uvMatrix, &g_tfbUvScroll, rwCOMBINEREPLACE);
	}
}

// 0x45e5d0
void TFBMaterial_UpdateTexAnim(RpMaterial *material, float time)
{
	if (g_tfbMatOfs > 0) {
		TFBMaterialData *d = TFBMaterialGetData(material);
		int *count = &d->m_texListCount;
		TexList **list = &d->m_texList;
		int n = *count;
		if (n > 0) {
			for (int i = 0; i < *count; i++) {
				TexList *entry = &(*list)[i];
				entry->Update(time);
				RpMaterialSetTexture(material, entry->Pick());
			}
		}
	}
}

// 0x45ec30
void TFBMaterial::UpdateAnimation(RpMaterial *material, float time, int halve, const RwRGBA *tint)
{
	RwRGBA *color = RpMaterialGetColorRef(material);
	unsigned char red0 = color->red;
	unsigned char green0 = color->green;
	unsigned char alpha0 = color->alpha;
	unsigned char blue0 = color->blue;
	TFBMaterialData *d = TFBMaterialGetData(material);
	unsigned flags = d->m_flags;
	if (flags & TFB_MATERIAL_COLOR_SAVED) {
		*color = d->m_savedColor;
	} else {
		d->m_flags = flags | TFB_MATERIAL_COLOR_SAVED;
		d->m_savedColor = *color;
		if (halve) {
			RwRGBA *saved = &d->m_savedColor;
			saved->red >>= 1;
			saved->green >>= 1;
			saved->blue >>= 1;
			*color = *saved;
		}
	}
	if (d->m_flags & TFB_MATERIAL_ANIMATED) {
		if (d->m_flags & TFB_MATERIAL_UV_SCROLL)
			TFBMaterial_UpdateUVScroll(material, time);
		if (d->m_texListCount)
			TFBMaterial_UpdateTexAnim(material, time);
		if (d->m_colorAnim) {
			KeyValue value; // four floats: red, green, blue, (alpha unused)
			RwRGBA animated;
			d->m_colorAnim->SetTime(time);
			d->m_colorAnim->GetCurrentValue(&value);
			if (g_tfbMatOfs > 0) {
				animated.alpha = color->alpha;
				animated.red = (unsigned char)(int)(value.v[0] * 255.0f);
				animated.green = (unsigned char)(int)(value.v[1] * 255.0f);
				animated.blue = (unsigned char)(int)(value.v[2] * 255.0f);
				*color = animated;
			}
		}
		if (d->m_alphaAnim) {
			KeyValue value; // the first float is the alpha
			RwRGBA animated;
			d->m_alphaAnim->SetTime(time);
			d->m_alphaAnim->GetCurrentValue(&value);
			if (g_tfbMatOfs > 0) {
				animated.alpha = (unsigned char)(int)(value.v[0] * 255.0f);
				animated.red = color->red;
				animated.green = color->green;
				animated.blue = color->blue;
				*color = animated;
			}
		}
	}
	if (tint) {
		color->red = (unsigned char)(int)(color->red * (float)tint->red * 0.003921569f);
		color->green = (unsigned char)(int)(color->green * (float)tint->green * 0.003921569f);
		color->blue = (unsigned char)(int)(color->blue * (float)tint->blue * 0.003921569f);
		color->alpha = (unsigned char)(int)(color->alpha * (float)tint->alpha * 0.003921569f);
	}
	// the alpha test below runs with or without the tint; the game reads the alpha byte back from the material, so it is read through the const accessor here
	if (RpMaterialGetColor(material)->alpha == 0xff) {
		d->m_flags &= ~TFB_MATERIAL_HIDDEN;
	} else {
		unsigned newFlags = d->m_flags;
		if (color->alpha <= 0)
			newFlags |= TFB_MATERIAL_HIDDEN;
		else
			newFlags &= ~TFB_MATERIAL_HIDDEN;
		d->m_flags = newFlags;
		// the game jumps over this block when alpha == 0xff
		if (d->m_blendMode == 0 && g_tfbMatOfs > 0) {
			TFBMaterialData *data = TFBMaterialGetData(material);
			data->m_blendMode = 3;
			data->m_renderState.m_srcBlend = rwBLENDSRCALPHA;
			data->m_renderState.m_destBlend = rwBLENDINVSRCALPHA;
		}
	}
	unsigned changedFlags;
	if (alpha0 == color->alpha && red0 == color->red && green0 == color->green && blue0 == color->blue)
		changedFlags = d->m_flags & ~TFB_MATERIAL_COLOR_CHANGED;
	else
		changedFlags = d->m_flags | TFB_MATERIAL_COLOR_CHANGED;
	d->m_flags = changedFlags;
	if (RpMatFXMaterialGetEffects(material) == rpMATFXEFFECTENVMAP) {
		int redBlue = abs((int)color->red - (int)color->blue);
		int redGreen = abs((int)color->red - (int)color->green);
		g_tfbEnvScale = 0.8f;
		if (redBlue >= 16 || redGreen >= 16)
			g_tfbEnvScale = 0.3f;
		RpMatFXMaterialSetEnvMapCoefficient(material, (float)color->alpha * g_tfbEnvScale * 0.003921569f);
	}
}
