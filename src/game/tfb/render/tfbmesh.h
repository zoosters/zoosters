// TFBMesh: the refcounted 0x44-byte mesh block of the geometry plugin's older stream formats (per-vertex colours and texture coordinates, four keyframe track arrays).
#ifndef ZOOSTERS_TFB_RENDER_TFBMESH_H
#define ZOOSTERS_TFB_RENDER_TFBMESH_H

#include "rw/rwcore.h"
#include "tfb/render/flags32.h"

class VertAnimTrack;

// The counts of a mesh block: vertices, and the sets of texture coordinates (8 bytes per vertex) and colours (4 bytes per vertex).
struct TFBMeshHdr {
	Flags32 m_flags;
	int m_vertexCount;
	int m_uvSetCount;     // m_uvs holds vertexCount * uvSetCount * 8 bytes
	int m_colorSetCount;  // m_colors holds vertexCount * colorSetCount * 4 bytes
	void Reset() { m_flags.Set(0); m_colorSetCount = 0; m_uvSetCount = 0; m_vertexCount = 0; }
};

class TFBMesh {
public:
	int m_format;                 // stream format id (0xF0F0002 / 0xF0F0003; an older block starts with its flags instead)
	TFBMeshHdr m_hdr;
	int m_unk14;                  // read by format 3 only
	RwRGBA *m_colors;             // heap block (PTR32: formats 1 and 2 also stream a stale pointer into it)
	RwV2d *m_uvs;                 // heap block (PTR32: the same)
	int m_trackCount[4];          // element count of each track array (formats 2 and 3)
	VertAnimTrack *m_tracks[4];   // track arrays (new[])
	int m_refCount;
	TFBMesh();
	~TFBMesh();
	void Release();  // drops a reference; at 0 frees the blocks
	char Alloc(int vertexCount, int colorSetCount, int uvSetCount);
	char Alloc2();  // Alloc with the header's own counts
	int ReadV1(RwStream *stream);  // returns the bytes read, 0 on a short stream
	int ReadV2(RwStream *stream);
	int ReadV3(RwStream *stream);
	int Read(RwStream *stream);  // dispatches on the format id; returns the bytes read
};
ZOOSTERS_ASSERT_SIZE(TFBMesh, 0x44);
ZOOSTERS_ASSERT_OFFSET(TFBMesh, m_hdr, 0x04);
ZOOSTERS_ASSERT_OFFSET(TFBMesh, m_trackCount, 0x20);
ZOOSTERS_ASSERT_OFFSET(TFBMesh, m_tracks, 0x30);
ZOOSTERS_ASSERT_OFFSET(TFBMesh, m_refCount, 0x40);

#endif
