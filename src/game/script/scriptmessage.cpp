#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"

namespace Script { void ExpireSenderMessages(); }   // 0x43aaf0: drops the expired messages

// Expires every pending message at once: the script clock reads -1 while ExpireSenderMessages runs.
// 0x43ab60
void Script::ScriptMessage::vf10()
{
	*Script::ScriptSystem::GetScriptTimePtr() = -1;
	Script::ExpireSenderMessages();
	*Script::ScriptSystem::GetScriptTimePtr() = 0;
}
