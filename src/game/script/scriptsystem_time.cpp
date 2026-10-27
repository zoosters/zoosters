#include "script/script.h"
#include "script/types/scriptsystem.h"
using namespace Script;


// GLOBAL: 0x5d4ca0 Script::ScriptSystem::ExposeScriptTime::vftable
// 0x43fa90
ScriptSystem::ExposeScriptTime::ExposeScriptTime()
{
	m_timeMs = 0;
	((ScriptReference *)this)->ResolveSymbol(TFBType_System(), 0, 0);
}
