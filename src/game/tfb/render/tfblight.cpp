// TFB light plugin 0x800000C2 ("TFBLIGHT"): callbacks and the world level accessors.
#include "rw/rwcore.h"
#include "tfb/render/tfblight.h"

// GLOBAL: 0x606894 g_tfbLightEngOfs
// GLOBAL: 0x606898 g_tfbLightOfs
// GLOBAL: 0x606890 g_tfbLightMagic

// The light plugin streams nothing: its read callback is TFBPlugin_Identity and its size callback the folded `xor eax,eax; ret` (0x43e6f0).
int Fold_Return0();

// 0x45df80
void *TFBPlugin_C2_EngineCtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*TFBEnginePluginName(g_tfbLightEngOfs) = "TFBLIGHT";
	return engine;
}

// 0x45dfa0
void *TFBPlugin_C2_EngineDtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const char **name = TFBEnginePluginName(g_tfbLightEngOfs);
	if (*name)
		*name = 0;
	return engine;
}

// 0x45dfc0
void *TFBPlugin_C2_Ctor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	int offset = g_tfbLightOfs;
	if (offset > 0) {
		int zero = 0;
		TFBLightData *d = TFBPluginDataAt<TFBLightData>(object, offset);
		d->m_version = g_tfbLightMagic;
		d->m_unk04 = zero;
		d->m_level = zero;
	}
	return object;
}

// 0x45dff0
void *TFBPlugin_C2_Copy(void *dst, const void *src, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	int offset = g_tfbLightOfs;
	TFBLightData *d = TFBPluginDataAt<TFBLightData>(dst, offset);
	const TFBLightData *s = TFBPluginDataAt<TFBLightData>(src, offset);
	d->m_version = s->m_version;
	d->m_unk04 = s->m_unk04;
	d->m_level = s->m_level;
	return dst;
}

// 0x45e0b0
void TFBPlugin_C2_SetLevel(RpLight *light, int level)
{
	if (g_tfbLightOfs > 0)
		TFBLightGetData(light)->m_level = level;
}

// 0x45e0d0
int TFBPlugin_C2_GetLevel(RpLight *light)
{
	if (g_tfbLightOfs > 0)
		return TFBLightGetData(light)->m_level;
	return 0;
}

// 0x45e030
RwBool TFBPlugin_C2_Attach()
{
	g_tfbLightEngOfs = RwEngineRegisterPlugin(sizeof(const char *), TFB_PLUGIN_LIGHT, TFBPlugin_C2_EngineCtor, TFBPlugin_C2_EngineDtor);
	if (g_tfbLightEngOfs < 0)
		return 0;
	g_tfbLightOfs = RpLightRegisterPlugin(sizeof(TFBLightData), TFB_PLUGIN_LIGHT, TFBPlugin_C2_Ctor, (RwPluginObjectDestructor)TFBPlugin_Identity,
	                                      TFBPlugin_C2_Copy);
	if (g_tfbLightOfs < 0)
		return 0;
	RwInt32 registered = RpLightRegisterPluginStream(TFB_PLUGIN_LIGHT, (RwPluginDataChunkReadCallBack)TFBPlugin_Identity, ZOOSTERS_TFB_NO_WRITER,
	                                                 (RwPluginDataChunkGetSizeCallBack)Fold_Return0);
	if (registered != g_tfbLightOfs)
		return 0;
	return 1;
}
