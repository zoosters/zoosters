// TFB geometry plugin 0x800000E1 ("TFBGEOMETRY"): flags, shared TFBMesh, sorted-triangle tracks and a refcounted block of each RpGeometry, and the geometry helpers of the atomic renderer.
#ifndef ZOOSTERS_TFB_RENDER_TFBGEOMETRY_H
#define ZOOSTERS_TFB_RENDER_TFBGEOMETRY_H

#include "rw/rpworld.h"
#include "tfb/render/tfbplugins.h"

class TFBMesh;
class VertAnimTrack;

struct TFBGeometryData {
	unsigned m_version;        // g_tfbGeomMagic after the constructor; the stream format id (0xF0F0005..8) after a read
	unsigned m_flags;          // bit 0: a TFBMesh follows in the stream (format 6); bit 3 is set by AtomicApplyUserDataFlags' callers
	TFBMesh *m_mesh;           // shared by copies (refcount at TFBMesh +0x40); nothing in the reader fills it
	float m_sortTime;          // the time the triangles were last sorted for (ResortTriangles)
	int m_trackCount;          // elements of m_tracks (format 8)
	VertAnimTrack *m_tracks;     // track array; the destructor deletes it
	int *m_sharedBlock;        // refcounted block (its first int is the count), shared by copies
};
ZOOSTERS_ASSERT_SIZE(TFBGeometryData, 0x1c);
ZOOSTERS_ASSERT_OFFSET(TFBGeometryData, m_mesh, 0x08);
ZOOSTERS_ASSERT_OFFSET(TFBGeometryData, m_sortTime, 0x0c);
ZOOSTERS_ASSERT_OFFSET(TFBGeometryData, m_trackCount, 0x10);
ZOOSTERS_ASSERT_OFFSET(TFBGeometryData, m_sharedBlock, 0x18);

extern int g_tfbGeomEngOfs;  // engine plugin offset (holds "TFBGEOMETRY")
extern int g_tfbGeomOfs;  // object plugin offset inside an RpGeometry (<= 0: not registered)
extern unsigned g_tfbGeomMagicLo;  // oldest stream format id the reader accepts
extern unsigned g_tfbGeomMagic;  // newest stream format id; the constructor stamps it

// The plugin data of a geometry (the plugin must be registered).
static inline TFBGeometryData *TFBGeometryGetData(RpGeometry *geometry)
{
	return (TFBGeometryData *)((char *)geometry + g_tfbGeomOfs);
}
static inline const TFBGeometryData *TFBGeometryGetData(const RpGeometry *geometry)
{
	return (const TFBGeometryData *)((const char *)geometry + g_tfbGeomOfs);
}

int TFBPlugin_E1_SetFlagBits(RpGeometry *geometry, unsigned bits);  // flags |= bits; 1 when applied
unsigned TFBPlugin_E1_TestFlags(RpGeometry *geometry, unsigned bits);  // flags & bits

// Relocks the geometry's pre-light colours when one of its materials changed colour this frame.
void TFBGeometry_RelockPrelight(RpGeometry *geometry);
// Re-sorts the triangles for `time` and relocks the geometry when the time changed; false when it did not.
bool TFBGeometry_ResortTriangles(RpGeometry *geometry, float time);
RwBool TFBPlugin_E1_Attach();

#endif
