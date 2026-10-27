// Script::ActorMessage (queued script message): constructor, compiler-made scalar deleting destructor, position accessor, release.
#include "script/types/actormessage.h"

namespace Script {

// GLOBAL: 0x43b3d0 Script::ActorMessage::~ActorMessage
// GLOBAL: 0x439d10 ??3ActorMessage@Script@@SAXPAX@Z
// GLOBAL: 0x5cdc08 Script::ActorMessage::vftable
// 0x42f4e0
ActorMessage::ActorMessage(intptr_t msgType, ScriptReference *receiver, const Vec3 *pos, int arg)
{
	m_pos = *pos;
	m_arg.i = arg;
	m_msgType = msgType;
	m_receiver = receiver;
	receiver->AdjustRefCount(1);
}

// 0x42f5a0

// 0x42f590
Vec3 *ActorMessage::GetVector(ScriptReference *owner)
{
	return &static_cast<ActorMessage *>(owner)->m_pos;
}

// 0x42f560
int ActorMessage::AdjustRefCount(int delta)
{
	if (delta < 0) {
		m_receiver->AdjustRefCount(-1);
		delete this;
		return 0;
	}
	return 1;
}

}  // namespace Script
