// TFB world plugin 0x800000B0 ("TFBWORLD"): the world's animated texture-node lists and its occluder records.
#ifndef ZOOSTERS_TFB_RENDER_TFBWORLD_H
#define ZOOSTERS_TFB_RENDER_TFBWORLD_H

#include "rw/rpworld.h"
#include "tfb/render/tfbplugins.h"

namespace TFB { class MasterVertAnimation; }

// Plugin data in every RpWorld. Each record is 4 points (0x30 bytes): the level's occluder quads, handed to BuildOccluderPlanes with count * 4.
struct TFBWorldData {
	unsigned m_version;        // g_tfbWorldMagic after the constructor; the stream format id (0xF0F0001..3) after a read
	int m_unk04;               // read by every format; copied by the copy callback
	int m_nodeCount;           // MasterVertAnimation elements
	TFB::MasterVertAnimation *m_nodes; // array of m_nodeCount elements, each is linked into g_masterVertAnimList after the stream read
	int m_recordCount;         // occluder records (format 3)
	float *m_recordPoints;     // m_recordCount * 4 points of 3 floats (heap block, freed by the destructor)
};
ZOOSTERS_ASSERT_SIZE(TFBWorldData, 0x18);
ZOOSTERS_ASSERT_OFFSET(TFBWorldData, m_nodes, 0x0c);
ZOOSTERS_ASSERT_OFFSET(TFBWorldData, m_recordPoints, 0x14);

extern int g_tfbWorldEngOfs;  // engine plugin offset (holds "TFBWORLD")
extern int g_tfbWorldOfs;  // object plugin offset inside an RpWorld (<= 0: not registered)
extern int g_tfbWorldMagic;

// The plugin data of a world (the plugin must be registered).
static inline TFBWorldData *TFBWorldGetData(RpWorld *world)
{
	return (TFBWorldData *)((char *)world + g_tfbWorldOfs);
}

int TFBWorld_GetRecordCount(RpWorld *world);  // 0 for no world or no plugin
float *TFBWorld_GetRecords(RpWorld *world);
void TFBWorld_UpdateMaterials(RpWorld *world, float time);  // advances the animation of every material and node list
RwBool TFBPlugin_B0_Attach();

#endif
