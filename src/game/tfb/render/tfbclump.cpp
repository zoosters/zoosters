// TFB clump plugin 0x800000B1 ("TFBCLUMP"): callbacks, flag and render-frame accessors, and the clump render.
#include "rw/rwcore.h"
#include "tfb/render/tfbatomic.h"
#include "tfb/render/tfbclump.h"

// GLOBAL: 0x6067d8 g_tfbB1EngOfs
// GLOBAL: 0x6067dc g_tfbB1Ofs
// GLOBAL: 0x6067d4 g_tfbB1Magic

// 0x45cfa0
void *TFBPlugin_B1_EngineCtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*TFBEnginePluginName(g_tfbB1EngOfs) = "TFBCLUMP";
	return engine;
}

// 0x45cfc0
void *TFBPlugin_B1_EngineDtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const char **name = TFBEnginePluginName(g_tfbB1EngOfs);
	if (*name)
		*name = 0;
	return engine;
}

// 0x45cfe0
void *TFBPlugin_B1_Ctor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	int offset = g_tfbB1Ofs;
	if (offset > 0) {
		int magic = g_tfbB1Magic;
		TFBClumpData *d = TFBPluginDataAt<TFBClumpData>(object, offset);
		d->m_flags = 0;
		d->m_version = magic;
	}
	return object;
}

// Copy callback: only the flags are copied. The game's returns nothing; it is registered through a cast.
void TFBPlugin_B1_Copy(void *dst, const void *src)
{
	TFBClumpGetData((RpClump *)dst)->m_flags = TFBPluginDataAt<TFBClumpData>(src, g_tfbB1Ofs)->m_flags;
}

// 0x45d020
RwStream *TFBPlugin_B1_Read(RwStream *stream, RwInt32 binaryLength, void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBClumpData *d = TFBClumpGetData((RpClump *)object);
	if (!RwStreamReadInt32(stream, &d->m_version, 4))
		return 0;
	if (d->m_version != g_tfbB1Magic) {
		RwStreamSkip(stream, binaryLength - 4);
		return stream;
	}
	return RwStreamReadInt32(stream, (RwInt32 *)&d->m_flags, 4) ? stream : 0;
}

// The stream carries a magic word and the flags.
// 0x45d080
RwInt32 TFBPlugin_B1_GetSize(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	return 8;
}

// 0x45d110
int TFBPlugin_B1_SetFlagBits(RpClump *clump, unsigned bits)
{
	TFBClumpData *d = TFBClumpGetData(clump);
	if (g_tfbB1Ofs > 0) {
		d->m_flags |= bits;
		return 1;
	}
	return 0;
}

// 0x45d130
unsigned TFBPlugin_B1_TestFlags(RpClump *clump, unsigned bits)
{
	if (g_tfbB1Ofs > 0)
		return TFBClumpGetData(clump)->m_flags & bits;
	return 0;
}

// 0x45d150
int TFBPlugin_B1_GetRenderFrame(RpClump *clump)
{
	if (g_tfbB1Ofs > 0)
		return TFBClumpGetData(clump)->m_renderFrame;
	return 0;
}

// 0x45d170
int TFBPlugin_B1_SetRenderFrame(RpClump *clump, int renderFrame)
{
	if (g_tfbB1Ofs > 0) {
		TFBClumpGetData(clump)->m_renderFrame = renderFrame;
		return 1;
	}
	return 0;
}

// 0x45d090
RwBool TFBPlugin_B1_Attach()
{
	g_tfbB1EngOfs = RwEngineRegisterPlugin(sizeof(const char *), TFB_PLUGIN_CLUMP, TFBPlugin_B1_EngineCtor, TFBPlugin_B1_EngineDtor);
	if (g_tfbB1EngOfs < 0)
		return 0;
	g_tfbB1Ofs = RpClumpRegisterPlugin(sizeof(TFBClumpData), TFB_PLUGIN_CLUMP, TFBPlugin_B1_Ctor, (RwPluginObjectDestructor)TFBPlugin_Identity,
	                                   (RwPluginObjectCopy)TFBPlugin_B1_Copy);
	if (g_tfbB1Ofs < 0)
		return 0;
	RwInt32 registered = RpClumpRegisterPluginStream(TFB_PLUGIN_CLUMP, TFBPlugin_B1_Read, ZOOSTERS_TFB_NO_WRITER, TFBPlugin_B1_GetSize);
	if (registered != g_tfbB1Ofs)
		return 0;
	return 1;
}

// 0x45ccc0
RpClump *TFBClump_Render(RpClump *clump)
{
	RpClump *result = clump;
	RwLLLink *head = &clump->atomicList.link;
	for (RwLLLink *link = head->next; link != head; link = link->next) {
		RpAtomic *atomic = RpAtomicFromClumpLink(link);
		if (RpAtomicGetFlags(atomic) & rpATOMICRENDER) {
			RwFrameGetLTM(RpAtomicGetFrame(atomic));
			if (!TFBAtomic::RenderCallback(atomic))
				result = 0;
		}
	}
	return result;
}
