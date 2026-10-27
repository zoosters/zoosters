// Prototype getters of actor ops. Own TU: ops_actor.h clashes with ops_value.h.
#include "script/ops_actor.h"
using namespace Script;

// GLOBAL: 0x5ced1c Script::ActorPlayAnim::vftable
// 0x435480
ScriptReference *OpProto_ActorPlayAnim()
{
	static ActorPlayAnim s;
	return &s;
}
