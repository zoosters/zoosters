// Script value ops. Address comments: the game function in that slot. Members past the base are sized from CloneToPool (m_XX = offset).
#ifndef ZOOSTERS_SCRIPT_OPS_VALUE_H
#define ZOOSTERS_SCRIPT_OPS_VALUE_H

#include "script/opcode.h"
#include "script/runtime.h"
#include "script/value.h"
#include "script/referenceset.h"

// 0x550170: the game's malloc wrapper (what `new` of these classes calls); 0x5500e0: its free (`delete`, `if (p) free(p)`)
void *MemAlloc(size_t size);
void operator_delete_unwind(void *p);

namespace Script {

// vtable 0x5cdcf0, "inc value", 12 B
class ValueInc : public OpCode {
public:
	const char *GetName();                      // 0: 0x42f970
	OpCode *Clone();                            // 12: 0x4303f0
	int Execute();                              // 13: 0x42f980
	int Parse();                                // 14: 0x42fec0
	ZOOSTERS_NOINLINE static ValueInc *CloneToPool(const ValueInc *src);
	ScriptRef m_target;
};
ZOOSTERS_ASSERT_SIZE(ValueInc, 0xc);

// vtable 0x5cdd38, "dec value", 12 B
class ValueDec : public OpCode {
public:
	const char *GetName();                      // 0: 0x42fa60
	OpCode *Clone();                            // 12: 0x430440
	int Execute();                              // 13: 0x42fa70
	int Parse();                                // 14: 0x42ff10
	ZOOSTERS_NOINLINE static ValueDec *CloneToPool(const ValueDec *src);
	ScriptRef m_target;
};
ZOOSTERS_ASSERT_SIZE(ValueDec, 0xc);

// vtable 0x5cdd80, "set value", 24 B
class ValueSet : public OpCode {
public:
	const char *GetName();                      // 0: 0x42fb20
	OpCode *Clone();                            // 12: 0x430490
	int Execute();                              // 13: 0x42fb30
	int Parse();                                // 14: 0x42fb50
	ZOOSTERS_NOINLINE static ValueSet *CloneToPool(const ValueSet *src);
	ValueSet() { m_value.opArg = 0; m_value.kind = 0; m_value.rhsKind = 0; m_value.op = 0; m_value.rhs.bits = 0; m_value.lhs.bits = 0; }
	ScriptRef m_target;
	ScriptValue m_value;
};
ZOOSTERS_ASSERT_SIZE(ValueSet, 0x18);

// vtable 0x5cddc8, "check value", 24 B
class ValueCheck : public OpBlock {
public:
	const char *GetName();                      // 0: 0x42fba0
	OpCode *Clone();                            // 12: 0x430b00
	int Execute();                              // 13: 0x430580
	int Parse();                                // 14: 0x42fc00
	ZOOSTERS_NOINLINE static ValueCheck *CloneToPool(const ValueCheck *src);
	ScriptRef m_target;
	// +0xc: the value compared against; its opArg (= +0x14 of the op) holds the comparison operator (an enum
	// read by the ScriptEnumReader, 0..5), which the ScriptValueReader constructor leaves alone
	ScriptValue m_value;
	ValueCheck() { m_value.opArg = 0; m_value.kind = 0; m_value.rhsKind = 0; m_value.op = 0; m_value.rhs.bits = 0; m_value.lhs.bits = 0; }
};
ZOOSTERS_ASSERT_SIZE(ValueCheck, 0x18);

// vtable 0x5cde10, "loop value", 20 B
class ValueLoop : public OpBlock {
public:
	const char *GetName();                      // 0: 0x42fbb0
	int IsBlock();                              // 11: 0x42fbc0
	OpCode *Clone();                            // 12: 0x430b90
	int Execute();                              // 13: 0x430500
	int Parse();                                // 14: 0x42fbd0
	ZOOSTERS_NOINLINE static ValueLoop *CloneToPool(const ValueLoop *src);
	ScriptValue m_times;  // the repeat count
	ValueLoop() { m_times.opArg = 0; m_times.kind = 0; m_times.rhsKind = 0; m_times.op = 0; m_times.rhs.bits = 0; m_times.lhs.bits = 0; }
};
ZOOSTERS_ASSERT_SIZE(ValueLoop, 0x14);

// 0x42f7b0 / vptr 0x5cdcc4 (0x30 B): one running slide of "slide value" (a value moving towards a target over some seconds);
// 0x42fcc0 returns the global set (0x61f520) of the slide ops that have running slides.
// The members are what ValueSlide::Execute reads/writes; the rest is the constructor's business.
// normal build: the real class (vtable 0x5cdcc4: the ScriptReference slots, AdjustRefCount = release); script/slideentry.cpp
class SlideEntry : public ScriptReference {
public:
	SlideEntry(Measurement *obj, ScriptReference *owner, int *value, float seconds, int type);   // 0x42f7b0
	int AdjustRefCount(int delta);   // 2: 0x42f860
	static void *operator new(size_t size) { return MemAlloc(size); }
	static void operator delete(void *p) { operator_delete_unwind(p); }
	int m_target;  // target value bits (copy of value[0])
	int m_start;            // start value bits (a float or an int, whatever the target measurement holds)
	int m_current;          // the value bits written last
	int m_end;              // the target converted to the measurement's own value type
	int m_startTime;        // script time at the start minus the context's m_startTime
	float m_durationMs;     // seconds * 1000
	Measurement *m_measurement;  // the value object being slid
	ScriptReference *m_owner;    // its owner
	ScriptReference *m_24;       // the global reference 0x621f34 (reference counted by the ctor)
	OpContext *m_context;   // the script that started the slide (g_curScript)
	int m_valueType;        // the ctor's `type`
};
static_assert(sizeof(void *) != 4 || sizeof(SlideEntry) == 0x30, "SlideEntry layout (32-bit)");

// Member of ValueSlide at +0x2c (24 B, vptr 0x5ca608, out-of-line copy constructor 0x430e50): the set of the op's running
// SlideEntry objects. vptr 0x5ca608 is the Script::ReferenceSet vtable (ops_set.h), so this is a ReferenceSet in the same
// layout; the class name is a leftover. normal build: the real set class (the game's vtable 0x5ca608 has 18 slots; ReferenceSet
// declares them, same layout)
class ValueSlideCurve : public ReferenceSet {
public:
	ValueSlideCurve() {}
	ValueSlideCurve(const ValueSlideCurve &o);   // 0x430e50
	void Insert(void *obj, float key) { ReferenceSet::Insert((ScriptReference *)obj, key); }
};
ZOOSTERS_ASSERT_SIZE(ValueSlideCurve, 0x18);

ValueSlideCurve *GetSlideSet();  // 0x42fcc0: the global set (0x61f520) of the slide ops with running slides

// Empty base of ValueSlide with an out-of-line destructor: stand-in for ScriptReference's virtual destructor (see script.h). It gives
// ValueSlide's constructor the EH frame the game's has (the game unwinds a constructed base by calling ~ScriptReference). Costs no space.
struct ValueSlideEhTag {
	ValueSlideEhTag() {}
	ZOOSTERS_NOINLINE ~ValueSlideEhTag();
};

// ValueSlide::Parse keeps an unused local object of a "value" measurement class (vptr 0x5cdf80) alive for the whole
// function. Its destructor is inline and empty, so the normal path has nothing to do, but the unwind action calls the
// real ~ScriptReference (0x43b3d0): that is what gives the function its EH frame.
// GLOBAL: 0x5cdf80 Script::SlideParseTag::vftable
class SlideParseTag {
public:
	virtual ~SlideParseTag() {}
};

// vtable 0x5cde78, "slide value", 68 B
// a ScriptValue that zeroes itself in the constructor (the prototype instance's member initialisers)
struct ScriptValueZ : ScriptValue {
	ScriptValueZ() { opArg = 0; kind = 0; rhsKind = 0; op = 0; rhs.bits = 0; lhs.bits = 0; }
};

class ValueSlide : public OpBlock, private ValueSlideEhTag {
public:
	const char *GetName();                      // 0: 0x42fd80
	OpCode *Clone();                            // 12: 0x430d40
	int Execute();                              // 13: 0x430820
	int Parse();                                // 14: 0x430210
	ZOOSTERS_NOINLINE static ValueSlide *CloneToPool(const ValueSlide *src);
	// 0x08-0x43: holds a member with a copy constructor and destructor (CloneToPool 0x430d50 has an EH frame);
	// both copy constructors are out of line in the game.
	ZOOSTERS_NOINLINE ValueSlide(const ValueSlide &o);
	ValueSlide();   // 0x42fd20: prototype instance, defined in script/w2c_o.cpp
	// its presence gives the placement new in CloneToPool the EH frame that calls it when the constructor throws
	static void operator delete(void *p, void *pool);
#if !defined(_MSC_VER) || defined(__clang__)
	// the placement form above hides the usual one, which the virtual destructor needs in standard C++ (clang, also in MSVC
	// mode; MSVC accepts it)
	static void operator delete(void *p) { ::operator delete(p); }
#endif
	ScriptRef m_target;
	ScriptValueZ m_value;  // the value to slide to (its opArg = the comparison operator, 1 after Parse)
	ScriptValueZ m_seconds;
	unsigned int m_easeOut;  // percent, x 0.02 = Math_EaseInOut 0x4350a0 easeOut
	unsigned int m_easeIn;  // percent, its easeIn (Parse reads it before m_easeOut)
	ValueSlideCurve m_slides;  // the running SlideEntry objects of this op
};
ZOOSTERS_ASSERT_SIZE(ValueSlide, 0x44);
ZOOSTERS_ASSERT_OFFSET(ValueSlide, m_slides, 0x2c);   // cutscene_reset.cpp: the game build reaches it as `obj + 0x2c`

}  // namespace Script

#endif
