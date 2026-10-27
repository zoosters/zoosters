// TFB script runtime: operand types, parse-time readers and the interpreter state shared by every op's Parse (14) and Execute (13).
// Layouts from the loader 0x4349e0, the readers 0x4346f0 / 0x43fd20, the resolver 0x43c5a0 and the evaluator 0x4400c0.
// The class names of the non-polymorphic helpers are ours (PS2 RTTI only names polymorphic classes).
#ifndef ZOOSTERS_SCRIPT_RUNTIME_H
#define ZOOSTERS_SCRIPT_RUNTIME_H

#include "zoosters/base.h"   // ZOOSTERS_PTR64
#include "script/opcode.h"

namespace Script {

// The script object (PS2 RTTI name; vtable 0x5cd2bc), loaded by OpContext::Load 0x4349e0.
class OpContext : public NamedObject {
public:
	const char *GetName();      // 0: 0x429750 "script"
	const char *GetAltName();   // 1: 0x429740 "TFB Script"
	int m_behaviorNode;  // node index of the active behaviour (OpPreScript::Execute 0x432db0 indexes m_nodes with it,
	                            // -1 = none, 0 after Reset); OpStartup::Execute 0x432e60 skips its block while it is != 0
	int m_startTime;  // script time when the script was instantiated (GetVariableList 0x42d680; ExposeScriptTime
	                            // and the SlideEntry ctor subtract it)
	int m_0c;
	OpCode **m_nodes;  // one op per script node (null = disabled node), null-terminated
	int m_localCount;  // number of m_locals (0 after Load; Reset releases that many)
	ScriptReference **m_locals;  // local objects (behaviours); reference index 0x3fc0d + k
	void Reset();               // 0x432880: clears the fields, releases the locals
	void FreeNodes();           // 0x432bc0
	int ExecuteNext();          // 0x431060: run the node at g_scriptPC, result - 1 (0 for a null node)
	ScriptReference *Lookup(void *ref, int mode);   // 0x432f70 (ret 8): the object a reference names, evaluated in this (running) script
	ScriptReference *FindBlockType(int kind);       // 0x433140 (ret 4): the type of the innermost enclosing block of a kind
	bool Load(unsigned arg, unsigned char **buf);   // 0x4349e0 (ret 8): loads the script `buf` points to
	void Instantiate(OpContext *dest);              // 0x42cc20 (ret 4): copies this script's nodes into dest
};

// "R" operand: bits 14..31 object index, 8..13 member, 6..7 selector, 0..5 sub-member;
// 0xffffffff = none. 0x3fc0d + k = local object k of the running script, 0x3fffa..0x3ffff built-ins.
struct ScriptRef {
	unsigned bits;

	// 0x43c5a0: the object the reference names (after member/selector/sub-member); *owner = the object before
	// the member step, *set = the set it went through (0 if none). resolveMember: also step into a member
	// whose getter returns another object.
	ScriptReference *Resolve(ScriptReference **owner, ScriptReference **set, bool resolveMember);
};
ZOOSTERS_ASSERT_SIZE(ScriptRef, 4);

// "V" operand: a constant, a reference, or `lhs op rhs`. kind: 2 = reference/expression, else type in the high
// nibble (0x00 int, 0x10 float, 0x20 4 bytes, 0x30 two u16) and 1 in the low nibble = random [0, N).
struct ScriptValue {
	ScriptRef lhs;  // constant bits or a reference
	ScriptRef rhs;  // rhs constant bits or reference (expressions)
	// +8: a byte argument of the op that owns the value, read by its Parse (comparison operator of ValueCheck/ValueSlide/
	// SetFindSub/CheckFOV, message enum of MessageCheck/MessageSend, set walk direction of MoveTo/TeleportTo, animation of
	// TurnTo, line-of-sight flag of CheckFOV); the value reader leaves it alone
	unsigned char opArg;
	unsigned char kind;
	unsigned char rhsKind;
	unsigned char op;  // 0 + 1 - 2 * 3 /

	// 0x4400c0: result into *out (int bits or float); returns 0 int, 1 float, -1 error (unresolved, / 0).
	// max != 0 clamps the result to [0, max].
	int Evaluate(float *out, int max);
	// 0x440450: evaluate and store into the value object `target` names.
	void ApplyTo(ScriptRef *target);
	// 0x440570: evaluate into *out (a float in every caller); pitch 0 = heading of the (dx, dz) direction (0x4404c0),
	// 1 = elevation of (dx, dy, dz) (0x440520) (ObjectDisplace passes 0 for the azimuth, 1 for the elevation, ObjectSpawn 0)
	char EvalAngle(void *out, bool pitch);
	// 0x430160 / 0x4301c0: evaluate against the "value" prototype max into int / float;
	// false if Evaluate failed.
	bool EvalInt(int *out);
	bool EvalFloat(float *out);
};
ZOOSTERS_ASSERT_SIZE(ScriptValue, 12);

// the zeroing the prototype instances' constructors do on an embedded ScriptValue (ops_actor.h, ops_object.h)
inline void ZeroScriptValue(ScriptValue &v)
{
	v.opArg = 0; v.kind = 0; v.rhsKind = 0; v.op = 0; v.rhs.bits = 0; v.lhs.bits = 0;
}

// Stateless reader for values and enums (1-byte local in the game).
class ScriptValueReader {
public:
	// A user-provided constructor makes `ScriptValueReader.Read(...)` a packed 1-byte stack temporary without
	// zero-initialisation, like the game's (three of them live at once in ObjectDisplace::Parse).
	ScriptValueReader() {}
	// 0x43fd20: reads the value
	// operand for a target of type `type` into *v: `ScriptValueReader(&v, type, 0, 4);` (a temporary). Args 3/4 as in
	// the callers; a3 is a tag string ("times", "seconds", "feet") or 0.
	ScriptValueReader(ScriptValue *v, ScriptReference *type, int a3, int a4);
	// normal build: the same constructor (0x43fa00): the label is only tested against 0 (reader.cpp), so the pointer is passed as that test
	// (a (const char *) -> int cast would truncate on x64)
	ScriptValueReader(ScriptValue *v, ScriptReference *type, const char *a3, int a4) { new (this) ScriptValueReader(v, type, a3 != 0 ? 1 : 0, a4); }
	// 0x43fc50 / 0x43fc90 / 0x43fcd0: constant operand of a value, called by the
	// constructor on its ScriptRefReader stack object (4 stack arguments: destination, kind byte, expected type, a tag string)
	ScriptValueReader *ConstU32(void *dest, unsigned char *kind, ScriptReference *type, unsigned tag);
	// normal build: forwards to the one real body (the game's linker folded the int and float variants into 0x43fc50)
	ScriptValueReader *ConstU32(int *dest, unsigned char *kind, ScriptReference *type, unsigned tag) { return ConstU32((void *)dest, kind, type, tag); }
	ScriptValueReader *ConstU32(float *dest, unsigned char *kind, ScriptReference *type, unsigned tag) { return ConstU32((void *)dest, kind, type, tag); }
	ScriptValueReader *ConstU8(void *dest, unsigned char *kind, ScriptReference *type, unsigned tag);
	ScriptValueReader *ConstU16(void *dest, unsigned char *kind, ScriptReference *type, unsigned tag);
};

// The enum reader: 0x42fc60 is its constructor (ret 0xc, returns this; it initialises the 0x11c bytes like
// ScriptRefReader's does and reads the u8 into *out). A Parse that reads an enum has one of these on the stack as
// well as the ScriptRefReader (frame 0x23c in ValueCheck::Parse).
class ScriptEnumReader {
public:
	ScriptEnumReader(unsigned char *out, const char **names, void *a3);  // 0x42fc60
	char m_000[0x100];
	int m_100;                  // -1 (ConstU8/16/32 store the same)
	int m_104;
	int m_tag;  // 0x616161 for an enum (ConstU* store their tag here)
	int m_mode;  // 5 = enum (ConstU* store 3; ScriptRefReader::m_10c)
	const char **m_names;  // the enum's name table
	void *m_arg;  // the ctor's a3 ((void *)-1 = do not read the byte)
	unsigned char *m_out;  // where the byte goes
};
ZOOSTERS_ASSERT_SIZE(ScriptEnumReader, 0x11c);

// Every Parse has one of these (0x11c bytes) on its stack to read references: 0x4346f0 reads the u32, links it to the
// script's symbol table and sets g_scriptLastType. Only +0x10c / +0x110 are seen written (variable declarations).
// The constructor takes the expected type, which the game evaluates and drops (the getter call stays).
class ScriptRefReader : public ScriptValueReader {
public:
	ScriptRefReader(ScriptReference *) {}
	void Read(ScriptRef *ref);  // 0x4346f0
	char m_000[0x10c];
	int m_10c;                  // 1 = not a `user` variable (set by variable declarations); the same slot is the read mode of
	                            // ConstU* (3) and ScriptEnumReader (5)
	ScriptReference *m_110;     // declared type (variable declarations); ScriptEnumReader keeps its name table here
	int m_114;  // unused; the size comes from the Parse stack frames (ValueSet/ValueInc)
	int m_118;
#ifdef ZOOSTERS_PTR64
	char m_pad120[8];           // a ScriptEnumReader is constructed over the same storage (ScriptValueReader::ScriptValueReader, ValueCheck::Parse):
	                            // its three pointers end at +0x128 with 8-byte pointers (the stack object overran: /GS cookie failure)
#endif
};
ZOOSTERS_ASSERT_SIZE(ScriptRefReader, 0x11c);
static_assert(sizeof(ScriptRefReader) >= sizeof(ScriptEnumReader), "a ScriptEnumReader is placement-constructed over a ScriptRefReader's storage");

}  // namespace Script

// Byte-stream readers of the script being parsed
void TFBRead_u8(unsigned char *out);
void TFBRead_u16(unsigned short *out);
void TFBRead_u32(unsigned int *out);
void TFBRead_pstr(char *out);

// 0x43fff0 (compare): compare a with b by operator `op` (0..5; "==" uses a small epsilon)
bool TFB_CompareFloat(float a, int op, float b);

// Type prototypes (function-local statics)
Script::ScriptReference *TFBType_Value();     // 0x42f900 "value"
Script::ScriptReference *TFBType_Position();  // 0x430300
Script::ScriptReference *TFBType_Actor();     // 0x436e30
Script::ScriptReference *TFBType_Placement(); // 0x42f5c0
Script::ScriptReference *TFBType_OpCode();    // 0x4310c0
Script::ScriptReference *TFBType_Behavior();  // 0x4318f0
Script::ScriptReference *TFBType_User();      // 0x43c530
Script::ScriptReference *TFBType_Set();       // 0x43d2d0
Script::ScriptReference *TFBType_Int();      // static "value" measurement prototype, vtable 0x5ca670
Script::ScriptReference *TFBType_Angle();      // value type for an operand (Parse of TeleportTo/TurnTo/CheckFOV)

// The running op's "self" and "owner" objects are g_scriptSelf / g_scriptOwner (script/bearings.h)
namespace Script { void SetScriptContext(ScriptReference *self, ScriptReference *owner); }   // 0x43bb70

// Free list of 0x10-byte list nodes (FreeListAlloc takes from it before asking MemAlloc)
struct FreeNode { void *m_pad; FreeNode *m_next; };
extern FreeNode *g_nodeFreeList;                 // 0x620168
void *MemAlloc(size_t n);                        // 0x550170
void *FreeListAlloc(size_t n);                   // 0x439cf0

// Interpreter / parser state
extern int g_scriptPC;                         // 0x61f658: index of the node being executed / parsed
extern Script::OpContext *g_curScript;         // 0x61f65c: script being executed / parsed
extern Script::ScriptReference *g_scriptLastType;  // 0x61f870: type of the last reference read
extern int g_tfbParseEnd;                      // 0x61f87c: stream offset where the current node's operands end
extern Script::OpCode *g_tfbParseOp;           // 0x61f880: op being parsed

namespace Script {

// Block loop (inlined in OpCode::Execute 0x431130 and OpIfElse::Execute 0x431790): runs the nodes up to `end` (the m_flags >> 11
// children after this node). A child returning 1 means go on; anything else stops the block: PC jumps to the end, the done bit
// 0x80 is cleared and r - 1 is returned. A finished block sets 0x80 and returns its result bits.
// The index must be a local taken with `g_scriptPC++` before g_curScript->m_nodes is read; `m_nodes[g_scriptPC++]` or
// `i = PC; PC = i + 1` compile differently.
inline int OpCode::RunBlock(unsigned end)
{
	while ((unsigned)g_scriptPC < end) {
		int idx = g_scriptPC++;
		OpCode *op = g_curScript->m_nodes[idx];
		if (op != 0) {
			int r = op->Execute() - 1;
			if (r != 0) {
				g_scriptPC = end;
				m_flags &= ~0x80;
				return r;
			}
		}
	}
	m_flags |= 0x80;
	return m_flags & 7;
}

// A block op that doesn't enter its block: clear the done bit and jump over the child nodes (the shift takes the
// masked flags; one form serves the actor, set and value ops).
inline void OpCode::SkipBlock()
{
	m_flags &= ~0x80;
	g_scriptPC += m_flags >> 11;
}

}  // namespace Script

#endif
