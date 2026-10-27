// TFB material plugin 0x800000F6: layer / body helpers and the plugin callbacks (constructor, destructor, copy, stream size, registration). Stream readers: tfbmaterial_read.cpp.
#include "rw/rwcore.h"
#include "tfb/anim/kfanimation.h"
#include "tfb/anim/texlist.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbplugins.h"
#include <string.h>

// GLOBAL: 0x6068c8 g_tfbMatEngOfs
// GLOBAL: 0x6068cc g_tfbMatOfs
// GLOBAL: 0x6068c4 g_tfbMatMagic

// 0x45e7a0
void TFBMatBody::Destroy()
{
	for (int i = 0; i < 4; i++) {
		TFBMatLayer *layer = Layer(i);
		if (layer->m_texture)
			RwTextureDestroy(layer->m_texture);
		if (layer->m_90)
			delete layer->m_90;
		if (layer->m_94)
			delete layer->m_94;
	}
}

// 0x45f200
void TFBMatBody::Init()
{
	m_flags.Set(0);
	m_texCount = 0;
	for (int i = 0; i < 4; i++)
		Layer(i)->Init();
}

// Resets a layer: white, flags 0, no texture, a block of version 0x0F0F0001, bytes 2 and 5, no owned objects.
// 0x45f140
void TFBMatLayer::Init()
{
	unsigned full = 0xff;
	m_unk18 = 0;
	m_color.alpha = (unsigned char)full;
	m_color.red = (unsigned char)full;
	m_color.green = (unsigned char)full;
	m_color.blue = (unsigned char)full;
	m_unk1c = full;
	m_flags.Set(0);
	m_texture = 0;
	m_unk0c = 0;
	m_unk08 = 0;
	m_block.m_version = 0x0F0F0001;
	m_block.m_unk04 = 0;
	m_block.m_unk08 = 0;
	m_block.m_unk0c[1] = 0;
	m_block.m_unk0c[0] = 0;
	m_block.m_unk0c[3] = 0;
	m_block.m_unk0c[2] = 0;
	m_block.m_unk0c[5] = 0;
	m_block.m_unk0c[4] = 0;
	m_block.m_unk0c[7] = 0;
	m_block.m_unk0c[6] = 0;
	m_block.m_unk0c[9] = 0;
	m_block.m_unk0c[8] = 0;
	m_block.m_unk38 = 0;
	m_block.m_unk34 = 0;
	m_unk60[0] = 0;
	m_unk60[2] = 0;
	m_unk60[1] = 0;
	m_unk60[3] = 0;
	m_unk60[4] = 0;
	m_unk20 = 2;
	m_unk21 = 5;
	m_sub.m_unk04 = 0;
	m_sub.m_unk08 = 0;
	m_sub.m_unk0c = 0;
	m_90 = 0;
	m_94 = 0;
}

// Constructor of the plugin data: version stamp, flags 0, blend mode 3, the default render state, layers reset, everything else cleared.
// 0x45f240
void *TFBPlugin_F6_Ctor(void *object, int offset, int size)
{
	RpMaterial *material = (RpMaterial *)object;
	if (g_tfbMatOfs > 0) {
		TFBMaterialData *d = TFBMaterialGetData(material);
		d->m_version = g_tfbMatMagic;
		TFBMaterialGetData(material)->m_flags = 0;
		TFBMaterialGetData(material)->m_blendMode = 3;
		TFBRenderState *state = &TFBMaterialGetData(material)->m_renderState;
		*state = g_tfbMatDefault;
		TFBMatBody *body = g_tfbMatOfs > 0 ? &TFBMaterialGetData(material)->m_body : 0;
		body->Init();
		memset(TFBMaterialGetData(material)->m_materials, 0, sizeof(TFBMaterialGetData(material)->m_materials));
		d->m_uvMatrix = 0;
		d->m_uvOffset[1] = 0;
		d->m_uvOffset[0] = 0;
		d->m_texListCount = 0;
		d->m_texList = 0;
		d->m_colorAnim = 0;
		d->m_alphaAnim = 0;
		d->m_skipShadow = 0;
	}
	return material;
}

// 0x45e800
void *TFBPlugin_F6_Dtor(void *object, int offset, int size)
{
	RpMaterial *material = (RpMaterial *)object;
	if (g_tfbMatOfs > 0) {
		TFBMaterialData *d = TFBMaterialGetData(material);
		d->m_body.Destroy();
		RpMaterial **sub = d->m_materials;
		for (int i = 4; i != 0; i--, sub++) {
			if (*sub)
				RpMaterialDestroy(*sub);
		}
		TexList *texList = d->m_texList;
		if (texList) {
			d->m_texListCount = 0;
			delete[] texList;
			d->m_texList = 0;
		}
		TFB::KFAnimation *colorAnim = d->m_colorAnim;
		if (colorAnim)
			delete colorAnim;
		TFB::KFAnimation *alphaAnim = d->m_alphaAnim;
		if (alphaAnim)
			delete alphaAnim;
		RwMatrix *uvMatrix = d->m_uvMatrix;
		if (uvMatrix)
			RwMatrixDestroy(uvMatrix);
	}
	return material;
}

// 0x45e1a0
int TFBPlugin_F6_GetSize(const void *object, int offset, int size)
{
	const TFBMaterialData *d = TFBPluginDataAt<TFBMaterialData>(object, g_tfbMatOfs);
	int total = 0x14;
	if (d->m_flagBytes[0] & TFB_MATERIAL_UV_SCROLL)
		total = 0x34;
	total += 4;
	for (int i = 0; i < d->m_texListCount; i++)
		total += d->m_texList[i].StreamSize();
	total += 4;
	TFB::KFAnimation *colorAnim = d->m_colorAnim;
	if (colorAnim)
		total += colorAnim->SerialSize();
	TFB::KFAnimation *alphaAnim = d->m_alphaAnim;
	total += 4;
	if (alphaAnim)
		total += alphaAnim->SerialSize();
	return total + 1;
}

// 0x45ef50
void *TFBPlugin_F6_Copy(void *dst, const void *src, int offset, int size)
{
	RpMaterial *dstMaterial = (RpMaterial *)dst;
	TFBMaterialData *d = TFBMaterialGetData(dstMaterial);
	const TFBMaterialData *s = TFBPluginDataAt<TFBMaterialData>(src, g_tfbMatOfs);
	d->m_version = s->m_version;
	TFBMaterialGetData(dstMaterial)->m_flags = TFBPluginDataAt<TFBMaterialData>(src, g_tfbMatOfs)->m_flags;
	TFBMaterialGetData(dstMaterial)->m_blendMode = TFBPluginDataAt<TFBMaterialData>(src, g_tfbMatOfs)->m_blendMode;
	TFBMaterialGetData(dstMaterial)->m_renderState = TFBPluginDataAt<TFBMaterialData>(src, g_tfbMatOfs)->m_renderState;
	const TFBMatBody *srcBody = g_tfbMatOfs > 0 ? &TFBPluginDataAt<TFBMaterialData>(src, g_tfbMatOfs)->m_body : 0;
	TFBMatBody *dstBody = g_tfbMatOfs > 0 ? &TFBMaterialGetData(dstMaterial)->m_body : 0;
	*dstBody = *srcBody;
	if (s->m_texList) {
		int count = s->m_texListCount;
		d->m_texListCount = count;
		d->m_texList = new TexList[count];
		for (int i = 0; i < d->m_texListCount; i++) {
			// TexList is larger with 8-byte pointers: the members are copied, a shallow copy like the game's word-by-word one.
			d->m_texList[i] = s->m_texList[i];
		}
	}
	return dst;
}

// 0x45e160
void *TFBPlugin_F6_EngineCtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*TFBEnginePluginName(g_tfbMatEngOfs) = "TFB Material Plugin";
	return engine;
}

// 0x45e180
void *TFBPlugin_F6_EngineDtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const char **name = TFBEnginePluginName(g_tfbMatEngOfs);
	if (*name)
		*name = 0;
	return engine;
}

// 0x45f920
RwBool TFBPlugin_F6_Attach()
{
	g_tfbMatEngOfs = RwEngineRegisterPlugin(sizeof(const char *), TFB_PLUGIN_MATERIAL, TFBPlugin_F6_EngineCtor, TFBPlugin_F6_EngineDtor);
	if (g_tfbMatEngOfs < 0)
		return 0;
	g_tfbMatOfs = RpMaterialRegisterPlugin(sizeof(TFBMaterialData), TFB_PLUGIN_MATERIAL, TFBPlugin_F6_Ctor, TFBPlugin_F6_Dtor, TFBPlugin_F6_Copy);
	if (g_tfbMatOfs < 0)
		return 0;
	RwInt32 registered = RpMaterialRegisterPluginStream(TFB_PLUGIN_MATERIAL, TFBPlugin_F6_Read, ZOOSTERS_TFB_NO_WRITER, TFBPlugin_F6_GetSize);
	return registered == g_tfbMatOfs;
}
