#include <math.h>
#include "modules/tfb/ctfbworld.h"
#include "modules/tfb/renderqueue.h"
#include "tfb/level/level.h"
#include "tfb/render/tfbsector.h"
#include "tfb/render/tfbworld.h"
#include "rw/rpplugins.h"
#include "tfb/render/occluder.h"
#include "modules/tfb/gamecamera.h"

// GLOBAL: 0x62a8d4 cameraExtOffset
// GLOBAL: 0x6221b0 g_levelZone
// GLOBAL: 0x629728 RWS::CLevel::sm_pTranslucentWorld
// GLOBAL: 0x629734 RWS::CLevel::sm_pSkyWorld
// GLOBAL: 0x61811c g_renderQueueFreeList
// GLOBAL: 0x40a5d0 ??0RenderQueueNode@@QAE@PAXMHEE@Z
extern int cameraExtOffset;   // the offset of RW's world camera plugin data (RpWorldCameraExt) in an RwCamera, 0x62a8d4

float TFBClock_GetRawSeconds();                                      // 0x461cd0
int rppvs_static_4b0e00(RpWorldSector *sector);                      // 0x4b0e00 RW's PVS test of a sector

#include "modules/tfb/ctfbworld_views.h"
#include "rw/rpsectorview.h"

namespace RWS { namespace TFB {

RpWorldSector *RenderWorldSector(RpWorldSector *sector, CTFBWorld *world);

// 0x41b150
void CTFBWorld::RenderSectorsPVS(RwCamera *camera, RpWorld *world)
{
	RpPVSSetViewPosition(world, &RwFrameGetLTM(CameraFrame(camera))->pos);
	TFBWorld_UpdateMaterials(world, TFBClock_GetRawSeconds());
	RpWorldCameraExt *ext = (RpWorldCameraExt *)((char *)camera + cameraExtOffset);
	int n = ext->position;
	RpWorldSector **p = ext->frustumSectors;
	if (n) {
		do {
			RpWorldSector *sector = *p;
			if (Fold_ReturnTrue(sector, g_levelZone) && rppvs_static_4b0e00(sector))
				TFBSector_Render(sector);
			p++;
			if (!sector)
				break;
		} while (--n);
	}
}

// 0x41b5f0
void CTFBWorld::RenderSectors(RwCamera *camera, RpWorld *world)
{
	TFBWorld_UpdateMaterials(world, TFBClock_GetRawSeconds());
	RpWorldCameraExt *ext = (RpWorldCameraExt *)((char *)camera + cameraExtOffset);
	int n = ext->position;
	RpWorldSector **p = ext->frustumSectors;
	if (n) {
		do {
			if (!RenderWorldSector(*p++, this))
				break;
		} while (--n);
	}
}

// Renders one sector of the non-PVS walk: sectors with triangles that pass the occluder test are faded by view distance
// (TFBSector_SetFadeDistance), then drawn at once, or queued when the translucent world is being rendered. Returns the
// sector (the walk stops on null).
// Near-match of 0x41b430: m_numTriangles test (two tails), frame 0x24 vs 0x20, z-term of the at-dot first.
// 0x41b2b0 is a thiscall member of CTFBWorld; a fastcall model with a dummy edx needs an `xor edx,edx` the game lacks.
RpWorldSector *RenderWorldSector(RpWorldSector *sector, CTFBWorld *world)
{
	RpWorldSectorView *s = (RpWorldSectorView *)sector;
	if (!Fold_ReturnTrue(sector, g_levelZone))
		return sector;
	{
		if (s->numTriangles > 0) {
			void *curWorld = RwEngineGlobals()->curWorld;
			RwBBox *b = &s->tightBoundingBox;
			int vis = OccluderTestBox(RwCameraGetCurrentCamera(), b, curWorld != CLevel::sm_pSkyWorld);
			if (vis) {
				float ext, dist, fade;
				RwV3d mid;
				mid.x = b->inf.x + b->sup.x;
				mid.y = b->inf.y + b->sup.y;
				mid.z = b->inf.z + b->sup.z;
				mid.x *= 0.5f;
				mid.y *= 0.5f;
				mid.z *= 0.5f;
				dist = CameraSignedViewDistance(RwCameraGetCurrentCamera(), &mid);
				RwV3d h;
				h.x = b->sup.x - mid.x;
				h.y = b->sup.y - mid.y;
				h.z = b->sup.z - mid.z;
				RwMatrix *ltm = RwFrameGetLTM(CameraFrame(RwCameraGetCurrentCamera()));
				{
					float tz = (float)fabs(h.z * ltm->at.z);
					float ty = (float)fabs(h.y * ltm->at.y);
					float tx = (float)fabs(h.x * ltm->at.x);
					ext = (tz + ty) + tx;
				}
				if (dist > ext)
					fade = dist - ext;
				else
					fade = 0;
				TFBSector_SetFadeDistance(sector, fade);
				if (curWorld != CLevel::sm_pTranslucentWorld) {
					TFBSector_Render(sector);
					world->QueueSectorAtomics(sector);
					return sector;
				}
				void *mem = RwFreeListAlloc(g_renderQueueFreeList, 0x40000);
				if (mem)
					new (mem) RenderQueueNode(sector, ext + dist, vis == 1, 1, 1);
			}
		}
		world->QueueSectorAtomics(sector);
		return sector;
	}
}


// 0x41b2b0
void CTFBWorld::QueueSectorAtomics(RpWorldSector *sector)
{
	RpWorldSectorTies *s = (RpWorldSectorTies *)sector;
	PORT_RQ(8);
	for (RpTieView *tie = s->collAtomicsInWorldSector.next; tie != &s->collAtomicsInWorldSector; tie = tie->next) {
		RpAtomic *queued = tie->atomic;
		RpAtomicRenderView *atomic = (RpAtomicRenderView *)queued;
		PORT_RQ(9);
		PORT_RQ_IF(!(atomic->object.object.flags & 4), 10);
		if (atomic->object.object.flags & 4) {
			unsigned short stamp = RwEngineGlobals()->renderFrame;
			PORT_RQ_IF(atomic->renderFrame == stamp, 11);
			if (atomic->renderFrame != stamp) {
				atomic->renderFrame = stamp;
				PORT_RQ(12);
				RenderQueue_AddAtomic(RwCameraGetCurrentCamera(), queued);
			}
		}
	}
}

}}
