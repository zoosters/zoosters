// Prototype getters (static T s; return &s;) whose constructor is an out-of-line call (EH frame), defined elsewhere.
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptsets.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptshadow.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptmathops.h"
#include "script/types/scriptmotionblur.h"
#include "script/types/scriptmemcard.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"
using namespace Script;


// 0x43a3a0
ScriptReference *TFBMember_JumpStartedMessage()
{
	static ExposeJumpStartedMessage s;
	return &s;
}

// 0x43a460
ScriptReference *TFBMember_ActivateMessage()
{
	static ExposeActivateMessage s;
	return &s;
}

// 0x43a520
ScriptReference *TFBMember_DeactivateMessage()
{
	static ExposeDeactivateMessage s;
	return &s;
}

// 0x43a5e0
ScriptReference *TFBMember_DeathMessage()
{
	static ExposeDeathMessage s;
	return &s;
}

// 0x43aa90
ScriptReference *TFB_GetMessageSenderSet()
{
	static ExposeSendersSet s;
	return &s;
}

// 0x43b000
ScriptReference *TFBMember_HitActorMessage()
{
	static ExposeHitActorMessage s;
	return &s;
}


// 0x43b730
ReferenceSet *TFB_GetActiveParticleSet()
{
	static _ParticleShield::ExposeActiveSet s;
	return &s;
}

// 0x43b790
ReferenceSet *TFB_GetInactiveParticleSet()
{
	static _ParticleShield::ExposeInactiveSet s;
	return &s;
}

// 0x43b7f0
ReferenceSet *TFB_GetRemovedParticleSet()
{
	static _ParticleShield::ExposeRemovedSet s;
	return &s;
}


namespace Script {
// 0x43e520
ReferenceSet *TFB_GetActiveSpriteSet()
{
	static _SpriteShield::ExposeActiveSet s;
	return &s;
}

// 0x43e580
ReferenceSet *TFB_GetRemovedSpriteSet()
{
	static _SpriteShield::ExposeRemovedSet s;
	return &s;
}
}  // namespace Script

// GLOBAL: 0x43fa90 Script::ScriptSystem::ExposeScriptTime::ExposeScriptTime

// the game returns the address of the time stamp member (object + 4)
// 0x43fb40
int *ScriptSystem::GetScriptTimePtr()
{
	static ScriptSystem::ExposeScriptTime s;
	return &s.m_timeMs;
}
