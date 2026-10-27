// Vptr-only constructors of further Expose* members; the class declarations carry every override of the game vtable.
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "script/types/actorturret.h"

namespace Script {

// GLOBAL: 0x5cd924 Script::ActorTurret::ExposeLookAtPointer::vftable
// 0x42dee0
ActorTurret::ExposeLookAtPointer::ExposeLookAtPointer() {}

// GLOBAL: 0x5cd958 Script::ActorTurret::ExposeLookAtHorzOffs::vftable
// 0x42df00
ActorTurret::ExposeLookAtHorzOffs::ExposeLookAtHorzOffs() {}

// GLOBAL: 0x5cd9b8 Script::ActorTurret::ExposeLookAtVertOffs::vftable
// 0x42df10
ActorTurret::ExposeLookAtVertOffs::ExposeLookAtVertOffs() {}

// GLOBAL: 0x5cda18 Script::ActorTurret::ExposeLookAtHeadingRel::vftable
// 0x42df20
ActorTurret::ExposeLookAtHeadingRel::ExposeLookAtHeadingRel() {}

// GLOBAL: 0x5cd7c8 Script::ActorAttachment::ExposeLocalRoll::vftable
// 0x42df30
ActorAttachment::ExposeLocalRoll::ExposeLocalRoll() {}

// GLOBAL: 0x5cda78 Script::ActorTurret::ExposeLookAtHeadingOffs::vftable
// 0x42df40
ActorTurret::ExposeLookAtHeadingOffs::ExposeLookAtHeadingOffs() {}

// GLOBAL: 0x5cac50 Script::VectorMeasurement::ExposeHorzComponent::vftable
// 0x430140
VectorMeasurement::ExposeHorzComponent::ExposeHorzComponent() {}

// GLOBAL: 0x5cac08 Script::VectorMeasurement::ExposeVertComponent::vftable
// 0x430150
VectorMeasurement::ExposeVertComponent::ExposeVertComponent() {}

// GLOBAL: 0x5cace0 Script::VectorMeasurement::ExposeHeading::vftable
// 0x430ae0
VectorMeasurement::ExposeHeading::ExposeHeading() {}

// GLOBAL: 0x5cac98 Script::VectorMeasurement::ExposePitch::vftable
// 0x430af0
VectorMeasurement::ExposePitch::ExposePitch() {}

}  // namespace Script
