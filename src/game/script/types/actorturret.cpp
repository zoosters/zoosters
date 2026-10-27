// Script::ActorTurret (the "turret" script type, vtable 0x5cd480): destructor, name, look-at offsets, heading and FOV members.
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "script/types/actorturret.h"

ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Turret();

namespace Script {

// GLOBAL: 0x42bf20 Script::ActorAttachment::~ActorAttachment
// GLOBAL: 0x5cd480 Script::ActorTurret::vftable
ActorTurret::ActorTurret() {}

// 0x42ce50
ActorTurret::~ActorTurret()
{
	m_lookAt.m_pendingName = 0;
	if (m_lookAt.m_actor) {
		m_lookAt.m_actor->AdjustRefCount(-1);
		m_lookAt.m_actor = 0;
	}
}

// 0x42cec0
const char *ActorTurret::GetName()
{
	return "turret";
}

// 0x42cee0
intptr_t ActorTurret::vf26(intptr_t v)
{
	return m_2c == v ? 0 : m_2c;
}

// 0x429960
void ActorTurret::ExposeLookAtHorzOffs::SetValue(float value, ScriptReference *owner)
{
	((ActorTurret *)owner)->m_lookAt.m_horzOffs = value;
}

// 0x429980
float ActorTurret::ExposeLookAtVertOffs::GetFloat(ScriptReference *owner)
{
	return ((ActorTurret *)owner)->m_lookAt.m_vertOffs;
}

// 0x429990
void ActorTurret::ExposeLookAtVertOffs::SetValue(float value, ScriptReference *owner)
{
	((ActorTurret *)owner)->m_lookAt.m_vertOffs = value;
}

// 0x4299b0
float ActorTurret::ExposeLookAtHeadingOffs::GetFloat(ScriptReference *owner)
{
	return ((ActorTurret *)owner)->m_lookAt.m_headingOffs;
}

// 0x4299c0
void ActorTurret::ExposeLookAtHeadingOffs::SetValue(float value, ScriptReference *owner)
{
	((ActorTurret *)owner)->m_lookAt.m_headingOffs = value;
}

// 0x429a10
float ActorTurret::ExposeFOV::GetFloat(ScriptReference *owner)
{
	return ((ActorTurret *)owner)->m_fov;
}

// 0x429a20
void ActorTurret::ExposeFOV::SetValue(float value, ScriptReference *owner)
{
	((ActorTurret *)owner)->m_fov = value;
}

// 0x4299e0
int ActorTurret::ExposeLookAtHeadingRel::GetInt(ScriptReference *owner)
{
	return ((ActorTurret *)owner)->m_lookAt.m_headingRel;
}

// 0x4299f0
void ActorTurret::ExposeLookAtHeadingRel::SetValue(int value, ScriptReference *owner)
{
	((ActorTurret *)owner)->m_lookAt.m_headingRel = value != 0;
}

}  // namespace Script

// 0x42ced0
Script::ScriptReference *ActorTurret_GetType()
{
	return TFBType_Turret();
}

// Shared with other classes' virtuals in the game (PositionMeasurement / ActorTurret_GetType 0x42ced0, FUN 0x43e6f0); each override gets its own body here.

Script::ScriptReference *Script::ActorTurret::GetType() { return TFBType_Turret(); }
