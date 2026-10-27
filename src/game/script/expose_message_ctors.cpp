// Real constructors of Expose* classes (EH frame: the inlined ScriptReference base has a virtual destructor, and
// TFB_ResolveSymbol can throw).
#include "script/script.h"
#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"

namespace Script {

// GLOBAL: 0x5d1b88 Script::ExposeHitWallMessage::vftable
// 0x439d40
ExposeHitWallMessage::ExposeHitWallMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1bc0 Script::ExposeHitTriggerMessage::vftable
// 0x439e00
ExposeHitTriggerMessage::ExposeHitTriggerMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1c00 Script::ExposeWallNormalMessage::vftable
// 0x439ec0
ExposeWallNormalMessage::ExposeWallNormalMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1c38 Script::ExposeLedgeNearbyMessage::vftable
// 0x439f80
ExposeLedgeNearbyMessage::ExposeLedgeNearbyMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1c74 Script::ExposeFootPrintMessage::vftable
// 0x43a040
ExposeFootPrintMessage::ExposeFootPrintMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1cac Script::ExposeBounceNormalMessage::vftable
// 0x43a100
ExposeBounceNormalMessage::ExposeBounceNormalMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1ce8 Script::ExposeHitGroundMessage::vftable
// 0x43a1c0
ExposeHitGroundMessage::ExposeHitGroundMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1d20 Script::ExposeLeftGroundMessage::vftable
// 0x43a280
ExposeLeftGroundMessage::ExposeLeftGroundMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1d58 Script::ExposeJumpStartedMessage::vftable
// 0x43a340
ExposeJumpStartedMessage::ExposeJumpStartedMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1d94 Script::ExposeActivateMessage::vftable
// 0x43a400
ExposeActivateMessage::ExposeActivateMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1dcc Script::ExposeDeactivateMessage::vftable
// 0x43a4c0
ExposeDeactivateMessage::ExposeDeactivateMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1e08 Script::ExposeDeathMessage::vftable
// 0x43a580
ExposeDeathMessage::ExposeDeathMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d1f38 Script::ExposeHitActorMessage::vftable
// 0x43afa0
ExposeHitActorMessage::ExposeHitActorMessage()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Message(), 0, 0);
}

// GLOBAL: 0x5d3638 Script::ScriptSound::ExposeEnvironmentType::vftable
// 0x43dd80
ScriptSound::ExposeEnvironmentType::ExposeEnvironmentType()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Sound(), 0, 0);
}

// GLOBAL: 0x5d3698 Script::ScriptSound::ExposeEnvironmentLevel::vftable
// 0x43dde0
ScriptSound::ExposeEnvironmentLevel::ExposeEnvironmentLevel()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Sound(), 0, 0);
}

// GLOBAL: 0x5d3ce0 Script::ScriptSystem::ExposeHardwareMetrics::vftable
// 0x43e7a0
ScriptSystem::ExposeHardwareMetrics::ExposeHardwareMetrics()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_System(), 0, 0);
}

}  // namespace Script
