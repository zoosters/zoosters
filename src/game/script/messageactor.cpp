// Placement::FindMessage (find a pending message by id/sender/range).
#include "script/ops_value.h"
#include "script/ops_message.h"
#include "script/types/scriptsystem.h"
#include "script/types/actormessage.h"
#include <float.h>
using namespace Script;

ScriptReference *TFBType_Set();   // 0x43d2d0

// 0x42f260
intptr_t Placement::FindMessage(ScriptReference *msgType, ScriptValue *value, float range, Vec3 *out)
{
	unsigned now = *ScriptSystem::GetScriptTimePtr();
	ScriptReference *self = (ScriptReference *)g_scriptSelf;
	SetNode *nodeCur;
	ReferenceSet *set = &m_08;
	for (ActorMessage *m = (ActorMessage *)set->Next(1, &nodeCur); m != 0; m = (ActorMessage *)set->Next(5, &nodeCur)) {
		if (nodeCur->m_postTime >= now)
			break;
		if ((ScriptReference *)m->m_msgType != msgType)
			continue;
		ScriptReference *recv = m->m_receiver;
		if (recv->GetType() == TFBType_Set()) {   // addressed to a set: this actor must be a member
			if (((ReferenceSet *)recv)->FindNode(self) == 0)
				continue;
		} else if (recv != self) {
			continue;
		}
		if (TFB_CompareFloat(m->GetFloat(m) - m->m_arg.f, value->opArg, range)) {
			*out = *m->GetVector(m);
			return (intptr_t)m;
		}
	}
	return 0;
}

Script::ScriptReference *TFB_GetMessageSenderSet();     // the set of message-holding actors

namespace Script {

// Near miss: PostMessage differs only in the register pair: ours keeps the script time in ebp and `b` in ebx, the game the reverse.
// 0x42f660 Script::Placement::PostMessage
void Placement::PostMessage(ScriptReference *msgType, ScriptReference *receiver, Vec3 *pos, float arg)
{
	unsigned now = *ScriptSystem::GetScriptTimePtr();
	if (arg == -1.0f) {
		for (SetNode *n = m_08.m_tail; n != 0; n = n->m_prev) {
			if (n->m_postTime == now) {
				ActorMessage *m = (ActorMessage *)n->m_object;
				if (m->m_msgType == (intptr_t)msgType && m->m_receiver == receiver) {
					m->m_pos = *pos;
					return;
				}
			}
		}
		arg = 0.0f;
	}
	if (m_08.m_count == 0)
		((ReferenceSet *)TFB_GetMessageSenderSet())->Insert(this, FLT_MAX);
	SetNode *n = m_08.Insert(new ActorMessage((intptr_t)msgType, receiver, pos, *(int *)&arg), FLT_MAX);
	if (n != 0)
		n->m_postTime = now;
}


}  // namespace Script

#include "script/referenceset.h"

Script::ScriptReference *TFB_GetMessageSenderSet();        // 0x43aa90: the global set of message senders
namespace Script { void ExpireSenderMessages(); }

// Drops the expired messages (key = expiry time) of every sender and removes the senders whose set got empty.
// 0x43aaf0
void Script::ExpireSenderMessages()
{
	Script::SetNode *n = ((Script::ReferenceSet *)TFB_GetMessageSenderSet())->m_head;
	if (n) {
		unsigned now = *Script::ScriptSystem::GetScriptTimePtr();
		do {
			Script::ReferenceSet *msgs = &((Script::Placement *)n->m_object)->m_08;
			Script::SetNode *next = n->m_next;
			Script::SetNode *msg = msgs->m_head;
			while (msg && msg->m_postTime < now) {
				Script::SetNode *msgNext = msg->m_next;
				msgs->RemoveNode(msg);
				msg = msgNext;
			}
			if (!msgs->m_count)
				((Script::ReferenceSet *)TFB_GetMessageSenderSet())->RemoveNode(n);
			n = next;
		} while (n);
	}
}
