// Prototype getters of ops_actor.h classes; see proto_op_getters.cpp.
#include "script/ops_actor.h"
using namespace Script;

// GLOBAL: 0x5ceea0 Script::ActorRunAsPlayer::vftable
// 0x4366c0
ScriptReference *OpProto_ActorRunAsPlayer()
{
	static ActorRunAsPlayer s;
	return &s;
}
