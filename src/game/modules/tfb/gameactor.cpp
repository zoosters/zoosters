#include <math.h>
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "modules/fx/attachpoint.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"

// GLOBAL: 0x618318 g_frameDt
extern float g_frameDt;
// GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatZero;

struct SetCB {
	int Remove(void *obj);   // 0x43cec0 (returns the node count, 0 = absent)
};
// GLOBAL: 0x43cec0 ?Remove@SetCB@@QAEHPAX@Z
struct LeaveCB {
	void ActivateActor(int a);
};
// GLOBAL: 0x437d40 ?ActivateActor@LeaveCB@@QAEXH@Z
void CutScene_RemoveActor(void *actor);

namespace RWS { namespace TFB {

// 0x42b1d0
bool GameActor::PlayAnimation(int index)
{
	if (m_corpus && index >= -1) {
		if (index >= 0)
			m_corpus->m_animOverride = index;
		return m_corpus->m_model.PlaySlot(index, g_frameDt);
	}
	return false;
}

// 0x42b210
int GameActor::AccessBehavior(int index, int value)
{
	Corpus *body = m_corpus;
	if (body) {
		if (index == -1)
			return body->m_model.GetDstAnimIndex();
		return m_corpus->m_model.ExchangeSlotIndex(index, value);
	}
	return 0;
}

// 0x42b250
void GameActor::SetAttachTarget(Script::ScriptReference *target)
{
	if (m_corpus) {
		intptr_t carrier = ((Script::PositionMeasurement *)target)->vf26((intptr_t)this);
		if (carrier) {
			m_corpus->m_link.m_index = -1;
			m_corpus->m_link.m_attachPoint = (AttachPoint *)target;
			m_corpus->m_link.m_frame = m_corpus->m_link.m_attachPoint->m_frame;
			m_corpus->AttachToCarrier((GameActor *)carrier);
		} else if (m_corpus->m_link.m_carrier) {
			m_corpus->LeaveGround(0, false);
		}
	}
}

// 0x42b8f0
float GameActor::GetFacing()
{
	float f = m_heading + m_facing;
	if (f < 0.0f)
		return f + 360.0f;
	if (f >= 360.0f)
		return f - 360.0f;
	return f;
}

// 0x42b930
bool GameActor::SetFacing(float v, int snap)
{
	float &target = m_targetFacing;
	target = v;
	if (v < 0.0f)
		target = (float)fmod((double)v, 360.0) + 360.0f;
	else if (v > 360.0f)
		target = (float)fmod((double)v, 360.0) - 360.0f;
	if ((char)snap) {
		float t = target;
		m_heading = 0.0f;
		m_facing = t;
		return true;
	}
	float cur = m_heading + m_facing;
	if (cur < 0.0f)
		cur += 360.0f;
	else if (cur >= 360.0f)
		cur -= 360.0f;
	float d = target - cur;
	if (d > 180.0f)
		d -= 360.0f;
	else if (d < -180.0f)
		d += 360.0f;
	return d < 0.5f && d > -0.5f;
}

// 0x42ba90
void GameActor::SetVector(const Vec3 *v, Script::ScriptReference *)
{
	const Vec3 *cur = m_corpus ? (const Vec3 *)&m_corpus->m_lastFrame.pos : (const Vec3 *)&m_matrix.pos;
	float dx = v->x - cur->x;
	float dy = v->y - cur->y;
	float dz = v->z - cur->z;
	float d2 = dy * dy;
	d2 += dx * dx;
	d2 += dz * dz;
	if (d2 > 144.0f)
		m_warpTime = *Script::ScriptSystem::GetScriptTimePtr();
	PlaceAt(v);
}

// 0x42bb00
void GameActor::SetDestinationTimed(Vec3 *pos, float seconds)
{
	SetDestination(pos, 0);
	m_timedMoveStart = *Script::ScriptSystem::GetScriptTimePtr();
	m_timedMoveMs = (int)(seconds * 1000.0f);
	const Vec3 *cur = m_corpus ? (const Vec3 *)&m_corpus->m_lastFrame.pos : (const Vec3 *)&m_matrix.pos;
	m_timedMoveFrom = *cur;
	if (m_corpus)
		m_corpus->SetEnabled(-1);
	Collider::Reset();
}

// 0x42b190
void GameActor::DetachProto()
{
	for (Script::SetNode *n = m_20.m_head; n; n = n->m_next)
		((GameActor *)n->m_object)->DetachProto();
	((Script::ReferenceSet *)m_wayPoints)->Clear();
	m_proto = 0;
}

// Near-match of 0x42d430: mov ecx,esi vs push eax order only.
char GameActor::Control(bool on)
{
	CutScene_RemoveActor(this);
	int state = m_controlMode;
	if (state != 3) {
		if (state == 2) {
			LeaveCB *lc = (LeaveCB *)this;
			lc->ActivateActor(((SetCB *)TFB_GetInactiveSet())->Remove(this));
		}
		if (on)
			m_scriptClockHold = 1;
		return 1;
	}
	return 0;
}

// 0x42d940
void GameActor::SetDestination(Vec3 *pos, int flag)
{
	m_destination.x = pos->x;
	m_destination.y = pos->y;
	m_destination.z = pos->z;
	Corpus *corpus = m_corpus;
	if (corpus) {
		if ((unsigned char)flag) {
			corpus->m_moveInput.x = -1.0f;
			return;
		}
		if (pos->x == corpus->m_lastFrame.pos.x && pos->y == corpus->m_lastFrame.pos.y && pos->z == corpus->m_lastFrame.pos.z)
			corpus->m_moveInput.x = g_floatZero;
		else
			corpus->m_moveInput.x = 1.0f;
	}
}

} }

namespace RWS { namespace TFB {

// 0x42ba50
float GameActor::GetTilt()
{
	return m_tilt;
}

// 0x42ba60
bool GameActor::SetTilt(float v, Script::ScriptReference *)
{
	m_tilt = v;
	return true;
}

// 0x42ba70
float GameActor::GetRoll()
{
	return m_roll;
}

// 0x42ba80
bool GameActor::SetRoll(float v, Script::ScriptReference *)
{
	m_roll = v;
	return true;
}

// 0x42d8e0
int GameActor::IsSolidCollider()
{
	return m_params.m_flags & ACTOR_SOLID_COLLIDER;
}

// 0x42d8f0
int GameActor::IsMeshCollider()
{
	return (m_params.m_flags & ACTOR_MESH_COLLIDER) && m_corpus;
}

// 0x42d910
int GameActor::IsMeshCollidee()
{
	return (m_params.m_flags & ACTOR_MESH_COLLIDEE) && m_corpus;
}

} }

inline int SetCB::Remove(void *obj) { return ((Script::ReferenceSet *)this)->Remove((Script::ScriptReference *)obj); }
inline void LeaveCB::ActivateActor(int a) { ((RWS::TFB::GameActor *)this)->ActivateActor((unsigned char)a != 0); }   // (the callee reads the low byte only: callers pass wider ints)
