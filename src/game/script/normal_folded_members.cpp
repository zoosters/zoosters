// Virtual slots and destructors the game's linker folded into one shared function (ICF): each is a real member calling the shared body
// (or is its constant body). Annotation `// FOLDED: 0xADDR Member` = the address of the shared function; vtcheck reads it.
#include "script/actor.h"
#include "script/opcode.h"
#include "script/ops_actor.h"
#include "script/ops_media.h"
#include "script/ops_object.h"
#include "script/ops_set.h"
#include "script/script.h"
#include "script/types/actorattachment.h"
#include "script/types/actormessage.h"
#include "script/types/actorturret.h"
#include "script/types/actorwaypoint.h"
#include "script/types/anglemeasurement.h"
#include "script/types/animmap.h"
#include "script/types/colormeasurement.h"
#include "script/types/scriptactor.h"
#include "script/types/scriptcamera.h"
#include "script/types/scriptcontroller.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptlight.h"
#include "script/types/scriptmedia.h"
#include "script/types/scriptmemcard.h"
#include "script/types/scriptmessage.h"
#include "script/types/scriptmotionblur.h"
#include "script/types/scriptparticle.h"
#include "script/types/scriptprop.h"
#include "script/types/scriptsets.h"
#include "script/types/scriptshadow.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptsprite.h"
#include "script/types/scriptsurfaceanim.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptvertanim.h"
#include "script/types/twodmeasurement.h"
#include "script/value.h"
#include "modules/fx/gamelight.h"
#include "modules/tfb/memcard.h"
#include "modules/tfb/sprites/gamesprite.h"

#include "script/membertable.h"
#include "tfb/hud/hud2d.h"
#include "script/referenceset.h"
#include "script/types/scriptmathops.h"
#include "script/folded.h"
#include "modules/tfb/levelhub.h"
#include "script/bearings.h"

// ---- script/actor.h
// FOLDED: 0x4025a0 Script::Placement::SetDestination
void Script::Placement::SetDestination(Vec3 * /*a1*/, int /*a2*/) { }
// FOLDED: 0x4025a0 Script::Placement::SetDestinationTimed
void Script::Placement::SetDestinationTimed(Vec3 * /*a1*/, float /*a2*/) { }
// FOLDED: 0x4039e0 Script::Placement::GetType
Script::ScriptReference * Script::Placement::GetType() { return Placement_GetType(); }
// FOLDED: 0x4039f0 Script::Placement::GetBaseType
Script::ScriptReference * Script::Placement::GetBaseType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x403a00 Script::Placement::Control
char Script::Placement::Control(bool a1) { return Placement_Control(a1); }
// FOLDED: 0x403a10 Script::Placement::AccessBehavior
int Script::Placement::AccessBehavior(int a1, int a2) { return Placement_Spawn(a1, a2); }
// FOLDED: 0x403a10 Script::Placement::Spawn
intptr_t Script::Placement::Spawn(intptr_t a1, int a2) { return Placement_Spawn(a1, a2); }
// FOLDED: 0x403a20 Script::Placement::PlayAnimation
bool Script::Placement::PlayAnimation(int a1) { return Placement_PlayAnimation(a1); }
// FOLDED: 0x43e6f0 Script::Placement::GetSpawnSource
Script::ScriptReference * Script::Placement::GetSpawnSource() { return 0; }
// FOLDED: 0x445e10 Script::Placement::IsMeshCollidee
int Script::Placement::IsMeshCollidee() { return Fold_ReturnFalse(); }
// FOLDED: 0x445e10 Script::Placement::IsMeshCollider
int Script::Placement::IsMeshCollider() { return Fold_ReturnFalse(); }
// FOLDED: 0x445e10 Script::Placement::IsSolidCollider
int Script::Placement::IsSolidCollider() { return Fold_ReturnFalse(); }
// FOLDED: 0x464ef0 Script::Placement::Remove
void Script::Placement::Remove() { }
// FOLDED: 0x464ef0 Script::Placement::Reset
void Script::Placement::Reset() { }
// FOLDED: 0x403990 Script::PositionMeasurement::GetName
char const * Script::PositionMeasurement::GetName() { return PositionMeasurement_GetName(); }
// FOLDED: 0x4039a0 Script::PositionMeasurement::GetBaseType
Script::ScriptReference * Script::PositionMeasurement::GetBaseType() { return VectorMeasurement_GetType(); }
// FOLDED: 0x4039c0 Script::PositionMeasurement::SetFacing
bool Script::PositionMeasurement::SetFacing(float a1, int a2) { return PositionMeasurement_SetFacing(a1, a2); }
// FOLDED: 0x4039c0 Script::PositionMeasurement::SetRoll
bool Script::PositionMeasurement::SetRoll(float /*a1*/, Script::ScriptReference * /*a2*/) { return false; }
// FOLDED: 0x4039c0 Script::PositionMeasurement::SetTilt
bool Script::PositionMeasurement::SetTilt(float /*a1*/, Script::ScriptReference * /*a2*/) { return false; }
// FOLDED: 0x4039f0 Script::PositionMeasurement::GetType
Script::ScriptReference * Script::PositionMeasurement::GetType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x416530 Script::PositionMeasurement::vf26
intptr_t Script::PositionMeasurement::vf26(intptr_t /*a1*/) { return 0; }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeDestination::ExposeVertComponent::SetValue
void Script::ScriptActor::ExposeDestination::ExposeVertComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetValueType
int Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403ec0 Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetName
char const * Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetName() { return ExposeVertComponent_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetKind
int Script::ScriptActor::ExposeDestination::ExposeVertComponent::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::VectorMeasurement::SetValue
void Script::VectorMeasurement::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x4025a0 Script::VectorMeasurement::SetVector
void Script::VectorMeasurement::SetVector(Vec3 const * /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x4034a0 Script::VectorMeasurement::GetValueType
int Script::VectorMeasurement::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4034b0 Script::VectorMeasurement::GetName
char const * Script::VectorMeasurement::GetName() { return VectorMeasurement_GetName(); }
// FOLDED: 0x4039a0 Script::VectorMeasurement::GetType
Script::ScriptReference * Script::VectorMeasurement::GetType() { return VectorMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::VectorMeasurement::GetBaseType
Script::ScriptReference * Script::VectorMeasurement::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::VectorMeasurement::GetKind
int Script::VectorMeasurement::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::VectorMeasurement::ExposeHeading::SetValue
void Script::VectorMeasurement::ExposeHeading::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x4034a0 Script::VectorMeasurement::ExposeHeading::GetValueType
int Script::VectorMeasurement::ExposeHeading::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::VectorMeasurement::ExposeHeading::GetType
Script::ScriptReference * Script::VectorMeasurement::ExposeHeading::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x403ea0 Script::VectorMeasurement::ExposeHeading::GetName
char const * Script::VectorMeasurement::ExposeHeading::GetName() { return ExposeHeading_GetName(); }
// FOLDED: 0x436a50 Script::VectorMeasurement::ExposeHeading::GetBaseType
Script::ScriptReference * Script::VectorMeasurement::ExposeHeading::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::VectorMeasurement::ExposeHeading::GetKind
int Script::VectorMeasurement::ExposeHeading::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::VectorMeasurement::ExposeHeading::GetMax
int Script::VectorMeasurement::ExposeHeading::GetMax() { return 0; }
// FOLDED: 0x4025a0 Script::VectorMeasurement::ExposeHorzComponent::SetValue
void Script::VectorMeasurement::ExposeHorzComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::VectorMeasurement::ExposeHorzComponent::GetBaseType
Script::ScriptReference * Script::VectorMeasurement::ExposeHorzComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::VectorMeasurement::ExposeHorzComponent::GetValueType
int Script::VectorMeasurement::ExposeHorzComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403eb0 Script::VectorMeasurement::ExposeHorzComponent::GetName
char const * Script::VectorMeasurement::ExposeHorzComponent::GetName() { return ExposeHorzComponent_GetName(); }
// FOLDED: 0x436a50 Script::VectorMeasurement::ExposeHorzComponent::GetType
Script::ScriptReference * Script::VectorMeasurement::ExposeHorzComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::VectorMeasurement::ExposeHorzComponent::GetKind
int Script::VectorMeasurement::ExposeHorzComponent::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::VectorMeasurement::ExposePitch::SetValue
void Script::VectorMeasurement::ExposePitch::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x4034a0 Script::VectorMeasurement::ExposePitch::GetValueType
int Script::VectorMeasurement::ExposePitch::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::VectorMeasurement::ExposePitch::GetType
Script::ScriptReference * Script::VectorMeasurement::ExposePitch::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::VectorMeasurement::ExposePitch::GetBaseType
Script::ScriptReference * Script::VectorMeasurement::ExposePitch::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43dbf0 Script::VectorMeasurement::ExposePitch::GetName
char const * Script::VectorMeasurement::ExposePitch::GetName() { return ExposePitch_GetName(); }
// FOLDED: 0x43e6f0 Script::VectorMeasurement::ExposePitch::GetKind
int Script::VectorMeasurement::ExposePitch::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::VectorMeasurement::ExposePitch::GetMax
int Script::VectorMeasurement::ExposePitch::GetMax() { return 0; }
// FOLDED: 0x4025a0 Script::VectorMeasurement::ExposeVertComponent::SetValue
void Script::VectorMeasurement::ExposeVertComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::VectorMeasurement::ExposeVertComponent::GetBaseType
Script::ScriptReference * Script::VectorMeasurement::ExposeVertComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::VectorMeasurement::ExposeVertComponent::GetValueType
int Script::VectorMeasurement::ExposeVertComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403ec0 Script::VectorMeasurement::ExposeVertComponent::GetName
char const * Script::VectorMeasurement::ExposeVertComponent::GetName() { return ExposeVertComponent_GetName(); }
// FOLDED: 0x436a50 Script::VectorMeasurement::ExposeVertComponent::GetType
Script::ScriptReference * Script::VectorMeasurement::ExposeVertComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::VectorMeasurement::ExposeVertComponent::GetKind
int Script::VectorMeasurement::ExposeVertComponent::GetKind() { return 0; }

// ---- script/opcode.h
// FOLDED: 0x43efe0 Script::OpBlock::IsBlock
int Script::OpBlock::IsBlock() { return Script_AlwaysTrue(); }
// FOLDED: 0x43e6f0 Script::OpCode::Clone
Script::OpCode * Script::OpCode::Clone() { return 0; }
// FOLDED: 0x43e6f0 Script::OpCode::IsBlock
int Script::OpCode::IsBlock() { return 0; }
// FOLDED: 0x43e6f0 Script::OpCode::Parse
int Script::OpCode::Parse() { return 0; }
// FOLDED: 0x43efe0 Script::OpCode::GetKind
int Script::OpCode::GetKind() { return Script_AlwaysTrue(); }

// ---- script/ops_actor.h

// ---- script/ops_media.h

// ---- script/ops_object.h
// FOLDED: 0x45d080 Script::OpIfElse::Parse
int Script::OpIfElse::Parse() { return 8; }

// ---- script/ops_set.h

// ---- script/script.h
// FOLDED: 0x4025a0 Script::ScriptReference::SetReferent
void Script::ScriptReference::SetReferent(Script::ScriptReference * /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x416530 Script::ScriptReference::AdjustRefCount
int Script::ScriptReference::AdjustRefCount(int /*a1*/) { return 0; }
// FOLDED: 0x42cbe0 Script::ScriptReference::GetKind
int Script::ScriptReference::GetKind() { return 2; }
// FOLDED: 0x43e6f0 Script::ScriptReference::GetBaseType
Script::ScriptReference * Script::ScriptReference::GetBaseType() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptReference::GetType
Script::ScriptReference * Script::ScriptReference::GetType() { return 0; }
// FOLDED: 0x464ef0 Script::ScriptReference::vf10
void Script::ScriptReference::vf10() { }

// ---- script/types/actorattachment.h
// FOLDED: 0x4025a0 Script::ActorAttachment::SetLocalOffset
void Script::ActorAttachment::SetLocalOffset(Vec3 const * /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x403a20 Script::ActorAttachment::SetLocalFacing
bool Script::ActorAttachment::SetLocalFacing(float a1) { return Placement_PlayAnimation(a1); }
// FOLDED: 0x403a20 Script::ActorAttachment::SetLocalRoll
bool Script::ActorAttachment::SetLocalRoll(float a1) { return Placement_PlayAnimation(a1); }
// FOLDED: 0x403a20 Script::ActorAttachment::SetLocalTilt
bool Script::ActorAttachment::SetLocalTilt(float a1) { return Placement_PlayAnimation(a1); }
// FOLDED: 0x4034a0 Script::ActorAttachment::ExposeLocalRoll::GetValueType
int Script::ActorAttachment::ExposeLocalRoll::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ActorAttachment::ExposeLocalRoll::GetType
Script::ScriptReference * Script::ActorAttachment::ExposeLocalRoll::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ActorAttachment::ExposeLocalRoll::GetBaseType
Script::ScriptReference * Script::ActorAttachment::ExposeLocalRoll::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ActorAttachment::ExposeLocalRoll::GetMax
int Script::ActorAttachment::ExposeLocalRoll::GetMax() { return 0; }

// ---- script/types/actormessage.h
// FOLDED: 0x432c90 Script::ActorMessage::GetName
char const * Script::ActorMessage::GetName() { return ScriptMessage_GetName(); }

// ---- script/types/actorturret.h
// FOLDED: 0x4039f0 Script::ActorTurret::GetBaseType
Script::ScriptReference * Script::ActorTurret::GetBaseType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ActorTurret::GetKind
int Script::ActorTurret::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ActorTurret::ExposeFOV::GetValueType
int Script::ActorTurret::ExposeFOV::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ActorTurret::ExposeFOV::GetType
Script::ScriptReference * Script::ActorTurret::ExposeFOV::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ActorTurret::ExposeFOV::GetBaseType
Script::ScriptReference * Script::ActorTurret::ExposeFOV::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x4390e0 Script::ActorTurret::ExposeFOV::GetName
char const * Script::ActorTurret::ExposeFOV::GetName() { return ExposeFOV_GetName(); }
// FOLDED: 0x4390f0 Script::ActorTurret::ExposeFOV::GetMax
int Script::ActorTurret::ExposeFOV::GetMax() { return ExposeFOV_GetMax(); }
// FOLDED: 0x4034a0 Script::ActorTurret::ExposeLookAtHeadingOffs::GetValueType
int Script::ActorTurret::ExposeLookAtHeadingOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ActorTurret::ExposeLookAtHeadingOffs::GetType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtHeadingOffs::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ActorTurret::ExposeLookAtHeadingOffs::GetBaseType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtHeadingOffs::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x4390b0 Script::ActorTurret::ExposeLookAtHeadingOffs::GetName
char const * Script::ActorTurret::ExposeLookAtHeadingOffs::GetName() { return ExposeLookAtHeadingOffs_GetName(); }
// FOLDED: 0x43e6f0 Script::ActorTurret::ExposeLookAtHeadingOffs::GetMax
int Script::ActorTurret::ExposeLookAtHeadingOffs::GetMax() { return 0; }
// FOLDED: 0x402b70 Script::ActorTurret::ExposeLookAtHeadingRel::GetBaseType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtHeadingRel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x407ab0 Script::ActorTurret::ExposeLookAtHeadingRel::SetValue
void Script::ActorTurret::ExposeLookAtHeadingRel::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x436a50 Script::ActorTurret::ExposeLookAtHeadingRel::GetType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtHeadingRel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x438ca0 Script::ActorTurret::ExposeLookAtHeadingRel::GetName
char const * Script::ActorTurret::ExposeLookAtHeadingRel::GetName() { return ExposeLookAtHeadingRel_GetName(); }
// FOLDED: 0x43efe0 Script::ActorTurret::ExposeLookAtHeadingRel::GetMax
int Script::ActorTurret::ExposeLookAtHeadingRel::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ActorTurret::ExposeLookAtHorzOffs::GetBaseType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtHorzOffs::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ActorTurret::ExposeLookAtHorzOffs::GetValueType
int Script::ActorTurret::ExposeLookAtHorzOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x40ace0 Script::ActorTurret::ExposeLookAtHorzOffs::GetFloat
float Script::ActorTurret::ExposeLookAtHorzOffs::GetFloat(Script::ScriptReference * a1) { return ((Script::ScriptParticle::ExposePrimaryESizeSpread *)this)->Script::ScriptParticle::ExposePrimaryESizeSpread::GetFloat(a1); }   // cross-class fold: same owner field offset in the game
// FOLDED: 0x436a50 Script::ActorTurret::ExposeLookAtHorzOffs::GetType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtHorzOffs::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x438c40 Script::ActorTurret::ExposeLookAtHorzOffs::GetName
char const * Script::ActorTurret::ExposeLookAtHorzOffs::GetName() { return ExposeLookAtHorzOffs_GetName(); }
// FOLDED: 0x4039f0 Script::ActorTurret::ExposeLookAtPointer::GetBaseType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtPointer::GetBaseType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x42def0 Script::ActorTurret::ExposeLookAtPointer::GetKind
int Script::ActorTurret::ExposeLookAtPointer::GetKind() { return GetBaseType()->GetKind(); }   // ProtoSlot6::GetKind (script/actorprotos.cpp)
// FOLDED: 0x438950 Script::ActorTurret::ExposeLookAtPointer::GetType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtPointer::GetType() { return TFBType_User(); }   // UserReference::GetType
// FOLDED: 0x438960 Script::ActorTurret::ExposeLookAtPointer::GetName
char const * Script::ActorTurret::ExposeLookAtPointer::GetName() { return ExposeLookAtPointer_GetName(); }
// FOLDED: 0x402b70 Script::ActorTurret::ExposeLookAtVertOffs::GetBaseType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtVertOffs::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ActorTurret::ExposeLookAtVertOffs::GetValueType
int Script::ActorTurret::ExposeLookAtVertOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ActorTurret::ExposeLookAtVertOffs::GetType
Script::ScriptReference * Script::ActorTurret::ExposeLookAtVertOffs::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x438c70 Script::ActorTurret::ExposeLookAtVertOffs::GetName
char const * Script::ActorTurret::ExposeLookAtVertOffs::GetName() { return ExposeLookAtVertOffs_GetName(); }
// FOLDED: 0x43e6f0 Script::ActorTurret::ExposeLookAtVertOffs::GetMax
int Script::ActorTurret::ExposeLookAtVertOffs::GetMax() { return 0; }

// ---- script/types/actorwaypoint.h
// FOLDED: 0x4039f0 Script::ActorWayPoint::GetBaseType
Script::ScriptReference * Script::ActorWayPoint::GetBaseType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x42d4a0 Script::ActorWayPoint::GetType
Script::ScriptReference * Script::ActorWayPoint::GetType() { return ActorWayPoint_GetType(); }
// FOLDED: 0x43e6f0 Script::ActorWayPoint::GetKind
int Script::ActorWayPoint::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ActorWayPoint::ExposeIndex::SetValue
void Script::ActorWayPoint::ExposeIndex::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ActorWayPoint::ExposeIndex::GetBaseType
Script::ScriptReference * Script::ActorWayPoint::ExposeIndex::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ActorWayPoint::ExposeIndex::GetType
Script::ScriptReference * Script::ActorWayPoint::ExposeIndex::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ActorWayPoint::ExposeIndex::GetKind
int Script::ActorWayPoint::ExposeIndex::GetKind() { return 0; }

// ---- script/types/anglemeasurement.h
// FOLDED: 0x4025a0 Script::AngleMeasurement::SetValue
void Script::AngleMeasurement::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x403460 Script::AngleMeasurement::GetInt
int Script::AngleMeasurement::GetInt(Script::ScriptReference * a1) { return Script::Measurement::GetInt(a1); }
// FOLDED: 0x403480 Script::AngleMeasurement::SetValue
void Script::AngleMeasurement::SetValue(int a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x4034a0 Script::AngleMeasurement::GetValueType
int Script::AngleMeasurement::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::AngleMeasurement::GetType
Script::ScriptReference * Script::AngleMeasurement::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x41aa20 Script::AngleMeasurement::GetFloat
float Script::AngleMeasurement::GetFloat(Script::ScriptReference * a1) { return g_floatZero; }   // VectorMeasurement::GetFloat
// FOLDED: 0x436a50 Script::AngleMeasurement::GetBaseType
Script::ScriptReference * Script::AngleMeasurement::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::AngleMeasurement::GetMax
int Script::AngleMeasurement::GetMax() { return 0; }

// ---- script/types/colormeasurement.h
// FOLDED: 0x4025a0 Script::ColorMeasurement::SetValue
void Script::ColorMeasurement::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ColorMeasurement::GetBaseType
Script::ScriptReference * Script::ColorMeasurement::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::ColorMeasurement::GetFloat
float Script::ColorMeasurement::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::ColorMeasurement::SetValue
void Script::ColorMeasurement::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x407aa0 Script::ColorMeasurement::GetType
Script::ScriptReference * Script::ColorMeasurement::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x407ad0 Script::ColorMeasurement::GetValueType
int Script::ColorMeasurement::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x416530 Script::ColorMeasurement::GetInt
int Script::ColorMeasurement::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x43e6f0 Script::ColorMeasurement::GetMax
int Script::ColorMeasurement::GetMax() { return 0; }

// ---- script/types/scriptactor.h
// FOLDED: 0x4039e0 Script::ScriptActor::GetBaseType
Script::ScriptReference * Script::ScriptActor::GetBaseType() { return Placement_GetType(); }

// ---- script/types/scriptcamera.h
// FOLDED: 0x4025a0 Script::ScriptCamera::ExposeCameraLocation::SetValue
void Script::ScriptCamera::ExposeCameraLocation::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeCameraLocation::GetValueType
int Script::ScriptCamera::ExposeCameraLocation::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4039a0 Script::ScriptCamera::ExposeCameraLocation::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeCameraLocation::GetBaseType() { return VectorMeasurement_GetType(); }
// FOLDED: 0x4039f0 Script::ScriptCamera::ExposeCameraLocation::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeCameraLocation::GetType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x42f770 Script::ScriptCamera::ExposeCameraLocation::GetFloat
float Script::ScriptCamera::ExposeCameraLocation::GetFloat(Script::ScriptReference * a1) { return Script::PositionMeasurement::GetFloat(a1); }
// FOLDED: 0x430ca0 Script::ScriptCamera::ExposeCameraLocation::GetMembers
void * Script::ScriptCamera::ExposeCameraLocation::GetMembers(int * a1) { return Script::PositionMeasurement::GetMembers(a1); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposeCameraLocation::GetKind
int Script::ScriptCamera::ExposeCameraLocation::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptController::ScriptRumble::ExposeTimeRemaining::SetValue
void Script::ScriptController::ScriptRumble::ExposeTimeRemaining::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetValueType
int Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x438d40 Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetName
char const * Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetName() { return ExposeTimeRemaining_GetName(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetKind
int Script::ScriptController::ScriptRumble::ExposeTimeRemaining::GetKind() { return 0; }

// ---- script/types/scriptcontroller.h
// FOLDED: 0x439450 Script::ScriptController::GetType
Script::ScriptReference * Script::ScriptController::GetType() { return ScriptController_GetType(); }
// FOLDED: 0x439450 Script::ControllerPlayerRef::GetType
Script::ScriptReference * Script::ControllerPlayerRef::GetType() { return ScriptController_GetType(); }
// FOLDED: 0x439810 Script::ControllerPlayerRef::GetMembers
void * Script::ControllerPlayerRef::GetMembers(int * a1) { return Script::ScriptController::Members(a1); }   // cross-class fold; the shared body is a static and does not read this
// FOLDED: 0x4039e0 Script::RumbleProto::GetBaseType
Script::ScriptReference * Script::RumbleProto::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x403a50 Script::RumbleProto::GetName
char const * Script::RumbleProto::GetName() { return ScriptRumble_GetName(); }
// FOLDED: 0x403a60 Script::RumbleProto::GetType
Script::ScriptReference * Script::RumbleProto::GetType() { return ScriptRumble_GetType(); }
// FOLDED: 0x403a70 Script::RumbleProto::GetMembers
void * Script::RumbleProto::GetMembers(int * a1) { return Script::ScriptController::ScriptRumble::Members(a1); }   // cross-class fold; the shared body is a static and does not read this
// FOLDED: 0x402b70 Script::ScriptController::ExposeActiveControllers::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeActiveControllers::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptController::ExposeActiveControllers::GetType
Script::ScriptReference * Script::ScriptController::ExposeActiveControllers::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x4025a0 Script::ScriptController::ScriptRumble::ExposePercent5cab20::SetValue
void Script::ScriptController::ScriptRumble::ExposePercent5cab20::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetFloat
float Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::ScriptController::ScriptRumble::ExposePercent5cab20::SetValue
void Script::ScriptController::ScriptRumble::ExposePercent5cab20::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x416530 Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetInt
int Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x42fbc0 Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetMax
int Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposePercent5cab20::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposeRumbleDisabled::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeRumbleDisabled::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposeRumbleDisabled::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeRumbleDisabled::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptController::ScriptRumble::ExposeRumbleDisabled::GetMax
int Script::ScriptController::ScriptRumble::ExposeRumbleDisabled::GetMax() { return Script_AlwaysTrue(); }

// ---- script/types/scriptfog.h
// FOLDED: 0x402b70 Script::ScriptFog::ExposeBloomAddFactor::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeBloomAddFactor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptFog::ExposeBloomAddFactor::GetValueType
int Script::ScriptFog::ExposeBloomAddFactor::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeBloomAddFactor::GetType
Script::ScriptReference * Script::ScriptFog::ExposeBloomAddFactor::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptFog::ExposeBloomAddFactor::GetMax
int Script::ScriptFog::ExposeBloomAddFactor::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeBloomEnable::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeBloomEnable::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeBloomEnable::GetType
Script::ScriptReference * Script::ScriptFog::ExposeBloomEnable::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptFog::ExposeBloomEnable::GetMax
int Script::ScriptFog::ExposeBloomEnable::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeBloomHaloSize::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeBloomHaloSize::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptFog::ExposeBloomHaloSize::GetValueType
int Script::ScriptFog::ExposeBloomHaloSize::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeBloomHaloSize::GetType
Script::ScriptReference * Script::ScriptFog::ExposeBloomHaloSize::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptFog::ExposeBloomHaloSize::GetMax
int Script::ScriptFog::ExposeBloomHaloSize::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeDOFBluriness::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeDOFBluriness::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptFog::ExposeDOFBluriness::GetMax
int Script::ScriptFog::ExposeDOFBluriness::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeDOFBluriness::GetType
Script::ScriptReference * Script::ScriptFog::ExposeDOFBluriness::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelFogColor::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x407aa0 Script::ScriptFog::ExposePixelFogColor::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x407ad0 Script::ScriptFog::ExposePixelFogColor::GetValueType
int Script::ScriptFog::ExposePixelFogColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x43e6f0 Script::ScriptFog::ExposePixelFogColor::GetMax
int Script::ScriptFog::ExposePixelFogColor::GetMax() { return 0; }

// ---- script/types/scriptlight.h
// FOLDED: 0x4039e0 Script::ScriptLight::GetBaseType
Script::ScriptReference * Script::ScriptLight::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x407a20 Script::ScriptLight::GetName
char const * Script::ScriptLight::GetName() { return ScriptLight_GetName(); }
// FOLDED: 0x407a30 Script::ScriptLight::GetType
Script::ScriptReference * Script::ScriptLight::GetType() { return ScriptLight_GetType(); }

// ---- script/types/scriptmedia.h
// FOLDED: 0x418b70 Script::ScriptMedia::GetType
Script::ScriptReference * Script::ScriptMedia::GetType() { return thunk_FUN_0043da40(); }

// ---- script/types/scriptmemcard.h
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardAutoCreate::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoCreate::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42cbe0 Script::ScriptMemCard::ExposeMemCardAutoCreate::GetMax
int Script::ScriptMemCard::ExposeMemCardAutoCreate::GetMax() { return 2; }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardAutoCreate::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoCreate::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardAutoDelete::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoDelete::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42cbe0 Script::ScriptMemCard::ExposeMemCardAutoDelete::GetMax
int Script::ScriptMemCard::ExposeMemCardAutoDelete::GetMax() { return 2; }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardAutoDelete::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoDelete::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardAutoFormat::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoFormat::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42cbe0 Script::ScriptMemCard::ExposeMemCardAutoFormat::GetMax
int Script::ScriptMemCard::ExposeMemCardAutoFormat::GetMax() { return 2; }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardAutoFormat::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoFormat::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardChangedState::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardChangedState::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardChangedState::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardChangedState::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptMemCard::ExposeMemCardChangedState::GetMax
int Script::ScriptMemCard::ExposeMemCardChangedState::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptMemCard::ExposeMemCardLanguage::SetValue
void Script::ScriptMemCard::ExposeMemCardLanguage::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardLanguage::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardLanguage::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardLanguage::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardLanguage::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x4025a0 Script::ScriptMemCard::ExposeMemCardSlotGameTime::SetValue
void Script::ScriptMemCard::ExposeMemCardSlotGameTime::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardSlotGameTime::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardSlotGameTime::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardSlotGameTime::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardSlotGameTime::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMemCard::ExposeMemCardSlotGameTime::GetKind
int Script::ScriptMemCard::ExposeMemCardSlotGameTime::GetKind() { return 0; }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardSlotPercent::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardSlotPercent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardSlotPercent::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardSlotPercent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardSlotSize::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardSlotSize::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardSlotSize::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardSlotSize::GetType() { return IntMeasurement_GetType(); }

// ---- script/types/scriptmessage.h
// FOLDED: 0x43efe0 Script::ExposeActivateMessage::GetKind
int Script::ExposeActivateMessage::GetKind() { return Script_AlwaysTrue(); }
// FOLDED: 0x43e6f0 Script::ExposeBounceNormalMessage::GetKind
int Script::ExposeBounceNormalMessage::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ExposeDeactivateMessage::GetKind
int Script::ExposeDeactivateMessage::GetKind() { return Script_AlwaysTrue(); }
// FOLDED: 0x43efe0 Script::ExposeDeathMessage::GetKind
int Script::ExposeDeathMessage::GetKind() { return Script_AlwaysTrue(); }
// FOLDED: 0x43e6f0 Script::ExposeFootPrintMessage::GetKind
int Script::ExposeFootPrintMessage::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ExposeHitActorMessage::GetKind
int Script::ExposeHitActorMessage::GetKind() { return Script_AlwaysTrue(); }
// FOLDED: 0x43e6f0 Script::ExposeHitGroundMessage::GetKind
int Script::ExposeHitGroundMessage::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ExposeHitTriggerMessage::GetKind
int Script::ExposeHitTriggerMessage::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ExposeHitWallMessage::GetKind
int Script::ExposeHitWallMessage::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ExposeJumpStartedMessage::GetKind
int Script::ExposeJumpStartedMessage::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ExposeLedgeNearbyMessage::GetKind
int Script::ExposeLedgeNearbyMessage::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ExposeLeftGroundMessage::GetKind
int Script::ExposeLeftGroundMessage::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ExposeWallNormalMessage::GetKind
int Script::ExposeWallNormalMessage::GetKind() { return 0; }
// FOLDED: 0x432c90 Script::ScriptMessage::GetName
char const * Script::ScriptMessage::GetName() { return ScriptMessage_GetName(); }

// ---- script/types/scriptmotionblur.h
// FOLDED: 0x4034a0 Script::ScriptMotionBlur::ExposeField40::GetValueType
int Script::ScriptMotionBlur::ExposeField40::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ScriptMotionBlur::ExposeField40::GetType
Script::ScriptReference * Script::ScriptMotionBlur::ExposeField40::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x416660 Script::ScriptMotionBlur::ExposeField40::GetName
char const * Script::ScriptMotionBlur::ExposeField40::GetName() { return "angle"; }
// FOLDED: 0x436a50 Script::ScriptMotionBlur::ExposeField40::GetBaseType
Script::ScriptReference * Script::ScriptMotionBlur::ExposeField40::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMotionBlur::ExposeField40::GetMax
int Script::ScriptMotionBlur::ExposeField40::GetMax() { return 0; }
// FOLDED: 0x402b70 Script::ScriptMotionBlur::ExposeIsoScale::GetBaseType
Script::ScriptReference * Script::ScriptMotionBlur::ExposeIsoScale::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptMotionBlur::ExposeIsoScale::GetValueType
int Script::ScriptMotionBlur::ExposeIsoScale::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptMotionBlur::ExposeIsoScale::GetType
Script::ScriptReference * Script::ScriptMotionBlur::ExposeIsoScale::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMotionBlur::ExposeIsoScale::GetMax
int Script::ScriptMotionBlur::ExposeIsoScale::GetMax() { return 0; }
// FOLDED: 0x43f5f0 Script::ScriptMotionBlur::ExposeIsoScale::GetName
char const * Script::ScriptMotionBlur::ExposeIsoScale::GetName() { return ExposeScales_GetName(); }
// FOLDED: 0x402b70 Script::ScriptMotionBlur::ExposeScaleY::GetBaseType
Script::ScriptReference * Script::ScriptMotionBlur::ExposeScaleY::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptMotionBlur::ExposeScaleY::GetValueType
int Script::ScriptMotionBlur::ExposeScaleY::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptMotionBlur::ExposeScaleX::GetType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeScaleX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMotionBlur::ExposeScaleX::GetBaseType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeScaleX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMotionBlur::ExposeScaleX::GetMax
int Script::ScriptMotionBlur::ExposeScaleX::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptMotionBlur::ExposeScaleX::GetValueType
int Script::ScriptMotionBlur::ExposeScaleX::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptMotionBlur::ExposeOffsetY::GetType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeOffsetY::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMotionBlur::ExposeOffsetY::GetBaseType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeOffsetY::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMotionBlur::ExposeOffsetY::GetMax
int Script::ScriptMotionBlur::ExposeOffsetY::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptMotionBlur::ExposeOffsetY::GetValueType
int Script::ScriptMotionBlur::ExposeOffsetY::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptMotionBlur::ExposeOffsetX::GetType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeOffsetX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMotionBlur::ExposeOffsetX::GetBaseType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeOffsetX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMotionBlur::ExposeOffsetX::GetMax
int Script::ScriptMotionBlur::ExposeOffsetX::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptMotionBlur::ExposeOffsetX::GetValueType
int Script::ScriptMotionBlur::ExposeOffsetX::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptMotionBlur::ExposeAmount::GetType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeAmount::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMotionBlur::ExposeAmount::GetBaseType
Script::ScriptReference *Script::ScriptMotionBlur::ExposeAmount::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptMotionBlur::ExposeAmount::GetMax
int Script::ScriptMotionBlur::ExposeAmount::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4034a0 Script::ScriptMotionBlur::ExposeAmount::GetValueType
int Script::ScriptMotionBlur::ExposeAmount::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptMotionBlur::ExposeScaleY::GetType
Script::ScriptReference * Script::ScriptMotionBlur::ExposeScaleY::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMotionBlur::ExposeScaleY::GetMax
int Script::ScriptMotionBlur::ExposeScaleY::GetMax() { return 0; }

// ---- script/types/scriptparticle.h
// FOLDED: 0x4039e0 Script::ScriptParticle::GetBaseType
Script::ScriptReference * Script::ScriptParticle::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x40cf90 Script::ScriptParticle::GetName
char const * Script::ScriptParticle::GetName() { return ScriptParticle_GetName(); }
// FOLDED: 0x43b870 Script::ScriptParticle::GetType
Script::ScriptReference * Script::ScriptParticle::GetType() { return ScriptParticle_GetType(); }
// FOLDED: 0x43b880 Script::ScriptParticle::vf10
void Script::ScriptParticle::vf10() { Script::DetachParticleSetClones(); }
// FOLDED: 0x402bb0 Script::ScriptSprite::ExposeClones::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeClones::GetType() { return ReferenceSet_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeClones::GetKind
int Script::ScriptSprite::ExposeClones::GetKind() { return 0; }

// ---- script/types/scriptprop.h
// FOLDED: 0x4039e0 Script::PropProto::GetBaseType
Script::ScriptReference * Script::PropProto::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x4039e0 Script::ScriptProp::GetBaseType
Script::ScriptReference * Script::ScriptProp::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x4025a0 Script::ScriptProp::ExposeLoopCount::SetValue
void Script::ScriptProp::ExposeLoopCount::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeLoopCount::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeLoopCount::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::ScriptProp::ExposeLoopCount::GetFloat
float Script::ScriptProp::ExposeLoopCount::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::ScriptProp::ExposeLoopCount::SetValue
void Script::ScriptProp::ExposeLoopCount::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeLoopCount::GetType
Script::ScriptReference * Script::ScriptProp::ExposeLoopCount::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptProp::ExposeLoopCount::GetKind
int Script::ScriptProp::ExposeLoopCount::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptProp::ExposeLoopStart::SetValue
void Script::ScriptProp::ExposeLoopStart::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeLoopStart::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeLoopStart::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::ScriptProp::ExposeLoopStart::GetFloat
float Script::ScriptProp::ExposeLoopStart::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::ScriptProp::ExposeLoopStart::SetValue
void Script::ScriptProp::ExposeLoopStart::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x416530 Script::ScriptProp::ExposeLoopStart::GetInt
int Script::ScriptProp::ExposeLoopStart::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeLoopStart::GetType
Script::ScriptReference * Script::ScriptProp::ExposeLoopStart::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptProp::ExposeLoopStart::GetKind
int Script::ScriptProp::ExposeLoopStart::GetKind() { return 0; }

// ---- script/types/scriptsets.h
// FOLDED: 0x437bd0 Script::ExposeInactiveSet::GetName
char const * Script::ExposeInactiveSet::GetName() { return ExposeInactiveSet_GetName(); }
// FOLDED: 0x43b870 Script::_ParticleShield::ExposeActiveSet::GetBaseType
Script::ScriptReference * Script::_ParticleShield::ExposeActiveSet::GetBaseType() { return ScriptParticle_GetType(); }
// FOLDED: 0x43e470 Script::_ParticleShield::ExposeActiveSet::GetName
char const * Script::_ParticleShield::ExposeActiveSet::GetName() { return ExposeActiveSet_GetName(); }
// FOLDED: 0x437bd0 Script::_ParticleShield::ExposeInactiveSet::GetName
char const * Script::_ParticleShield::ExposeInactiveSet::GetName() { return ExposeInactiveSet_GetName(); }
// FOLDED: 0x43b870 Script::_ParticleShield::ExposeInactiveSet::GetBaseType
Script::ScriptReference * Script::_ParticleShield::ExposeInactiveSet::GetBaseType() { return ScriptParticle_GetType(); }
// FOLDED: 0x43b870 Script::_ParticleShield::ExposeRemovedSet::GetBaseType
Script::ScriptReference * Script::_ParticleShield::ExposeRemovedSet::GetBaseType() { return ScriptParticle_GetType(); }
// FOLDED: 0x43e510 Script::_ParticleShield::ExposeRemovedSet::GetName
char const * Script::_ParticleShield::ExposeRemovedSet::GetName() { return ExposeRemovedSet_GetName(); }
// FOLDED: 0x416490 Script::_SpriteShield::ExposeActiveSet::GetBaseType
Script::ScriptReference * Script::_SpriteShield::ExposeActiveSet::GetBaseType() { return thunk_FUN_0043e330(); }
// FOLDED: 0x43e470 Script::_SpriteShield::ExposeActiveSet::GetName
char const * Script::_SpriteShield::ExposeActiveSet::GetName() { return ExposeActiveSet_GetName(); }
// FOLDED: 0x416490 Script::_SpriteShield::ExposeRemovedSet::GetBaseType
Script::ScriptReference * Script::_SpriteShield::ExposeRemovedSet::GetBaseType() { return thunk_FUN_0043e330(); }
// FOLDED: 0x43e510 Script::_SpriteShield::ExposeRemovedSet::GetName
char const * Script::_SpriteShield::ExposeRemovedSet::GetName() { return ExposeRemovedSet_GetName(); }

// ---- script/types/scriptshadow.h
// FOLDED: 0x402b70 Script::ScriptShadow::ExposeEnableFading::GetBaseType
Script::ScriptReference * Script::ScriptShadow::ExposeEnableFading::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptShadow::ExposeEnableFading::GetType
Script::ScriptReference * Script::ScriptShadow::ExposeEnableFading::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptShadow::ExposeEnableFading::GetMax
int Script::ScriptShadow::ExposeEnableFading::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ScriptShadow::ExposeFadeRadius::GetBaseType
Script::ScriptReference * Script::ScriptShadow::ExposeFadeRadius::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptShadow::ExposeFadeRadius::GetValueType
int Script::ScriptShadow::ExposeFadeRadius::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptShadow::ExposeFadeRadius::GetType
Script::ScriptReference * Script::ScriptShadow::ExposeFadeRadius::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptShadow::ExposeFadeRadius::GetMax
int Script::ScriptShadow::ExposeFadeRadius::GetMax() { return ExposeParticleSpread_GetMax(); }

// ---- script/types/scriptsound.h
// FOLDED: 0x402b70 Script::ScriptSound::ExposeEffectsVolume::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposeEffectsVolume::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptSound::ExposeEffectsVolume::GetMax
int Script::ScriptSound::ExposeEffectsVolume::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptSound::ExposeEffectsVolume::GetType
Script::ScriptReference * Script::ScriptSound::ExposeEffectsVolume::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSound::ExposeEnvironmentLevel::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposeEnvironmentLevel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptSound::ExposeEnvironmentLevel::GetMax
int Script::ScriptSound::ExposeEnvironmentLevel::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptSound::ExposeEnvironmentLevel::GetType
Script::ScriptReference * Script::ScriptSound::ExposeEnvironmentLevel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSound::ExposeEnvironmentType::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposeEnvironmentType::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSound::ExposeEnvironmentType::GetType
Script::ScriptReference * Script::ScriptSound::ExposeEnvironmentType::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSound::ExposeMusicVolume::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposeMusicVolume::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptSound::ExposeMusicVolume::GetMax
int Script::ScriptSound::ExposeMusicVolume::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptSound::ExposeMusicVolume::GetType
Script::ScriptReference * Script::ScriptSound::ExposeMusicVolume::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x4025a0 Script::ScriptSound::ExposeStreamCount::SetValue
void Script::ScriptSound::ExposeStreamCount::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSound::ExposeStreamCount::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposeStreamCount::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptSound::ExposeStreamCount::GetMax
int Script::ScriptSound::ExposeStreamCount::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptSound::ExposeStreamCount::GetType
Script::ScriptReference * Script::ScriptSound::ExposeStreamCount::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSound::ExposeStreamCount::GetKind
int Script::ScriptSound::ExposeStreamCount::GetKind() { return 0; }

// ---- script/types/scriptsprite.h
// FOLDED: 0x4039e0 Script::ScriptSprite::GetBaseType
Script::ScriptReference * Script::ScriptSprite::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x413b80 Script::ScriptSprite::GetVector
Vec3 * Script::ScriptSprite::GetVector(Script::ScriptReference * a1) { return Script::ScriptSprite::ExposeLocation::LocationVector(a1); }   // cross-class fold; the shared body is a static and reads only the owner
// FOLDED: 0x416490 Script::ScriptSprite::GetType
Script::ScriptReference * Script::ScriptSprite::GetType() { return thunk_FUN_0043e330(); }
// FOLDED: 0x43e5e0 Script::ScriptSprite::vf10
void Script::ScriptSprite::vf10() { Script::DetachSpriteSetClones(); }
// FOLDED: 0x4025a0 Script::ScriptSprite::ExposePixelExtents::SetValue
void Script::ScriptSprite::ExposePixelExtents::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposePixelExtents::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposePixelExtents::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::ScriptSprite::ExposePixelExtents::GetFloat
float Script::ScriptSprite::ExposePixelExtents::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::ScriptSprite::ExposePixelExtents::SetValue
void Script::ScriptSprite::ExposePixelExtents::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposePixelExtents::GetType
Script::ScriptReference * Script::ScriptSprite::ExposePixelExtents::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposePixelExtents::GetKind
int Script::ScriptSprite::ExposePixelExtents::GetKind() { return 0; }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposePriority::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposePriority::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::ScriptSprite::ExposePriority::GetFloat
float Script::ScriptSprite::ExposePriority::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::ScriptSprite::ExposePriority::SetValue
void Script::ScriptSprite::ExposePriority::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposePriority::GetType
Script::ScriptReference * Script::ScriptSprite::ExposePriority::GetType() { return IntMeasurement_GetType(); }

// ---- script/types/scriptsurfaceanim.h
// FOLDED: 0x43bf40 Script::ScriptSurfaceAnim::GetName
char const * Script::ScriptSurfaceAnim::GetName() { return ScriptSurfaceAnim_GetName(); }
// FOLDED: 0x43c210 Script::ScriptSurfaceAnim::GetType
Script::ScriptReference * Script::ScriptSurfaceAnim::GetType() { return ScriptSurfaceAnim_GetType(); }
// FOLDED: 0x43c220 Script::ScriptSurfaceAnim::GetMembers
void * Script::ScriptSurfaceAnim::GetMembers(int * a1) { return Folded_SurfaceAnimW_GetMembers(this, a1); }

// ---- script/types/scriptsystem.h
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeBeenoxQuitGame::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeBeenoxQuitGame::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x416530 Script::ScriptSystem::ExposeBeenoxQuitGame::GetInt
int Script::ScriptSystem::ExposeBeenoxQuitGame::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeBeenoxQuitGame::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeBeenoxQuitGame::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e790 Script::ScriptSystem::ExposeBeenoxQuitGame::GetName
char const * Script::ScriptSystem::ExposeBeenoxQuitGame::GetName() { return ExposeBeenoxQuitGame_GetName(); }
// FOLDED: 0x43efe0 Script::ScriptSystem::ExposeBeenoxQuitGame::GetMax
int Script::ScriptSystem::ExposeBeenoxQuitGame::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptSystem::ExposeConsole::SetValue
void Script::ScriptSystem::ExposeConsole::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeConsole::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeConsole::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeConsole::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeConsole::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSystem::ExposeConsole::GetKind
int Script::ScriptSystem::ExposeConsole::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptSystem::ExposeCurrentLevel::SetValue
void Script::ScriptSystem::ExposeCurrentLevel::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeCurrentLevel::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeCurrentLevel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeCurrentLevel::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeCurrentLevel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSystem::ExposeCurrentLevel::GetKind
int Script::ScriptSystem::ExposeCurrentLevel::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptSystem::ExposeCutSceneActive::SetValue
void Script::ScriptSystem::ExposeCutSceneActive::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeCutSceneActive::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeCutSceneActive::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeCutSceneActive::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeCutSceneActive::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSystem::ExposeCutSceneActive::GetKind
int Script::ScriptSystem::ExposeCutSceneActive::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptSystem::ExposeCutSceneActive::GetMax
int Script::ScriptSystem::ExposeCutSceneActive::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptSystem::ExposeDesignBuild::SetValue
void Script::ScriptSystem::ExposeDesignBuild::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeDesignBuild::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeDesignBuild::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x416530 Script::ScriptSystem::ExposeDesignBuild::GetInt
#ifdef ZOOSTERS_DEBUGUI
extern "C" int port_designbuild;   // debugui_vars.cpp: the overlay's switch (0 = retail)
int Script::ScriptSystem::ExposeDesignBuild::GetInt(Script::ScriptReference *) { return port_designbuild; }
#else
int Script::ScriptSystem::ExposeDesignBuild::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
#endif
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeDesignBuild::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeDesignBuild::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSystem::ExposeDesignBuild::GetKind
int Script::ScriptSystem::ExposeDesignBuild::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptSystem::ExposeDesignBuild::GetMax
int Script::ScriptSystem::ExposeDesignBuild::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeFarClip::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeFarClip::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptSystem::ExposeFarClip::GetValueType
int Script::ScriptSystem::ExposeFarClip::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeFarClip::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeFarClip::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeGameTime::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeGameTime::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptSystem::ExposeGameTime::GetValueType
int Script::ScriptSystem::ExposeGameTime::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeGameTime::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeGameTime::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSystem::ExposeGameTime::GetKind
int Script::ScriptSystem::ExposeGameTime::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptSystem::ExposeHardwareMetrics::SetValue
void Script::ScriptSystem::ExposeHardwareMetrics::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeHardwareMetrics::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeHardwareMetrics::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x416530 Script::ScriptSystem::ExposeHardwareMetrics::GetInt
int Script::ScriptSystem::ExposeHardwareMetrics::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeHardwareMetrics::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeHardwareMetrics::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptSystem::ExposeHardwareMetrics::GetMax
int Script::ScriptSystem::ExposeHardwareMetrics::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeLevelZone::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeLevelZone::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeLevelZone::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeLevelZone::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeNearClip::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeNearClip::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptSystem::ExposeNearClip::GetValueType
int Script::ScriptSystem::ExposeNearClip::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeNearClip::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeNearClip::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeNextLevel::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeNextLevel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeNextLevel::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeNextLevel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x4025a0 Script::ScriptSystem::ExposePlayMode::SetValue
void Script::ScriptSystem::ExposePlayMode::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposePlayMode::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposePlayMode::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposePlayMode::GetType
Script::ScriptReference * Script::ScriptSystem::ExposePlayMode::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSystem::ExposePlayMode::GetKind
int Script::ScriptSystem::ExposePlayMode::GetKind() { return 0; }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeRadarControl::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeRadarControl::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeRadarControl::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeRadarControl::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptSystem::ExposeRadarControl::GetMax
int Script::ScriptSystem::ExposeRadarControl::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeRenderYonIgnore::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeRenderYonIgnore::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeRenderYonIgnore::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeRenderYonIgnore::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptSystem::ExposeRenderYonIgnore::GetMax
int Script::ScriptSystem::ExposeRenderYonIgnore::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptSystem::ExposeScriptTime::SetValue
void Script::ScriptSystem::ExposeScriptTime::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeScriptTime::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeScriptTime::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptSystem::ExposeScriptTime::GetValueType
int Script::ScriptSystem::ExposeScriptTime::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeScriptTime::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeScriptTime::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSystem::ExposeScriptTime::GetKind
int Script::ScriptSystem::ExposeScriptTime::GetKind() { return 0; }
// FOLDED: 0x402b70 Script::ScriptSystem::ExposeVSyncFlipCount::GetBaseType
Script::ScriptReference * Script::ScriptSystem::ExposeVSyncFlipCount::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSystem::ExposeVSyncFlipCount::GetType
Script::ScriptReference * Script::ScriptSystem::ExposeVSyncFlipCount::GetType() { return IntMeasurement_GetType(); }

// ---- script/types/twodmeasurement.h
// FOLDED: 0x4025a0 Script::TwoDMeasurement::SetValue
void Script::TwoDMeasurement::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::TwoDMeasurement::GetBaseType
Script::ScriptReference * Script::TwoDMeasurement::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::TwoDMeasurement::GetFloat
float Script::TwoDMeasurement::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::TwoDMeasurement::SetValue
void Script::TwoDMeasurement::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x416410 Script::TwoDMeasurement::GetValueType
int Script::TwoDMeasurement::GetValueType() { return TwoDMeasurement_GetValueType(); }
// FOLDED: 0x416530 Script::TwoDMeasurement::GetInt
int Script::TwoDMeasurement::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x43e6f0 Script::TwoDMeasurement::GetMax
int Script::TwoDMeasurement::GetMax() { return 0; }

// ---- script/value.h
// FOLDED: 0x4025a0 Script::BoolMeasurement::SetValue
void Script::BoolMeasurement::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::BoolMeasurement::GetBaseType
Script::ScriptReference * Script::BoolMeasurement::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402b80 Script::BoolMeasurement::GetFloat
float Script::BoolMeasurement::GetFloat(Script::ScriptReference * a1) { return Script::Measurement::GetFloat(a1); }
// FOLDED: 0x407ab0 Script::BoolMeasurement::SetValue
void Script::BoolMeasurement::SetValue(float a1, Script::ScriptReference * a2) { Script::Measurement::SetValue(a1, a2); }
// FOLDED: 0x416530 Script::BoolMeasurement::GetInt
int Script::BoolMeasurement::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x436a50 Script::BoolMeasurement::GetType
Script::ScriptReference * Script::BoolMeasurement::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43efe0 Script::BoolMeasurement::GetMax
int Script::BoolMeasurement::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ExposeGenericValue::SetValue
void Script::ExposeGenericValue::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ExposeGenericValue::GetBaseType
Script::ScriptReference * Script::ExposeGenericValue::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x416530 Script::ExposeGenericValue::GetInt
int Script::ExposeGenericValue::GetInt(Script::ScriptReference * a1) { return Fold_Return0_Arg1(a1); }
// FOLDED: 0x436a50 Script::ExposeGenericValue::GetType
Script::ScriptReference * Script::ExposeGenericValue::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ExposeGenericValue::GetKind
int Script::ExposeGenericValue::GetKind() { return 0; }
// FOLDED: 0x402b00 Script::ValueInterface::GetName
char const * Script::ValueInterface::GetName() { return ValueInterface_GetName(); }
// FOLDED: 0x402b10 Script::ValueInterface::GetMax
int Script::ValueInterface::GetMax() { return ValueInterface_GetMax(); }
// FOLDED: 0x402b70 Script::ValueInterface::GetType
Script::ScriptReference * Script::ValueInterface::GetType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ValueInterface::GetValueType
int Script::ValueInterface::GetValueType() { return 0; }

// ---- modules/fx/gamelight.h
// FOLDED: 0x407a40 RWS::FX::GameLight::GetMembers
void * RWS::FX::GameLight::GetMembers(int * a1) { return Script::ScriptLight::GetMembers(a1); }

// ---- modules/tfb/memcard.h
// FOLDED: 0x445eb0 MemCard::HandleAttributes
void MemCard::HandleAttributes(RWS::CAttributePacket const & a1) { RWS::CAttributeHandler::HandleAttributes(a1); }   // GameController::HandleAttributes: the base call

// ---- modules/tfb/sprites/gamesprite.h
// FOLDED: 0x445e10 RWS::TFB::GameSprite::IsMeshCollidee
int RWS::TFB::GameSprite::IsMeshCollidee() { return Fold_ReturnFalse(); }
// FOLDED: 0x445e10 RWS::TFB::GameSprite::IsMeshCollider
int RWS::TFB::GameSprite::IsMeshCollider() { return Fold_ReturnFalse(); }
// FOLDED: 0x445e10 RWS::TFB::GameSprite::IsSolidCollider
int RWS::TFB::GameSprite::IsSolidCollider() { return Fold_ReturnFalse(); }

// ---- were hand-written aliases in script/normal_bringup.cpp and single pragmas in the type files
// FOLDED: 0x42cbe0 Script::ScriptSubset::GetKind
int Script::ScriptSubset::GetKind() { return 2; }
// FOLDED: 0x4025a0 Script::ReferenceSet::SetValue
void Script::ReferenceSet::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) { }
// FOLDED: 0x402b70 Script::ReferenceSet::GetBaseType
Script::ScriptReference * Script::ReferenceSet::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x402ba0 Script::ReferenceSet::GetName
char const * Script::ReferenceSet::GetName() { return ReferenceSet_GetName(); }
// FOLDED: 0x402bb0 Script::ReferenceSet::GetType
Script::ScriptReference * Script::ReferenceSet::GetType() { return ReferenceSet_GetType(); }
// FOLDED: 0x43e6f0 Script::ReferenceSet::GetKind
int Script::ReferenceSet::GetKind() { return 0; }
// FOLDED: 0x4039f0 Script::ActorAttachment::GetBaseType
Script::ScriptReference * Script::ActorAttachment::GetBaseType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x42d650 Script::ActorAttachment::GetType
Script::ScriptReference * Script::ActorAttachment::GetType() { return ActorAttachment_GetType(); }
// FOLDED: 0x42d5f0 Script::ScriptActor::GetName
char const * Script::ScriptActor::GetName() { return ScriptActor_GetName(); }
// FOLDED: 0x4386c0 Script::ScriptActor::vf10
void Script::ScriptActor::vf10() { ScriptActor_Reset(); }
// FOLDED: 0x438b10 Script::ScriptCamera::vf10
void Script::ScriptCamera::vf10() { ScriptCamera_Reset(); }
// FOLDED: 0x403d80 Script::RumbleProto::Spawn
intptr_t Script::RumbleProto::Spawn(intptr_t a1, int /*a2*/) { return Script::ScriptController::ScriptRumble::SpawnFrom(this, a1); }   // the game's shared body (ScriptRumble::Spawn's)
// FOLDED: 0x4039e0 Script::ScriptController::ScriptRumble::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x403a50 Script::ScriptController::ScriptRumble::GetName
char const * Script::ScriptController::ScriptRumble::GetName() { return ScriptRumble_GetName(); }
// FOLDED: 0x403a60 Script::ScriptController::ScriptRumble::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::GetType() { return ScriptRumble_GetType(); }
// FOLDED: 0x43e640 Script::ScriptFog::GetKind
int Script::ScriptFog::GetKind() { return -1; }
// FOLDED: 0x43e640 Script::ScriptMathOps::GetKind
int Script::ScriptMathOps::GetKind() { return -1; }
// FOLDED: 0x410380 Script::ScriptMedia::vf11
void Script::ScriptMedia::vf11(void * /*a1*/) { }
// FOLDED: 0x410380 Script::ScriptMedia::vf12
void Script::ScriptMedia::vf12(void * /*a1*/) { }
// FOLDED: 0x43e640 Script::ScriptMemCard::GetKind
int Script::ScriptMemCard::GetKind() { return -1; }
// FOLDED: 0x43e640 Script::ScriptMotionBlur::GetKind
int Script::ScriptMotionBlur::GetKind() { return -1; }
// FOLDED: 0x416490 Script::ScriptSprite::ExposeClones::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeClones::GetBaseType() { return thunk_FUN_0043e330(); }
// FOLDED: 0x41cc50 Script::PropProto::GetMembers
void * Script::PropProto::GetMembers(int * a1) { return ((Script::ScriptProp *)this)->Script::ScriptProp::GetMembers(a1); }   // cross-class fold; the classes share no base here
// FOLDED: 0x4604c0 Script::ScriptProp::IsMeshCollidee
int Script::ScriptProp::IsMeshCollidee() { return true; }
// FOLDED: 0x4604c0 Script::ScriptProp::IsSolidCollider
int Script::ScriptProp::IsSolidCollider() { return true; }
// FOLDED: 0x43e640 Script::ScriptShadow::GetKind
int Script::ScriptShadow::GetKind() { return -1; }
// FOLDED: 0x43e640 Script::ScriptSystem::GetKind
int Script::ScriptSystem::GetKind() { return -1; }
// FOLDED: 0x4391b0 Script::ScriptCamera::ExposeCameraLocation::GetVector
Vec3 *Script::ScriptCamera::ExposeCameraLocation::GetVector(Script::ScriptReference *owner) { return (Vec3 *)ExposeCameraLocation_GetVector(owner); }

// ---- written by hand (the shared body does not fit the member's signature or class: its real body, or the bits forwarded unchanged)
// FOLDED: 0x43aa60 Script::ExposeDeadSet::GetBaseType
Script::ScriptReference *Script::ExposeDeadSet::GetBaseType() { return TFBType_Actor(); }   // ScriptActor::GetType
// FOLDED: 0x43aa60 Script::ExposeInactiveSet::GetBaseType
Script::ScriptReference *Script::ExposeInactiveSet::GetBaseType() { return TFBType_Actor(); }   // ScriptActor::GetType
// FOLDED: 0x43aa60 Script::ExposeNPCsSet::GetBaseType
Script::ScriptReference *Script::ExposeNPCsSet::GetBaseType() { return TFBType_Actor(); }   // ScriptActor::GetType
// FOLDED: 0x43aa60 Script::ExposePlayerSet::GetBaseType
Script::ScriptReference *Script::ExposePlayerSet::GetBaseType() { return TFBType_Actor(); }   // ScriptActor::GetType
// FOLDED: 0x43aa60 Script::ExposeSendersSet::GetBaseType
Script::ScriptReference *Script::ExposeSendersSet::GetBaseType() { return TFBType_Actor(); }   // ScriptActor::GetType
// FOLDED: 0x43dc80 Script::ScriptReference::GetMembers
void *Script::ScriptReference::GetMembers(int *count) { *count = 0; return 0; }   // CAttributeHandler::GetAttrPostUpdateFunc (pThis = 0; return 0): as an int count on x64 too
// FOLDED: 0x4039b0 Script::PositionMeasurement::GetRoll
float Script::PositionMeasurement::GetRoll() { return g_floatZero; }   // PositionMeasurement::GetFacing
// FOLDED: 0x4039b0 Script::PositionMeasurement::GetTilt
float Script::PositionMeasurement::GetTilt() { return g_floatZero; }   // PositionMeasurement::GetFacing
// FOLDED: 0x4039d0 Script::PositionMeasurement::GetVelocity
Vec3 *Script::PositionMeasurement::GetVelocity(Script::ScriptReference *) { return GetZeroVec3(); }   // VectorMeasurement::GetVector
// FOLDED: 0x410380 Script::Placement::SetAttachTarget
void Script::Placement::SetAttachTarget(Script::ScriptReference *) {}   // the shared empty function (ret 4)
// FOLDED: 0x410380 RWS::TFB::LevelHub::vf4
void RWS::TFB::LevelHub::vf4(const RWS::CAttributePacket &) {}   // the shared empty function (ret 4)


// ---- destructors the game folded into their base's destructor (ICF): the real destructor is empty (the compiler chains the bases)
// script/membertable.h
// FOLDED: 0x43b3d0 Script::MemberSlot::~MemberSlot
Script::MemberSlot::~MemberSlot() {}   // the game folded it into Script::RootRefDtor::~RootRefDtor
// script/types/actormessage.h
// FOLDED: 0x43b3d0 Script::ActorMessage::~ActorMessage
Script::ActorMessage::~ActorMessage() {}   // the game folded it into Script::RootRefDtor::~RootRefDtor
// script/types/scriptparticle.h
// FOLDED: 0x43b0b0 Script::ScriptParticle::ExposeClones::~ExposeClones
Script::ScriptParticle::ExposeClones::~ExposeClones() {}   // the game folded it into Script::ExposeClonesImpl::~ExposeClonesImpl
// script/types/scriptsets.h
// FOLDED: 0x43b0b0 Script::_ParticleShield::ExposeActiveSet::~ExposeActiveSet
Script::_ParticleShield::ExposeActiveSet::~ExposeActiveSet() {}   // the game folded it into Script::ExposeClonesImpl::~ExposeClonesImpl
// FOLDED: 0x43e2e0 Script::_SpriteShield::ExposeActiveSet::~ExposeActiveSet
Script::_SpriteShield::ExposeActiveSet::~ExposeActiveSet() {}   // the game folded it into Script::RefSetDtor43e::~RefSetDtor43e
// FOLDED: 0x435230 Script::ExposeDeadSet::~ExposeDeadSet
Script::ExposeDeadSet::~ExposeDeadSet() {}   // the game folded it into Script::CsSet::~CsSet
// FOLDED: 0x435230 Script::ExposeInactiveSet::~ExposeInactiveSet
Script::ExposeInactiveSet::~ExposeInactiveSet() {}   // the game folded it into Script::CsSet::~CsSet
// FOLDED: 0x43b0b0 Script::_ParticleShield::ExposeInactiveSet::~ExposeInactiveSet
Script::_ParticleShield::ExposeInactiveSet::~ExposeInactiveSet() {}   // the game folded it into Script::ExposeClonesImpl::~ExposeClonesImpl
// FOLDED: 0x435230 Script::ExposeNPCsSet::~ExposeNPCsSet
Script::ExposeNPCsSet::~ExposeNPCsSet() {}   // the game folded it into Script::CsSet::~CsSet
// FOLDED: 0x435230 Script::ExposePlayerSet::~ExposePlayerSet
Script::ExposePlayerSet::~ExposePlayerSet() {}   // the game folded it into Script::CsSet::~CsSet
// FOLDED: 0x43b0b0 Script::_ParticleShield::ExposeRemovedSet::~ExposeRemovedSet
Script::_ParticleShield::ExposeRemovedSet::~ExposeRemovedSet() {}   // the game folded it into Script::ExposeClonesImpl::~ExposeClonesImpl
// FOLDED: 0x43e2e0 Script::_SpriteShield::ExposeRemovedSet::~ExposeRemovedSet
Script::_SpriteShield::ExposeRemovedSet::~ExposeRemovedSet() {}   // the game folded it into Script::RefSetDtor43e::~RefSetDtor43e
// FOLDED: 0x43a640 Script::ExposeSendersSet::~ExposeSendersSet
Script::ExposeSendersSet::~ExposeSendersSet() {}   // the game folded it into Script::ExposeSendersSetImpl::~ExposeSendersSetImpl
// tfb/hud/hud2d.h
// FOLDED: 0x464ef0 Hud2DObject::~Hud2DObject
Hud2DObject::~Hud2DObject() {}   // the game folded it into RWS::CEventId::~CEventId

// FOLDED: 0x436950 Script::ColorMeasurement::ExposeA::GetName
const char * Script::ColorMeasurement::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ColorMeasurement::ExposeA::GetType
Script::ScriptReference * Script::ColorMeasurement::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ColorMeasurement::ExposeA::GetBaseType
Script::ScriptReference * Script::ColorMeasurement::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ColorMeasurement::ExposeA::GetMax
int Script::ColorMeasurement::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ColorMeasurement::ExposeB::GetName
const char * Script::ColorMeasurement::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ColorMeasurement::ExposeB::GetType
Script::ScriptReference * Script::ColorMeasurement::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ColorMeasurement::ExposeB::GetBaseType
Script::ScriptReference * Script::ColorMeasurement::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ColorMeasurement::ExposeB::GetMax
int Script::ColorMeasurement::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ColorMeasurement::ExposeG::GetName
const char * Script::ColorMeasurement::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ColorMeasurement::ExposeG::GetType
Script::ScriptReference * Script::ColorMeasurement::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ColorMeasurement::ExposeG::GetBaseType
Script::ScriptReference * Script::ColorMeasurement::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ColorMeasurement::ExposeG::GetMax
int Script::ColorMeasurement::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ColorMeasurement::ExposeR::GetName
const char * Script::ColorMeasurement::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ColorMeasurement::ExposeR::GetType
Script::ScriptReference * Script::ColorMeasurement::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ColorMeasurement::ExposeR::GetBaseType
Script::ScriptReference * Script::ColorMeasurement::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ColorMeasurement::ExposeR::GetMax
int Script::ColorMeasurement::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436950 Script::ScriptActor::ExposeBlipColor::ExposeA::GetName
const char * Script::ScriptActor::ExposeBlipColor::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeBlipColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeBlipColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeBlipColor::ExposeA::GetMax
int Script::ScriptActor::ExposeBlipColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptActor::ExposeBlipColor::ExposeB::GetName
const char * Script::ScriptActor::ExposeBlipColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeBlipColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeBlipColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeBlipColor::ExposeB::GetMax
int Script::ScriptActor::ExposeBlipColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptActor::ExposeBlipColor::ExposeG::GetName
const char * Script::ScriptActor::ExposeBlipColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeBlipColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeBlipColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeBlipColor::ExposeG::GetMax
int Script::ScriptActor::ExposeBlipColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptActor::ExposeBlipColor::ExposeR::GetName
const char * Script::ScriptActor::ExposeBlipColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeBlipColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeBlipColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeBlipColor::ExposeR::GetMax
int Script::ScriptActor::ExposeBlipColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436950 Script::ScriptActor::ExposeConeColor::ExposeA::GetName
const char * Script::ScriptActor::ExposeConeColor::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeConeColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeConeColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeConeColor::ExposeA::GetMax
int Script::ScriptActor::ExposeConeColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptActor::ExposeConeColor::ExposeB::GetName
const char * Script::ScriptActor::ExposeConeColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeConeColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeConeColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeConeColor::ExposeB::GetMax
int Script::ScriptActor::ExposeConeColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptActor::ExposeConeColor::ExposeG::GetName
const char * Script::ScriptActor::ExposeConeColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeConeColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeConeColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeConeColor::ExposeG::GetMax
int Script::ScriptActor::ExposeConeColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptActor::ExposeConeColor::ExposeR::GetName
const char * Script::ScriptActor::ExposeConeColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeConeColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeConeColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeConeColor::ExposeR::GetMax
int Script::ScriptActor::ExposeConeColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x430b70 Script::ScriptActor::ExposeDestination::ExposeHeading::GetName
const char * Script::ScriptActor::ExposeDestination::ExposeHeading::GetName() { return ExposeHeadingFromMe_GetName(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeDestination::ExposeHeading::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposeHeading::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeDestination::ExposeHeading::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposeHeading::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeDestination::ExposeHeading::GetKind
int Script::ScriptActor::ExposeDestination::ExposeHeading::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeDestination::ExposeHeading::GetMax
int Script::ScriptActor::ExposeDestination::ExposeHeading::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeDestination::ExposeHeading::GetValueType
int Script::ScriptActor::ExposeDestination::ExposeHeading::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeDestination::ExposeHeading::SetValue
void Script::ScriptActor::ExposeDestination::ExposeHeading::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403eb0 Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetName
char const * Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetName() { return ExposeHorzComponent_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetKind
int Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetValueType
int Script::ScriptActor::ExposeDestination::ExposeHorzComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeDestination::ExposeHorzComponent::SetValue
void Script::ScriptActor::ExposeDestination::ExposeHorzComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x430b80 Script::ScriptActor::ExposeDestination::ExposePitch::GetName
const char * Script::ScriptActor::ExposeDestination::ExposePitch::GetName() { return ExposePitchFromMe_GetName(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeDestination::ExposePitch::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposePitch::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeDestination::ExposePitch::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::ExposePitch::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeDestination::ExposePitch::GetKind
int Script::ScriptActor::ExposeDestination::ExposePitch::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeDestination::ExposePitch::GetMax
int Script::ScriptActor::ExposeDestination::ExposePitch::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeDestination::ExposePitch::GetValueType
int Script::ScriptActor::ExposeDestination::ExposePitch::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeDestination::ExposePitch::SetValue
void Script::ScriptActor::ExposeDestination::ExposePitch::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x430b70 Script::ScriptActor::ExposeOrigin::ExposeHeading::GetName
const char * Script::ScriptActor::ExposeOrigin::ExposeHeading::GetName() { return ExposeHeadingFromMe_GetName(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeOrigin::ExposeHeading::GetType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposeHeading::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeOrigin::ExposeHeading::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposeHeading::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeOrigin::ExposeHeading::GetKind
int Script::ScriptActor::ExposeOrigin::ExposeHeading::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeOrigin::ExposeHeading::GetMax
int Script::ScriptActor::ExposeOrigin::ExposeHeading::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeOrigin::ExposeHeading::GetValueType
int Script::ScriptActor::ExposeOrigin::ExposeHeading::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeOrigin::ExposeHeading::SetValue
void Script::ScriptActor::ExposeOrigin::ExposeHeading::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403eb0 Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetName
char const * Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetName() { return ExposeHorzComponent_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetKind
int Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetValueType
int Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::SetValue
void Script::ScriptActor::ExposeOrigin::ExposeHorzComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x430b80 Script::ScriptActor::ExposeOrigin::ExposePitch::GetName
const char * Script::ScriptActor::ExposeOrigin::ExposePitch::GetName() { return ExposePitchFromMe_GetName(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeOrigin::ExposePitch::GetType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposePitch::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeOrigin::ExposePitch::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposePitch::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeOrigin::ExposePitch::GetKind
int Script::ScriptActor::ExposeOrigin::ExposePitch::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeOrigin::ExposePitch::GetMax
int Script::ScriptActor::ExposeOrigin::ExposePitch::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeOrigin::ExposePitch::GetValueType
int Script::ScriptActor::ExposeOrigin::ExposePitch::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeOrigin::ExposePitch::SetValue
void Script::ScriptActor::ExposeOrigin::ExposePitch::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403ec0 Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetName
char const * Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetName() { return ExposeVertComponent_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetKind
int Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetValueType
int Script::ScriptActor::ExposeOrigin::ExposeVertComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeOrigin::ExposeVertComponent::SetValue
void Script::ScriptActor::ExposeOrigin::ExposeVertComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403eb0 Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetName
char const * Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetName() { return ExposeHorzComponent_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetKind
int Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetValueType
int Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::SetValue
void Script::ScriptActor::ExposeSpeed::ExposeHorzComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x43dbf0 Script::ScriptActor::ExposeSpeed::ExposePitch::GetName
char const * Script::ScriptActor::ExposeSpeed::ExposePitch::GetName() { return ExposePitch_GetName(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeSpeed::ExposePitch::GetType
Script::ScriptReference * Script::ScriptActor::ExposeSpeed::ExposePitch::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeSpeed::ExposePitch::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeSpeed::ExposePitch::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeSpeed::ExposePitch::GetKind
int Script::ScriptActor::ExposeSpeed::ExposePitch::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeSpeed::ExposePitch::GetMax
int Script::ScriptActor::ExposeSpeed::ExposePitch::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeSpeed::ExposePitch::GetValueType
int Script::ScriptActor::ExposeSpeed::ExposePitch::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeSpeed::ExposePitch::SetValue
void Script::ScriptActor::ExposeSpeed::ExposePitch::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403ec0 Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetName
char const * Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetName() { return ExposeVertComponent_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetKind
int Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetValueType
int Script::ScriptActor::ExposeSpeed::ExposeVertComponent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeSpeed::ExposeVertComponent::SetValue
void Script::ScriptActor::ExposeSpeed::ExposeVertComponent::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403ea0 Script::ScriptActor::ExposeSpeedHeading::GetName
char const * Script::ScriptActor::ExposeSpeedHeading::GetName() { return ExposeHeading_GetName(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeSpeedHeading::GetType
Script::ScriptReference * Script::ScriptActor::ExposeSpeedHeading::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeSpeedHeading::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeSpeedHeading::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeSpeedHeading::GetKind
int Script::ScriptActor::ExposeSpeedHeading::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeSpeedHeading::GetMax
int Script::ScriptActor::ExposeSpeedHeading::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeSpeedHeading::GetValueType
int Script::ScriptActor::ExposeSpeedHeading::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeSpeedHeading::SetValue
void Script::ScriptActor::ExposeSpeedHeading::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436950 Script::ScriptActor::ExposeTintColor::ExposeA::GetName
const char * Script::ScriptActor::ExposeTintColor::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeTintColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeTintColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeTintColor::ExposeA::GetMax
int Script::ScriptActor::ExposeTintColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptActor::ExposeTintColor::ExposeB::GetName
const char * Script::ScriptActor::ExposeTintColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeTintColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeTintColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeTintColor::ExposeB::GetMax
int Script::ScriptActor::ExposeTintColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptActor::ExposeTintColor::ExposeG::GetName
const char * Script::ScriptActor::ExposeTintColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeTintColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeTintColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeTintColor::ExposeG::GetMax
int Script::ScriptActor::ExposeTintColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptActor::ExposeTintColor::ExposeR::GetName
const char * Script::ScriptActor::ExposeTintColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeTintColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeTintColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptActor::ExposeTintColor::ExposeR::GetMax
int Script::ScriptActor::ExposeTintColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436a50 Script::ScriptController::ExposeCircle::GetType
Script::ScriptReference * Script::ScriptController::ExposeCircle::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeCircle::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeCircle::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeCircle::GetKind
int Script::ScriptController::ExposeCircle::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeCircle::GetMax
int Script::ScriptController::ExposeCircle::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeCircle::SetValue
void Script::ScriptController::ExposeCircle::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeControllerUnplugged::GetType
Script::ScriptReference * Script::ScriptController::ExposeControllerUnplugged::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeControllerUnplugged::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeControllerUnplugged::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeControllerUnplugged::GetKind
int Script::ScriptController::ExposeControllerUnplugged::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeControllerUnplugged::GetMax
int Script::ScriptController::ExposeControllerUnplugged::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeControllerUnplugged::SetValue
void Script::ScriptController::ExposeControllerUnplugged::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeDPadD::GetType
Script::ScriptReference * Script::ScriptController::ExposeDPadD::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeDPadD::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeDPadD::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeDPadD::GetKind
int Script::ScriptController::ExposeDPadD::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeDPadD::GetMax
int Script::ScriptController::ExposeDPadD::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeDPadD::SetValue
void Script::ScriptController::ExposeDPadD::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeDPadL::GetType
Script::ScriptReference * Script::ScriptController::ExposeDPadL::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeDPadL::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeDPadL::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeDPadL::GetKind
int Script::ScriptController::ExposeDPadL::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeDPadL::GetMax
int Script::ScriptController::ExposeDPadL::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeDPadL::SetValue
void Script::ScriptController::ExposeDPadL::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeDPadR::GetType
Script::ScriptReference * Script::ScriptController::ExposeDPadR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeDPadR::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeDPadR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeDPadR::GetKind
int Script::ScriptController::ExposeDPadR::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeDPadR::GetMax
int Script::ScriptController::ExposeDPadR::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeDPadR::SetValue
void Script::ScriptController::ExposeDPadR::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeDPadU::GetType
Script::ScriptReference * Script::ScriptController::ExposeDPadU::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeDPadU::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeDPadU::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeDPadU::GetKind
int Script::ScriptController::ExposeDPadU::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeDPadU::GetMax
int Script::ScriptController::ExposeDPadU::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeDPadU::SetValue
void Script::ScriptController::ExposeDPadU::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeL1::GetType
Script::ScriptReference * Script::ScriptController::ExposeL1::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeL1::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeL1::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeL1::GetKind
int Script::ScriptController::ExposeL1::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeL1::GetMax
int Script::ScriptController::ExposeL1::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeL1::SetValue
void Script::ScriptController::ExposeL1::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeL2::GetType
Script::ScriptReference * Script::ScriptController::ExposeL2::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeL2::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeL2::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeL2::GetKind
int Script::ScriptController::ExposeL2::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeL2::GetMax
int Script::ScriptController::ExposeL2::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeL2::SetValue
void Script::ScriptController::ExposeL2::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403e90 Script::ScriptController::ExposeLeftAnalogAngle::GetType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogAngle::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptController::ExposeLeftAnalogAngle::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogAngle::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeLeftAnalogAngle::GetKind
int Script::ScriptController::ExposeLeftAnalogAngle::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeLeftAnalogAngle::GetMax
int Script::ScriptController::ExposeLeftAnalogAngle::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptController::ExposeLeftAnalogAngle::GetValueType
int Script::ScriptController::ExposeLeftAnalogAngle::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeLeftAnalogAngle::SetValue
void Script::ScriptController::ExposeLeftAnalogAngle::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeLeftAnalogD::GetType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogD::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeLeftAnalogD::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogD::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeLeftAnalogD::GetKind
int Script::ScriptController::ExposeLeftAnalogD::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeLeftAnalogD::GetMax
int Script::ScriptController::ExposeLeftAnalogD::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeLeftAnalogD::SetValue
void Script::ScriptController::ExposeLeftAnalogD::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeLeftAnalogL::GetType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogL::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeLeftAnalogL::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogL::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeLeftAnalogL::GetKind
int Script::ScriptController::ExposeLeftAnalogL::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeLeftAnalogL::GetMax
int Script::ScriptController::ExposeLeftAnalogL::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeLeftAnalogL::SetValue
void Script::ScriptController::ExposeLeftAnalogL::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeLeftAnalogMagnitude::GetType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogMagnitude::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeLeftAnalogMagnitude::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogMagnitude::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeLeftAnalogMagnitude::GetKind
int Script::ScriptController::ExposeLeftAnalogMagnitude::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptController::ExposeLeftAnalogMagnitude::GetValueType
int Script::ScriptController::ExposeLeftAnalogMagnitude::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeLeftAnalogMagnitude::SetValue
void Script::ScriptController::ExposeLeftAnalogMagnitude::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeLeftAnalogR::GetType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeLeftAnalogR::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeLeftAnalogR::GetKind
int Script::ScriptController::ExposeLeftAnalogR::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeLeftAnalogR::GetMax
int Script::ScriptController::ExposeLeftAnalogR::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeLeftAnalogR::SetValue
void Script::ScriptController::ExposeLeftAnalogR::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeLeftAnalogU::GetType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogU::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeLeftAnalogU::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeLeftAnalogU::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeLeftAnalogU::GetKind
int Script::ScriptController::ExposeLeftAnalogU::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeLeftAnalogU::GetMax
int Script::ScriptController::ExposeLeftAnalogU::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeLeftAnalogU::SetValue
void Script::ScriptController::ExposeLeftAnalogU::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeR1::GetType
Script::ScriptReference * Script::ScriptController::ExposeR1::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeR1::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeR1::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeR1::GetKind
int Script::ScriptController::ExposeR1::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeR1::GetMax
int Script::ScriptController::ExposeR1::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeR1::SetValue
void Script::ScriptController::ExposeR1::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeR2::GetType
Script::ScriptReference * Script::ScriptController::ExposeR2::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeR2::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeR2::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeR2::GetKind
int Script::ScriptController::ExposeR2::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeR2::GetMax
int Script::ScriptController::ExposeR2::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeR2::SetValue
void Script::ScriptController::ExposeR2::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x403e90 Script::ScriptController::ExposeRightAnalogAngle::GetType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogAngle::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptController::ExposeRightAnalogAngle::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogAngle::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeRightAnalogAngle::GetKind
int Script::ScriptController::ExposeRightAnalogAngle::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeRightAnalogAngle::GetMax
int Script::ScriptController::ExposeRightAnalogAngle::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptController::ExposeRightAnalogAngle::GetValueType
int Script::ScriptController::ExposeRightAnalogAngle::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeRightAnalogAngle::SetValue
void Script::ScriptController::ExposeRightAnalogAngle::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeRightAnalogD::GetType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogD::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeRightAnalogD::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogD::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeRightAnalogD::GetKind
int Script::ScriptController::ExposeRightAnalogD::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeRightAnalogD::GetMax
int Script::ScriptController::ExposeRightAnalogD::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeRightAnalogD::SetValue
void Script::ScriptController::ExposeRightAnalogD::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeRightAnalogL::GetType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogL::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeRightAnalogL::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogL::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeRightAnalogL::GetKind
int Script::ScriptController::ExposeRightAnalogL::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeRightAnalogL::GetMax
int Script::ScriptController::ExposeRightAnalogL::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeRightAnalogL::SetValue
void Script::ScriptController::ExposeRightAnalogL::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeRightAnalogMagnitude::GetType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogMagnitude::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeRightAnalogMagnitude::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogMagnitude::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeRightAnalogMagnitude::GetKind
int Script::ScriptController::ExposeRightAnalogMagnitude::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptController::ExposeRightAnalogMagnitude::GetValueType
int Script::ScriptController::ExposeRightAnalogMagnitude::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeRightAnalogMagnitude::SetValue
void Script::ScriptController::ExposeRightAnalogMagnitude::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeRightAnalogR::GetType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeRightAnalogR::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeRightAnalogR::GetKind
int Script::ScriptController::ExposeRightAnalogR::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeRightAnalogR::GetMax
int Script::ScriptController::ExposeRightAnalogR::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeRightAnalogR::SetValue
void Script::ScriptController::ExposeRightAnalogR::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeRightAnalogU::GetType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogU::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeRightAnalogU::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeRightAnalogU::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeRightAnalogU::GetKind
int Script::ScriptController::ExposeRightAnalogU::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptController::ExposeRightAnalogU::GetMax
int Script::ScriptController::ExposeRightAnalogU::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeRightAnalogU::SetValue
void Script::ScriptController::ExposeRightAnalogU::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeSelect::GetType
Script::ScriptReference * Script::ScriptController::ExposeSelect::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeSelect::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeSelect::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeSelect::GetKind
int Script::ScriptController::ExposeSelect::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeSelect::GetMax
int Script::ScriptController::ExposeSelect::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeSelect::SetValue
void Script::ScriptController::ExposeSelect::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeSquare::GetType
Script::ScriptReference * Script::ScriptController::ExposeSquare::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeSquare::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeSquare::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeSquare::GetKind
int Script::ScriptController::ExposeSquare::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeSquare::GetMax
int Script::ScriptController::ExposeSquare::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeSquare::SetValue
void Script::ScriptController::ExposeSquare::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeStart::GetType
Script::ScriptReference * Script::ScriptController::ExposeStart::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeStart::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeStart::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeStart::GetKind
int Script::ScriptController::ExposeStart::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeStart::GetMax
int Script::ScriptController::ExposeStart::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeStart::SetValue
void Script::ScriptController::ExposeStart::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeTriangle::GetType
Script::ScriptReference * Script::ScriptController::ExposeTriangle::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeTriangle::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeTriangle::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeTriangle::GetKind
int Script::ScriptController::ExposeTriangle::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeTriangle::GetMax
int Script::ScriptController::ExposeTriangle::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeTriangle::SetValue
void Script::ScriptController::ExposeTriangle::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptController::ExposeX::GetType
Script::ScriptReference * Script::ScriptController::ExposeX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ExposeX::GetBaseType
Script::ScriptReference * Script::ScriptController::ExposeX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptController::ExposeX::GetKind
int Script::ScriptController::ExposeX::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptController::ExposeX::GetMax
int Script::ScriptController::ExposeX::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptController::ExposeX::SetValue
void Script::ScriptController::ExposeX::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptFog::ExposePixelCutoffIntensity::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelCutoffIntensity::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelCutoffIntensity::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelCutoffIntensity::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptFog::ExposePixelCutoffIntensity::GetMax
int Script::ScriptFog::ExposePixelCutoffIntensity::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptFog::ExposePixelFogColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelFogColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposePixelFogColor::ExposeA::GetMax
int Script::ScriptFog::ExposePixelFogColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptFog::ExposePixelFogColor::ExposeB::GetName
const char * Script::ScriptFog::ExposePixelFogColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposePixelFogColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelFogColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposePixelFogColor::ExposeB::GetMax
int Script::ScriptFog::ExposePixelFogColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptFog::ExposePixelFogColor::ExposeG::GetName
const char * Script::ScriptFog::ExposePixelFogColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposePixelFogColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelFogColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposePixelFogColor::ExposeG::GetMax
int Script::ScriptFog::ExposePixelFogColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptFog::ExposePixelFogColor::ExposeR::GetName
const char * Script::ScriptFog::ExposePixelFogColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposePixelFogColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelFogColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposePixelFogColor::ExposeR::GetMax
int Script::ScriptFog::ExposePixelFogColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposePixelFogDistance::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogDistance::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelFogDistance::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogDistance::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptFog::ExposePixelFogDistance::GetValueType
int Script::ScriptFog::ExposePixelFogDistance::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposePixelFogEnable::GetType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogEnable::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposePixelFogEnable::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposePixelFogEnable::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptFog::ExposePixelFogEnable::GetMax
int Script::ScriptFog::ExposePixelFogEnable::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x407aa0 Script::ScriptFog::ExposeVertFogColor::GetType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeVertFogColor::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptFog::ExposeVertFogColor::GetMax
int Script::ScriptFog::ExposeVertFogColor::GetMax() { return 0; }
// FOLDED: 0x407ad0 Script::ScriptFog::ExposeVertFogColor::GetValueType
int Script::ScriptFog::ExposeVertFogColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436950 Script::ScriptFog::ExposeVertFogColor::ExposeA::GetName
const char * Script::ScriptFog::ExposeVertFogColor::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeVertFogColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeVertFogColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposeVertFogColor::ExposeA::GetMax
int Script::ScriptFog::ExposeVertFogColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptFog::ExposeVertFogColor::ExposeB::GetName
const char * Script::ScriptFog::ExposeVertFogColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeVertFogColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeVertFogColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposeVertFogColor::ExposeB::GetMax
int Script::ScriptFog::ExposeVertFogColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptFog::ExposeVertFogColor::ExposeG::GetName
const char * Script::ScriptFog::ExposeVertFogColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeVertFogColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeVertFogColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposeVertFogColor::ExposeG::GetMax
int Script::ScriptFog::ExposeVertFogColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptFog::ExposeVertFogColor::ExposeR::GetName
const char * Script::ScriptFog::ExposeVertFogColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeVertFogColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeVertFogColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptFog::ExposeVertFogColor::ExposeR::GetMax
int Script::ScriptFog::ExposeVertFogColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436a50 Script::ScriptFog::ExposeVertFogDistance::GetType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogDistance::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptFog::ExposeVertFogDistance::GetBaseType
Script::ScriptReference * Script::ScriptFog::ExposeVertFogDistance::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptFog::ExposeVertFogDistance::GetValueType
int Script::ScriptFog::ExposeVertFogDistance::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x407aa0 Script::ScriptLight::ExposeLightColor::GetType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptLight::ExposeLightColor::GetBaseType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptLight::ExposeLightColor::GetMax
int Script::ScriptLight::ExposeLightColor::GetMax() { return 0; }
// FOLDED: 0x407ad0 Script::ScriptLight::ExposeLightColor::GetValueType
int Script::ScriptLight::ExposeLightColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436950 Script::ScriptLight::ExposeLightColor::ExposeA::GetName
const char * Script::ScriptLight::ExposeLightColor::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptLight::ExposeLightColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptLight::ExposeLightColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptLight::ExposeLightColor::ExposeA::GetMax
int Script::ScriptLight::ExposeLightColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptLight::ExposeLightColor::ExposeB::GetName
const char * Script::ScriptLight::ExposeLightColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptLight::ExposeLightColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptLight::ExposeLightColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptLight::ExposeLightColor::ExposeB::GetMax
int Script::ScriptLight::ExposeLightColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptLight::ExposeLightColor::ExposeG::GetName
const char * Script::ScriptLight::ExposeLightColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptLight::ExposeLightColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptLight::ExposeLightColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptLight::ExposeLightColor::ExposeG::GetMax
int Script::ScriptLight::ExposeLightColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptLight::ExposeLightColor::ExposeR::GetName
const char * Script::ScriptLight::ExposeLightColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptLight::ExposeLightColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptLight::ExposeLightColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptLight::ExposeLightColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptLight::ExposeLightColor::ExposeR::GetMax
int Script::ScriptLight::ExposeLightColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x403e90 Script::ScriptMathOps::ExposeNormalizeAngle::GetType
Script::ScriptReference * Script::ScriptMathOps::ExposeNormalizeAngle::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMathOps::ExposeNormalizeAngle::GetBaseType
Script::ScriptReference * Script::ScriptMathOps::ExposeNormalizeAngle::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMathOps::ExposeNormalizeAngle::GetMax
int Script::ScriptMathOps::ExposeNormalizeAngle::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptMathOps::ExposeNormalizeAngle::GetValueType
int Script::ScriptMathOps::ExposeNormalizeAngle::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436950 Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetName
const char * Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetMax
int Script::ScriptParticle::ExposePrimaryEColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetName
const char * Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetMax
int Script::ScriptParticle::ExposePrimaryEColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetName
const char * Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetMax
int Script::ScriptParticle::ExposePrimaryEColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetName
const char * Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetMax
int Script::ScriptParticle::ExposePrimaryEColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436950 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetName
const char * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetMax
int Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetName
const char * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetMax
int Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetName
const char * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetMax
int Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetName
const char * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetMax
int Script::ScriptParticle::ExposePrimaryEColorSpread::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436950 Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetName
const char * Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetMax
int Script::ScriptParticle::ExposePrimarySColor::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetName
const char * Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetMax
int Script::ScriptParticle::ExposePrimarySColor::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetName
const char * Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetMax
int Script::ScriptParticle::ExposePrimarySColor::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetName
const char * Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetMax
int Script::ScriptParticle::ExposePrimarySColor::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436950 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetName
const char * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetMax
int Script::ScriptParticle::ExposePrimarySColorSpread::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetName
const char * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetMax
int Script::ScriptParticle::ExposePrimarySColorSpread::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetName
const char * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetMax
int Script::ScriptParticle::ExposePrimarySColorSpread::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetName
const char * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetMax
int Script::ScriptParticle::ExposePrimarySColorSpread::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436a50 Script::ScriptShadow::ExposeAreaRadius::GetType
Script::ScriptReference * Script::ScriptShadow::ExposeAreaRadius::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptShadow::ExposeAreaRadius::GetBaseType
Script::ScriptReference * Script::ScriptShadow::ExposeAreaRadius::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptShadow::ExposeAreaRadius::GetValueType
int Script::ScriptShadow::ExposeAreaRadius::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403ea0 Script::ScriptShadow::ExposeHeading::GetName
char const * Script::ScriptShadow::ExposeHeading::GetName() { return ExposeHeading_GetName(); }
// FOLDED: 0x403e90 Script::ScriptShadow::ExposeHeading::GetType
Script::ScriptReference * Script::ScriptShadow::ExposeHeading::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptShadow::ExposeHeading::GetBaseType
Script::ScriptReference * Script::ScriptShadow::ExposeHeading::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptShadow::ExposeHeading::GetMax
int Script::ScriptShadow::ExposeHeading::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptShadow::ExposeHeading::GetValueType
int Script::ScriptShadow::ExposeHeading::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x43dbf0 Script::ScriptShadow::ExposePitch::GetName
char const * Script::ScriptShadow::ExposePitch::GetName() { return ExposePitch_GetName(); }
// FOLDED: 0x403e90 Script::ScriptShadow::ExposePitch::GetType
Script::ScriptReference * Script::ScriptShadow::ExposePitch::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptShadow::ExposePitch::GetBaseType
Script::ScriptReference * Script::ScriptShadow::ExposePitch::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptShadow::ExposePitch::GetMax
int Script::ScriptShadow::ExposePitch::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptShadow::ExposePitch::GetValueType
int Script::ScriptShadow::ExposePitch::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptShadow::ExposeSoftness::GetType
Script::ScriptReference * Script::ScriptShadow::ExposeSoftness::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptShadow::ExposeSoftness::GetBaseType
Script::ScriptReference * Script::ScriptShadow::ExposeSoftness::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptShadow::ExposeSoftness::GetMax
int Script::ScriptShadow::ExposeSoftness::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4034a0 Script::ScriptShadow::ExposeSoftness::GetValueType
int Script::ScriptShadow::ExposeSoftness::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptShadow::ExposeVerticalOffset::GetType
Script::ScriptReference * Script::ScriptShadow::ExposeVerticalOffset::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptShadow::ExposeVerticalOffset::GetBaseType
Script::ScriptReference * Script::ScriptShadow::ExposeVerticalOffset::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptShadow::ExposeVerticalOffset::GetMax
int Script::ScriptShadow::ExposeVerticalOffset::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptShadow::ExposeVerticalOffset::GetValueType
int Script::ScriptShadow::ExposeVerticalOffset::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x438d30 Script::ScriptSound::ExposePercentComplete::GetName
const char * Script::ScriptSound::ExposePercentComplete::GetName() { return ExposePercentComplete_GetName(); }
// FOLDED: 0x436a50 Script::ScriptSound::ExposePercentComplete::GetType
Script::ScriptReference * Script::ScriptSound::ExposePercentComplete::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSound::ExposePercentComplete::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposePercentComplete::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSound::ExposePercentComplete::GetKind
int Script::ScriptSound::ExposePercentComplete::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptSound::ExposePercentComplete::GetMax
int Script::ScriptSound::ExposePercentComplete::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptSound::ExposePercentComplete::SetValue
void Script::ScriptSound::ExposePercentComplete::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x43dbf0 Script::ScriptSound::ExposePitch::GetName
char const * Script::ScriptSound::ExposePitch::GetName() { return ExposePitch_GetName(); }
// FOLDED: 0x436a50 Script::ScriptSound::ExposePitch::GetType
Script::ScriptReference * Script::ScriptSound::ExposePitch::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSound::ExposePitch::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposePitch::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptSound::ExposePitch::GetMax
int Script::ScriptSound::ExposePitch::GetMax() { return ExposeParticleSpread_GetMax(); }
// FOLDED: 0x438d40 Script::ScriptSound::ExposeTimeRemaining::GetName
char const * Script::ScriptSound::ExposeTimeRemaining::GetName() { return ExposeTimeRemaining_GetName(); }
// FOLDED: 0x436a50 Script::ScriptSound::ExposeTimeRemaining::GetType
Script::ScriptReference * Script::ScriptSound::ExposeTimeRemaining::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSound::ExposeTimeRemaining::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposeTimeRemaining::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSound::ExposeTimeRemaining::GetKind
int Script::ScriptSound::ExposeTimeRemaining::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptSound::ExposeTimeRemaining::GetValueType
int Script::ScriptSound::ExposeTimeRemaining::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptSound::ExposeTimeRemaining::SetValue
void Script::ScriptSound::ExposeTimeRemaining::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptSound::ExposeVolume::GetType
Script::ScriptReference * Script::ScriptSound::ExposeVolume::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSound::ExposeVolume::GetBaseType
Script::ScriptReference * Script::ScriptSound::ExposeVolume::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptSound::ExposeVolume::GetMax
int Script::ScriptSound::ExposeVolume::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeLocation::ExposeX::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeLocation::ExposeX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeLocation::ExposeX::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeLocation::ExposeX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeLocation::ExposeX::GetMax
int Script::ScriptSprite::ExposeLocation::ExposeX::GetMax() { return 0; }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeLocation::ExposeY::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeLocation::ExposeY::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeLocation::ExposeY::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeLocation::ExposeY::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeLocation::ExposeY::GetMax
int Script::ScriptSprite::ExposeLocation::ExposeY::GetMax() { return 0; }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposePixelExtents::ExposeX::GetType
Script::ScriptReference * Script::ScriptSprite::ExposePixelExtents::ExposeX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposePixelExtents::ExposeX::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposePixelExtents::ExposeX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposePixelExtents::ExposeX::GetMax
int Script::ScriptSprite::ExposePixelExtents::ExposeX::GetMax() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptSprite::ExposePixelExtents::ExposeX::SetValue
void Script::ScriptSprite::ExposePixelExtents::ExposeX::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposePixelExtents::ExposeY::GetType
Script::ScriptReference * Script::ScriptSprite::ExposePixelExtents::ExposeY::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposePixelExtents::ExposeY::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposePixelExtents::ExposeY::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposePixelExtents::ExposeY::GetMax
int Script::ScriptSprite::ExposePixelExtents::ExposeY::GetMax() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptSprite::ExposePixelExtents::ExposeY::SetValue
void Script::ScriptSprite::ExposePixelExtents::ExposeY::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeScales::ExposeX::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeScales::ExposeX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeScales::ExposeX::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeScales::ExposeX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeScales::ExposeX::GetMax
int Script::ScriptSprite::ExposeScales::ExposeX::GetMax() { return 0; }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeScales::ExposeY::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeScales::ExposeY::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeScales::ExposeY::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeScales::ExposeY::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeScales::ExposeY::GetMax
int Script::ScriptSprite::ExposeScales::ExposeY::GetMax() { return 0; }
// FOLDED: 0x436950 Script::ScriptSprite::ExposeTint::ExposeA::GetName
const char * Script::ScriptSprite::ExposeTint::ExposeA::GetName() { return ExposeA_GetName(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeTint::ExposeA::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeA::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeTint::ExposeA::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeA::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptSprite::ExposeTint::ExposeA::GetMax
int Script::ScriptSprite::ExposeTint::ExposeA::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436940 Script::ScriptSprite::ExposeTint::ExposeB::GetName
const char * Script::ScriptSprite::ExposeTint::ExposeB::GetName() { return ExposeB_GetName(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeTint::ExposeB::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeB::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeTint::ExposeB::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeB::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptSprite::ExposeTint::ExposeB::GetMax
int Script::ScriptSprite::ExposeTint::ExposeB::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436930 Script::ScriptSprite::ExposeTint::ExposeG::GetName
const char * Script::ScriptSprite::ExposeTint::ExposeG::GetName() { return ExposeG_GetName(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeTint::ExposeG::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeG::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeTint::ExposeG::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeG::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptSprite::ExposeTint::ExposeG::GetMax
int Script::ScriptSprite::ExposeTint::ExposeG::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436920 Script::ScriptSprite::ExposeTint::ExposeR::GetName
const char * Script::ScriptSprite::ExposeTint::ExposeR::GetName() { return ExposeR_GetName(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeTint::ExposeR::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeR::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeTint::ExposeR::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::ExposeR::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b4e0 Script::ScriptSprite::ExposeTint::ExposeR::GetMax
int Script::ScriptSprite::ExposeTint::ExposeR::GetMax() { return ExposeColorComponent_GetMax(); }
// FOLDED: 0x436a50 Script::TwoDMeasurement::ExposeX::GetType
Script::ScriptReference * Script::TwoDMeasurement::ExposeX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::TwoDMeasurement::ExposeX::GetBaseType
Script::ScriptReference * Script::TwoDMeasurement::ExposeX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::TwoDMeasurement::ExposeX::GetMax
int Script::TwoDMeasurement::ExposeX::GetMax() { return 0; }
// FOLDED: 0x436a50 Script::TwoDMeasurement::ExposeY::GetType
Script::ScriptReference * Script::TwoDMeasurement::ExposeY::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::TwoDMeasurement::ExposeY::GetBaseType
Script::ScriptReference * Script::TwoDMeasurement::ExposeY::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::TwoDMeasurement::ExposeY::GetMax
int Script::TwoDMeasurement::ExposeY::GetMax() { return 0; }

// FOLDED: 0x4166e0 Script::ScriptSprite::ExposeLocation::ExposeX::GetName
const char * Script::ScriptSprite::ExposeLocation::ExposeX::GetName() { return "X"; }
// FOLDED: 0x4166f0 Script::ScriptSprite::ExposeLocation::ExposeY::GetName
const char * Script::ScriptSprite::ExposeLocation::ExposeY::GetName() { return "Y"; }
// FOLDED: 0x416630 Script::ScriptSprite::ExposeLocation::ExposeY::GetFloat
float Script::ScriptSprite::ExposeLocation::ExposeY::GetFloat(Script::ScriptReference *owner) { return (float)(short)GetInt(owner); }
// FOLDED: 0x4166e0 Script::ScriptSprite::ExposePixelExtents::ExposeX::GetName
const char * Script::ScriptSprite::ExposePixelExtents::ExposeX::GetName() { return "X"; }
// FOLDED: 0x416630 Script::ScriptSprite::ExposePixelExtents::ExposeX::GetFloat
float Script::ScriptSprite::ExposePixelExtents::ExposeX::GetFloat(Script::ScriptReference *owner) { return (float)(short)GetInt(owner); }
// FOLDED: 0x4166f0 Script::ScriptSprite::ExposePixelExtents::ExposeY::GetName
const char * Script::ScriptSprite::ExposePixelExtents::ExposeY::GetName() { return "Y"; }
// FOLDED: 0x416630 Script::ScriptSprite::ExposePixelExtents::ExposeY::GetFloat
float Script::ScriptSprite::ExposePixelExtents::ExposeY::GetFloat(Script::ScriptReference *owner) { return (float)(short)GetInt(owner); }
// FOLDED: 0x416630 Script::ScriptSprite::ExposeScales::ExposeX::GetFloat
float Script::ScriptSprite::ExposeScales::ExposeX::GetFloat(Script::ScriptReference *owner) { return (float)(short)GetInt(owner); }
// FOLDED: 0x416630 Script::ScriptSprite::ExposeScales::ExposeY::GetFloat
float Script::ScriptSprite::ExposeScales::ExposeY::GetFloat(Script::ScriptReference *owner) { return (float)(short)GetInt(owner); }
// FOLDED: 0x416630 Script::TwoDMeasurement::ExposeX::GetFloat
float Script::TwoDMeasurement::ExposeX::GetFloat(Script::ScriptReference *owner) { return (float)(short)GetInt(owner); }
// FOLDED: 0x416630 Script::TwoDMeasurement::ExposeY::GetFloat
float Script::TwoDMeasurement::ExposeY::GetFloat(Script::ScriptReference *owner) { return (float)(short)GetInt(owner); }

// FOLDED: 0x403e90 Script::ActorAttachment::ExposeLocalFacing::GetType
Script::ScriptReference * Script::ActorAttachment::ExposeLocalFacing::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ActorAttachment::ExposeLocalFacing::GetBaseType
Script::ScriptReference * Script::ActorAttachment::ExposeLocalFacing::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ActorAttachment::ExposeLocalFacing::GetMax
int Script::ActorAttachment::ExposeLocalFacing::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ActorAttachment::ExposeLocalFacing::GetValueType
int Script::ActorAttachment::ExposeLocalFacing::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ActorAttachment::ExposeLocalTilt::GetType
Script::ScriptReference * Script::ActorAttachment::ExposeLocalTilt::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ActorAttachment::ExposeLocalTilt::GetBaseType
Script::ScriptReference * Script::ActorAttachment::ExposeLocalTilt::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ActorAttachment::ExposeLocalTilt::GetMax
int Script::ActorAttachment::ExposeLocalTilt::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ActorAttachment::ExposeLocalTilt::GetValueType
int Script::ActorAttachment::ExposeLocalTilt::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ExposeAnimIndex::GetType
Script::ScriptReference * Script::ExposeAnimIndex::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ExposeAnimIndex::GetBaseType
Script::ScriptReference * Script::ExposeAnimIndex::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4386a0 Script::ExposeAnimIndex::GetMax
int Script::ExposeAnimIndex::GetMax() { return AnimMapMemberCount(); }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposeActuator1Level::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeActuator1Level::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposeActuator1Level::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeActuator1Level::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptController::ScriptRumble::ExposeActuator1Level::GetMax
int Script::ScriptController::ScriptRumble::ExposeActuator1Level::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposeActuator2Level::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeActuator2Level::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposeActuator2Level::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeActuator2Level::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptController::ScriptRumble::ExposeActuator2Level::GetMax
int Script::ScriptController::ScriptRumble::ExposeActuator2Level::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposeAttenuation::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeAttenuation::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposeAttenuation::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeAttenuation::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposeDuration::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeDuration::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposeDuration::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeDuration::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptController::ScriptRumble::ExposeDuration::GetValueType
int Script::ScriptController::ScriptRumble::ExposeDuration::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptController::ScriptRumble::ExposeRadius::GetType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeRadius::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptController::ScriptRumble::ExposeRadius::GetBaseType
Script::ScriptReference * Script::ScriptController::ScriptRumble::ExposeRadius::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptController::ScriptRumble::ExposeRadius::GetValueType
int Script::ScriptController::ScriptRumble::ExposeRadius::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardAutoLoad::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoLoad::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardAutoLoad::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoLoad::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42cbe0 Script::ScriptMemCard::ExposeMemCardAutoLoad::GetMax
int Script::ScriptMemCard::ExposeMemCardAutoLoad::GetMax() { return 2; }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardAutoSave::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoSave::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardAutoSave::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardAutoSave::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42cbe0 Script::ScriptMemCard::ExposeMemCardAutoSave::GetMax
int Script::ScriptMemCard::ExposeMemCardAutoSave::GetMax() { return 2; }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardCurrentSlot::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardCurrentSlot::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardCurrentSlot::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardCurrentSlot::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetKind
int Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptMemCard::ExposeMemCardFreeBlocks::SetValue
void Script::ScriptMemCard::ExposeMemCardFreeBlocks::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardStatus::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardStatus::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardStatus::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardStatus::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptMemCard::ExposeMemCardUnplugged::GetType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardUnplugged::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptMemCard::ExposeMemCardUnplugged::GetBaseType
Script::ScriptReference * Script::ScriptMemCard::ExposeMemCardUnplugged::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptMemCard::ExposeMemCardUnplugged::GetKind
int Script::ScriptMemCard::ExposeMemCardUnplugged::GetKind() { return 0; }
// FOLDED: 0x43efe0 Script::ScriptMemCard::ExposeMemCardUnplugged::GetMax
int Script::ScriptMemCard::ExposeMemCardUnplugged::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4025a0 Script::ScriptMemCard::ExposeMemCardUnplugged::SetValue
void Script::ScriptMemCard::ExposeMemCardUnplugged::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeAnimationDirection::GetType
Script::ScriptReference * Script::ScriptProp::ExposeAnimationDirection::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeAnimationDirection::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeAnimationDirection::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptProp::ExposeAnimationDirection::GetMax
int Script::ScriptProp::ExposeAnimationDirection::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeAnimationRate::GetType
Script::ScriptReference * Script::ScriptProp::ExposeAnimationRate::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeAnimationRate::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeAnimationRate::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeCurrentFrame::GetType
Script::ScriptReference * Script::ScriptProp::ExposeCurrentFrame::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeCurrentFrame::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeCurrentFrame::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeLoopEnable::GetType
Script::ScriptReference * Script::ScriptProp::ExposeLoopEnable::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeLoopEnable::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeLoopEnable::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptProp::ExposeLoopEnable::GetMax
int Script::ScriptProp::ExposeLoopEnable::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeTotalFrames::GetType
Script::ScriptReference * Script::ScriptProp::ExposeTotalFrames::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeTotalFrames::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeTotalFrames::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptProp::ExposeTotalFrames::GetKind
int Script::ScriptProp::ExposeTotalFrames::GetKind() { return 0; }
// FOLDED: 0x4025a0 Script::ScriptProp::ExposeTotalFrames::SetValue
void Script::ScriptProp::ExposeTotalFrames::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptProp::ExposeVisibility::GetType
Script::ScriptReference * Script::ScriptProp::ExposeVisibility::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptProp::ExposeVisibility::GetBaseType
Script::ScriptReference * Script::ScriptProp::ExposeVisibility::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptProp::ExposeVisibility::GetMax
int Script::ScriptProp::ExposeVisibility::GetMax() { return Script_AlwaysTrue(); }

struct AnimIndexName { const char *GetNameText(); };   // 0x438540 (script/animindexname.cpp)
// FOLDED: 0x438540 Script::ExposeAnimIndex::GetName
const char * Script::ExposeAnimIndex::GetName() { return ((AnimIndexName *)this)->GetNameText(); }
// FOLDED: 0x431780 Script::ScriptMemCard::ExposeMemCardCurrentSlot::GetMax
int Script::ScriptMemCard::ExposeMemCardCurrentSlot::GetMax() { return 5; }
// FOLDED: 0x446250 Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetInt
int Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetInt(Script::ScriptReference *owner) { return 0xf4240; }

// FOLDED: 0x438950 Script::ScriptCamera::ExposePosPointer::GetType
Script::ScriptReference *Script::ScriptCamera::ExposePosPointer::GetType() { return TFBType_User(); }   // UserReference::GetType
// FOLDED: 0x43aa60 Script::ScriptCamera::ExposePosPointer::GetBaseType
Script::ScriptReference *Script::ScriptCamera::ExposePosPointer::GetBaseType() { return TFBType_Actor(); }   // ScriptActor::GetType
// FOLDED: 0x42def0 Script::ScriptCamera::ExposePosPointer::GetKind
int Script::ScriptCamera::ExposePosPointer::GetKind() { return GetBaseType()->GetKind(); }   // ProtoSlot6::GetKind (script/actorprotos.cpp)
// FOLDED: 0x4390e0 Script::ScriptCamera::ExposeFOV::GetName
char const * Script::ScriptCamera::ExposeFOV::GetName() { return ExposeFOV_GetName(); }
// FOLDED: 0x403e90 Script::ScriptCamera::ExposeFOV::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeFOV::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeFOV::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeFOV::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x4390f0 Script::ScriptCamera::ExposeFOV::GetMax
int Script::ScriptCamera::ExposeFOV::GetMax() { return ExposeFOV_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeFOV::GetValueType
int Script::ScriptCamera::ExposeFOV::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ScriptCamera::ExposeFacing::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeFacing::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeFacing::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeFacing::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposeFacing::GetMax
int Script::ScriptCamera::ExposeFacing::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeFacing::GetValueType
int Script::ScriptCamera::ExposeFacing::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeHorzFollow::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeHorzFollow::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeHorzFollow::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeHorzFollow::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptCamera::ExposeHorzFollow::GetMax
int Script::ScriptCamera::ExposeHorzFollow::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeLazy::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeLazy::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeLazy::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeLazy::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptCamera::ExposeLazy::GetMax
int Script::ScriptCamera::ExposeLazy::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x438d30 Script::ScriptCamera::ExposePercentComplete::GetName
const char * Script::ScriptCamera::ExposePercentComplete::GetName() { return ExposePercentComplete_GetName(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposePercentComplete::GetType
Script::ScriptReference * Script::ScriptCamera::ExposePercentComplete::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposePercentComplete::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposePercentComplete::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposePercentComplete::GetKind
int Script::ScriptCamera::ExposePercentComplete::GetKind() { return 0; }
// FOLDED: 0x42fbc0 Script::ScriptCamera::ExposePercentComplete::GetMax
int Script::ScriptCamera::ExposePercentComplete::GetMax() { return 100; }
// FOLDED: 0x4025a0 Script::ScriptCamera::ExposePercentComplete::SetValue
void Script::ScriptCamera::ExposePercentComplete::SetValue(int /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposePitchControl::GetType
Script::ScriptReference * Script::ScriptCamera::ExposePitchControl::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposePitchControl::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposePitchControl::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptCamera::ExposePitchControl::GetMax
int Script::ScriptCamera::ExposePitchControl::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x403e90 Script::ScriptCamera::ExposePosHeadingOffs::GetType
Script::ScriptReference * Script::ScriptCamera::ExposePosHeadingOffs::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposePosHeadingOffs::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposePosHeadingOffs::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposePosHeadingOffs::GetMax
int Script::ScriptCamera::ExposePosHeadingOffs::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposePosHeadingOffs::GetValueType
int Script::ScriptCamera::ExposePosHeadingOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposePosHeadingRel::GetType
Script::ScriptReference * Script::ScriptCamera::ExposePosHeadingRel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposePosHeadingRel::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposePosHeadingRel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptCamera::ExposePosHeadingRel::GetMax
int Script::ScriptCamera::ExposePosHeadingRel::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposePosHorzOffs::GetType
Script::ScriptReference * Script::ScriptCamera::ExposePosHorzOffs::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposePosHorzOffs::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposePosHorzOffs::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposePosHorzOffs::GetValueType
int Script::ScriptCamera::ExposePosHorzOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposePosVertOffs::GetType
Script::ScriptReference * Script::ScriptCamera::ExposePosVertOffs::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposePosVertOffs::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposePosVertOffs::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposePosVertOffs::GetMax
int Script::ScriptCamera::ExposePosVertOffs::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposePosVertOffs::GetValueType
int Script::ScriptCamera::ExposePosVertOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x439170 Script::ScriptCamera::ExposeRoll::GetName
const char * Script::ScriptCamera::ExposeRoll::GetName() { return ExposeRoll_GetName(); }
// FOLDED: 0x403e90 Script::ScriptCamera::ExposeRoll::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeRoll::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeRoll::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeRoll::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposeRoll::GetMax
int Script::ScriptCamera::ExposeRoll::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeRoll::GetValueType
int Script::ScriptCamera::ExposeRoll::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4390b0 Script::ScriptCamera::ExposeTgtHeadingOffs::GetName
char const * Script::ScriptCamera::ExposeTgtHeadingOffs::GetName() { return ExposeLookAtHeadingOffs_GetName(); }
// FOLDED: 0x403e90 Script::ScriptCamera::ExposeTgtHeadingOffs::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtHeadingOffs::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeTgtHeadingOffs::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtHeadingOffs::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposeTgtHeadingOffs::GetMax
int Script::ScriptCamera::ExposeTgtHeadingOffs::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeTgtHeadingOffs::GetValueType
int Script::ScriptCamera::ExposeTgtHeadingOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x438ca0 Script::ScriptCamera::ExposeTgtHeadingRel::GetName
char const * Script::ScriptCamera::ExposeTgtHeadingRel::GetName() { return ExposeLookAtHeadingRel_GetName(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeTgtHeadingRel::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtHeadingRel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeTgtHeadingRel::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtHeadingRel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptCamera::ExposeTgtHeadingRel::GetMax
int Script::ScriptCamera::ExposeTgtHeadingRel::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x438c40 Script::ScriptCamera::ExposeTgtHorzOffs::GetName
char const * Script::ScriptCamera::ExposeTgtHorzOffs::GetName() { return ExposeLookAtHorzOffs_GetName(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeTgtHorzOffs::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtHorzOffs::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeTgtHorzOffs::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtHorzOffs::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeTgtHorzOffs::GetValueType
int Script::ScriptCamera::ExposeTgtHorzOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x438c70 Script::ScriptCamera::ExposeTgtVertOffs::GetName
char const * Script::ScriptCamera::ExposeTgtVertOffs::GetName() { return ExposeLookAtVertOffs_GetName(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeTgtVertOffs::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtVertOffs::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeTgtVertOffs::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeTgtVertOffs::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposeTgtVertOffs::GetMax
int Script::ScriptCamera::ExposeTgtVertOffs::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeTgtVertOffs::GetValueType
int Script::ScriptCamera::ExposeTgtVertOffs::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x438d40 Script::ScriptCamera::ExposeTimeRemaining::GetName
char const * Script::ScriptCamera::ExposeTimeRemaining::GetName() { return ExposeTimeRemaining_GetName(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeTimeRemaining::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeTimeRemaining::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeTimeRemaining::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeTimeRemaining::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptCamera::ExposeTimeRemaining::GetKind
int Script::ScriptCamera::ExposeTimeRemaining::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptCamera::ExposeTimeRemaining::GetValueType
int Script::ScriptCamera::ExposeTimeRemaining::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptCamera::ExposeTimeRemaining::SetValue
void Script::ScriptCamera::ExposeTimeRemaining::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeVertFollowAt::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeVertFollowAt::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeVertFollowAt::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeVertFollowAt::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x439160 Script::ScriptCamera::ExposeVertFollowAt::GetMax
int Script::ScriptCamera::ExposeVertFollowAt::GetMax() { return ExposeVertFollow_GetMax(); }
// FOLDED: 0x436a50 Script::ScriptCamera::ExposeVertFollowFrom::GetType
Script::ScriptReference * Script::ScriptCamera::ExposeVertFollowFrom::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptCamera::ExposeVertFollowFrom::GetBaseType
Script::ScriptReference * Script::ScriptCamera::ExposeVertFollowFrom::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x439160 Script::ScriptCamera::ExposeVertFollowFrom::GetMax
int Script::ScriptCamera::ExposeVertFollowFrom::GetMax() { return ExposeVertFollow_GetMax(); }

// FOLDED: 0x435db0 Script::ScriptActor::ExposeActivationRange::GetName
const char * Script::ScriptActor::ExposeActivationRange::GetName() { return ExposeActivationRange_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeActivationRange::GetType
Script::ScriptReference * Script::ScriptActor::ExposeActivationRange::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeActivationRange::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeActivationRange::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeActivationRange::GetValueType
int Script::ScriptActor::ExposeActivationRange::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeAirAccel::GetType
Script::ScriptReference * Script::ScriptActor::ExposeAirAccel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeAirAccel::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeAirAccel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeAirAccel::GetValueType
int Script::ScriptActor::ExposeAirAccel::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeAirDecel::GetType
Script::ScriptReference * Script::ScriptActor::ExposeAirDecel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeAirDecel::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeAirDecel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeAirDecel::GetValueType
int Script::ScriptActor::ExposeAirDecel::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeAirSpeed::GetType
Script::ScriptReference * Script::ScriptActor::ExposeAirSpeed::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeAirSpeed::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeAirSpeed::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeAirSpeed::GetValueType
int Script::ScriptActor::ExposeAirSpeed::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeAirTurnRate::GetType
Script::ScriptReference * Script::ScriptActor::ExposeAirTurnRate::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeAirTurnRate::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeAirTurnRate::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeAirTurnRate::GetValueType
int Script::ScriptActor::ExposeAirTurnRate::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeAltitude::GetType
Script::ScriptReference * Script::ScriptActor::ExposeAltitude::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeAltitude::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeAltitude::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeAltitude::GetKind
int Script::ScriptActor::ExposeAltitude::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeAltitude::GetValueType
int Script::ScriptActor::ExposeAltitude::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeAltitude::SetValue
void Script::ScriptActor::ExposeAltitude::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeAnimationRate::GetType
Script::ScriptReference * Script::ScriptActor::ExposeAnimationRate::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeAnimationRate::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeAnimationRate::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeAnimationRate::GetMax
int Script::ScriptActor::ExposeAnimationRate::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeAnimationRate::GetValueType
int Script::ScriptActor::ExposeAnimationRate::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x407ad0 Script::ScriptActor::ExposeBlipColor::GetValueType
int Script::ScriptActor::ExposeBlipColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeBounceRestitution::GetType
Script::ScriptReference * Script::ScriptActor::ExposeBounceRestitution::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeBounceRestitution::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeBounceRestitution::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeBounceRestitution::GetMax
int Script::ScriptActor::ExposeBounceRestitution::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeBounceRestitution::GetValueType
int Script::ScriptActor::ExposeBounceRestitution::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeCoastFrames::GetType
Script::ScriptReference * Script::ScriptActor::ExposeCoastFrames::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeCoastFrames::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeCoastFrames::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeConeAngle::GetType
Script::ScriptReference * Script::ScriptActor::ExposeConeAngle::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeConeAngle::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeConeAngle::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x436b00 Script::ScriptActor::ExposeConeAngle::GetMax
int Script::ScriptActor::ExposeConeAngle::GetMax() { return ExposeConeAngle_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeConeAngle::GetValueType
int Script::ScriptActor::ExposeConeAngle::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x407ad0 Script::ScriptActor::ExposeConeColor::GetValueType
int Script::ScriptActor::ExposeConeColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeConeLength::GetType
Script::ScriptReference * Script::ScriptActor::ExposeConeLength::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeConeLength::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeConeLength::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeConeLength::GetValueType
int Script::ScriptActor::ExposeConeLength::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x435dc0 Script::ScriptActor::ExposeDeactivationRange::GetName
const char * Script::ScriptActor::ExposeDeactivationRange::GetName() { return ExposeDeactivationRange_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeDeactivationRange::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDeactivationRange::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeDeactivationRange::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDeactivationRange::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeDeactivationRange::GetValueType
int Script::ScriptActor::ExposeDeactivationRange::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeDownAccel::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDownAccel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeDownAccel::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDownAccel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeDownAccel::GetValueType
int Script::ScriptActor::ExposeDownAccel::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeDownSpeed::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDownSpeed::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeDownSpeed::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDownSpeed::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeDownSpeed::GetValueType
int Script::ScriptActor::ExposeDownSpeed::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeFacingObsolete::GetType
Script::ScriptReference * Script::ScriptActor::ExposeFacingObsolete::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeFacingObsolete::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeFacingObsolete::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeFacingObsolete::GetKind
int Script::ScriptActor::ExposeFacingObsolete::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeFacingObsolete::GetMax
int Script::ScriptActor::ExposeFacingObsolete::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeFacingObsolete::GetValueType
int Script::ScriptActor::ExposeFacingObsolete::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeFacingObsolete::SetValue
void Script::ScriptActor::ExposeFacingObsolete::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeFadePercent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeFadePercent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeFadePercent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeFadePercent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x42fbc0 Script::ScriptActor::ExposeFadePercent::GetMax
int Script::ScriptActor::ExposeFadePercent::GetMax() { return 100; }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeGroundAccel::GetType
Script::ScriptReference * Script::ScriptActor::ExposeGroundAccel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeGroundAccel::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeGroundAccel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeGroundAccel::GetValueType
int Script::ScriptActor::ExposeGroundAccel::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeGroundDecel::GetType
Script::ScriptReference * Script::ScriptActor::ExposeGroundDecel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeGroundDecel::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeGroundDecel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeGroundDecel::GetValueType
int Script::ScriptActor::ExposeGroundDecel::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeGroundSpeed::GetType
Script::ScriptReference * Script::ScriptActor::ExposeGroundSpeed::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeGroundSpeed::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeGroundSpeed::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeGroundSpeed::GetValueType
int Script::ScriptActor::ExposeGroundSpeed::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeGroundTilter::GetType
Script::ScriptReference * Script::ScriptActor::ExposeGroundTilter::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeGroundTilter::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeGroundTilter::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeGroundTilter::GetMax
int Script::ScriptActor::ExposeGroundTilter::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeGroundTurnRate::GetType
Script::ScriptReference * Script::ScriptActor::ExposeGroundTurnRate::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeGroundTurnRate::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeGroundTurnRate::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeGroundTurnRate::GetValueType
int Script::ScriptActor::ExposeGroundTurnRate::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeHeadingObsolete::GetType
Script::ScriptReference * Script::ScriptActor::ExposeHeadingObsolete::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeHeadingObsolete::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeHeadingObsolete::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeHeadingObsolete::GetKind
int Script::ScriptActor::ExposeHeadingObsolete::GetKind() { return 0; }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeHeadingObsolete::GetMax
int Script::ScriptActor::ExposeHeadingObsolete::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeHeadingObsolete::GetValueType
int Script::ScriptActor::ExposeHeadingObsolete::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::ScriptActor::ExposeHeadingObsolete::SetValue
void Script::ScriptActor::ExposeHeadingObsolete::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeHitPoints::GetType
Script::ScriptReference * Script::ScriptActor::ExposeHitPoints::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeHitPoints::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeHitPoints::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeID::GetType
Script::ScriptReference * Script::ScriptActor::ExposeID::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeID::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeID::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x435fd0 Script::ScriptActor::ExposeIgnoresCutScenes::GetName
const char * Script::ScriptActor::ExposeIgnoresCutScenes::GetName() { return ExposeIgnoreCutScenes_GetName(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeIgnoresCutScenes::GetType
Script::ScriptReference * Script::ScriptActor::ExposeIgnoresCutScenes::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeIgnoresCutScenes::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeIgnoresCutScenes::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeIgnoresCutScenes::GetMax
int Script::ScriptActor::ExposeIgnoresCutScenes::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeIgnoresGroundColor::GetType
Script::ScriptReference * Script::ScriptActor::ExposeIgnoresGroundColor::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeIgnoresGroundColor::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeIgnoresGroundColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeIgnoresGroundColor::GetMax
int Script::ScriptActor::ExposeIgnoresGroundColor::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeInitialJumpVelocity::GetType
Script::ScriptReference * Script::ScriptActor::ExposeInitialJumpVelocity::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeInitialJumpVelocity::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeInitialJumpVelocity::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeInitialJumpVelocity::GetValueType
int Script::ScriptActor::ExposeInitialJumpVelocity::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeJumpDelay::GetType
Script::ScriptReference * Script::ScriptActor::ExposeJumpDelay::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeJumpDelay::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeJumpDelay::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeMeshCollidee::GetType
Script::ScriptReference * Script::ScriptActor::ExposeMeshCollidee::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeMeshCollidee::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeMeshCollidee::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeMeshCollidee::GetMax
int Script::ScriptActor::ExposeMeshCollidee::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeMeshCollider::GetType
Script::ScriptReference * Script::ScriptActor::ExposeMeshCollider::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeMeshCollider::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeMeshCollider::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeMeshCollider::GetMax
int Script::ScriptActor::ExposeMeshCollider::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeMoveType::GetType
Script::ScriptReference * Script::ScriptActor::ExposeMoveType::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeMoveType::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeMoveType::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeNoShadow::GetType
Script::ScriptReference * Script::ScriptActor::ExposeNoShadow::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeNoShadow::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeNoShadow::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeNoShadow::GetMax
int Script::ScriptActor::ExposeNoShadow::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeSolidCollider::GetType
Script::ScriptReference * Script::ScriptActor::ExposeSolidCollider::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeSolidCollider::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeSolidCollider::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeSolidCollider::GetMax
int Script::ScriptActor::ExposeSolidCollider::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x403e90 Script::ScriptActor::ExposeSweepAngle::GetType
Script::ScriptReference * Script::ScriptActor::ExposeSweepAngle::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeSweepAngle::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeSweepAngle::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeSweepAngle::GetMax
int Script::ScriptActor::ExposeSweepAngle::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeSweepAngle::GetValueType
int Script::ScriptActor::ExposeSweepAngle::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x407ad0 Script::ScriptActor::ExposeTintColor::GetValueType
int Script::ScriptActor::ExposeTintColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeUpAccel::GetType
Script::ScriptReference * Script::ScriptActor::ExposeUpAccel::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeUpAccel::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeUpAccel::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeUpAccel::GetValueType
int Script::ScriptActor::ExposeUpAccel::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeUpSpeed::GetType
Script::ScriptReference * Script::ScriptActor::ExposeUpSpeed::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeUpSpeed::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeUpSpeed::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeUpSpeed::GetValueType
int Script::ScriptActor::ExposeUpSpeed::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeVisibleOnRadar::GetType
Script::ScriptReference * Script::ScriptActor::ExposeVisibleOnRadar::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeVisibleOnRadar::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeVisibleOnRadar::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeVisibleOnRadar::GetMax
int Script::ScriptActor::ExposeVisibleOnRadar::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeWallClimber::GetType
Script::ScriptReference * Script::ScriptActor::ExposeWallClimber::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeWallClimber::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeWallClimber::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeWallClimber::GetMax
int Script::ScriptActor::ExposeWallClimber::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeWallCollision::GetType
Script::ScriptReference * Script::ScriptActor::ExposeWallCollision::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeWallCollision::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeWallCollision::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptActor::ExposeWallCollision::GetMax
int Script::ScriptActor::ExposeWallCollision::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeWeight::GetType
Script::ScriptReference * Script::ScriptActor::ExposeWeight::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeWeight::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeWeight::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeWeight::GetValueType
int Script::ScriptActor::ExposeWeight::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeXExtent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeXExtent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeXExtent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeXExtent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeXExtent::GetValueType
int Script::ScriptActor::ExposeXExtent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeXOffset::GetType
Script::ScriptReference * Script::ScriptActor::ExposeXOffset::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeXOffset::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeXOffset::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeXOffset::GetMax
int Script::ScriptActor::ExposeXOffset::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeXOffset::GetValueType
int Script::ScriptActor::ExposeXOffset::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeYExtent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeYExtent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeYExtent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeYExtent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeYExtent::GetValueType
int Script::ScriptActor::ExposeYExtent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeYOffset::GetType
Script::ScriptReference * Script::ScriptActor::ExposeYOffset::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeYOffset::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeYOffset::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeYOffset::GetMax
int Script::ScriptActor::ExposeYOffset::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeYOffset::GetValueType
int Script::ScriptActor::ExposeYOffset::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeZExtent::GetType
Script::ScriptReference * Script::ScriptActor::ExposeZExtent::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeZExtent::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeZExtent::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeZExtent::GetValueType
int Script::ScriptActor::ExposeZExtent::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptActor::ExposeZOffset::GetType
Script::ScriptReference * Script::ScriptActor::ExposeZOffset::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeZOffset::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeZOffset::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeZOffset::GetMax
int Script::ScriptActor::ExposeZOffset::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptActor::ExposeZOffset::GetValueType
int Script::ScriptActor::ExposeZOffset::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x435db0 Script::ScriptParticle::ExposeActivationRange::GetName
const char * Script::ScriptParticle::ExposeActivationRange::GetName() { return ExposeActivationRange_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeActivationRange::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeActivationRange::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeActivationRange::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeActivationRange::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeActivationRange::GetValueType
int Script::ScriptParticle::ExposeActivationRange::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ScriptParticle::ExposeAngularSpreadX::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeAngularSpreadX::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeAngularSpreadX::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeAngularSpreadX::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposeAngularSpreadX::GetMax
int Script::ScriptParticle::ExposeAngularSpreadX::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeAngularSpreadX::GetValueType
int Script::ScriptParticle::ExposeAngularSpreadX::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ScriptParticle::ExposeAngularSpreadZ::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeAngularSpreadZ::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeAngularSpreadZ::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeAngularSpreadZ::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposeAngularSpreadZ::GetMax
int Script::ScriptParticle::ExposeAngularSpreadZ::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeAngularSpreadZ::GetValueType
int Script::ScriptParticle::ExposeAngularSpreadZ::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x435dc0 Script::ScriptParticle::ExposeDeactivationRange::GetName
const char * Script::ScriptParticle::ExposeDeactivationRange::GetName() { return ExposeDeactivationRange_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeDeactivationRange::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeDeactivationRange::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeDeactivationRange::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeDeactivationRange::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeDeactivationRange::GetValueType
int Script::ScriptParticle::ExposeDeactivationRange::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeEmitterSpreadX::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeEmitterSpreadX::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeEmitterSpreadX::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeEmitterSpreadX::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptParticle::ExposeEmitterSpreadX::GetMax
int Script::ScriptParticle::ExposeEmitterSpreadX::GetMax() { return ExposeParticleSpread_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeEmitterSpreadX::GetValueType
int Script::ScriptParticle::ExposeEmitterSpreadX::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeEmitterSpreadY::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeEmitterSpreadY::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeEmitterSpreadY::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeEmitterSpreadY::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptParticle::ExposeEmitterSpreadY::GetMax
int Script::ScriptParticle::ExposeEmitterSpreadY::GetMax() { return ExposeParticleSpread_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeEmitterSpreadY::GetValueType
int Script::ScriptParticle::ExposeEmitterSpreadY::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeEmitterSpreadZ::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeEmitterSpreadZ::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeEmitterSpreadZ::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeEmitterSpreadZ::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptParticle::ExposeEmitterSpreadZ::GetMax
int Script::ScriptParticle::ExposeEmitterSpreadZ::GetMax() { return ExposeParticleSpread_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeEmitterSpreadZ::GetValueType
int Script::ScriptParticle::ExposeEmitterSpreadZ::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x435fd0 Script::ScriptParticle::ExposeIgnoreCutScenes::GetName
const char * Script::ScriptParticle::ExposeIgnoreCutScenes::GetName() { return ExposeIgnoreCutScenes_GetName(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeIgnoreCutScenes::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeIgnoreCutScenes::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeIgnoreCutScenes::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeIgnoreCutScenes::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptParticle::ExposeIgnoreCutScenes::GetMax
int Script::ScriptParticle::ExposeIgnoreCutScenes::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeLifetime::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeLifetime::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeLifetime::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeLifetime::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeLifetime::GetValueType
int Script::ScriptParticle::ExposeLifetime::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeLifetimeSpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeLifetimeSpread::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeLifetimeSpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeLifetimeSpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeLifetimeSpread::GetValueType
int Script::ScriptParticle::ExposeLifetimeSpread::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeOutputRate::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeOutputRate::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeOutputRate::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeOutputRate::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b110 Script::ScriptParticle::ExposeOutputRate::GetMax
int Script::ScriptParticle::ExposeOutputRate::GetMax() { return ExposeOutputRate_GetMax(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeOutputRateSpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeOutputRateSpread::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeOutputRateSpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeOutputRateSpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptParticle::ExposeOutputRateSpread::GetMax
int Script::ScriptParticle::ExposeOutputRateSpread::GetMax() { return ExposeParticleSpread_GetMax(); }
// FOLDED: 0x407ad0 Script::ScriptParticle::ExposePrimaryEColor::GetValueType
int Script::ScriptParticle::ExposePrimaryEColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x407ad0 Script::ScriptParticle::ExposePrimaryEColorSpread::GetValueType
int Script::ScriptParticle::ExposePrimaryEColorSpread::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEDrag::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEDrag::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEDrag::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEDrag::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptParticle::ExposePrimaryEDrag::GetMax
int Script::ScriptParticle::ExposePrimaryEDrag::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimaryEDrag::GetValueType
int Script::ScriptParticle::ExposePrimaryEDrag::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEGravity::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEGravity::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEGravity::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEGravity::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposePrimaryEGravity::GetMax
int Script::ScriptParticle::ExposePrimaryEGravity::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimaryEGravity::GetValueType
int Script::ScriptParticle::ExposePrimaryEGravity::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryESize::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryESize::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryESize::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryESize::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b250 Script::ScriptParticle::ExposePrimaryESize::GetMax
int Script::ScriptParticle::ExposePrimaryESize::GetMax() { return ExposePrimaryVelocity_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimaryESize::GetValueType
int Script::ScriptParticle::ExposePrimaryESize::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryESizeSpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryESizeSpread::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryESizeSpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryESizeSpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b270 Script::ScriptParticle::ExposePrimaryESizeSpread::GetMax
int Script::ScriptParticle::ExposePrimaryESizeSpread::GetMax() { return ExposePrimarySizeSpread_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimaryESizeSpread::GetValueType
int Script::ScriptParticle::ExposePrimaryESizeSpread::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEVelocity::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEVelocity::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEVelocity::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEVelocity::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b250 Script::ScriptParticle::ExposePrimaryEVelocity::GetMax
int Script::ScriptParticle::ExposePrimaryEVelocity::GetMax() { return ExposePrimaryVelocity_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimaryEVelocity::GetValueType
int Script::ScriptParticle::ExposePrimaryEVelocity::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetMax
int Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetMax() { return ExposeParticleSpread_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetValueType
int Script::ScriptParticle::ExposePrimaryEVelocitySpread::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x407ad0 Script::ScriptParticle::ExposePrimarySColor::GetValueType
int Script::ScriptParticle::ExposePrimarySColor::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x407ad0 Script::ScriptParticle::ExposePrimarySColorSpread::GetValueType
int Script::ScriptParticle::ExposePrimarySColorSpread::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySDrag::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySDrag::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySDrag::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySDrag::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptParticle::ExposePrimarySDrag::GetMax
int Script::ScriptParticle::ExposePrimarySDrag::GetMax() { return Script_AlwaysTrue(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimarySDrag::GetValueType
int Script::ScriptParticle::ExposePrimarySDrag::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySGravity::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySGravity::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySGravity::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySGravity::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposePrimarySGravity::GetMax
int Script::ScriptParticle::ExposePrimarySGravity::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimarySGravity::GetValueType
int Script::ScriptParticle::ExposePrimarySGravity::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySSize::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySSize::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySSize::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySSize::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b250 Script::ScriptParticle::ExposePrimarySSize::GetMax
int Script::ScriptParticle::ExposePrimarySSize::GetMax() { return ExposePrimaryVelocity_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimarySSize::GetValueType
int Script::ScriptParticle::ExposePrimarySSize::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySSizeSpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySSizeSpread::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySSizeSpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySSizeSpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b270 Script::ScriptParticle::ExposePrimarySSizeSpread::GetMax
int Script::ScriptParticle::ExposePrimarySSizeSpread::GetMax() { return ExposePrimarySizeSpread_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimarySSizeSpread::GetValueType
int Script::ScriptParticle::ExposePrimarySSizeSpread::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySVelocity::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySVelocity::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySVelocity::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySVelocity::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b250 Script::ScriptParticle::ExposePrimarySVelocity::GetMax
int Script::ScriptParticle::ExposePrimarySVelocity::GetMax() { return ExposePrimaryVelocity_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimarySVelocity::GetValueType
int Script::ScriptParticle::ExposePrimarySVelocity::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposePrimarySVelocitySpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySVelocitySpread::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySVelocitySpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySVelocitySpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43b2b0 Script::ScriptParticle::ExposePrimarySVelocitySpread::GetMax
int Script::ScriptParticle::ExposePrimarySVelocitySpread::GetMax() { return ExposeParticleSpread_GetMax(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposePrimarySVelocitySpread::GetValueType
int Script::ScriptParticle::ExposePrimarySVelocitySpread::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptParticle::ExposeVisualRadius::GetType
Script::ScriptReference * Script::ScriptParticle::ExposeVisualRadius::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposeVisualRadius::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposeVisualRadius::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x4034a0 Script::ScriptParticle::ExposeVisualRadius::GetValueType
int Script::ScriptParticle::ExposeVisualRadius::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeData::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeData::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeData::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeData::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeJustification::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeJustification::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeJustification::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeJustification::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x416410 Script::ScriptSprite::ExposeLocation::GetValueType
int Script::ScriptSprite::ExposeLocation::GetValueType() { return TwoDMeasurement_GetValueType(); }
// FOLDED: 0x403e90 Script::ScriptSprite::ExposeRotation::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeRotation::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeRotation::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeRotation::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeRotation::GetMax
int Script::ScriptSprite::ExposeRotation::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::ScriptSprite::ExposeRotation::GetValueType
int Script::ScriptSprite::ExposeRotation::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x416410 Script::ScriptSprite::ExposeScales::GetValueType
int Script::ScriptSprite::ExposeScales::GetValueType() { return TwoDMeasurement_GetValueType(); }
// FOLDED: 0x407ad0 Script::ScriptSprite::ExposeTint::GetValueType
int Script::ScriptSprite::ExposeTint::GetValueType() { return ColorMeasurement_GetValueType(); }
// FOLDED: 0x436a50 Script::ScriptSprite::ExposeVisible::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeVisible::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeVisible::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeVisible::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43efe0 Script::ScriptSprite::ExposeVisible::GetMax
int Script::ScriptSprite::ExposeVisible::GetMax() { return Script_AlwaysTrue(); }

// FOLDED: 0x407aa0 Script::ScriptActor::ExposeBlipColor::GetType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeBlipColor::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeBlipColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeBlipColor::GetMax
int Script::ScriptActor::ExposeBlipColor::GetMax() { return 0; }
// FOLDED: 0x407aa0 Script::ScriptActor::ExposeConeColor::GetType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeConeColor::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeConeColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeConeColor::GetMax
int Script::ScriptActor::ExposeConeColor::GetMax() { return 0; }
// FOLDED: 0x4039f0 Script::ScriptActor::ExposeDestination::GetType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::GetType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x4039a0 Script::ScriptActor::ExposeDestination::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeDestination::GetBaseType() { return VectorMeasurement_GetType(); }
// FOLDED: 0x4039f0 Script::ScriptActor::ExposeOrigin::GetType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::GetType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x4039a0 Script::ScriptActor::ExposeOrigin::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeOrigin::GetBaseType() { return VectorMeasurement_GetType(); }
// FOLDED: 0x4039a0 Script::ScriptActor::ExposeSpeed::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeSpeed::GetBaseType() { return VectorMeasurement_GetType(); }
// FOLDED: 0x407aa0 Script::ScriptActor::ExposeTintColor::GetType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptActor::ExposeTintColor::GetBaseType
Script::ScriptReference * Script::ScriptActor::ExposeTintColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptActor::ExposeTintColor::GetMax
int Script::ScriptActor::ExposeTintColor::GetMax() { return 0; }
// FOLDED: 0x407aa0 Script::ScriptParticle::ExposePrimaryEColor::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColor::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposePrimaryEColor::GetMax
int Script::ScriptParticle::ExposePrimaryEColor::GetMax() { return 0; }
// FOLDED: 0x407aa0 Script::ScriptParticle::ExposePrimaryEColorSpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimaryEColorSpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimaryEColorSpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposePrimaryEColorSpread::GetMax
int Script::ScriptParticle::ExposePrimaryEColorSpread::GetMax() { return 0; }
// FOLDED: 0x407aa0 Script::ScriptParticle::ExposePrimarySColor::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColor::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColor::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposePrimarySColor::GetMax
int Script::ScriptParticle::ExposePrimarySColor::GetMax() { return 0; }
// FOLDED: 0x407aa0 Script::ScriptParticle::ExposePrimarySColorSpread::GetType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptParticle::ExposePrimarySColorSpread::GetBaseType
Script::ScriptReference * Script::ScriptParticle::ExposePrimarySColorSpread::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptParticle::ExposePrimarySColorSpread::GetMax
int Script::ScriptParticle::ExposePrimarySColorSpread::GetMax() { return 0; }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeLocation::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeLocation::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeLocation::GetMax
int Script::ScriptSprite::ExposeLocation::GetMax() { return 0; }
// FOLDED: 0x43f5f0 Script::ScriptSprite::ExposeScales::GetName
char const * Script::ScriptSprite::ExposeScales::GetName() { return ExposeScales_GetName(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeScales::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeScales::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeScales::GetMax
int Script::ScriptSprite::ExposeScales::GetMax() { return 0; }
// FOLDED: 0x407aa0 Script::ScriptSprite::ExposeTint::GetType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::GetType() { return ColorMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::ScriptSprite::ExposeTint::GetBaseType
Script::ScriptReference * Script::ScriptSprite::ExposeTint::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::ScriptSprite::ExposeTint::GetMax
int Script::ScriptSprite::ExposeTint::GetMax() { return 0; }

// ---- (set members, Position bases, TwoD types)
// FOLDED: 0x416480 Script::ScriptActor::ExposeClones::GetName
const char *Script::ScriptActor::ExposeClones::GetName() { return ((Script::ScriptSprite::ExposeClones *)this)->Script::ScriptSprite::ExposeClones::GetName(); }   // cross-class fold
// FOLDED: 0x43aa60 Script::ScriptActor::ExposeClones::GetBaseType
Script::ScriptReference *Script::ScriptActor::ExposeClones::GetBaseType() { return TFBType_Actor(); }   // ScriptActor::GetType
// FOLDED: 0x43b150 Script::ScriptActor::ExposeClones::Deref
Script::ScriptReference *Script::ScriptActor::ExposeClones::Deref(Script::ScriptReference *owner, int flags) { return &static_cast<Script::Placement *>(owner)->m_20; }
// FOLDED: 0x416480 Script::ScriptParticle::ExposeClones::GetName
const char *Script::ScriptParticle::ExposeClones::GetName() { return ((Script::ScriptSprite::ExposeClones *)this)->Script::ScriptSprite::ExposeClones::GetName(); }   // cross-class fold
// FOLDED: 0x43b870 Script::ScriptParticle::ExposeClones::GetBaseType
Script::ScriptReference *Script::ScriptParticle::ExposeClones::GetBaseType() { return ScriptParticle_GetType(); }
// FOLDED: 0x43b150 Script::ScriptParticle::ExposeClones::Deref
Script::ScriptReference *Script::ScriptParticle::ExposeClones::Deref(Script::ScriptReference *owner, int flags) { return &static_cast<Script::Placement *>(owner)->m_20; }
// FOLDED: 0x4039e0 Script::ActorAttachment::AttacherSet::GetBaseType
Script::ScriptReference *Script::ActorAttachment::AttacherSet::GetBaseType() { return Placement_GetType(); }
// FOLDED: 0x42ae10 Script::ScriptActor::ExposeDestination::SetVector
void Script::ScriptActor::ExposeDestination::SetVector(const Vec3 *v, Script::ScriptReference *owner) { SetPosition(v, owner); }
// FOLDED: 0x43a6a0 Script::ScriptActor::ExposeMoveType::GetMax
int Script::ScriptActor::ExposeMoveType::GetMax() { return 4; }   // the game's MessageCheck::IsBlock
// FOLDED: 0x416400 Script::ScriptSprite::ExposeLocation::GetType
Script::ScriptReference *Script::ScriptSprite::ExposeLocation::GetType() { return TFBType_TwoD(); }   // cross-class fold of TwoDMeasurement::GetType (0x416400)
// FOLDED: 0x416400 Script::ScriptSprite::ExposeScales::GetType
Script::ScriptReference *Script::ScriptSprite::ExposeScales::GetType() { return TFBType_TwoD(); }   // cross-class fold of TwoDMeasurement::GetType (0x416400)

// FOLDED: 0x430b70 Script::PositionMeasurement::ExposeHeading::GetName
const char * Script::PositionMeasurement::ExposeHeading::GetName() { return ExposeHeadingFromMe_GetName(); }
// FOLDED: 0x430b80 Script::PositionMeasurement::ExposePitch::GetName
const char * Script::PositionMeasurement::ExposePitch::GetName() { return ExposePitchFromMe_GetName(); }
// FOLDED: 0x438d30 Script::ScriptVertAnim::ExposePercentComplete::GetName
const char * Script::ScriptVertAnim::ExposePercentComplete::GetName() { return ExposePercentComplete_GetName(); }
// FOLDED: 0x438d40 Script::ScriptVertAnim::ExposeTimeRemaining::GetName
const char * Script::ScriptVertAnim::ExposeTimeRemaining::GetName() { return ExposeTimeRemaining_GetName(); }

// FOLDED: 0x430340 Script::PositionMeasurement::ExposeFacing::GetName
const char * Script::PositionMeasurement::ExposeFacing::GetName() { return ExposeFacing_GetName(); }
// FOLDED: 0x403e90 Script::PositionMeasurement::ExposeFacing::GetType
Script::ScriptReference * Script::PositionMeasurement::ExposeFacing::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::PositionMeasurement::ExposeFacing::GetBaseType
Script::ScriptReference * Script::PositionMeasurement::ExposeFacing::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::PositionMeasurement::ExposeFacing::GetMax
int Script::PositionMeasurement::ExposeFacing::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::PositionMeasurement::ExposeFacing::GetValueType
int Script::PositionMeasurement::ExposeFacing::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x439170 Script::PositionMeasurement::ExposeRoll::GetName
const char * Script::PositionMeasurement::ExposeRoll::GetName() { return ExposeRoll_GetName(); }
// FOLDED: 0x403e90 Script::PositionMeasurement::ExposeRoll::GetType
Script::ScriptReference * Script::PositionMeasurement::ExposeRoll::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::PositionMeasurement::ExposeRoll::GetBaseType
Script::ScriptReference * Script::PositionMeasurement::ExposeRoll::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::PositionMeasurement::ExposeRoll::GetMax
int Script::PositionMeasurement::ExposeRoll::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::PositionMeasurement::ExposeRoll::GetValueType
int Script::PositionMeasurement::ExposeRoll::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x430380 Script::PositionMeasurement::ExposeTilt::GetName
const char * Script::PositionMeasurement::ExposeTilt::GetName() { return ExposeTilt_GetName(); }
// FOLDED: 0x403e90 Script::PositionMeasurement::ExposeTilt::GetType
Script::ScriptReference * Script::PositionMeasurement::ExposeTilt::GetType() { return AngleMeasurement_GetType(); }
// FOLDED: 0x436a50 Script::PositionMeasurement::ExposeTilt::GetBaseType
Script::ScriptReference * Script::PositionMeasurement::ExposeTilt::GetBaseType() { return IntMeasurement_GetType(); }
// FOLDED: 0x43e6f0 Script::PositionMeasurement::ExposeTilt::GetMax
int Script::PositionMeasurement::ExposeTilt::GetMax() { return 0; }
// FOLDED: 0x4034a0 Script::PositionMeasurement::ExposeTilt::GetValueType
int Script::PositionMeasurement::ExposeTilt::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x42feb0 Script::PositionMeasurement::ExposeViewPyramidDist::GetName
const char * Script::PositionMeasurement::ExposeViewPyramidDist::GetName() { return ExposeViewPyramidDist_GetName(); }
// FOLDED: 0x436a50 Script::PositionMeasurement::ExposeViewPyramidDist::GetType
Script::ScriptReference * Script::PositionMeasurement::ExposeViewPyramidDist::GetType() { return IntMeasurement_GetType(); }
// FOLDED: 0x402b70 Script::PositionMeasurement::ExposeViewPyramidDist::GetBaseType
Script::ScriptReference * Script::PositionMeasurement::ExposeViewPyramidDist::GetBaseType() { return ValueInterface_GetType(); }
// FOLDED: 0x43e6f0 Script::PositionMeasurement::ExposeViewPyramidDist::GetKind
int Script::PositionMeasurement::ExposeViewPyramidDist::GetKind() { return 0; }
// FOLDED: 0x4034a0 Script::PositionMeasurement::ExposeViewPyramidDist::GetValueType
int Script::PositionMeasurement::ExposeViewPyramidDist::GetValueType() { return FloatMeasurement_GetValueType(); }
// FOLDED: 0x4025a0 Script::PositionMeasurement::ExposeViewPyramidDist::SetValue
void Script::PositionMeasurement::ExposeViewPyramidDist::SetValue(float /*a1*/, Script::ScriptReference * /*a2*/) {  }

