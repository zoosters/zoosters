#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "script/types/actorturret.h"
#include "rw/rwcore.h"

struct RwFrame;
Vec3 *GetZeroVec3();
#define AP_POS m_pos

namespace Script {

// 0x429760
Vec3 *ActorAttachment::GetVector(ScriptReference *owner)
{
	RwFrame *frame = ((ActorAttachment *)owner)->m_parentFrame;
	if (!frame)
		return GetZeroVec3();
	char world[0x40];   // owner's local matrix under the frame's LTM
	RwMatrixMultiply(reinterpret_cast<RwMatrix *>(world), RwFrameGetLTM(frame), reinterpret_cast<const RwMatrix *>(((ActorAttachment *)owner)->m_localMatrix));
	Vec3 *out = &((ActorAttachment *)owner)->AP_POS;
	Vec3 *pos = (Vec3 *)(world + 0x30);
	((ActorAttachment *)owner)->AP_POS.x = pos->x;
	((ActorAttachment *)owner)->AP_POS.y = pos->y;
	((ActorAttachment *)owner)->AP_POS.z = pos->z;
	return out;
}

// 0x429800
bool ActorTurret::SetLocalFacing(float facing)
{
	m_localFacing = facing;
	return m_localDirty = true;
}

// 0x429820
bool ActorTurret::SetLocalTilt(float tilt)
{
	m_localTilt = tilt;
	return m_localDirty = true;
}

// 0x429840
bool ActorTurret::SetLocalRoll(float roll)
{
	m_localRoll = roll;
	return m_localDirty = true;
}

// 0x429910
void ActorTurret::ExposeLookAtPointer::SetReferent(ScriptReference *value, ScriptReference *owner)
{
	// slot 8 = store an object value into the owner (ObjectReference::Execute calls it as SetReferent(object, owner))
	ActorTurret *turret = (ActorTurret *)owner;
	if (value) {
		turret->m_lookAt.m_pendingName = 0;
		ScriptReference *cur = turret->m_lookAt.m_actor;
		if (value != cur) {
			if (cur)
				cur->AdjustRefCount(-1);
			turret->m_lookAt.m_actor = value;
			value->AdjustRefCount(1);
		}
	}
}

}  // namespace Script
