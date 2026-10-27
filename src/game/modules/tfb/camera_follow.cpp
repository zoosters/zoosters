// Per tick: places the two script cameras' targets (look-at / look-from) from the followed actors, then calls the camera update.
#include <math.h>
#include <stddef.h>
#include "modules/tfb/gamecamera.h"
#include "modules/tfb/gameactor.h"
#include "modules/tfb/cameradata.h"
#include "script/types/scriptcamera.h"
#include "script/types/scriptsystem.h"
#include "tfb/corpus/corpus.h"

using RWS::TFB::GameActor;
using Script::PointerTarget;
using Script::ScriptCamera;

void *ScriptCamera_DefaultLookAtSlot();   // 0x438910 (&0x61ff68)
void *ScriptCamera_DefaultLookFromSlot();   // 0x438920 (&0x61ff6c)
extern void *g_camObjPrev;                        // GLOBAL: 0x618194 g_camObjPrev
extern GameActor *g_camActorPrev;                 // GLOBAL: 0x618190 g_camActorPrev
extern GameActor *g_cameraFollowActor;            // GLOBAL: 0x61818c g_cameraFollowActor

// 0x41dc60
void Camera_PostTickFollowUpdate()
{
	unsigned char changed = 0;
	ScriptCamera *cam = (ScriptCamera *)GetCameraObject();
	if (cam) {
		CameraSetCut(cam != g_camObjPrev);
		PointerTarget *a = &cam->m_lookAt;
		g_camObjPrev = cam;
		GameActor *act = (GameActor *)a->GetActor();
		if (!act) {
			act = *(GameActor **)ScriptCamera_DefaultLookAtSlot();
		}
		if (act) {
			GameActor **slot = (GameActor **)ScriptCamera_DefaultLookAtSlot();
			if (act != *slot) {
				if (*slot)
					(*slot)->AdjustRefCount(-1);
				*slot = act;
				if (act)
					act->AdjustRefCount(1);
			}
			RwV3d *pos = (RwV3d *)act->GetVector(act);
			RwV3d v;
			v.y = a->m_vertOffs * 12.0f + pos->y;
			float base;
			if (a->m_headingRel)
				base = act->GameActor::GetFacing() + 180.0f;
			else
				base = g_floatZero;
			float ang = (base + a->m_headingOffs) * 0.017453292f;
			v.x = (float)sin(ang) * a->m_horzOffs * 12.0f + pos->x;
			v.z = (float)cos(ang) * a->m_horzOffs * 12.0f + pos->z;
			int now = *Script::ScriptSystem::GetScriptTimePtr();
			if (act->m_warpTime == now)
				CameraSetCut(true);
			RwV3d *p = act->m_corpus ? &act->m_corpus->m_contactPoint : &act->m_matrix.pos;
			CameraSetLookAtPoint(cam, &v, p->y, pos->y);
			if (act != g_camActorPrev) {
				changed = 1;
				g_camActorPrev = act;
			}
		}
		PointerTarget *b = &cam->m_lookFrom;
		act = (GameActor *)b->GetActor();
		if (!act) {
			act = *(GameActor **)ScriptCamera_DefaultLookFromSlot();
		}
		if (act) {
			GameActor **slot = (GameActor **)ScriptCamera_DefaultLookFromSlot();
			if (act != *slot) {
				if (*slot)
					(*slot)->AdjustRefCount(-1);
				*slot = act;
				if (act)
					act->AdjustRefCount(1);
			}
			RwV3d *pos = (RwV3d *)act->GetVector(act);
			float heading = b->m_headingOffs;
			RwV3d v;
			v.y = b->m_vertOffs * 12.0f + pos->y;
			if (cam->m_flags & 1)
				heading = CameraGetHeading();
			else if (b->m_headingRel)
				heading = act->GameActor::GetFacing() + heading + 180.0f;
			CameraSetHeading(heading);
			float ang = heading * 0.017453292f;
			v.x = (float)sin(ang) * b->m_horzOffs * 12.0f + pos->x;
			v.z = (float)cos(ang) * b->m_horzOffs * 12.0f + pos->z;
			int now = *Script::ScriptSystem::GetScriptTimePtr();
			if (act->m_warpTime == now)
				CameraSetCut(true);
			if (act != g_cameraFollowActor) {
				g_cameraFollowActor = act;
				if (changed)
					CameraSetCut(true);
			}
			RwV3d *p = act->m_corpus ? &act->m_corpus->m_contactPoint : &act->m_matrix.pos;
			CameraSetLookFromPoint(cam, &v, p->y, pos->y);
		}
		if (cam->m_ownerSlot) {   // &CameraData::m_cam of the owning behaviour
			CameraSetAnimation((RtAnimAnimation *)RWS::TFB::CameraData::FromCamSlot(cam->m_ownerSlot)->GetAnimation());
		}
	}
	Camera_Update();
}
