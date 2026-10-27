// TFB world plugin 0x800000B0: animated node lists and occluder records; callbacks, stream readers (formats 0xF0F0001..3) and the per-frame material update.
#include "rw/rwcore.h"
#include "tfb/anim/mastervertanimation.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbworld.h"

// GLOBAL: 0x6069b8 g_tfbWorldEngOfs
// GLOBAL: 0x6069bc g_tfbWorldOfs
// GLOBAL: 0x6069b4 g_tfbWorldMagic

// Stream format ids of the plugin data.
enum TFBWorldFormat {
	TFB_WORLD_FORMAT_1 = 0xF0F0001,   // one int
	TFB_WORLD_FORMAT_2 = 0xF0F0002,   // + the node lists
	TFB_WORLD_FORMAT_3 = 0xF0F0003    // + the occluder records
};

// 0x45f9f0
void *TFBPlugin_B0_EngineCtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*TFBEnginePluginName(g_tfbWorldEngOfs) = "TFBWORLD";
	return engine;
}

// 0x45fa10
void *TFBPlugin_B0_EngineDtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const char **name = TFBEnginePluginName(g_tfbWorldEngOfs);
	if (*name)
		*name = 0;
	return engine;
}

// 0x45fa30
void *TFBPlugin_B0_Ctor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	int offset = g_tfbWorldOfs;
	if (offset > 0) {
		int zero = 0;
		TFBWorldData *d = TFBPluginDataAt<TFBWorldData>(object, offset);
		d->m_version = g_tfbWorldMagic;
		d->m_unk04 = zero;
		d->m_nodeCount = zero;
		d->m_nodes = 0;
		d->m_recordCount = zero;
		d->m_recordPoints = 0;
	}
	return object;
}

// 0x45fa60
void *TFBPlugin_B0_Dtor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBWorldData *d = TFBWorldGetData((RpWorld *)object);
	if (d->m_nodes) {
		g_masterVertAnimList = 0;
		delete[] d->m_nodes;
	}
	if (d->m_recordPoints) {
		d->m_recordCount = 0;
		operator_delete(d->m_recordPoints);
	}
	return object;
}

// Copy callback: only the word at +4 is copied. The game's returns nothing; it is registered through a cast.
void TFBPlugin_B0_Copy(void *dst, const void *src)
{
	TFBWorldGetData((RpWorld *)dst)->m_unk04 = TFBPluginDataAt<TFBWorldData>(src, g_tfbWorldOfs)->m_unk04;
}

// Format 2: the int at +4 and the node lists.
// 0x45fae0
RwStream *TFBPlugin_B0_ReadV2(RwStream *stream, void *object)
{
	TFBWorldData *d = TFBWorldGetData((RpWorld *)object);
	if (!RwStreamReadInt32(stream, &d->m_unk04, 4))
		return 0;
	int *nodeCount = &d->m_nodeCount;
	if (!RwStreamReadInt32(stream, nodeCount, 4))
		return 0;
	if (*nodeCount > 0) {
		d->m_nodes = new TFB::MasterVertAnimation[*nodeCount];
		for (int i = 0; i < *nodeCount; i++)
			d->m_nodes[i].Read(stream);
	}
	return stream;
}

static RwStream *TFBPlugin_B0_ReadV3(RwStream *stream, int length, void *object, int offset, int size);

// 0x45fd10
RwStream *TFBPlugin_B0_Read(RwStream *stream, RwInt32 binaryLength, void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBWorldData *d = TFBWorldGetData((RpWorld *)object);
	if (!RwStreamReadInt32(stream, (RwInt32 *)&d->m_version, 4))
		return 0;
	if (d->m_version <= TFB_WORLD_FORMAT_1) {
		RwStreamReadInt32(stream, &TFBWorldGetData((RpWorld *)object)->m_unk04, 4);
	} else if (d->m_version <= TFB_WORLD_FORMAT_2) {
		TFBPlugin_B0_ReadV2(stream, object);
	} else if (d->m_version <= TFB_WORLD_FORMAT_3) {
		TFBPlugin_B0_ReadV3(stream, binaryLength, object, offsetInObject, sizeInObject);
	}
	for (int i = 0; i < d->m_nodeCount; i++) {
		TFB::MasterVertAnimation *node = d->m_nodes + i;
		node->m_next = g_masterVertAnimList;
		g_masterVertAnimList = node;
	}
	return stream;
}

// Format 3: format 2 plus the occluder records (a count, then 4 points of 3 reals per record).
// 0x45fc40
static __declspec(noinline) RwStream *TFBPlugin_B0_ReadV3(RwStream *stream, int length, void *object, int offset, int size)
{
	TFBWorldData *d = TFBWorldGetData((RpWorld *)object);
	if (!TFBPlugin_B0_ReadV2(stream, object))
		return 0;
	if (!RwStreamReadInt32(stream, &d->m_recordCount, 4))
		return 0;
	int recordCount = d->m_recordCount;
	if (recordCount > 0) {
		int pointCount = recordCount * 4;
		d->m_recordPoints = (float *)MemAlloc(pointCount * 12);
		for (int i = 0; i < pointCount; i++) {
			if (!RwStreamReadReal(stream, &d->m_recordPoints[i * 3], 4))
				return 0;
			if (!RwStreamReadReal(stream, &d->m_recordPoints[i * 3 + 1], 4))
				return 0;
			if (!RwStreamReadReal(stream, &d->m_recordPoints[i * 3 + 2], 4))
				return 0;
		}
	}
	return stream;
}

// 0x45fdd0
RwInt32 TFBPlugin_B0_GetSize(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const TFBWorldData *d = TFBPluginDataAt<TFBWorldData>(object, g_tfbWorldOfs);
	int size = 0xc;
	for (int i = 0; i < d->m_nodeCount; i++)
		size += d->m_nodes[i].SerialSize();
	size += 4;
	if (d->m_recordCount > 0)
		size += d->m_recordCount * 0x30;
	return size;
}

// 0x45ff20
int TFBWorld_GetRecordCount(RpWorld *world)
{
	if (world && g_tfbWorldOfs > 0)
		return TFBWorldGetData(world)->m_recordCount;
	return 0;
}

// 0x45ff40
float *TFBWorld_GetRecords(RpWorld *world)
{
	if (world && g_tfbWorldOfs > 0)
		return TFBWorldGetData(world)->m_recordPoints;
	return 0;
}

// 0x45feb0
void TFBWorld_UpdateMaterials(RpWorld *world, float time)
{
	int offset = g_tfbWorldOfs;
	if (offset > 0) {
		TFBWorldData *d;
		for (int i = 0; i < RpWorldGetNumMaterials(world); i++)
			TFBMaterial::UpdateAnimation(RpWorldGetMaterial(world, i), time, 0, 0);
		d = TFBPluginDataAt<TFBWorldData>(world, offset);
		for (int i = 0; i < d->m_nodeCount; i++)
			d->m_nodes[i].Update(time);
	}
}

// 0x45fe30
RwBool TFBPlugin_B0_Attach()
{
	g_tfbWorldEngOfs = RwEngineRegisterPlugin(sizeof(const char *), TFB_PLUGIN_WORLD, TFBPlugin_B0_EngineCtor, TFBPlugin_B0_EngineDtor);
	if (g_tfbWorldEngOfs < 0)
		return 0;
	g_tfbWorldOfs = RpWorldRegisterPlugin(sizeof(TFBWorldData), TFB_PLUGIN_WORLD, TFBPlugin_B0_Ctor, TFBPlugin_B0_Dtor,
	                                      (RwPluginObjectCopy)TFBPlugin_B0_Copy);
	if (g_tfbWorldOfs < 0)
		return 0;
	RwInt32 registered = RpWorldRegisterPluginStream(TFB_PLUGIN_WORLD, TFBPlugin_B0_Read, ZOOSTERS_TFB_NO_WRITER, TFBPlugin_B0_GetSize);
	if (registered != g_tfbWorldOfs)
		return 0;
	return 1;
}
