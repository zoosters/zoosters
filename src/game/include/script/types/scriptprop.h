// Script::ScriptProp: the script object of a WorldObject, an animated prop of the level (PS2 RTTI has no class for it: the names are
// ours), and the Expose* member classes the script engine registers for it. 0x198 bytes, vtable 0x5ccc68, constructor 0x41ca40
// (out of line).
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTPROP_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTPROP_H

#include "framework/toolkits/animation/canimset.h"   // first: <vector> before the framework headers
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "script/actor.h"
#include "rw/rpworld.h"

class WorldObject;   // modules/tfb/worldobject.h

namespace Script {

// A Placement with the four script-visible prop properties (0x54 bytes): what the prop type's prototype object is (at 0x5ccbc0)
// and the first base of a ScriptProp.
class ScriptPropBase : public Placement {
public:
	__forceinline ScriptPropBase() : m_visible(1), m_animForward(1), m_loopEnable(0), m_animRate(100) {}
	char m_pad40[0xc];
	bool m_visible;  // ExposeVisibility (bool: WorldObject::HandleEvents passes it to the clump's visibility call)
	unsigned char m_animForward;  // ExposeAnimationDirection (the member is true when 0: WorldObject negates the rate then)
	unsigned char m_loopEnable;  // ExposeLoopEnable
	int m_animRate;  // ExposeAnimationRate, in percent (WorldObject scales it by 0.01)
};
ZOOSTERS_ASSERT_SIZE(ScriptPropBase, 0x54);
ZOOSTERS_ASSERT_OFFSET(ScriptPropBase, m_visible, 0x4c);
ZOOSTERS_ASSERT_OFFSET(ScriptPropBase, m_animRate, 0x50);

// The clump holder of a prop (0xd4 bytes at +0x54): a block of state the constructor initialises and nothing in the tree reads, and the
// owned clump, which the destructor 0x41c5b0 destroys. Which class it is in the original is not known.
struct PropClumpState {
	// The constructor writes through the owner pointer, as the game's code addresses these fields from the ScriptProp
	// (plain member stores are scheduled differently).
	// the same stores through the members (the offsets are the ScriptProp's, +0x54 for this base)
	__forceinline PropClumpState(char *) : m_unk10(0)
	{
		m_unk04 = -3.4028235e38f;
		m_unk20 = 0;
		m_unk14 = -3.4028235e38f;
		m_unkC8 = 1.0f;
		m_unkCC = 0;
		m_clump = 0;
		m_unkC4 = 0;
		m_unkC0 = 0;
		m_unkBC = 0;
	}
	~PropClumpState();             // 0x41c5b0

	void *m_self;  // the ScriptProp (stored at the end of its constructor)
	float m_unk04;  // -FLT_MAX after the constructor
	char m_pad08[8];
	int m_unk10;
	float m_unk14;  // -FLT_MAX after the constructor
	char m_pad18[8];
	int m_unk20;
	char m_pad24[0x98];
	int m_unkBC, m_unkC0, m_unkC4;  // ..+0x118
	float m_unkC8;  // 1.0 after the constructor
	unsigned char m_unkCC;
	RpClump *m_clump;  // the owned clump (a clone of the WorldObject's)
};
ZOOSTERS_ASSERT_SIZE(PropClumpState, 0xd4);
// normal builds: the PropClumpState constructor stores through these members, the game build through ScriptProp +0x58 / 0x74 / 0x68 /
// 0x11c / 0x120 / 0x124 / 0x118 / 0x114 / 0x110 (the base sits at +0x54 of the ScriptProp: checked below with ScriptProp)
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unk04, 0x04);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unk14, 0x14);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unk20, 0x20);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unkBC, 0xbc);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unkC0, 0xc0);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unkC4, 0xc4);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unkC8, 0xc8);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_unkCC, 0xcc);
ZOOSTERS_ASSERT_OFFSET(PropClumpState, m_clump, 0xd0);

// The type prototype of the prop (TFBType_Prop 0x43fc00): a ScriptPropBase object with its own vtable 0x5ccbc0, whose slots 0, 4, 5, 7,
// 19 and 20 are the functions ScriptProp's vtable holds too.
// GLOBAL: 0x5ccbc0 Script::PropProto::vftable
class PropProto : public ScriptPropBase {
public:
	PropProto(PrototypeInit);                   // 0x43fba0 (the argument is ignored)
	const char *GetName();                      // 0: 0x41ca20
	ScriptReference *GetType();                 // 4: 0x41ca30
	ScriptReference *GetBaseType();             // 5: 0x4039e0
	void *GetMembers(int *count);               // 7: 0x41cc50
	float GetFacing();                          // 19: 0x41c420
	bool SetFacing(float v, int flag);          // 20: 0x41c600
};

// GLOBAL: 0x5ccc68 Script::ScriptProp::vftable
class ScriptProp : public ScriptPropBase, public PropClumpState {
public:
	ScriptProp(WorldObject *owner);             // 0x41ca40 (ret 4): clones the owner's clump and files it into the level
	~ScriptProp();                              // 0x41cbd0 (scalar deleting destructor 0x41cbb0)
	const char *GetName();                      // 0: 0x41ca20 ("prop")
	int AdjustRefCount(int delta);              // 2: 0x41cb80 (deletes the object when the count reaches 0)
	ScriptReference *GetType();                 // 4: 0x41ca30
	ScriptReference *GetBaseType();             // 5: 0x4039e0
	void *GetMembers(int *count);               // 7: 0x41cc50
	int IsSolidCollider();                      // 32: 0x4604c0 (returns true; bound in script/normal_bringup.cpp)
	int IsMeshCollidee();                       // 34: 0x4604c0
	float GetFacing();                          // 19: 0x41c420
	bool SetFacing(float angle, int unused);    // 20: 0x41c600 turns the clump about its vertical axis, keeping its position
	void *FUN_0041c410();                       // 0x41c410 (returns this; the this-adjusting stub of the first base)
	void SetAnimRate(float rate);               // 0x41c670 (ret 4): the rate multiplier of every animation of the set
	void SetAnimEndBehavior(bool loop);         // 0x41c6c0 (ret 4): the end behaviour of every animation (loop, else hold)

	int m_refCount;
	int m_unk12c;
	WorldObject *m_owner;  // the WorldObject that drives it (WorldObject attribute 0 re-links it)
	unsigned m_tint;  // tint colour (WorldObject attribute 4)
	RWS::CAnimSet m_animSet;  // the animations of the prop's clump
	int m_animIndex;  // the animation the prop plays (ExposeCurrentFrame ...)
	float m_facing;  // the facing angle (GetFacing / SetFacing, degrees; the constructor stores 0)

	// GLOBAL: 0x5cce70 Script::ScriptProp::ExposeVisibility::vftable
	class ExposeVisibility : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cce28 Script::ScriptProp::ExposeAnimationDirection::vftable
	class ExposeAnimationDirection : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5ccde0 Script::ScriptProp::ExposeAnimationRate::vftable
	class ExposeAnimationRate : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5ccd98 Script::ScriptProp::ExposeCurrentFrame::vftable
	class ExposeCurrentFrame : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5ccd50 Script::ScriptProp::ExposeTotalFrames::vftable
	class ExposeTotalFrames : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	// GLOBAL: 0x5ccd08 Script::ScriptProp::ExposeLoopEnable::vftable
	class ExposeLoopEnable : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	class ExposeLoopStart : public Measurement {
	public:
		ExposeLoopStart();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		int GetInt(ScriptReference *owner);
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
	};
	class ExposeLoopCount : public Measurement {
	public:
		ExposeLoopCount();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		int GetInt(ScriptReference *owner);
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptProp, 0x198);
ZOOSTERS_ASSERT_OFFSET(ScriptProp, m_self, 0x54);   // so the PropClumpState members above sit at 0x58 .. 0x124 of the prop
ZOOSTERS_ASSERT_OFFSET(ScriptProp, m_refCount, 0x128);
ZOOSTERS_ASSERT_OFFSET(ScriptProp, m_owner, 0x130);
ZOOSTERS_ASSERT_OFFSET(ScriptProp, m_animSet, 0x138);
ZOOSTERS_ASSERT_OFFSET(ScriptProp, m_animIndex, 0x190);
ZOOSTERS_ASSERT_OFFSET(ScriptProp, m_facing, 0x194);

}  // namespace Script

#endif
