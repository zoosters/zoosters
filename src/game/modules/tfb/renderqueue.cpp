#include <stdlib.h>
#include "modules/tfb/renderqueue.h"
#include "modules/tfb/ctfbworld.h"
#include "modules/fx/particlepass.h"
#include "tfb/render/tfbatomic.h"
#include "tfb/render/tfbrenderstate.h"
#include "tfb/render/tfbsector.h"
#include "tfb/render/occluder.h"
#include "modules/tfb/gamecamera.h"

// GLOBAL: 0x61811c g_renderQueueFreeList
// GLOBAL: 0x618124 g_renderQueueRefCount
// GLOBAL: 0x618114 g_renderQueueNodes
// GLOBAL: 0x618120 g_renderQueueCapacity
// GLOBAL: 0x618118 g_renderQueueCount
// GLOBAL: 0x618110 g_renderQueueHead


// 0x40a5d0
RenderQueueNode::RenderQueueNode(void *object, float distance, int kind, unsigned char enabled, unsigned char zWrite)
{
	m_distance = distance;
	m_object = object;
	m_kind = kind;
	m_enabled = enabled;
	m_zWrite = zWrite;
	m_next = g_renderQueueHead;
	g_renderQueueHead = this;
	g_renderQueueCount++;
}

// 0x40a620
void RenderQueueNode::operator delete(void *node)
{
	if (node)
		RwFreeListFree(g_renderQueueFreeList, node);
}

// 0x41a0c0
void RenderQueue_Open(unsigned count)
{
	g_renderQueueRefCount++;
	if (g_renderQueueFreeList == 0)
		g_renderQueueFreeList = RwFreeListCreate(sizeof(RenderQueueNode), count, 0x10, 0x40000);
	if (g_renderQueueNodes == 0) {
		g_renderQueueCapacity = count;
		g_renderQueueNodes = (RenderQueueNode **)RwMalloc(count * sizeof(RenderQueueNode *), 0x40000);
	}
}

// 0x41a130
void RenderQueue_BuildArray()
{
	if (g_renderQueueCount > g_renderQueueCapacity) {
		g_renderQueueCapacity = g_renderQueueCount;
		g_renderQueueNodes = (RenderQueueNode **)RwRealloc(g_renderQueueNodes, g_renderQueueCount * sizeof(RenderQueueNode *), 0x40000);
	}
	RenderQueueNode *node = g_renderQueueHead;
	for (unsigned i = 0; i < g_renderQueueCount; i++) {
		g_renderQueueNodes[i] = node;
		node = node->m_next;
	}
}

// 0x41a080
int LtMapCompareVertex(const void *a, const void *b)
{
	float x = (*(RenderQueueNode *const *)a)->m_distance;
	float y = (*(RenderQueueNode *const *)b)->m_distance;
	if (x > y)
		return -1;
	if (x == y)
		return 0;
	return 1;
}

// The game tests only al after the call (test al,al); TFBAtomic::Update is declared int because its own bytes match that way.
typedef bool (*TFBAtomicUpdateBool)(RpAtomic *atomic, RwCamera *camera, float viewDistance);

// 0x41ae40
void RenderQueue_AddAtomic(RwCamera *camera, RpAtomic *atomic)
{
	PORT_RQ(0);
	const RwSphere *sphere = RpAtomicGetWorldBoundingSphere(atomic);
	if (RwCameraFrustumTestSphere(camera, sphere)) {
		int vis = OccluderTestSphere(camera, (SphereP413 *)sphere, 1);
		PORT_RQ_IF(!vis, 2);
		if (vis) {
			PORT_RQ(3);
			float dist = CameraSignedViewDistance(camera, &RwFrameGetLTM(RpAtomicGetFrame(atomic))->pos);   // the RwObject parent: +4 / +8
			bool ok = ((TFBAtomicUpdateBool)&TFBAtomic::Update)(atomic, camera, dist);
			if (!ok) {
				PORT_RQ(4);
				dist += 1000000.0f;
			}
			if (TFBPlugin_FD_TestFlags(atomic, TFB_ATOMIC_NO_ZTEST))
				dist -= 1000000.0f;
			new RenderQueueNode(atomic, dist, vis == 1, ok, TFBPlugin_FD_IsVisible(atomic));
		}
	}
}

// Sorts the queued nodes back to front and draws them: a particle pass (kind >= 2) draws its PTanks, an atomic is drawn with the z-write
// state its node holds,
// a sector with z-write on. Every node goes back to the free list.
// 0x41b1d0
void RWS::TFB::CTFBWorld::FlushRenderQueue()
{
	if (g_renderQueueCount) {
		RenderQueue_BuildArray();
		qsort(g_renderQueueNodes, g_renderQueueCount, sizeof(RenderQueueNode *), LtMapCompareVertex);
		unsigned char restore = 0;
		for (unsigned i = 0; i < g_renderQueueCount; i++) {
			RenderQueueNode *node = g_renderQueueNodes[i];
			void *obj = node->m_object;
			if (node->m_kind >= 2) {
				((ParticlePass *)obj)->Render();
			} else if (*(unsigned char *)obj == 1) {   // an RpAtomic (RW object type 1), else a world sector
				unsigned char zWrite = node->m_zWrite;
				if (restore == zWrite) {
					RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)zWrite);
					restore = zWrite == 0;
				}
				StencilEnable(zWrite);
				TFBAtomic::RenderCallback((RpAtomic *)obj);
			} else {
				if (restore) {
					RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)1);
					restore = 0;
				}
				TFBSector_Render((RpWorldSector *)obj);
			}
			RwFreeListFree(g_renderQueueFreeList, node);
		}
	}
}
