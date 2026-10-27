// The game's own RenderWare plugins: TFB extensions of RpClump, RpAtomic, RpGeometry, RpLight, RpMaterial, RpWorld and RpWorldSector.
// Each registers an engine plugin (4 bytes in the engine instance, holding its name) and an object plugin whose data lies at a registered offset in each object; the stream callbacks read and size it.
#ifndef ZOOSTERS_TFB_RENDER_TFBPLUGINS_H
#define ZOOSTERS_TFB_RENDER_TFBPLUGINS_H

#include "rw/rwcore.h"

// Plugin ids as the registrations pass them.
enum TFBPluginId {
	TFB_PLUGIN_WORLD = 0x800000B0,      // RpWorld, "TFBWORLD", 0x18 bytes: TFBWorldData
	TFB_PLUGIN_CLUMP = 0x800000B1,      // RpClump, "TFBCLUMP", 0xC bytes: TFBClumpData
	TFB_PLUGIN_LIGHT = 0x800000C2,      // RpLight, "TFBLIGHT", 0xC bytes: TFBLightData
	TFB_PLUGIN_GEOMETRY = 0x800000E1,   // RpGeometry, "TFBGEOMETRY", 0x1C bytes: TFBGeometryData
	TFB_PLUGIN_MATERIAL = 0x800000F6,   // RpMaterial, "TFB Material Plugin", 0x2DC bytes: TFBMaterialData
	TFB_PLUGIN_ATOMIC = 0x800000FD,     // RpAtomic, "TFBATOMIC", 0x2C bytes: TFBAtomicData
	TFB_PLUGIN_SECTOR = 0x800000FE      // RpWorldSector, "TFBWORLDSECTOR", 0x1C bytes: TFBSectorData
};

// The name an engine plugin keeps in its 4-byte slot of the engine instance (set and cleared by its engine constructor / destructor).
static inline const char **TFBEnginePluginName(RwInt32 engineOffset)
{
	return (const char **)RwEngineGetPluginData(engineOffset);
}

// The plugin data inside an object for an offset the caller already holds (RW's RWPLUGINOFFSET).
template <class T>
static inline T *TFBPluginDataAt(void *object, RwInt32 offset)
{
	return (T *)((char *)object + offset);
}
template <class T>
static inline const T *TFBPluginDataAt(const void *object, RwInt32 offset)
{
	return (const T *)((const char *)object + offset);
}

// The game registers no stream writer for its plugins: the write-callback slot holds -1.
#define ZOOSTERS_TFB_NO_WRITER ((RwPluginDataChunkWriteCallBack)-1)

// A plugin callback that returns its first argument unchanged: the destructor of the clump and light plugins (the object) and the
// stream reader of the light plugin (the stream). Registered through a cast to the callback type, as the game passes one function.
void *TFBPlugin_Identity(void *first);

// Registers the game's RW plugins in the order MatFX, material, geometry, world sector, atomic, light, world, clump. Returns 0 when one registration fails.
RwBool TFBPlugins_Attach();

#endif
