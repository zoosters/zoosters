// Functions the game writes on stand-in classes with raw 32-bit offsets (gameactor_refcount.cpp, modules/tfb/gameactor_localpoint.cpp),
// written on the real classes. The stand-ins stay compiled (the vtable tables name their 32-bit symbols) but are not reached.
#include "rw/rwcore.h"
#include "script/actor.h"
#include "modules/tfb/gameactor.h"
#include "script/types/actorturret.h"

namespace RWS { namespace TFB {
// GLOBAL: 0x42c8c0 RWS::TFB::GameActor::AdjustRefCount
int GameActor::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		m_proto = 0;
		ReleaseBody();   // 0x42bca0
		delete this;
		return 0;
	}
	return m_refCount;
}
} }

namespace Script {
// 0x42cf90 (GameActor::SetLocalOffset in the game's naming): the turret's slot 31
void ActorTurret::SetLocalOffset(const Vec3 *v, ScriptReference *owner)
{
	m_localOffset = *v;
	m_localDirty = true;
}

// 0x42cf00 ActorLocalPoint::SetLocalPointFromWorld: the turret's slot 18
void ActorTurret::SetVector(const Vec3 *point, ScriptReference *owner)
{
	RwMatrix *local = reinterpret_cast<RwMatrix *>(m_localMatrix);
	local->pos.z = 0;
	local->pos.y = 0;
	local->pos.x = 0;
	RwMatrix world;
	RwMatrix inv;
	RwV3d pt;
	pt.x = point->x;
	pt.y = point->y;
	pt.z = point->z;
	RwMatrixMultiply(&world, RwFrameGetLTM(m_parentFrame), local);
	RwMatrixInvert(&inv, &world);
	RwV3dTransformPoint(reinterpret_cast<RwV3d *>(&m_localOffset), &pt, &inv);
	m_localDirty = true;
}
}  // namespace Script
