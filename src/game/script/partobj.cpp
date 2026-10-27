// PartObjCb: an actor part (attachment point / turret / corpus part).
#include "script/actor.h"
#include "framework/toolkits/animation/animhelper.h"
#include "script/runtime.h"
#include "script/referenceset.h"
#include "script/types/partentry.h"
#include <stddef.h>
#include <string.h>
#include "script/types/actorturret.h"
#include "modules/fx/attachpoint.h"
#include "modules/tfb/gameactor.h"
using namespace Script;

// 0x42e0c0 constructor of the part, which the game builds as an ActorTurret (vtable 0x5cd480, embedded AttacherSet vtable 0x5cd328): the base
// classes and the set are constructed by the compiler (m_04 = -1, the set empty and sorted), the rest as the game stores it.
static inline float PartBitsToFloat(int bits)
{
	float f;
	memcpy(&f, &bits, sizeof f);
	return f;
}

ActorTurret::ActorTurret(RWS::TFB::GameActor *owner, PartEntry *entry)
{
	m_index = -1;
	m_2c = (intptr_t)owner;
	m_localDirty = true;
	m_fov = 90.0f;
	m_aimRate = 0.85f;
	m_entry = entry;
	m_prevPos.x = m_prevPos.y = m_prevPos.z = 0.0f;
	m_curPos.x = m_curPos.y = m_curPos.z = 0.0f;
	m_pitch = 0.0f;
	m_yaw = 0.0f;
	m_114 = 0;
	m_effectSurface = 0;
	m_effectEnd = 0;
	m_localFacing = PartBitsToFloat(entry->m_facing);
	m_localTilt = PartBitsToFloat(entry->m_tilt);
	m_localRoll = PartBitsToFloat(entry->m_roll);
	m_localOffset = entry->m_localOffset;
	m_fov = PartBitsToFloat(entry->m_fov);
	m_aimRate = 1.0f - (float)entry->m_14 * 0.01f;
	static_assert(offsetof(ActorTurret, m_localMatrix) == offsetof(AttachPoint, m_local) && offsetof(ActorTurret, m_70) == offsetof(AttachPoint, m_localInv) &&
	              offsetof(ActorTurret, m_localRoll) == offsetof(AttachPoint, m_rotY) && offsetof(ActorTurret, m_localOffset) == offsetof(AttachPoint, m_position) &&
	              offsetof(ActorTurret, m_parentFrame) == offsetof(AttachPoint, m_frame) && offsetof(ActorTurret, m_localDirty) == offsetof(AttachPoint, m_dirty),
	              "AttachPoint is the view of ActorAttachment's local transform");
	reinterpret_cast<AttachPoint *>(this)->RebuildLocalMatrix();
	RpHAnimHierarchy *hier = 0;
	RwFrame *frame = owner->m_clump ? RpClumpGetFrame(owner->m_clump) : 0;
	if (frame)
		RWS::FindHAnimHierarchyCallback(frame, &hier);
	if (hier && entry->m_boneIndex < (unsigned)hier->numNodes)
		m_parentFrame = hier->pNodeInfo[entry->m_boneIndex].pFrame;
	else
		m_parentFrame = 0;
}
