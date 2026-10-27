// Vptr-only constructors of script classes, second part.
#include "script/actor.h"
#include "script/types/scriptactor.h"
#include "script/types/scriptcontroller.h"

namespace Script {

// GLOBAL: 0x5caac8 Script::ScriptController::ScriptRumble::ExposeTimeRemaining::vftable
// 0x403bd0
ScriptController::ScriptRumble::ExposeTimeRemaining::ExposeTimeRemaining() {}

// GLOBAL: 0x5cab20 Script::ScriptController::ScriptRumble::ExposePercent5cab20::vftable
// 0x403be0
ScriptController::ScriptRumble::ExposePercent5cab20::ExposePercent5cab20() {}

// GLOBAL: 0x5cc468 Script::BoolMeasurement::vftable
// 0x4165b0
BoolMeasurement::BoolMeasurement() {}

// GLOBAL: 0x5d09f8 Script::ScriptActor::ExposeDestination::ExposeVertComponent::vftable
// 0x4388f0
ScriptActor::ExposeDestination::ExposeVertComponent::ExposeVertComponent() {}

// GLOBAL: 0x5d11c8 Script::ControllerPlayerRef::vftable
// 0x439440
ControllerPlayerRef::ControllerPlayerRef() {}

}  // namespace Script
