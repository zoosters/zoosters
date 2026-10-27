// Script message ops: GetName / IsBlock / Clone / CloneToPool.
#include "script/ops_message.h"
#include "script/types/scriptmessage.h"
#include "script/referenceset.h"

// GLOBAL: 0x602f98 g_compareOpNames
// GLOBAL: 0x621f14 g_scriptSelf
// GLOBAL: 0x621f10 g_scriptSender
// GLOBAL: 0x43c2a0 Script::UserReference::UserReference

namespace Script {

// 0x43a690
const char *MessageCheck::GetName()
{
	return "check message";
}

// 0x43a6a0
int MessageCheck::IsBlock()
{
	return 4;
}

// 0x43adc0
OpCode *MessageCheck::Clone()
{
	return CloneToPool(this);
}

// 0x43add0
MessageCheck *MessageCheck::CloneToPool(const MessageCheck *src)
{
	void *mem = g_scriptPool;
	MessageCheck *r = new (mem) MessageCheck(*src);
	g_scriptPool += sizeof(MessageCheck);
	return r;
}

// 0x43a6b0
const char *MessageSend::GetName()
{
	return "send message";
}

// 0x43ac40
OpCode *MessageSend::Clone()
{
	return CloneToPool(this);
}

// 0x43ac50
MessageSend *MessageSend::CloneToPool(const MessageSend *src)
{
	void *mem = g_scriptPool;
	MessageSend *r = new (mem) MessageSend(*src);
	g_scriptPool += sizeof(MessageSend);
	return r;
}

// 0x43a6c0
int MessageSend::Execute()
{
	ScriptReference *x;
	ScriptReference *y;
	ScriptReference *message;
	ScriptReference *receiver;
	ScriptReference *self = g_scriptSelf;
	if (self->IsKindOf(TFBType_Actor()) && (self = g_scriptSelf) != 0 && (message = m_message.Resolve(&x, &y, true)) != 0
		&& (receiver = m_receiver.Resolve(&y, &x, true)) != 0 && m_value.EvalFloat((float *)&y)) {
		Placement *actor = (Placement *)self;
		actor->PostMessage(message, receiver, actor->GetVector(actor), *(float *)&y);
	}
	return 1;
}

// 0x43a760
int MessageCheck::Execute()
{
	ScriptReference *y, *message;
	UserReference *saved;
	ScriptReference *self;
	void *iter;   // the set cursor (a node pointer: 8 bytes on x64, the game keeps it in an int)
	ScriptReference *xz[2];
	Vec3 vec;
	saved = (UserReference *)g_scriptSender;
	self = g_scriptSelf;
	Placement *actor = self->IsKindOf(TFBType_Actor()) ? (Placement *)self : 0;
	ScriptReference *obj = 0;
	int result;
	if (actor != 0 && (message = m_message.Resolve(&xz[0], &y, true)) != 0) {
		if (message->GetKind() == SCRIPT_KIND_VALUE) {
			obj = self;
			*(unsigned *)&y = 0x7f7fffff;
		} else {
			ScriptValue *v = &m_value;
			if (v->EvalFloat((float *)&y))
				obj = m_receiver.Resolve(&xz[1], &xz[0], true);
		}
		if (obj != 0) {
			if (obj->GetType() != TFBType_Set()) {
				if (((Placement *)obj)->FindMessage(message, &m_value, *(float *)&y, &vec) == 0)
					obj = 0;
			} else {
#define SET_CURSOR(p) ((SetNode **)(p))
				ReferenceSet *set = (ReferenceSet *)obj;   // a set object: Next is ReferenceSet's, 0x43cd00
				obj = set->Next(1, SET_CURSOR(&iter));
				while (obj != 0) {
					if (obj != self && ((Placement *)obj)->FindMessage(message, &m_value, *(float *)&y, &vec))
						break;
					obj = set->Next(5, SET_CURSOR(&iter));
				}
			}
		}
	}
	if (obj == 0) {
		m_flags &= ~0x80;
		g_scriptPC += m_flags >> 11;
		result = 1;
	} else {
		g_scriptSender = new UserReference("^sender", obj, 0, 0);
		((UserReference *)g_scriptSender)->SetReferent(obj, 0);
		if (message == TFBMember_WallNormalMessage() || message == TFBMember_BounceNormalMessage()) {
			Vec3 *p = actor->GetVector(actor);
			vec.x += p->x;
			vec.y += p->y;
			vec.z += p->z;
		}
		((UserReference *)g_scriptSender)->SetVector(&vec, (UserReference *)g_scriptSender);
		result = OpCode::Execute();
	}
	if (obj != 0 && g_scriptSender != 0)
		delete g_scriptSender;
	g_scriptSender = saved;
	return result;
}

// 0x43aef0
int MessageSend::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Message());
	refs.Read(&m_message);
	g_scriptLastType = 0;
	ScriptRefReader refs2(TFBType_Actor());
	refs2.Read(&m_receiver);
	if (g_scriptLastType->GetType() == TFBType_Set())
		g_scriptLastType = g_scriptLastType->GetBaseType();
	ScriptEnumReader(&m_value.opArg, g_compareOpNames, 0);
	ScriptValueReader(&m_value, g_scriptLastType, 0, 4);
	return sizeof(MessageSend);
}

// 0x43ae30
int MessageCheck::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Message());
	refs.Read(&m_message);
	if (g_scriptLastType->GetKind() > SCRIPT_KIND_VALUE) {
		g_scriptLastType = 0;
		ScriptRefReader refs2(TFBType_Actor());
		refs2.Read(&m_receiver);
		if (g_scriptLastType->GetType() == TFBType_Set())
			g_scriptLastType = g_scriptLastType->GetBaseType();
		ScriptEnumReader(&m_value.opArg, g_compareOpNames, 0);
		ScriptValueReader(&m_value, g_scriptLastType, 0, 4);
	}
	return sizeof(MessageCheck);
}

}  // namespace Script

namespace Script {

// 0x432ca0
ScriptReference *ScriptMessage::GetType()
{
	return TFBType_Message();
}

}  // namespace Script

#include "script/referenceset.h"
namespace Script { inline ScriptReference *SetObject::Next(int mode, void *cursor) { return ((ReferenceSet *)this)->Next(mode, (SetNode **)cursor); } }
