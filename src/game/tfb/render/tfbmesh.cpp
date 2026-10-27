// TFBMesh: the refcounted mesh block of the geometry plugin's older formats, and the keyframe track element it holds.
#include "rw/rwcore.h"
#include "tfb/anim/vertanimtrack.h"
#include "tfb/render/tfbmesh.h"

extern int g_tfbMeshMagic; // GLOBAL: 0x606830 g_tfbMeshMagic

// 0x45d800
TFBMesh::TFBMesh()
{
	m_format = g_tfbMeshMagic;
	m_refCount = 0;
	m_hdr.Reset();
	m_unk14 = 0;
	m_colors = 0;
	m_uvs = 0;
	m_trackCount[0] = 0;
	m_tracks[0] = 0;
	m_trackCount[1] = 0;
	m_tracks[1] = 0;
	m_trackCount[2] = 0;
	m_tracks[2] = 0;
	m_trackCount[3] = 0;
	m_tracks[3] = 0;
}

// 0x45d200
char TFBMesh::Alloc(int vertexCount, int colorSetCount, int uvSetCount)
{
	if (vertexCount <= 0)
		return 0;
	if (colorSetCount > 0) {
		m_colors = (RwRGBA *)MemAlloc(vertexCount * colorSetCount * 4);
		if (!m_colors)
			return 0;
	}
	if (uvSetCount > 0) {
		m_uvs = (RwV2d *)MemAlloc(vertexCount * uvSetCount * 8);
		if (!m_uvs)
			return 0;
	}
	m_hdr.m_uvSetCount = uvSetCount;
	m_hdr.m_colorSetCount = colorSetCount;
	m_hdr.m_vertexCount = vertexCount;
	return 1;
}

// 0x45d280
char TFBMesh::Alloc2()
{
	int uvSets = m_hdr.m_uvSetCount; // locals in last-argument-first order keep the vertex count in esi
	int colorSets = m_hdr.m_colorSetCount;
	int vertices = m_hdr.m_vertexCount;
	return Alloc(vertices, colorSets, uvSets);
}

// 0x45d190
void TFBMesh::Release()
{
	if (--m_refCount > 0)
		return;
	if (m_colors)
		operator_delete(m_colors);
	if (m_uvs)
		operator_delete(m_uvs);
	m_hdr.Reset();
	m_colors = 0;
	m_uvs = 0;
	for (int i = 0; i < 4; i++) {
		// only the destructor of the first element runs, and the array block is not freed (as the game does)
		if (m_trackCount[i] > 0 && m_tracks[i])
			m_tracks[i]->~VertAnimTrack();
	}
}

// 0x45d860
TFBMesh::~TFBMesh()
{
	Release();
}

// Format without a version id: the header counts, two stale pointer words, then the colour and uv blocks.
// 0x45d2a0
int TFBMesh::ReadV1(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_hdr.m_vertexCount, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_hdr.m_uvSetCount, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_hdr.m_colorSetCount, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_colors, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_uvs, 4))
		return 0;
	int total = 0x14;
	int colorBytes = m_hdr.m_colorSetCount * m_hdr.m_vertexCount * 4;
	int uvBytes = m_hdr.m_uvSetCount * m_hdr.m_vertexCount * 8;
	Alloc2();
	if (colorBytes > 0) {
		if (!RwStreamRead(stream, m_colors, colorBytes))
			return 0;
		total += colorBytes;
	} else {
		m_colors = 0;
	}
	if (uvBytes > 0) {
		if (!RwStreamRead(stream, m_uvs, uvBytes))
			return 0;
		total += uvBytes;
	} else {
		m_uvs = 0;
	}
	return total;
}

// 0x45d3c0
int TFBMesh::ReadV2(RwStream *stream)
{
	int *uvSets = &m_hdr.m_uvSetCount;
	int *colorSets = &m_hdr.m_colorSetCount;
	if (!RwStreamReadInt32(stream, (int *)&m_hdr.m_flags.m_bits, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_hdr.m_vertexCount, 4))
		return 0;
	if (!RwStreamReadInt32(stream, uvSets, 4))
		return 0;
	if (!RwStreamReadInt32(stream, colorSets, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_colors, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_uvs, 4))
		return 0;
	int total = 0x18;
	int colorBytes = *colorSets * m_hdr.m_vertexCount * 4;
	int uvBytes = *uvSets * m_hdr.m_vertexCount * 8;
	Alloc2();
	if (colorBytes > 0) {
		if (!RwStreamRead(stream, m_colors, colorBytes))
			return 0;
		total += colorBytes;
	} else {
		m_colors = 0;
	}
	if (uvBytes > 0) {
		if (!RwStreamRead(stream, m_uvs, uvBytes))
			return 0;
		total += uvBytes;
	} else {
		m_uvs = 0;
	}
	int *count = m_trackCount;
	for (int k = 0; k < 4; k++, count++) {
		RwStreamReadInt32(stream, count, 4);
		total += 4;
		int n = *count;
		if (n > 0) {
			VertAnimTrack **slot = &m_tracks[k];                  // count + 4 is m_tracks[k] only with 4-byte pointers (x64: tracks 1-3 misplaced)
			*slot = new VertAnimTrack[n];
			if (!*slot)
				return 0;
			for (int i = 0; i < *count; i++)
				total += (*slot)[i].Read(stream);
		}
	}
	return total;
}

// 0x45d5d0
int TFBMesh::ReadV3(RwStream *stream)
{
	int *uvSets = &m_hdr.m_uvSetCount;
	int *colorSets = &m_hdr.m_colorSetCount;
	if (!RwStreamReadInt32(stream, (int *)&m_hdr.m_flags.m_bits, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_hdr.m_vertexCount, 4))
		return 0;
	if (!RwStreamReadInt32(stream, uvSets, 4))
		return 0;
	if (!RwStreamReadInt32(stream, colorSets, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_unk14, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_colors, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_uvs, 4))
		return 0;
	int total = 0x1c;
	int colorBytes = *colorSets * m_hdr.m_vertexCount * 4;
	int uvBytes = *uvSets * m_hdr.m_vertexCount * 8;
	Alloc2();
	if (colorBytes > 0) {
		if (!RwStreamRead(stream, m_colors, colorBytes))
			return 0;
		total += colorBytes;
	} else {
		m_colors = 0;
	}
	if (uvBytes > 0) {
		if (!RwStreamRead(stream, m_uvs, uvBytes))
			return 0;
		total += uvBytes;
	} else {
		m_uvs = 0;
	}
	int *count = m_trackCount;
	for (int k = 0; k < 4; k++, count++) {
		RwStreamReadInt32(stream, count, 4);
		total += 4;
		int n = *count;
		if (n > 0) {
			VertAnimTrack **slot = &m_tracks[k];                  // count + 4 is m_tracks[k] only with 4-byte pointers (x64: tracks 1-3 misplaced)
			*slot = new VertAnimTrack[n];
			if (!*slot)
				return 0;
			for (int i = 0; i < *count; i++)
				total += (*slot)[i].Read(stream);
		}
	}
	return total;
}

enum TFBMeshFormat { TFB_MESH_FORMAT_2 = 0xF0F0002, TFB_MESH_FORMAT_3 = 0xF0F0003 };

// 0x45d870
int TFBMesh::Read(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_format, 4))
		return 0;
	switch (m_format) {
	case TFB_MESH_FORMAT_2:
		return ReadV2(stream) + 4;
	case TFB_MESH_FORMAT_3:
		return ReadV3(stream) + 4;
	default:
		m_hdr.m_flags.Set(m_format);
		return ReadV1(stream) + 4;
	}
}
