// TFB world-sector plugin 0x800000FE: callbacks, stream readers (formats 0xF0F0002 / 3), and the sector render with its distance fade.
#include "rw/rwcore.h"
#include "tfb/anim/vertanimtrack.h"
#include "tfb/render/tfbgeometry.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbrenderstate.h"
#include "tfb/render/tfbsector.h"

// GLOBAL: 0x606a10 g_tfbSectorEngOfs
// GLOBAL: 0x606a14 g_tfbSectorOfs
// GLOBAL: 0x606a0c g_tfbSectorMagic
// GLOBAL: 0x6068d0 g_secFadeStart
// GLOBAL: 0x6068d4 g_secFadeRange

#include "tfb/render/tfbsector_views.h"

enum TFBSectorFormat {
	TFB_SECTOR_FORMAT_2 = 0xF0F0002,   // two ints
	TFB_SECTOR_FORMAT_3 = 0xF0F0003    // two ints, the track count and the tracks
};

extern unsigned char g_secFadeOn;     // GLOBAL: 0x62a134 g_secFadeOn
extern unsigned char g_secFadeHide;   // GLOBAL: 0x62a135 g_secFadeHide
extern int g_secStatMeshes;           // GLOBAL: 0x62a124 g_secStatMeshes
extern int g_secStatFaded;            // GLOBAL: 0x62a128 g_secStatFaded
extern int g_secStatIndices;          // GLOBAL: 0x62a12c g_secStatIndices
extern int g_secStatFadedIndices;     // GLOBAL: 0x62a130 g_secStatFadedIndices
extern const float g_floatZero;       // GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatOne;        // GLOBAL: 0x5ca668 g_floatOne

// 0x4601a0
void *TFBPlugin_FE_EngineCtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*TFBEnginePluginName(g_tfbSectorEngOfs) = "TFBWORLDSECTOR";
	return engine;
}

// 0x4601c0
void *TFBPlugin_FE_EngineDtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const char **name = TFBEnginePluginName(g_tfbSectorEngOfs);
	if (*name)
		*name = 0;
	return engine;
}

// 0x4601e0
void *TFBPlugin_FE_Ctor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	int offset = g_tfbSectorOfs;
	if (offset > 0) {
		int zero = 0;
		TFBSectorData *d = TFBPluginDataAt<TFBSectorData>(object, offset);
		d->m_version = g_tfbSectorMagic;
		d->m_unk04 = zero;
		d->m_unk08 = zero;
		d->m_unk0c = zero;
		d->m_trackCount = zero;
		d->m_tracks = 0;
	}
	return object;
}

// 0x460210
void *TFBPlugin_FE_Copy(void *dst, const void *src, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBSectorData *d = TFBSectorGetData((RpWorldSector *)dst);
	const TFBSectorData *s = TFBPluginDataAt<TFBSectorData>(src, g_tfbSectorOfs);
	d->m_unk04 = s->m_unk04;
	d->m_unk08 = s->m_unk08;
	if (s->m_tracks) { // tests the tracks pointer, not the count
		d->m_trackCount = s->m_trackCount;
		d->m_tracks = new VertAnimTrack[d->m_trackCount]; // game allocates count*0x1c+4 and runs the vector constructor with the source count (same value)
		for (int i = 0; i < d->m_trackCount; i++)
			d->m_tracks[i].Assign(&s->m_tracks[i]); // game reloads src->tracks each iteration
	}
	return dst;
}

// 0x460300
RwStream *TFBPlugin_FE_ReadV3(RwStream *stream, void *object)
{
	TFBSectorData *d = TFBSectorGetData((RpWorldSector *)object);
	RwStreamReadInt32(stream, &d->m_unk04, 4);
	RwStreamReadInt32(stream, &d->m_unk08, 4);
	RwStreamReadInt32(stream, &d->m_trackCount, 4);
	if (d->m_trackCount > 0) {
		d->m_tracks = new VertAnimTrack[d->m_trackCount];
		for (int i = 0; i < d->m_trackCount; i++)
			d->m_tracks[i].Read(stream);
	}
	return stream;
}

// 0x4603f0
RwStream *TFBPlugin_FE_Read(RwStream *stream, RwInt32 binaryLength, void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBSectorData *d = TFBSectorGetData((RpWorldSector *)object);
	if (!RwStreamReadInt32(stream, &d->m_version, 4))
		return 0;
	if (d->m_version == TFB_SECTOR_FORMAT_2) {
		d = TFBSectorGetData((RpWorldSector *)object);
		RwStreamReadInt32(stream, &d->m_unk04, 4);
		RwStreamReadInt32(stream, &d->m_unk08, 4);
		return stream;
	}
	if (d->m_version == TFB_SECTOR_FORMAT_3)
		TFBPlugin_FE_ReadV3(stream, object);
	return stream;
}

// 0x460470
RwInt32 TFBPlugin_FE_GetSize(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const TFBSectorData *d = TFBPluginDataAt<TFBSectorData>(object, g_tfbSectorOfs);
	int size = 0x10;
	for (int i = 0; i < d->m_trackCount; i++)
		size += d->m_tracks[i].SerialSize();
	return size;
}

// RpWorldSectorForAllMeshes callback (data = float *fade): scales the alpha of each material that fades with its sector
// (TFB_MATERIAL_SECTOR_FADE) from the saved colour, hides it (TFB_MATERIAL_HIDDEN) at alpha 0, and counts meshes for the statistics.
// 0x4604d0
RpMesh *TFBSector_FadeMeshCB(RpMesh *mesh, RpMeshHeader *meshHeader, void *data)
{
	float *fade = (float *)data;
	if (!g_secFadeOn) {
		RpMaterial *material = RpMeshGetMaterial(mesh);
		TFBMaterialGetData(material)->m_flags &= ~TFB_MATERIAL_HIDDEN;
		return mesh;
	}
	g_secStatMeshes++;
	g_secStatIndices += RpMeshGetNumIndices(mesh);
	RpMaterial *material = RpMeshGetMaterial(mesh);
	if (TFBMaterialGetData(material)->m_flagBytes[0] & TFB_MATERIAL_SECTOR_FADE) {
		unsigned char savedAlpha = TFBMaterialGetData(material)->m_savedColor.alpha;
		RpMaterialGetColorRef(material)->alpha = (unsigned char)(int)(savedAlpha * *fade);
		if (g_secFadeHide)
			RpMaterialGetColorRef(material)->alpha = 0;
		if (RpMaterialGetColorRef(material)->alpha > 0) {
			TFBMaterialGetData(material)->m_flags &= ~TFB_MATERIAL_HIDDEN;
			return mesh;
		}
		TFBMaterialGetData(material)->m_flags |= TFB_MATERIAL_HIDDEN;
		g_secStatFaded++;
		g_secStatFadedIndices += RpMeshGetNumIndices(mesh);
	}
	return mesh;
}

// 0x4605a0
RpWorldSector *TFBSector_Render(RpWorldSector *sector)
{
	RpWorldSectorView *view = (RpWorldSectorView *)sector;
	if (g_tfbSectorOfs > 0) {
		TFBSectorData *d = TFBSectorGetData(sector);
		if (d->m_trackCount) {
			ApplyVertAnimTracks(d->m_tracks, d->m_trackCount, view->preLitLum);
			if (view->repEntry) {
				RwResourcesFreeResEntry(view->repEntry);
				view->repEntry = 0;
			}
		}
		float f = (d->m_fadeDistance - g_secFadeStart) / g_secFadeRange;
		if (f < g_floatZero)
			f = g_floatZero;
		else if (f > g_floatOne)
			f = g_floatOne;
		float fade = g_floatOne - f;
		RpWorldSectorForAllMeshes(sector, TFBSector_FadeMeshCB, &fade);
	}
	FUN_004c80d0(TFBRenderStateApplyCB);
	if (view->numTriangles > 0) {
		RxPipeline *pipeline = view->pipeline;
		if (!pipeline) {
			RpWorldGetSectorPipeline((const RpWorld *)RwEngineGlobals()->curWorld, &pipeline);
			if (!pipeline)
				pipeline = RpWorldGetDefaultSectorPipeline();
		}
		RxPipelineExecute(pipeline, sector, 1);
	}
	FUN_004c80d0(0);
	return sector;
}

// 0x4606a0
void TFBSector_SetFadeDistance(RpWorldSector *sector, float distance)
{
	if (g_tfbSectorOfs > 0)
		TFBSectorGetData(sector)->m_fadeDistance = distance;
}

// 0x4606c0
void *TFBPlugin_FE_Dtor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBSectorData *d = TFBSectorGetData((RpWorldSector *)object);
	if (d->m_tracks) {
		delete[] d->m_tracks;
		d->m_tracks = 0;
		d->m_trackCount = 0;
	}
	return object;
}

// 0x460710
RwBool TFBPlugin_FE_Attach()
{
	g_tfbSectorEngOfs = RwEngineRegisterPlugin(sizeof(const char *), TFB_PLUGIN_SECTOR, TFBPlugin_FE_EngineCtor, TFBPlugin_FE_EngineDtor);
	if (g_tfbSectorEngOfs < 0)
		return 0;
	g_tfbSectorOfs = RpWorldSectorRegisterPlugin(sizeof(TFBSectorData), TFB_PLUGIN_SECTOR, TFBPlugin_FE_Ctor, TFBPlugin_FE_Dtor,
	                                             TFBPlugin_FE_Copy);
	if (g_tfbSectorOfs < 0)
		return 0;
	RwInt32 registered = RpWorldSectorRegisterPluginStream(TFB_PLUGIN_SECTOR, TFBPlugin_FE_Read, ZOOSTERS_TFB_NO_WRITER, TFBPlugin_FE_GetSize);
	if (registered != g_tfbSectorOfs)
		return 0;
	return 1;
}

// 0x4604c0
bool Fold_ReturnTrue(RpWorldSector *sector, int zone)
{
	return true;
}
