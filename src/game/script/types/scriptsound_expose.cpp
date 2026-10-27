#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"
#include "script/script.h"

namespace Script {

// GLOBAL: 0x5d3588 Script::ScriptSound::ExposeEffectsVolume::vftable
// 0x43dca0
ScriptSound::ExposeEffectsVolume::ExposeEffectsVolume()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Sound(), 0, 0);
	SetValue(100, this);
}

// GLOBAL: 0x5d35e0 Script::ScriptSound::ExposeMusicVolume::vftable
// 0x43dd10
ScriptSound::ExposeMusicVolume::ExposeMusicVolume()
{
	((ScriptReference *)this)->ResolveSymbol(TFBType_Sound(), 0, 0);
	SetValue(100, this);
}

}

using namespace Script;

// 0x43dee0
int ScriptSound_GetEffectsVolume()
{
	static ScriptSound::ExposeEffectsVolume s;
	return s.GetInt(&s);
}

// 0x43df50
int ScriptSound_GetMusicVolume()
{
	static ScriptSound::ExposeMusicVolume s;
	return s.GetInt(&s);
}

// 0x43dfc0
int ScriptSound_GetEnvironmentType()
{
	static ScriptSound::ExposeEnvironmentType s;
	return s.GetInt(&s);
}

// 0x43e030
int ScriptSound_GetEnvironmentLevel()
{
	static ScriptSound::ExposeEnvironmentLevel s;
	return s.GetInt(&s);
}

// 0x43e800
bool TFBMember_System_HardwareMetrics()
{
	static ScriptSystem::ExposeHardwareMetrics s;
	return false;
}
