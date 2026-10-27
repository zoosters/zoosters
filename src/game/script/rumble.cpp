// Script::ScriptController::ScriptRumble: constructor, creation of a rumble by the object that spawns it, and the small virtual overrides.
// Layout: a Placement (0x4c-byte base) plus the reference count, the source and the position.
#include "tfb/corpus/corpus.h"
#include "script/types/scriptcontroller.h"
#include "script/referenceset.h"
#include <float.h>

int *CutScene_StackSlot(int delta);              // 0x436c00
Script::ReferenceSet *RumbleSet_Get();           // 0x4034d0 (w2a_particle.cpp)

namespace Script {

// 0x403bf0
ScriptController::ScriptRumble::ScriptRumble(ScriptReference *source, const Vec3 *pos)
    : m_refCount(0), m_source(source), m_position(*pos)
{
	m_slotLevel = CutScene_StackSlot(0)[1];
	m_actuator1Level = 0;
	m_actuator2Level = 0;
	m_duration = 0;
	m_timeRemaining = 0;
	m_radius = 0;
	m_attenuation = 0;
}

// 0x403d80
// Slot 30 of ScriptRumble and RumbleProto is Placement::Spawn(int, int), so the body takes the position as int.
intptr_t ScriptController::ScriptRumble::SpawnFrom(ScriptReference *source, intptr_t pos)
{
	ScriptRumble *r = new ScriptRumble(source, (const Vec3 *)pos);
	if (r) {
		RumbleSet_Get()->Insert(r, FLT_MAX);
		return (intptr_t)r;
	}
	return 0;
}
intptr_t ScriptController::ScriptRumble::Spawn(intptr_t pos, int unused) { return SpawnFrom(this, pos); }
// RumbleProto slot 30 is the same game function 0x403d80 (vtable 0x5ca878 slot 30 = vtable 0x5cab68 slot 30).
#if defined(_MSC_VER)
#endif

// 0x403cd0
void ScriptController::ScriptRumble::Reset()
{
	Remove();
}

// 0x403ce0
ScriptReference *ScriptController::ScriptRumble::GetSpawnSource()
{
	return m_source;
}

// 0x403cf0
Vec3 *ScriptController::ScriptRumble::GetVector(ScriptReference *owner)
{
	return &m_position;
}

}  // namespace Script
