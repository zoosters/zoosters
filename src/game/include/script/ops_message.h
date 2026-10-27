// Script message ops. Address comments: the game function in that slot. Members past the base are sized from CloneToPool (m_XX = offset).
#ifndef ZOOSTERS_SCRIPT_OPS_MESSAGE_H
#define ZOOSTERS_SCRIPT_OPS_MESSAGE_H

#include "script/opcode.h"
#include "script/runtime.h"
#include "script/actor.h"

void *MemAlloc(size_t n);               // 0x550170
void operator_delete(void *p);          // 0x5500f0

namespace Script {

// vtable 0x5d2f70, 0x28 B: the "^sender" variable that MessageCheck publishes to its child ops
class UserReference : public PositionMeasurement {
public:
	UserReference(const char *name, ScriptReference *object, intptr_t a, int b);   // 0x43c2a0
	static void *operator new(size_t n) { return MemAlloc(n); }
	static void operator delete(void *p) { operator_delete(p); }
	int m_refCount;  // (AdjustRefCount 0x43c030: 0 -> DeletingDestructor(1))
	const char *m_name;  // name
	ScriptReference m_type;  // ScriptReference-shaped member (copy of the declared type's vptr)
	ScriptReference *m_object;  // the object this variable refers to (itself for a plain variable)
	int m_count;  // element count (0 = scalar)
	int m_valueType;  // value type (0x10 float)
	void *m_data;  // data (a scalar or a vector variable's value is stored inline here)
	int m_index;  // index
	int m_dataSize;  // data size in bytes
	~UserReference();           // 0x43bf50
	void SetReferent(ScriptReference *value, ScriptReference *owner);   // 8: 0x43bbe0
	int GetValueType();                                          // 12: 0x43c060
	ScriptReference *GetType();                                  // 4: 0x438950
	int GetKind();                                                  // 6: 0x43bc60
	void SetValue(int value, ScriptReference *owner);            // 16: 0x43be20
	void SetValue(float value, ScriptReference *owner);          // 15: 0x43bea0
	// small_c: overrides (declared without `virtual`, they override the ScriptReference/PositionMeasurement slots)
	const char *GetName();                                       // 0: 0x43ba80 (the variable's name, m_name)
	const char *GetAltName();                                    // 1: 0x43c050 "user"
	int AdjustRefCount(int delta);                               // 2: 0x43c030
	ScriptReference *GetBaseType();                              // 5: 0x43ba90 (the member at +0xc)
	void *GetMembers(int *count);                                // 7: 0x43bf30
	Vec3 *GetVector(ScriptReference *owner);                     // 17: 0x43c070
	void SetVector(const Vec3 *v, ScriptReference *owner);       // 18: 0x43c080
	int GetInt(ScriptReference *owner);                          // 13: 0x43bca0
	float GetFloat(ScriptReference *owner);                      // 14: 0x43bd60
	ScriptReference *Deref(ScriptReference *owner, int flags);   // 9: 0x43bb60
	int GetMax();                                                // 11: 0x43bf20
};

}  // namespace Script

// The "user" script variables the PC menus share with the mouse (g_userRefSlots): the scripts create them by name and
// FileMenuVariable (0x430e80) files them by name hash; ~UserReference clears the slot. Roles from MenuPointer_Update (0x463640).
enum MenuVariable {
	MENUVAR_SELECTION = 0,         // 0..2: the selected item (1..n) of up to three menu lists (name hash 0x3e817d38)
	MENUVAR_SELECTION_CHANGED = 3, // 3..5: set to 1 when the mouse changed that selection (0x419fbe2e)
	MENUVAR_SLIDER = 6,            // the options slider, 0..100 (0xe297e92c)
	MENUVAR_TOGGLE = 7,            // the options on/off toggle (0xe702a588)
	MENUVAR_COUNT = 8
};
extern Script::UserReference *g_userRefSlots[MENUVAR_COUNT];   // 0x61f6c4

namespace Script {
ZOOSTERS_ASSERT_SIZE(UserReference, 0x28);
ZOOSTERS_ASSERT_OFFSET(UserReference, m_data, 0x1c);   // measurement.cpp TwoDVariableView: the game build reads the two shorts at +0x1c / +0x1e

// A set object as MessageCheck iterates it: 0x43cd00(mode, state) = first (1) / next (5) member
class SetObject : public ScriptReference {
public:
	// GLOBAL: 0x43cd00 Script::SetObject::Next
	ScriptReference *Next(int mode, void *state);   // 0x43cd00 (Script::ReferenceSet::Next)
};

// vtable 0x5d1e3c, "check message", 28 B
class MessageCheck : public OpBlock {
public:
	MessageCheck() { m_value.opArg = 0; m_value.kind = 0; m_value.rhsKind = 0; m_value.op = 0; m_value.rhs.bits = 0; m_value.lhs.bits = 0; }
	const char *GetName();                      // 0: 0x43a690
	int IsBlock();                              // 11: 0x43a6a0
	OpCode *Clone();                            // 12: 0x43adc0
	int Execute();                              // 13: 0x43a760
	int Parse();                                // 14: 0x43ae30
	ZOOSTERS_NOINLINE static MessageCheck *CloneToPool(const MessageCheck *src);
	ScriptRef m_message;  // the message (type) object to look for
	ScriptRef m_receiver;  // the actor (or set of actors) whose message queue is searched (FindMessage)
	ScriptValue m_value;  // the value FindMessage compares with (operator = its opArg, the enum read by Parse, name table 0x602f98)
};
ZOOSTERS_ASSERT_SIZE(MessageCheck, 0x1c);

// vtable 0x5d1e88, "send message", 28 B
class MessageSend : public OpCode {
public:
	MessageSend() { m_value.opArg = 0; m_value.kind = 0; m_value.rhsKind = 0; m_value.op = 0; m_value.rhs.bits = 0; m_value.lhs.bits = 0; }
	const char *GetName();                      // 0: 0x43a6b0
	OpCode *Clone();                            // 12: 0x43ac40
	int Execute();                              // 13: 0x43a6c0
	int Parse();                                // 14: 0x43aef0
	ZOOSTERS_NOINLINE static MessageSend *CloneToPool(const MessageSend *src);
	ScriptRef m_message;  // the message (type) object to post
	ScriptRef m_receiver;  // the receiver (an actor or a set)
	ScriptValue m_value;  // the float argument of the message; its opArg is the enum read by Parse, name table 0x602f98
};
ZOOSTERS_ASSERT_SIZE(MessageSend, 0x1c);

}  // namespace Script

// Expected-type getter of MessageSend/MessageCheck::Parse
Script::ScriptReference *TFBType_Message();
extern const char *g_compareOpNames[7];
extern Script::ScriptReference *g_scriptSender;   // 0x621f10: the current "^sender" variable
Script::ScriptReference *TFBMember_WallNormalMessage();     // 0x439f20: type getters (function-local statics)
Script::ScriptReference *TFBMember_BounceNormalMessage();   // 0x43a160
extern Script::ScriptReference *g_scriptSelf;   // 0x621f14 (bearings.h)
   // 0x602f98
extern Script::ScriptReference *g_scriptOwner;   // 0x621f18 (bearings.h)

#endif
