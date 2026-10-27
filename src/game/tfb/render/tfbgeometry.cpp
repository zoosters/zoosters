// TFB geometry plugin callbacks, stream readers (formats 0xF0F0005..8) and the relock / re-sort helpers of the atomic renderer.
#include "rw/rwcore.h"
#include "tfb/anim/vertanimtrack.h"
#include "tfb/render/tfbgeometry.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbmesh.h"

// GLOBAL: 0x606860 g_tfbGeomEngOfs
// GLOBAL: 0x606864 g_tfbGeomOfs
// GLOBAL: 0x606858 g_tfbGeomMagicLo
// GLOBAL: 0x60685c g_tfbGeomMagic

// g_tfbGeomMagicLo .. g_tfbGeomMagic is the accepted range.
enum TFBGeometryFormat {
	TFB_GEOMETRY_FORMAT_5 = 0xF0F0005,   // flags, then a TFBMesh
	TFB_GEOMETRY_FORMAT_6 = 0xF0F0006,   // flags, then a TFBMesh when flag bit 0 is set
	TFB_GEOMETRY_FORMAT_7 = 0xF0F0007,   // flags only
	TFB_GEOMETRY_FORMAT_8 = 0xF0F0008    // flags and the track array
};

// 0x45d950
void *TFBPlugin_E1_EngineCtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*TFBEnginePluginName(g_tfbGeomEngOfs) = "TFBGEOMETRY";
	return engine;
}

// 0x45d970
void *TFBPlugin_E1_EngineDtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const char **name = TFBEnginePluginName(g_tfbGeomEngOfs);
	if (*name)
		*name = 0;
	return engine;
}

// 0x45d990
void *TFBPlugin_E1_Ctor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	int offset = g_tfbGeomOfs;
	if (offset > 0) {
		TFBGeometryData *d = TFBPluginDataAt<TFBGeometryData>(object, offset);
		d->m_version = g_tfbGeomMagic;
		d->m_flags = 0;
		d->m_mesh = 0;
		d->m_sortTime = 0;
		d->m_trackCount = 0;
		d->m_tracks = 0;
		d->m_sharedBlock = 0;
	}
	return object;
}

// 0x45dac0
int TFBPlugin_E1_SetFlagBits(RpGeometry *geometry, unsigned bits)
{
	TFBGeometryData *d = TFBGeometryGetData(geometry);
	if (g_tfbGeomOfs > 0) {
		d->m_flags |= bits;
		return 1;
	}
	return 0;
}

// 0x45dae0
unsigned TFBPlugin_E1_TestFlags(RpGeometry *geometry, unsigned bits)
{
	if (g_tfbGeomOfs > 0)
		return TFBGeometryGetData(geometry)->m_flags & bits;
	return 0;
}

// 0x45db60
void *TFBPlugin_E1_Dtor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	if (g_tfbGeomOfs > 0) {
		TFBGeometryData *d = TFBGeometryGetData((RpGeometry *)object);
		VertAnimTrack *tracks = d->m_tracks;
		if (tracks) {
			delete[] tracks;
			d->m_tracks = 0;
			d->m_trackCount = 0;
		}
	}
	return object;
}

// 0x45dbc0
void *TFBPlugin_E1_Copy(void *dst, const void *src, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	int offset = g_tfbGeomOfs;
	TFBGeometryData *d = TFBGeometryGetData((RpGeometry *)dst);
	const TFBGeometryData *s = TFBGeometryGetData((const RpGeometry *)src);
	d->m_version = s->m_version;
	d->m_flags = s->m_flags;
	if (offset > 0) {
		TFBMesh *mesh = s->m_mesh;
		if (mesh) {
			mesh->m_refCount++;
			d->m_mesh = mesh;
		} else {
			d->m_mesh = 0;
		}
	}
	if (s->m_tracks) {
		int count = s->m_trackCount;
		d->m_trackCount = count;
		d->m_tracks = new VertAnimTrack[count];
		for (int i = 0; i < d->m_trackCount; i++)
			d->m_tracks[i].Assign(&s->m_tracks[i]);
	}
	int offset2 = g_tfbGeomOfs;
	int *block = s->m_sharedBlock;
	if (block) {
		int refCount = *block + 1;
		*block = refCount;
		if (offset2 > 0)
			TFBPluginDataAt<TFBGeometryData>(dst, offset2)->m_sharedBlock = block;
	} else if (offset2 > 0) {
		TFBPluginDataAt<TFBGeometryData>(dst, offset2)->m_sharedBlock = 0;
	}
	return dst;
}

// 0x45d9d0
RwStream *TFBPlugin_E1_ReadV8(RwStream *stream, void *object)
{
	TFBGeometryData *d = TFBGeometryGetData((RpGeometry *)object);
	if (!RwStreamReadInt32(stream, (RwInt32 *)&d->m_flags, 4))
		return 0;
	RwStreamReadInt32(stream, &d->m_trackCount, 4);
	if (d->m_trackCount > 0) {
		d->m_tracks = new VertAnimTrack[d->m_trackCount];
		for (int i = 0; i < d->m_trackCount; i++)
			d->m_tracks[i].Read(stream);
	}
	return stream;
}

// 0x45dce0
static __declspec(noinline) RwStream *TFBPlugin_E1_ReadV5(void *object, RwStream *stream)
{
	if (!RwStreamReadInt32(stream, (RwInt32 *)&TFBGeometryGetData((RpGeometry *)object)->m_flags, 4))
		return 0;
	TFBMesh mesh;
	mesh.Read(stream);
	return stream;
}

static __forceinline unsigned TestGeometryFlags(RpGeometry *geometry, unsigned bits)
{
	if (g_tfbGeomOfs > 0)
		return TFBGeometryGetData(geometry)->m_flags & bits;
	return 0;
}

// 0x45dd60
static __declspec(noinline) RwStream *TFBPlugin_E1_ReadV6(RwStream *stream, void *object)
{
	if (!RwStreamReadInt32(stream, (RwInt32 *)&TFBGeometryGetData((RpGeometry *)object)->m_flags, 4))
		return 0;
	if (TestGeometryFlags((RpGeometry *)object, 1)) {
		TFBMesh mesh;
		mesh.Read(stream);
	}
	return stream;
}

// 0x45ddf0
RwStream *TFBPlugin_E1_Read(RwStream *stream, RwInt32 binaryLength, void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBGeometryData *d = TFBGeometryGetData((RpGeometry *)object);
	if (!RwStreamReadInt32(stream, (RwInt32 *)&d->m_version, 4))
		return 0;
	unsigned version = d->m_version;
	if (version >= g_tfbGeomMagicLo && version <= g_tfbGeomMagic) {
		switch (version) {
		case TFB_GEOMETRY_FORMAT_5:
			TFBPlugin_E1_ReadV5(object, stream);
			break;
		case TFB_GEOMETRY_FORMAT_6:
			TFBPlugin_E1_ReadV6(stream, object);
			break;
		case TFB_GEOMETRY_FORMAT_7:
			RwStreamReadInt32(stream, (RwInt32 *)&TFBGeometryGetData((RpGeometry *)object)->m_flags, 4);
			break;
		case TFB_GEOMETRY_FORMAT_8:
			TFBPlugin_E1_ReadV8(stream, object);
			break;
		}
	} else {
		RwStreamSkip(stream, binaryLength - 4);
	}
	return stream;
}

// 0x45d8e0
void TFBGeometry_RelockPrelight(RpGeometry *geometry)
{
	int i = 0;
	int numMaterials = RpGeometryGetNumMaterials(geometry);
	if (numMaterials > 0) {
		RpMaterial *const *materials = RpGeometryGetMaterialArray(geometry);
		int materialOffset = g_tfbMatOfs;
		for (; i < numMaterials; i++, materials++) {
			RpMaterial *material = *materials;
			if (material && (TFBPluginDataAt<TFBMaterialData>(material, materialOffset)->m_flagBytes[1] & 1)) { // TFB_MATERIAL_COLOR_CHANGED
				RpGeometryLock(geometry, rpGEOMETRYLOCKPRELIGHT);
				RpGeometryUnlock(geometry);
				return;
			}
		}
	}
}

// 0x45db00
bool TFBGeometry_ResortTriangles(RpGeometry *geometry, float time)
{
	int offset = g_tfbGeomOfs;
	int count = TFBPluginDataAt<TFBGeometryData>(geometry, offset)->m_trackCount;
	if (count) {
		if (TFBPluginDataAt<TFBGeometryData>(geometry, offset)->m_sortTime == time)
			return false;
		TFBPluginDataAt<TFBGeometryData>(geometry, offset)->m_sortTime = time;
		ApplyVertAnimTracks(TFBPluginDataAt<TFBGeometryData>(geometry, offset)->m_tracks, count, RpGeometryGetPreLightColors(geometry));
		RpGeometryLock(geometry, rpGEOMETRYLOCKALL);
		RpGeometryUnlock(geometry);
	}
	return true;
}

RwInt32 TFBPlugin_E1_GetSize(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject); // 0x45deb0

// 0x45df00
RwBool TFBPlugin_E1_Attach()
{
	g_tfbGeomEngOfs = RwEngineRegisterPlugin(sizeof(const char *), TFB_PLUGIN_GEOMETRY, TFBPlugin_E1_EngineCtor, TFBPlugin_E1_EngineDtor);
	if (g_tfbGeomEngOfs < 0)
		return 0;
	g_tfbGeomOfs = RpGeometryRegisterPlugin(sizeof(TFBGeometryData), TFB_PLUGIN_GEOMETRY, TFBPlugin_E1_Ctor, TFBPlugin_E1_Dtor,
	                                        TFBPlugin_E1_Copy);
	if (g_tfbGeomOfs < 0)
		return 0;
	RwInt32 registered = RpGeometryRegisterPluginStream(TFB_PLUGIN_GEOMETRY, TFBPlugin_E1_Read, ZOOSTERS_TFB_NO_WRITER, TFBPlugin_E1_GetSize);
	if (registered != g_tfbGeomOfs)
		return 0;
	return 1;
}
