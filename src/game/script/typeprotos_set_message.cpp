#include "script/script.h"
#include "script/poolnode.h"
#include "script/runtime.h"
#include "script/script.h"
#include "script/referenceset.h"
#include "script/types/scriptmessage.h"

Script::ScriptReference *TFBType_Set();   // 0x43d2d0
Script::ScriptReference *TFBType_Message();   // 0x43ad50
Script::ScriptReference *OpProto_SetFindSub();            // 0x43d0e0 "find subset" op prototype
Script::ScriptReference *OpProto_SetForEach();            // 0x43cef0 "for each"
Script::ScriptReference *OpProto_SetMembershipChange();   // 0x43cf40 "change membership"
Script::ScriptReference *OpProto_SetMembershipCheck();    // 0x43cf90 "check membership"
Script::ScriptReference *OpProto_MessageCheck();          // 0x43ab80 "check message" op prototype
Script::ScriptReference *OpProto_MessageSend();           // 0x43abe0 "send message" op prototype
Script::ScriptReference *TFBMember_WallNormalMessage();   // 0x439f20
Script::ScriptReference *TFBMember_BounceNormalMessage(); // 0x43a160

// Function-local statics of a class with no base destructor (the game's EH frame has only the guard-bit state): they declare all eleven
// ScriptReference slots themselves, none defined.
#define PROTO_SLOTS(C) 	virtual const char *GetName(); 	virtual const char *GetAltName(); 	virtual int AdjustRefCount(int delta); 	virtual ~C(); 	virtual void *GetType(); 	virtual void *GetBaseType(); 	virtual int GetKind(); 	virtual void *GetMembers(int *count); 	virtual void SetReferent(Script::ScriptReference *, Script::ScriptReference *); 	virtual void *Deref(Script::ScriptReference *, int); 	virtual void vf10();

Script::ScriptReference *TFB_GetMessageSenderSet();  // 0x43aa90
Script::ScriptReference *TFBMember_HitActorMessage();  // 0x43b000
Script::ScriptReference *TFBMember_HitWallMessage();  // 0x439da0
Script::ScriptReference *TFBMember_HitTriggerMessage();  // 0x439e60
Script::ScriptReference *TFBMember_HitGroundMessage();  // 0x43a220
Script::ScriptReference *TFBMember_LeftGroundMessage();  // 0x43a2e0
Script::ScriptReference *TFBMember_JumpStartedMessage();  // 0x43a3a0
Script::ScriptReference *TFBMember_ActivateMessage();  // 0x43a460
Script::ScriptReference *TFBMember_DeactivateMessage();  // 0x43a520
Script::ScriptReference *TFBMember_DeathMessage();  // 0x43a5e0
Script::ScriptReference *TFBMember_LedgeNearbyMessage();  // 0x439fe0
Script::ScriptReference *TFBMember_FootPrintMessage();  // 0x43a0a0

namespace Script {

// The "set" type registers its four ops as members: an empty helper object whose method is not inlined
class SetInit {
public:
	SetInit() {}
	void Run();       // 0x43d270
};

// The set type's prototype object (vptr 0x5ca608); constructing it registers the set ops
// GLOBAL: 0x5ca608 Script::SetProto::vftable
// GLOBAL: 0x43b3d0 ??1SetProto@Script@@UAE@XZ
// The prototype object has the game's vtable 0x5ca608 (ReferenceSet's, 18 slots): a real set class gives the compiler's vtable those slots
class SetProto : public ReferenceSet {
public:
	__forceinline SetProto() { }
};

// 0x43d270
void SetInit::Run()
{
	((ScriptReference *)OpProto_SetFindSub())->ResolveSymbol(::TFBType_Set(), 0, 0);
	((ScriptReference *)OpProto_SetForEach())->ResolveSymbol(::TFBType_Set(), 0, 0);
	((ScriptReference *)OpProto_SetMembershipChange())->ResolveSymbol(::TFBType_Set(), 0, 0);
	((ScriptReference *)OpProto_SetMembershipCheck())->ResolveSymbol(::TFBType_Set(), 0, 0);
}


// The script node pool (0xfa nodes of 0x1c bytes) and its free list head
// GLOBAL: 0x620170 Script::g_nodePool
extern PoolNodeW g_nodePool[];

// The message type: registers the two message ops and all message members, then threads the node pool into a free list
class MessageInit {
public:
	MessageInit() {}
	void Run();       // 0x43acb0
};

// 0x43acb0
void MessageInit::Run()
{
	((ScriptReference *)OpProto_MessageCheck())->ResolveSymbol(::TFBType_Message(), 0, 0);
	((ScriptReference *)OpProto_MessageSend())->ResolveSymbol(::TFBType_Message(), 0, 0);
	TFB_GetMessageSenderSet();
	TFBMember_HitActorMessage();
	TFBMember_HitWallMessage();
	TFBMember_HitTriggerMessage();
	TFBMember_WallNormalMessage();
	TFBMember_BounceNormalMessage();
	TFBMember_HitGroundMessage();
	TFBMember_LeftGroundMessage();
	TFBMember_JumpStartedMessage();
	TFBMember_ActivateMessage();
	TFBMember_DeactivateMessage();
	TFBMember_DeathMessage();
	TFBMember_LedgeNearbyMessage();
	TFBMember_FootPrintMessage();
	PoolNodeW *p = g_nodePool;
	g_nodeFreeList = (FreeNode *)p;
	unsigned n = 0xf9;
	do {
		p->m_next = p + 1;
		p++;
	} while (--n != 0);
	p->m_next = 0;
}

// GLOBAL: 0x5ce590 Script::MessageProto::vftable
// GLOBAL: 0x43b3d0 ??1MessageProto@Script@@UAE@XZ
class MessageProto : public ScriptMessage {
public:
	__forceinline MessageProto() { }
};

}  // namespace Script

// 0x43ad50
Script::ScriptReference *TFBType_Message()
{
	static Script::MessageProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(Script::MessageInit().Run());
	return (Script::ScriptReference *)&s;
}

using namespace Script;
// 0x43d2d0
ScriptReference *TFBType_Set()
{
	static SetProto s;
	ZOOSTERS_PROTO_REGISTER_AFTER(SetInit().Run());
	return (ScriptReference *)&s;
}
