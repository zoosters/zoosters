#ifndef ZOOSTERS_TFB_RENDERQUEUE_H
#define ZOOSTERS_TFB_RENDERQUEUE_H

#include "rw/rpworld.h"

extern "C" unsigned port_rq[24];   // port/provisional/semtrace.cpp: render-queue counters of the semantic trace (diagnostic, no behaviour)
#define PORT_RQ(i) (port_rq[i]++)
#define PORT_RQ_IF(c, i) do { if (c) port_rq[i]++; } while (0)

extern RwFreeList *g_renderQueueFreeList;   // free list of the nodes

// One queued object (0x14 bytes; the game called it PartListNode).
struct RenderQueueNode {
	RenderQueueNode(void *object, float distance, int kind, unsigned char enabled, unsigned char zWrite);
	static void *operator new(size_t size) { return RwFreeListAlloc(g_renderQueueFreeList, 0x40000); }
	static void *operator new(size_t size, void *where) { return where; }
	static void operator delete(void *node);

	float m_distance;            // +0x00 signed view distance, the sort key (farthest first)
	void *m_object;              // +0x04 an RpAtomic, an RpWorldSector, or a particle pass (kind >= 2)
	int m_kind;                  // +0x08 1 when the occluder test was only partial (vis == 1), 0 when fully visible; 2.. = particle pass
	unsigned char m_enabled;     // +0x0c
	unsigned char m_zWrite;      // +0x0d the ZWRITEENABLE value the object is drawn with
	char m_pad0e[2];
	RenderQueueNode *m_next;     // +0x10 the list through the nodes
};
ZOOSTERS_ASSERT_SIZE(RenderQueueNode, 0x14);
ZOOSTERS_ASSERT_OFFSET(RenderQueueNode, m_object, 0x04);
ZOOSTERS_ASSERT_OFFSET(RenderQueueNode, m_next, 0x10);

extern unsigned g_renderQueueRefCount;   // CTFBWorld instances that opened the queue
extern RenderQueueNode **g_renderQueueNodes;   // pointer array the flush sorts
extern unsigned g_renderQueueCapacity;   // entries of g_renderQueueNodes
extern unsigned g_renderQueueCount;   // nodes in the list
extern RenderQueueNode *g_renderQueueHead;   // first node of the list (RenderFrame empties the list each frame)

void RenderQueue_Open(unsigned count);   // reference count++, creates the free list and the array (count entries) once
void RenderQueue_BuildArray();
// Frustum, occluder and distance tests of one atomic, then queues it (callers: the sector walk, the yon box, Corpus).
void RenderQueue_AddAtomic(RwCamera *camera, RpAtomic *atomic);
int LtMapCompareVertex(const void *a, const void *b);   // qsort order: farthest node first

#endif
