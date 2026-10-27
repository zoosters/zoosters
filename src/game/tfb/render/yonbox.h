// YonBox: the octree of the distant ("yon") geometry, built once from the union of the level worlds' bounding boxes.
// AddAtomic files every atomic of a clump flagged LEVEL_CLUMP_YON; QueueVisibleAtomics walks it each frame against the camera's frustum and queues the atomics of the visible leaf cells.
// Cells are 8-byte slots (slot 0 is never used): an inner cell is eight 16-bit slot indices (two slots when it also splits in y), a leaf cell is the tag 0xffff and a pointer at +4 to a linked list of YonLink entries in the same slot array.
#ifndef ZOOSTERS_TFB_RENDER_YONBOX_H
#define ZOOSTERS_TFB_RENDER_YONBOX_H

#include <stddef.h>
#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"

void *MemAlloc(size_t size);          // 0x550170

struct YonBox {
	YonBox(const RwBBox *b);
	void AddAtomic(RpAtomic *atomic);  // files the atomic by its world bounding sphere (nothing for a geometry without triangles)
	// Files `atomic` into the cell `node` (centre a, b, c; half size ext) and the children it reaches; a cell is skipped when the atomic's bounding box bb misses it or none of its triangles touches it.
	void InsertNode(short *node, float a, float b, float c, RpAtomic *atomic, const RwBBox *bb, const RwV3d *ext);
	// Culls the cell against the camera's six frustum planes (depth 0 only), queues the atomics of a leaf (once per frame) or walks the eight children.
	void Walk(short index, float cx, float cy, float cz, int depth, const RwV3d *half);
	void QueueVisibleAtomics(RwCamera *camera);  // stores the camera, then Walk from the root

	RwV3d m_center;                   // centre of the bounds
	RwV3d m_half;                     // half extents (at least 480)
	RwV3d m_minCell;                  // smallest cell extent per axis: 480 when the bounds are that thin, else -1 until
	                                  // InsertNode stores the first leaf extent; InsertNode clamps the cell extent to it
	// A slot holds two pointers (leaf: tag + list pointer; link: atomic + next): 8 bytes on x86, 16 on x64.
	enum { kSlot = 2 * sizeof(void *), kSlots = 0x104f0 / 8 };
	alignas(void *) char m_cells[kSlots * kSlot];
	char *Slot(int index) { return m_cells + (index - 1) * kSlot; }
	static void **LeafList(short *cell) { return (void **)((char *)cell + sizeof(void *)); }
	int m_nodeCount;                  // next free cell slot (`*node = ++m_nodeCount` in InsertNode); 1, 2 when the y extent is too large
	RwCamera *m_camera;               // (QueueVisibleAtomics)
	static void *operator new(size_t n) { return MemAlloc(n); }
};
ZOOSTERS_ASSERT_SIZE(YonBox, 0x1051c);
ZOOSTERS_ASSERT_OFFSET(YonBox, m_cells, 0x24);
ZOOSTERS_ASSERT_OFFSET_AT(YonBox_slot1, offsetof(YonBox, m_cells), 0x1c + 1 * 8);
ZOOSTERS_ASSERT_OFFSET_AT(YonBox_slotSize, (size_t)YonBox::kSlot, 8);
ZOOSTERS_ASSERT_OFFSET(YonBox, m_nodeCount, 0x10514);

#endif
