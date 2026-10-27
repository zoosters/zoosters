#include "script/script.h"
#include "tfb/level/level.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "script/ops_message.h"
#include "script/ops_set.h"
#include "script/script.h"
#include "rw/rpcollis.h"
#include "modules/tfb/gameactor.h"
#include "tfb/collision/collider.h"

// The ray probe the game builds on the stack: vptr 0x5cd2d8 (callback data of 0x42b140), an RpIntersection (line), the line's direction,
// the result byte (preset 1) and one int taken from the target.
// GLOBAL: 0x5cd2d8 g_vtLosProbe
// GLOBAL: 0x618338 g_msgCorpusCamera
extern RWS::CEventId g_msgCorpusCamera;
// GLOBAL: 0x61837c g_msgInvalid61837c
extern RWS::CEventId g_msgInvalid61837c;
// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld
#define SELF ((Script::ScriptReference *)g_scriptSelf)
int LosProbeTriangleCB(int, int, int, int, unsigned char *p);   // 0x42b140 (callback; tfb/collision/losprobe.cpp)

#include "script/losprobe.h"

// the top of an actor's collision box: the world box's centre plus its up axis times the half height (actor +0x10c, +0x124.., +0x144..)
static void LosActorTop(Script::ScriptReference *actor, Vec3 *pos)
{
	// the game build's raw offsets from the actor: the Collider is GameActor's second base at +0x98 (ScriptActor 0x98 bytes, the
	// members after the Collider start at 0x98 + sizeof(Collider)); world box +0x70, velocity +0xbc, clump +0xd0 in it
	static_assert(sizeof(void *) != 4 || (sizeof(Script::ScriptActor) == 0x98 && offsetof(RWS::TFB::GameActor, m_matrix) == 0x98 + sizeof(RWS::TFB::Collider)), "Collider is not at +0x98 of the GameActor");
	static_assert(sizeof(void *) != 4 || 0x98 + offsetof(RWS::TFB::Collider, m_worldBox.m_extent[1]) == 0x10c, "world box extent[1] is not at actor +0x10c");
	static_assert(sizeof(void *) != 4 || 0x98 + offsetof(RWS::TFB::Collider, m_worldBox.m_frame.up) == 0x124, "world box up axis is not at actor +0x124");
	static_assert(sizeof(void *) != 4 || 0x98 + offsetof(RWS::TFB::Collider, m_worldBox.m_frame.pos) == 0x144, "world box centre is not at actor +0x144");
	static_assert(sizeof(void *) != 4 || 0x98 + offsetof(RWS::TFB::Collider, m_clump) == 0x168, "Collider::m_clump is not at actor +0x168");
	RWS::TFB::Collider &c = *static_cast<RWS::TFB::GameActor *>(actor);
	float k = c.m_worldBox.m_extent[1];
	pos->x = k * c.m_worldBox.m_frame.up.x;
	pos->y = k * c.m_worldBox.m_frame.up.y;
	pos->z = k * c.m_worldBox.m_frame.up.z;
	pos->x += c.m_worldBox.m_frame.pos.x;
	pos->y += c.m_worldBox.m_frame.pos.y;
	pos->z += c.m_worldBox.m_frame.pos.z;
}

// stand-in for the object's vtable up to slot 17 (position getter)
struct LosRef : Script::ScriptReference {
	virtual void a11();
	virtual void a12();
	virtual void a13();
	virtual void a14();
	virtual void a15();
	virtual void a16();
	virtual Vec3 *GetPos(Script::ScriptReference *owner);   // 17: 0x44
};

// 0x42d1f0 Vec3::LineOfSightTo
bool Vec3::LineOfSightTo(Script::ScriptReference *obj)
{
	LosProbe p;
	Vec3 pos;
	Vec3 q;
	Script::ScriptReference *self = SELF;
	if (self->IsKindOf(TFBType_Actor()) == 0) {
		q = *this;
		pos = q;
	} else {
		LosActorTop(SELF, &pos);
	}
	p.m_isectType = 1;
	p.m_lineStart = pos;
	if (obj->IsKindOf(TFBType_Actor()) == 0) {
		p.m_lineEnd = *((LosRef *)obj)->GetPos(obj);
		p.m_targetClump = 0;
	} else {
		LosActorTop(obj, &pos);
		p.m_lineEnd = pos;
		p.m_targetClump = (intptr_t)static_cast<RWS::TFB::Collider &>(*static_cast<RWS::TFB::GameActor *>(obj)).m_clump;
	}
	p.m_dir.x = p.m_lineEnd.x - p.m_lineStart.x;
	p.m_dir.y = p.m_lineEnd.y - p.m_lineStart.y;
	p.m_dir.z = p.m_lineEnd.z - p.m_lineStart.z;
	p.m_clear = 1;
	if (RWS::CLevel::sm_pActiveWorld != 0)
		RpCollisionWorldForAllIntersections(static_cast<RpWorld *>(RWS::CLevel::sm_pActiveWorld), reinterpret_cast<RpIntersection *>(&p.m_lineStart), reinterpret_cast<RpIntersectionCallBackWorldTriangle>(LosProbeTriangleCB), &p);
	RWS::CMsg msg(g_msgCorpusCamera, &p);
	if (msg.Id != (RWS::CRegisteredMsgs *)g_msgInvalid61837c && msg.Id->Erm.p_next)
		RWS::_SendMsg(msg);
	return p.m_clear;
}
