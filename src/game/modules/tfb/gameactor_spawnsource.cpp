#include "script/runtime.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"

// 0x42d930
Script::ScriptReference *RWS::TFB::GameActor::GetSpawnSource()
{
	return m_spawnSource;
}
