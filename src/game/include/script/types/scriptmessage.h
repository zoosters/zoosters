// Script::ScriptMessage (PS2 RTTI; base ScriptReference), the "message" script type, and its 13 Expose*Message member messages (the
// events a script can wait for with "check message": hit wall, hit ground, dying ...). No data members: only the prototype is built
// (TFBType_Message 0x43ad50, its constructor registers the members through the TFBMember_* getters); the member messages are
// vptr-only objects whose constructors (0x439d40 ...) just resolve their symbol against it (TFB_ResolveSymbol 0x434350).
// Not ActorMessage (actor.h's message sender, see script/ops_message.h).
// Vtable 0x5ce590 (11 slots): 0 GetName, 2 AdjustRefCount, 4 GetType, 10 below; 3 is the shared deleting destructor 0x4314a0
// (Script::OpCode::scalar_deleting_destructor), 5 0x43e6f0, 6 the object default 0x42cbe0, 7 0x43dc80 (default GetMembers).
// Member messages share slots 2 / 3 / 4 / 10; their slot 6 (GetKind) is 0x43e6f0 (value, the first nine) or 0x43efe0
// (op: Activate / Deactivate / Death / HitActor), slot 7 0x43dc80.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTMESSAGE_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTMESSAGE_H

#include "zoosters/base.h"
#include "script/script.h"

namespace Script {

// GLOBAL: 0x5ce590 Script::ScriptMessage::vftable
class ScriptMessage : public ScriptReference { // "message"
public:
	const char *GetName();         // 0: 0x432c90
	int AdjustRefCount(int delta); // 2: 0x432c80 (returns -1)
	ScriptReference *GetType();    // 4: 0x432ca0 (returns TFBType_Message)
	void vf10();                   // 10: 0x43ab60 (Reset: script clock -1, ExpireSenderMessages, clock 0)
};
ZOOSTERS_ASSERT_SIZE(ScriptMessage, 4);

class ExposeHitWallMessage : public ScriptMessage { // "hit wall", vtable 0x5d1b88
public:
	ExposeHitWallMessage(); // 0x439d40
	const char *GetName();  // 0: 0x439d90
	int GetKind();          // 6: 0x43e6f0
};

class ExposeHitTriggerMessage : public ScriptMessage { // "hit face trigger", vtable 0x5d1bc0
public:
	ExposeHitTriggerMessage(); // 0x439e00
	const char *GetName();     // 0: 0x439e50
	int GetKind();             // 6: 0x43e6f0
};

class ExposeWallNormalMessage : public ScriptMessage { // "wall normal", vtable 0x5d1c00
public:
	ExposeWallNormalMessage(); // 0x439ec0
	const char *GetName();     // 0: 0x439f10
	int GetKind();             // 6: 0x43e6f0
};

class ExposeLedgeNearbyMessage : public ScriptMessage { // "ledge nearby", vtable 0x5d1c38
public:
	ExposeLedgeNearbyMessage(); // 0x439f80
	const char *GetName();      // 0: 0x439fd0
	int GetKind();              // 6: 0x43e6f0
};

class ExposeFootPrintMessage : public ScriptMessage { // "footprint", vtable 0x5d1c74
public:
	ExposeFootPrintMessage(); // 0x43a040
	const char *GetName();    // 0: 0x43a090
	int GetKind();            // 6: 0x43e6f0
};

class ExposeBounceNormalMessage : public ScriptMessage { // "bounce normal", vtable 0x5d1cac
public:
	ExposeBounceNormalMessage(); // 0x43a100
	const char *GetName();       // 0: 0x43a150
	int GetKind();               // 6: 0x43e6f0
};

class ExposeHitGroundMessage : public ScriptMessage { // "hit ground", vtable 0x5d1ce8
public:
	ExposeHitGroundMessage(); // 0x43a1c0
	const char *GetName();    // 0: 0x43a210
	int GetKind();            // 6: 0x43e6f0
};

class ExposeLeftGroundMessage : public ScriptMessage { // "left ground", vtable 0x5d1d20
public:
	ExposeLeftGroundMessage(); // 0x43a280
	const char *GetName();     // 0: 0x43a2d0
	int GetKind();             // 6: 0x43e6f0
};

class ExposeJumpStartedMessage : public ScriptMessage { // "jump started", vtable 0x5d1d58
public:
	ExposeJumpStartedMessage(); // 0x43a340
	const char *GetName();      // 0: 0x43a390
	int GetKind();              // 6: 0x43e6f0
};

class ExposeActivateMessage : public ScriptMessage { // "activating", vtable 0x5d1d94
public:
	ExposeActivateMessage(); // 0x43a400
	const char *GetName();   // 0: 0x43a450
	int GetKind();           // 6: 0x43efe0
};

class ExposeDeactivateMessage : public ScriptMessage { // "deactivating", vtable 0x5d1dcc
public:
	ExposeDeactivateMessage(); // 0x43a4c0
	const char *GetName();     // 0: 0x43a510
	int GetKind();             // 6: 0x43efe0
};

class ExposeDeathMessage : public ScriptMessage { // "dying", vtable 0x5d1e08
public:
	ExposeDeathMessage();  // 0x43a580
	const char *GetName(); // 0: 0x43a5d0
	int GetKind();         // 6: 0x43efe0
};

class ExposeHitActorMessage : public ScriptMessage { // "hit actor", vtable 0x5d1f38
public:
	ExposeHitActorMessage(); // 0x43afa0
	const char *GetName();   // 0: 0x43aff0
	int GetKind();           // 6: 0x43efe0
};

} // namespace Script

Script::ScriptReference *TFBType_Message();   // 0x43ad50: the prototype getter of the message type (function-local static)

#endif
