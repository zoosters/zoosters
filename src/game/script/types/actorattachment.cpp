// Script::ActorAttachment: names, local offset / facing / tilt / roll members and the destructors (0x42be20 of its attacher set,
// 0x42bf20 of the attachment). GetVector is in script/r428_attach.cpp, GetMembers in script/member_tables.cpp.
#include "script/types/actorattachment.h"

ZOOSTERS_NOINLINE Script::ScriptReference *TFBType_Attachment();

// forwarding thunk whose target reads ecx (this) at entry: thiscall member, ecx passes through the jmp
// (0x42ce40: the destructor of the turret prototype's class, vtable 0x5cdb30, jumps to ~ActorAttachment)
struct GnThis_429000_441000 {
	void AttachmentW3a_Dtor_thunk();
};

namespace Script {

// 0x42cda0
const char *ActorAttachment::AttacherSet::GetName()
{
	return "attachers";
}

// 0x42cdb0
ScriptReference *ActorAttachment::AttacherSet::Deref(ScriptReference *owner, int flags)
{
	return &static_cast<ActorAttachment *>(owner)->m_attachers;
}

// 0x42d640
const char *ActorAttachment::GetName()
{
	return "attachment";
}

// 0x42dc60
const char *ActorAttachment::ExposeLocalOffset::GetName()
{
	return "local offset";
}

// 0x42dc90
const char *ActorAttachment::ExposeLocalFacing::GetName()
{
	return "local facing";
}

// 0x42dcd0
const char *ActorAttachment::ExposeLocalTilt::GetName()
{
	return "local tilt";
}

// 0x42dd10
const char *ActorAttachment::ExposeLocalRoll::GetName()
{
	return "local roll";
}

// 0x4297c0
Vec3 *ActorAttachment::GetLocalOffset(ScriptReference *owner)
{
	return &((ActorAttachment *)owner)->m_localOffset;
}

// 0x4297f0
float ActorAttachment::GetLocalFacing()
{
	return m_localFacing;
}

// 0x4297d0
float ActorAttachment::GetLocalTilt()
{
	return m_localTilt;
}

// 0x4297e0
float ActorAttachment::GetLocalRoll()
{
	return m_localRoll;
}

// 0x42dc70
Vec3 *ActorAttachment::ExposeLocalOffset::GetVector(ScriptReference *owner)
{
	return ((ActorAttachment *)owner)->GetLocalOffset(owner);
}

// 0x42dc80
void ActorAttachment::ExposeLocalOffset::SetVector(const Vec3 *v, ScriptReference *owner)
{
	((ActorAttachment *)owner)->SetLocalOffset(v, owner);
}

// 0x42dca0
float ActorAttachment::ExposeLocalFacing::GetFloat(ScriptReference *owner)
{
	return ((ActorAttachment *)owner)->GetLocalFacing();
}

// 0x42dcb0
void ActorAttachment::ExposeLocalFacing::SetValue(float value, ScriptReference *owner)
{
	((ActorAttachment *)owner)->SetLocalFacing(value);
}

// 0x42dce0
float ActorAttachment::ExposeLocalTilt::GetFloat(ScriptReference *owner)
{
	return ((ActorAttachment *)owner)->GetLocalTilt();
}

// 0x42dcf0
void ActorAttachment::ExposeLocalTilt::SetValue(float value, ScriptReference *owner)
{
	((ActorAttachment *)owner)->SetLocalTilt(value);
}

// 0x42dd20
float ActorAttachment::ExposeLocalRoll::GetFloat(ScriptReference *owner)
{
	return ((ActorAttachment *)owner)->GetLocalRoll();
}

// 0x42dd30
void ActorAttachment::ExposeLocalRoll::SetValue(float value, ScriptReference *owner)
{
	((ActorAttachment *)owner)->SetLocalRoll(value);
}

// 0x42be20
ActorAttachment::AttacherSet::~AttacherSet()
{
}

// 0x42bf20
ActorAttachment::~ActorAttachment()
{
}

}  // namespace Script

// 0x42ce40
void GnThis_429000_441000::AttachmentW3a_Dtor_thunk()
{
	((Script::ActorAttachment *)this)->Script::ActorAttachment::~ActorAttachment();
}

// 0x42d650
Script::ScriptReference *ActorAttachment_GetType()
{
	return TFBType_Attachment();
}
