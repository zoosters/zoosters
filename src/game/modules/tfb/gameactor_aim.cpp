// Per-attachment aim update of a GameActor's attached particle emitters: callback with four stack arguments (unused, id, matrix, actor),
// cdecl, returns 0.
#include <math.h>
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "script/referenceset.h"
#include "rw/rwcore.h"
#include "tfb/corpus/corpus.h"
#include <string.h>
#include "script/types/actorturret.h"
#include "modules/fx/attachpoint.h"

extern const RwV3d g_aimAxisA;     // GLOBAL: 0x5cd2b0 g_aimAxisA
extern const RwV3d g_aimAxisB;     // GLOBAL: 0x5cd2a4 g_aimAxisB
// the emitter list nodes are Script::SetNode (prev, next, key, object); the object is the emitter state
typedef Script::SetNode AimNode;
float LerpFloat(float t, float a, float b);                       // 0x464b10

static __forceinline void AimClamp(float &v, float lim)
{
	if (v < -lim)
		v = -lim;
	if (v > lim)
		v = lim;
}

static __forceinline void AimApply(RwMatrix *mA, RwMatrix *mat, Script::ActorTurret *p, float yaw, float pitch)
{
	RwMatrixRotate(mA, &g_aimAxisA, yaw, rwCOMBINEPRECONCAT);
	RwMatrixRotate(mA, &g_aimAxisB, -pitch, rwCOMBINEPRECONCAT);
	RwV3d saved = mat->pos;
	RwMatrixMultiply(mat, reinterpret_cast<RwMatrix *>(p->m_70), mA);
	mat->pos = saved;
}

// 0x42b2d0
// (declared with void * matrix / actor in tfb/corpus/corpus_private.h: the mangled name must stay the same)
int AimAttachmentsCB(int unused, void *id, void *matrix, void *actorPtr)
{
	RwMatrix *mat = (RwMatrix *)matrix;
	RWS::TFB::GameActor *actor = (RWS::TFB::GameActor *)actorPtr;
	RwMatrix mA, mB, mC;
	AimNode *n;
	Script::ActorTurret *p = 0;
	float pitch, yaw;
	int path = 0;
	for (n = ((Script::ReferenceSet *)actor->m_turrets)->m_head; n; n = n->m_next) {
		p = (Script::ActorTurret *)n->m_object;
		if ((intptr_t)p->m_entry->m_boneIndex == (intptr_t)id) {
			path = 1;
			break;
		}
	}
	if (!path) {
		Corpus *corpus = actor->m_corpus;
		for (n = corpus->m_feet.m_head; n; n = n->m_next) {
			p = (Script::ActorTurret *)n->m_object;
			if ((intptr_t)p->m_entry->m_boneIndex == (intptr_t)id) {
				path = 2;
				break;
			}
		}
		if (!path)
			return 0;
	}
	if (path == 1) {
		Script::PositionMeasurement *tgt = (Script::PositionMeasurement *)p->m_lookAt.m_actor;
		if (tgt && tgt != (Script::PositionMeasurement *)actor) {
			if (p->m_localDirty)
				reinterpret_cast<AttachPoint *>(p)->RebuildLocalMatrix();
			RwMatrixMultiply(&mA, reinterpret_cast<RwMatrix *>(p->m_localMatrix), mat);
			RwMatrixMultiply(&mB, &mA, RwFrameGetLTM(RwFrameGetParent(p->m_parentFrame)));
			RwMatrixInvert(&mC, &mB);
			Script::PositionMeasurement *target = tgt;
			RwV3d *targetPos = (RwV3d *)target->GetVector(target);
			float base;
			if (p->m_lookAt.m_headingRel)
				base = target->GetFacing() + 180.0f;
			else
				base = 0.0f;
			float ang = (base + p->m_lookAt.m_headingOffs) * 0.017453292f;
			RwV3d v;
			v.y = p->m_lookAt.m_vertOffs * 12.0f + targetPos->y;
			v.x = (float)sin(ang) * p->m_lookAt.m_horzOffs * 12.0f + targetPos->x;
			v.z = (float)cos(ang) * p->m_lookAt.m_horzOffs * 12.0f + targetPos->z;
			RwV3dTransformPoint(&v, &v, &mC);
			yaw = (float)atan2(v.x, v.z) * 57.2957763671875f;
			double vy = v.y;
			v.y = 0.0f;
			pitch = (float)atan2(vy, (double)RwV3dLength(&v)) * 57.2957763671875f;
			float yawLim = p->m_fov;
			PartEntry *limits = p->m_entry;
			float yawMax = yawLim + limits->m_margin;
			if (!(yaw < yawMax) || !(-yawMax < yaw)) {
				yaw = 0.0f;
				pitch = 0.0f;
			} else {
				if (!(yaw < yawLim && -yawLim < yaw))
					AimClamp(yaw, yawLim);
				float pitchLim = limits->m_pitchLimit;
				float pitchMax = pitchLim + limits->m_margin;
				if (!(pitch < pitchMax) || !(-pitchMax < pitch)) {
					yaw = 0.0f;
					pitch = 0.0f;
				} else {
					if (!(pitch < pitchLim && -pitchLim < pitch))
						AimClamp(pitch, pitchLim);
				}
			}
		} else {
			if (p->m_pitch == 0.0f && p->m_yaw == 0.0f)
				return 0;
			if (p->m_localDirty)
				reinterpret_cast<AttachPoint *>(p)->RebuildLocalMatrix();
			RwMatrixMultiply(&mA, reinterpret_cast<RwMatrix *>(p->m_localMatrix), mat);
			yaw = 0.0f;
			pitch = 0.0f;
		}
		pitch = LerpFloat(p->m_aimRate, p->m_pitch, pitch);
		if (fabsf(pitch) < 0.01f)
			pitch = 0.0f;
		p->m_pitch = pitch;
		yaw = LerpFloat(p->m_aimRate, p->m_yaw, yaw);
		if (fabsf(yaw) < 0.01f)
			yaw = 0.0f;
		p->m_yaw = yaw;
		AimApply(&mA, mat, p, yaw, pitch);
	} else {
		if (p->m_localDirty)
			reinterpret_cast<AttachPoint *>(p)->RebuildLocalMatrix();
		RwMatrixMultiply(&mA, reinterpret_cast<RwMatrix *>(p->m_localMatrix), mat);
		RwMatrixMultiply(&mB, &mA, RwFrameGetLTM(RwFrameGetParent(p->m_parentFrame)));
		Corpus *corpus = actor->m_corpus;
		RwMatrixInvert(&mC, &corpus->m_frame->modelling);
		RwV3d t;
		RwV3dTransformPoint(&t, &mB.pos, &mC);   // the matrix POSITION (+0x30), not its right vector
		p->m_prevPos = p->m_curPos;
		memcpy(&p->m_curPos, &t, sizeof t);
		if ((actor->m_corpus->m_flags & 0x200) && p->m_pitch == 0.0f && p->m_yaw == 0.0f)
			return 0;
		pitch = LerpFloat(p->m_aimRate, p->m_pitch, 0.0f);
		if (fabsf(pitch) < 0.01f)
			pitch = 0.0f;
		p->m_pitch = pitch;
		yaw = LerpFloat(p->m_aimRate, p->m_yaw, 0.0f);
		if (fabsf(yaw) < 0.01f)
			yaw = 0.0f;
		p->m_yaw = yaw;
		AimApply(&mA, mat, p, yaw, pitch);
	}
	return 0;
}

