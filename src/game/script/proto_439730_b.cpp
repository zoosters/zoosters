// Prototype getters (static T s; return &s;).
#include "script/actor.h"
#include "script/types/scriptactor.h"
#include "script/types/scriptmemcard.h"
#include "script/types/scriptshadow.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptcontroller.h"
#include "script/types/scriptcamera.h"
#include "script/types/scriptmathops.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptmotionblur.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptparticle.h"
#include "script/types/animmap.h"
using namespace Script;

// GLOBAL: 0x5d15a8 Script::ScriptController::ExposeActiveControllers::vftable
// 0x439730
ScriptReference *TFBMember_Controller_ActiveControllers()
{
	static ScriptController::ExposeActiveControllers s;
	return &s;
}

// GLOBAL: 0x5d3040 Script::ScriptSystem::ExposeNextLevel::vftable
// 0x43c410
ScriptReference *TFBMember_System_NextLevel()
{
	static ScriptSystem::ExposeNextLevel s;
	return &s;
}

// GLOBAL: 0x5d30a0 Script::ScriptSystem::ExposeGameTime::vftable
// 0x43c450
ScriptReference *TFBMember_System_GameTime()
{
	static ScriptSystem::ExposeGameTime s;
	return &s;
}
