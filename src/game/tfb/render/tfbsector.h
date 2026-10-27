// TFB world-sector plugin 0x800000FE ("TFBWORLDSECTOR"): per-sector keyframe track arrays (sorted triangles) and the fade
// distance, plus the sector render the world's draw loop calls.
#ifndef ZOOSTERS_TFB_RENDER_TFBSECTOR_H
#define ZOOSTERS_TFB_RENDER_TFBSECTOR_H

#include "rw/rpworld.h"
#include "tfb/render/tfbplugins.h"

class VertAnimTrack;

struct TFBSectorData {
	int m_version;             // g_tfbSectorMagic after the constructor; the stream format id (0xF0F0002 / 0xF0F0003) after a read
	int m_unk04;               // read by both formats, copied by the copy callback
	int m_unk08;               // the same
	int m_unk0c;               // zeroed by the constructor only
	int m_trackCount;          // elements of m_tracks (format 3)
	VertAnimTrack *m_tracks;     // track array (new[] of 0x1C-byte tracks)
	float m_fadeDistance;      // TFBSector_SetFadeDistance; TFBSector_Render fades by (it - g_secFadeStart) / g_secFadeRange
};
ZOOSTERS_ASSERT_SIZE(TFBSectorData, 0x1c);
ZOOSTERS_ASSERT_OFFSET(TFBSectorData, m_trackCount, 0x10);
ZOOSTERS_ASSERT_OFFSET(TFBSectorData, m_fadeDistance, 0x18);

extern int g_tfbSectorEngOfs;  // engine plugin offset (holds "TFBWORLDSECTOR")
extern int g_tfbSectorOfs;  // object plugin offset inside an RpWorldSector (<= 0: not registered)
extern int g_tfbSectorMagic;

static inline TFBSectorData *TFBSectorGetData(RpWorldSector *sector)
{
	return (TFBSectorData *)((char *)sector + g_tfbSectorOfs);
}

extern float g_secFadeStart;  // sector fade starts at this view distance ...
extern float g_secFadeRange;  // ... and completes after this much more

// Stores the distance the sector fades with (nothing without the plugin).
void TFBSector_SetFadeDistance(RpWorldSector *sector, float distance);
// The sector render callback: sorts the triangles, fades the sector's materials by distance and executes the pipeline.
RpWorldSector *TFBSector_Render(RpWorldSector *sector);
RwBool TFBPlugin_FE_Attach();
bool Fold_ReturnTrue(RpWorldSector *sector, int zone);  // (ignores its arguments)

#endif
