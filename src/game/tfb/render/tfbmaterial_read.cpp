// TFB material plugin 0x800000F6: the stream readers of the plugin data. A format id of 0xF0F0001..7 is the oldest layout (the
// layers and their embedded textures); each later id adds fields: 8 a TFBLayerBlock per layer, 9 two bytes per layer, 10 the blend
// mode, alpha test and UV animation, 11 the texture list, 12 the colour and alpha animation clips, 13 one more byte.
#include "rw/rwcore.h"
#include "tfb/anim/kfanimation.h"
#include "tfb/anim/texlist.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbplugins.h"

extern unsigned g_tfbMatMagicLo; // GLOBAL: 0x6068c0 g_tfbMatMagicLo

enum TFBMaterialFormat {
	TFB_MATERIAL_FORMAT_7 = 0xF0F0007,    // the last of the original layout (formats 1..7 read alike)
	TFB_MATERIAL_FORMAT_8 = 0xF0F0008,
	TFB_MATERIAL_FORMAT_9 = 0xF0F0009,
	TFB_MATERIAL_FORMAT_10 = 0xF0F000A,
	TFB_MATERIAL_FORMAT_11 = 0xF0F000B,
	TFB_MATERIAL_FORMAT_12 = 0xF0F000C,
	TFB_MATERIAL_FORMAT_13 = 0xF0F000D
};
enum { kChunkTexture = 6 };

// 0x45e650
int TFBLayerBlock::Read(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_version, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_unk04, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_unk08, 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[0], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[1], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[2], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[3], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[4], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[5], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[6], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[7], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[8], 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_unk0c[9], 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_unk34, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_unk38, 4))
		return 0;
	return 0x3c;
}

// Layers of the original layout (formats <= 7): per layer a real and seven words (the texture word is dropped), then the textures
// embedded after the layers are read and dropped.
// 0x45f320
RwStream *TFBPlugin_F6_ReadV7(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBMaterialData *d = TFBMaterialGetData((RpMaterial *)object);
	unsigned *flags = &d->m_flags;
	RwStreamReadInt32(stream, (RwInt32 *)flags, 4);
	if (!(d->m_flagBytes[0] & TFB_MATERIAL_ANIMATED)) {
		d->m_body.Init();
		return stream;
	}
	int *texCount = &d->m_body.m_texCount;
	RwStreamReadInt32(stream, texCount, 4);
	RwStreamReadInt32(stream, (RwInt32 *)&d->m_body.m_flags.m_bits, 4);
	for (int i = 0; i < 4; i++) {
		TFBMatLayer *layer = d->m_body.Layer(i);
		RwStreamReadReal(stream, &layer->m_unk00, 4);
		RwStreamReadInt32(stream, (RwInt32 *)&layer->m_color, 4);
		RwStreamReadInt32(stream, &layer->m_unk08, 4);
		RwStreamReadInt32(stream, &layer->m_unk0c, 4);
		RwStreamReadInt32(stream, (RwInt32 *)&layer->m_flags.m_bits, 4);
		RwTexture **texture = &layer->m_texture;
		RwStreamReadInt32(stream, (RwInt32 *)texture, 4); // PTR32: a stale pointer word, dropped below
		RwStreamReadInt32(stream, &layer->m_unk18, 4);
		RwStreamReadInt32(stream, &layer->m_unk1c, 4);
		*texture = 0;
	}
	for (int j = 0; j < *texCount; j++) {
		if (RwStreamFindChunk(stream, kChunkTexture, 0, 0)) {
			RwTextureStreamRead(stream);
			int skipped;
			RwStreamReadInt32(stream, &skipped, 4);
			RwStreamReadInt32(stream, &skipped, 4);
			RwStreamReadInt32(stream, &skipped, 4);
		}
	}
	RpMatFXMaterialSetEffects((RpMaterial *)object, rpMATFXEFFECTNULL);
	return stream;
}

// Format 8: each layer carries a TFBLayerBlock after its words.
// 0x45f4a0
RwStream *TFBPlugin_F6_ReadV8(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBMaterialData *d = TFBMaterialGetData((RpMaterial *)object);
	unsigned *flags = &d->m_flags;
	RwStreamReadInt32(stream, (RwInt32 *)flags, 4);
	if (!(d->m_flagBytes[0] & TFB_MATERIAL_ANIMATED)) {
		d->m_body.Init();
		return stream;
	}
	int *texCount = &d->m_body.m_texCount;
	RwStreamReadInt32(stream, texCount, 4);
	RwStreamReadInt32(stream, (RwInt32 *)&d->m_body.m_flags.m_bits, 4);
	for (int i = 0; i < 4; i++) {
		TFBMatLayer *layer = d->m_body.Layer(i);
		RwStreamReadReal(stream, &layer->m_unk00, 4);
		RwStreamReadInt32(stream, (RwInt32 *)&layer->m_color, 4);
		RwStreamReadInt32(stream, &layer->m_unk08, 4);
		RwStreamReadInt32(stream, &layer->m_unk0c, 4);
		RwStreamReadInt32(stream, (RwInt32 *)&layer->m_flags.m_bits, 4);
		RwTexture **texture = &layer->m_texture;
		RwStreamReadInt32(stream, (RwInt32 *)texture, 4);
		RwStreamReadInt32(stream, &layer->m_unk18, 4);
		RwStreamReadInt32(stream, &layer->m_unk1c, 4);
		*texture = 0;
		layer->m_block.Read(stream);
	}
	for (int j = 0; j < *texCount; j++) {
		if (RwStreamFindChunk(stream, kChunkTexture, 0, 0)) {
			RwTextureStreamRead(stream);
			int skipped;
			RwStreamReadInt32(stream, &skipped, 4);
			RwStreamReadInt32(stream, &skipped, 4);
			RwStreamReadInt32(stream, &skipped, 4);
		}
	}
	RpMatFXMaterialSetEffects((RpMaterial *)object, rpMATFXEFFECTNULL);
	return stream;
}

// Format 9: format 8 plus two bytes per layer.
// 0x45f630
RwStream *TFBPlugin_F6_ReadV9(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBMaterialData *d = TFBMaterialGetData((RpMaterial *)object);
	unsigned *flags = &d->m_flags;
	RwStreamReadInt32(stream, (RwInt32 *)flags, 4);
	if (!(d->m_flagBytes[0] & TFB_MATERIAL_ANIMATED)) {
		d->m_body.Init();
		return stream;
	}
	int *texCount = &d->m_body.m_texCount;
	RwStreamReadInt32(stream, texCount, 4);
	RwStreamReadInt32(stream, (RwInt32 *)&d->m_body.m_flags.m_bits, 4);
	for (int i = 0; i < 4; i++) {
		TFBMatLayer *layer = d->m_body.Layer(i);
		RwStreamReadReal(stream, &layer->m_unk00, 4);
		RwStreamReadInt32(stream, (RwInt32 *)&layer->m_color, 4);
		RwStreamReadInt32(stream, &layer->m_unk08, 4);
		RwStreamReadInt32(stream, &layer->m_unk0c, 4);
		RwStreamReadInt32(stream, (RwInt32 *)&layer->m_flags.m_bits, 4);
		RwTexture **texture = &layer->m_texture;
		RwStreamReadInt32(stream, (RwInt32 *)texture, 4);
		RwStreamReadInt32(stream, &layer->m_unk18, 4);
		RwStreamReadInt32(stream, &layer->m_unk1c, 4);
		RwStreamRead(stream, &layer->m_unk20, 1);
		RwStreamRead(stream, &layer->m_unk21, 1);
		*texture = 0;
		layer->m_block.Read(stream);
	}
	for (int j = 0; j < *texCount; j++) {
		if (RwStreamFindChunk(stream, kChunkTexture, 0, 0)) {
			RwTextureStreamRead(stream);
			int skipped;
			RwStreamReadInt32(stream, &skipped, 4);
			RwStreamReadInt32(stream, &skipped, 4);
			RwStreamReadInt32(stream, &skipped, 4);
		}
	}
	RpMatFXMaterialSetEffects((RpMaterial *)object, rpMATFXEFFECTNULL);
	return stream;
}

// Format 10: flags, blend mode (sets the blend pair), alpha test, and the UV animation reals (with a UV matrix) when the UV flag is set.
// 0x45e8d0
RwStream *TFBPlugin_F6_ReadV10(RwStream *stream, int length, void *object, int offset, int size)
{
	RpMaterial *material = (RpMaterial *)object;
	TFBMaterialData *d = TFBMaterialGetData(material);
	RwStreamReadInt32(stream, (RwInt32 *)&d->m_flags, 4);
	unsigned *blendMode = &d->m_blendMode;
	RwStreamReadInt32(stream, (RwInt32 *)blendMode, 4);
	TFBMaterial_SetBlendMode(material, *blendMode);
	RwStreamReadInt32(stream, &d->m_renderState.m_alphaRef, 4);
	RwStreamReadInt32(stream, &d->m_renderState.m_alphaFunc, 4);
	if (TFBMaterialGetData(material)->m_flagBytes[0] & TFB_MATERIAL_UV_SCROLL) {
		RwStreamReadReal(stream, &d->m_uvAnim[0], 4);
		RwStreamReadReal(stream, &d->m_uvAnim[1], 4);
		RwStreamReadReal(stream, &d->m_uvAnim[2], 4);
		RwStreamReadReal(stream, &d->m_uvAnim[3], 4);
		RwStreamReadReal(stream, &d->m_uvAnim[4], 4);
		RwStreamReadReal(stream, &d->m_uvAnim[5], 4);
		RwStreamReadReal(stream, &d->m_uvAnim[6], 4);
		RwStreamReadReal(stream, &d->m_uvAnim[7], 4);
		RpMatFXMaterialSetEffects(material, rpMATFXEFFECTUVTRANSFORM);
		RwMatrix *uvMatrix = RwMatrixCreate();
		d->m_uvMatrix = uvMatrix;
		RpMatFXMaterialSetUVTransformMatrices(material, uvMatrix, 0);
	}
	return stream;
}

// Format 11: format 10 plus the texture list (count, then the elements).
// 0x45e9e0
RwStream *TFBPlugin_F6_ReadV11(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBMaterialData *d = TFBMaterialGetData((RpMaterial *)object);
	TFBPlugin_F6_ReadV10(stream, length, object, offset, size);
	int *count = &d->m_texListCount;
	RwStreamReadInt32(stream, count, 4);
	if (*count > 0) {
		d->m_texList = new TexList[*count];
		for (int i = 0; i < *count; i++)
			d->m_texList[i].Load(stream);
	}
	return stream;
}

// Format 12: format 11 plus the colour and alpha animation clips (each present when its count word is positive).
// 0x45eaf0
RwStream *TFBPlugin_F6_ReadV12(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBMaterialData *d = TFBMaterialGetData((RpMaterial *)object);
	TFBPlugin_F6_ReadV11(stream, length, object, offset, size);
	int *present = (int *)&object; // the dead argument slot holds the word (keeps the game's stack layout)
	RwStreamReadInt32(stream, present, 4);
	if (*present > 0) {
		TFB::KFAnimation *clip = new TFB::KFAnimation;
		d->m_colorAnim = clip;
		clip->Read(stream);
	}
	RwStreamReadInt32(stream, present, 4);
	if (*present > 0) {
		TFB::KFAnimation *clip = new TFB::KFAnimation;
		d->m_alphaAnim = clip;
		clip->Read(stream);
	}
	return stream;
}

// Format 13: format 12 plus one byte.
// 0x45ebf0
RwStream *TFBPlugin_F6_ReadV13(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBMaterialData *d = TFBMaterialGetData((RpMaterial *)object);
	TFBPlugin_F6_ReadV12(stream, length, object, offset, size);
	RwStreamRead(stream, &d->m_skipShadow, 1);
	return stream;
}

// 0x45f7d0
RwStream *TFBPlugin_F6_Read(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBMaterialData *d = TFBMaterialGetData((RpMaterial *)object);
	if (!RwStreamReadInt32(stream, (RwInt32 *)&d->m_version, 4))
		return 0;
	unsigned version = d->m_version;
	if (version < g_tfbMatMagicLo || version > g_tfbMatMagic) {
		d->m_body.Init();
		return stream;
	}
	if (version <= TFB_MATERIAL_FORMAT_7)
		TFBPlugin_F6_ReadV7(stream, length, object, offset, size);
	else if (version == TFB_MATERIAL_FORMAT_8)
		TFBPlugin_F6_ReadV8(stream, length, object, offset, size);
	else if (version == TFB_MATERIAL_FORMAT_9)
		TFBPlugin_F6_ReadV9(stream, length, object, offset, size);
	else if (version == TFB_MATERIAL_FORMAT_10)
		TFBPlugin_F6_ReadV10(stream, length, object, offset, size);
	else if (version == TFB_MATERIAL_FORMAT_11)
		TFBPlugin_F6_ReadV11(stream, length, object, offset, size);
	else if (version == TFB_MATERIAL_FORMAT_12)
		TFBPlugin_F6_ReadV12(stream, length, object, offset, size);
	else if (version == TFB_MATERIAL_FORMAT_13)
		TFBPlugin_F6_ReadV13(stream, length, object, offset, size);
	if (d->m_flags & TFB_MATERIAL_STENCIL)
		d->m_renderState.m_flags |= 2;
	return stream;
}
