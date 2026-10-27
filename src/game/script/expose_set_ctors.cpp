#include "script/script.h"
#include "script/types/scriptsets.h"

namespace Script {

// GLOBAL: 0x435230 Script::ExposePlayerSet::~ExposePlayerSet
// GLOBAL: 0x5d0598 Script::ExposePlayerSet::vftable
// 0x4379a0
ExposePlayerSet::ExposePlayerSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Actor(), 0, TFBType_Actor());   // the third argument is the declared type
}

// GLOBAL: 0x5d05e8 Script::ExposeNPCsSet::vftable
// GLOBAL: 0x435230 Script::ExposeNPCsSet::~ExposeNPCsSet
// 0x437a80
ExposeNPCsSet::ExposeNPCsSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Actor(), 0, TFBType_Actor());
}

// GLOBAL: 0x5d0638 Script::ExposeInactiveSet::vftable
// GLOBAL: 0x435230 Script::ExposeInactiveSet::~ExposeInactiveSet
// 0x437b60
ExposeInactiveSet::ExposeInactiveSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Actor(), 0, TFBType_Actor());
}

// GLOBAL: 0x5d0690 Script::ExposeDeadSet::vftable
// GLOBAL: 0x435230 Script::ExposeDeadSet::~ExposeDeadSet
// 0x437c40
ExposeDeadSet::ExposeDeadSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Actor(), 0, TFBType_Actor());
}

// GLOBAL: 0x5d1ee0 Script::ExposeSendersSet::vftable
// GLOBAL: 0x43a640 Script::ExposeSendersSet::~ExposeSendersSet
// 0x43a9e0
ExposeSendersSet::ExposeSendersSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Actor(), 0, TFBType_Actor());
}

// GLOBAL: 0x5d2d50 Script::_ParticleShield::ExposeActiveSet::vftable
// GLOBAL: 0x43b0b0 Script::_ParticleShield::ExposeActiveSet::~ExposeActiveSet
// 0x43b5c0
_ParticleShield::ExposeActiveSet::ExposeActiveSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Particle(), 0, TFBType_Particle());
}

// GLOBAL: 0x5d2da0 Script::_ParticleShield::ExposeInactiveSet::vftable
// GLOBAL: 0x43b0b0 Script::_ParticleShield::ExposeInactiveSet::~ExposeInactiveSet
// 0x43b630
_ParticleShield::ExposeInactiveSet::ExposeInactiveSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Particle(), 0, TFBType_Particle());
}

// GLOBAL: 0x5d2de8 Script::_ParticleShield::ExposeRemovedSet::vftable
// GLOBAL: 0x43b0b0 Script::_ParticleShield::ExposeRemovedSet::~ExposeRemovedSet
// 0x43b6a0
_ParticleShield::ExposeRemovedSet::ExposeRemovedSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Particle(), 0, TFBType_Particle());
}

// GLOBAL: 0x5d3708 Script::_SpriteShield::ExposeActiveSet::vftable
// GLOBAL: 0x43e2e0 Script::_SpriteShield::ExposeActiveSet::~ExposeActiveSet
// 0x43e400
_SpriteShield::ExposeActiveSet::ExposeActiveSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Sprite(), 0, TFBType_Sprite());
}

// GLOBAL: 0x5d3750 Script::_SpriteShield::ExposeRemovedSet::vftable
// GLOBAL: 0x43e2e0 Script::_SpriteShield::ExposeRemovedSet::~ExposeRemovedSet
// 0x43e4a0
_SpriteShield::ExposeRemovedSet::ExposeRemovedSet()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Sprite(), 0, TFBType_Sprite());
}

}  // namespace Script
