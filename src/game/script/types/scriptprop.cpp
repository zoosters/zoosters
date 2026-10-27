// Script::ScriptProp: constructor, destructors, the reference count, the animation controls and the Expose* member accessors of the
// prop script object (Game.exe 0x41c410-0x41cec0). The vtable and the scalar deleting destructor are emitted from this file. The
// WorldObject (modules/tfb/worldobject.h) that owns the object passes itself as `owner` of the constructor.
#include "script/types/scriptprop.h"
#include "modules/tfb/worldobject.h"
#include "tfb/level/level.h"

// GLOBAL: 0x5ccb80 g_propRotateAxis
extern const RwV3d g_propRotateAxis;   // the axis SetFacing turns about (the vertical)
Script::ScriptReference *TFBType_Prop();   // 0x43fc00: the prop prototype getter (g/w3b_light.cpp)
namespace RWS { namespace ClumpHelper { int Destroy(RpClump *clump); } }   // 0x44eb70

namespace Script {

static inline ScriptProp *Prop(ScriptReference *owner)
{
	return static_cast<ScriptProp *>(owner);
}

// GLOBAL: 0x5ccc68 Script::ScriptProp::vftable
// 0x41cbb0
// 0x41ca40
ScriptProp::ScriptProp(WorldObject *owner) : PropClumpState((char *)this), m_refCount(0), m_unk12c(0), m_owner(owner)
{
	m_facing = 0;   // (m_animSet is constructed by RWS::CAnimSet::CAnimSet, 0x44a550)
	m_self = this;
	RpClump *clone = RpClumpClone(owner->m_clump);
	if (m_clump != clone) {
		if (m_clump)
			RWS::ClumpHelper::Destroy(m_clump);
		m_clump = clone;
	}
	RWS::CLevel::AddClump(m_clump);
}


// 0x41c5b0
PropClumpState::~PropClumpState()
{
	if (m_clump)
		RWS::ClumpHelper::Destroy(m_clump);
}

}  // namespace Script

// The number of animations of the set: the game's out-of-line copy of the inline GetAnimationCount (the loops below call it;
// WorldObject's constructor has it inlined).
// 0x41c5d0
RwUInt32 RWS::CAnimSet::GetAnimationCount() const
{
	return animData_.size();
}

namespace Script {

// 0x41cb80
int ScriptProp::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		delete this;
		return 0;
	}
	return m_refCount;
}

// 0x41c670
void ScriptProp::SetAnimRate(float rate)
{
	for (RwUInt32 i = 0; i < m_animSet.GetAnimationCount(); i++)
		m_animSet.SetRateMultiplier(i, rate);
}

// 0x41c6c0
void ScriptProp::SetAnimEndBehavior(bool loop)
{
	RWS::AnimEndBehavior endBehavior = (RWS::AnimEndBehavior)(loop == 0);   // not looping: hold the last pose
	for (RwUInt32 i = 0; i < m_animSet.GetAnimationCount(); i++)
		m_animSet.SetEndBehavior(i, endBehavior);
}

// 0x41c600
bool ScriptProp::SetFacing(float angle, int unused)
{
	m_facing = angle;
	RwV3d pos = RpClumpGetFrame(m_clump)->modelling.pos;
	RwFrameRotate(RpClumpGetFrame(m_clump), &g_propRotateAxis, angle, rwCOMBINEREPLACE);
	RwFrameTranslate(RpClumpGetFrame(m_clump), &pos, rwCOMBINEPOSTCONCAT);
	return true;
}

// 0x41c420
float ScriptProp::GetFacing()
{
	return m_facing;
}

// `mov eax,ecx; ret` (this-adjusting stub of the first base)
// 0x41c410
void *ScriptProp::FUN_0041c410()
{
	return this;
}

// 0x41ca30
ScriptReference *ScriptProp::GetType()
{
	return ::TFBType_Prop();
}


// 0x41ca20
const char *ScriptProp::GetName()
{
	return "prop";
}

// 0x41cd70
const char *ScriptProp::ExposeVisibility::GetName()
{
	return "visibility";
}

// 0x41cd80
const char *ScriptProp::ExposeAnimationDirection::GetName()
{
	return "animation direction";
}

// 0x41cd90
const char *ScriptProp::ExposeAnimationRate::GetName()
{
	return "animation rate";
}

// 0x41cda0
const char *ScriptProp::ExposeCurrentFrame::GetName()
{
	return "current frame";
}

// 0x41cdb0
const char *ScriptProp::ExposeTotalFrames::GetName()
{
	return "total frames";
}

// 0x41cdc0
const char *ScriptProp::ExposeLoopEnable::GetName()
{
	return "loop enable";
}

// 0x41cea0
const char *ScriptProp::ExposeLoopStart::GetName()
{
	return "loop start";
}

// 0x41cec0
const char *ScriptProp::ExposeLoopCount::GetName()
{
	return "loop count";
}


// 0x41cdd0
int ScriptProp::ExposeVisibility::GetInt(ScriptReference *owner)
{
	return Prop(owner)->m_visible;
}

// 0x41cde0
void ScriptProp::ExposeVisibility::SetValue(int value, ScriptReference *owner)
{
	Prop(owner)->m_visible = value != 0;
}

// 0x41ce00
int ScriptProp::ExposeAnimationDirection::GetInt(ScriptReference *owner)
{
	return Prop(owner)->m_animForward == 0;
}

// 0x41ce20
void ScriptProp::ExposeAnimationDirection::SetValue(int value, ScriptReference *owner)
{
	Prop(owner)->m_animForward = value == 0;
}

// 0x41ce40
int ScriptProp::ExposeAnimationRate::GetInt(ScriptReference *owner)
{
	return Prop(owner)->m_animRate;
}

// 0x41ce50
void ScriptProp::ExposeAnimationRate::SetValue(int value, ScriptReference *owner)
{
	Prop(owner)->m_animRate = value;
}

// 0x41ce60
int ScriptProp::ExposeLoopEnable::GetInt(ScriptReference *owner)
{
	return Prop(owner)->m_loopEnable;
}

// 0x41ce70
void ScriptProp::ExposeLoopEnable::SetValue(int value, ScriptReference *owner)
{
	Prop(owner)->m_loopEnable = value != 0;
}

// 0x41c430
int ScriptProp::ExposeCurrentFrame::GetInt(ScriptReference *owner)
{
	ScriptProp *p = Prop(owner);
	return p->m_animSet.FrameOf(p->m_animIndex);
}

// 0x41c450
void ScriptProp::ExposeCurrentFrame::SetValue(int value, ScriptReference *owner)
{
	ScriptProp *p = Prop(owner);
	p->m_animSet.SetFrameOf(p->m_animIndex, value);
}

// 0x41c470
int ScriptProp::ExposeTotalFrames::GetInt(ScriptReference *owner)
{
	ScriptProp *p = Prop(owner);
	return p->m_animSet.TotalOf(p->m_animIndex);
}

// 0x41c490
int ScriptProp::ExposeLoopCount::GetInt(ScriptReference *owner)
{
	ScriptProp *p = Prop(owner);
	return p->m_animSet.LoopCountOf(p->m_animIndex);
}

}  // namespace Script

// 0x41cbd0
// Destroys the animation set (m_animSet), then the clump holder (PropClumpState::~PropClumpState), then Placement.
Script::ScriptProp::~ScriptProp()
{
}

// 0x43fba0
Script::PropProto::PropProto(PrototypeInit)
{
}

// The prop prototype's vtable (0x5ccbc0) shares slots 0, 4, 5, 7, 19, 20 with ScriptProp's (one function each in the game); GetBaseType is
// Placement's type getter. A normal build gives PropProto its own overrides that run the ScriptProp bodies (as the game does; the
// prototype itself is never asked for its facing).

const char *Script::PropProto::GetName() { return ((ScriptProp *)this)->ScriptProp::GetName(); }
Script::ScriptReference *Script::PropProto::GetType() { return ((ScriptProp *)this)->ScriptProp::GetType(); }
// GetMembers (slot 7) is the same game function as ScriptProp's (0x41cc50): a linker alias, not a wrapper (a wrapper has its own address, which
// a vtable check reports as a different function).
#if defined(_MSC_VER)
#endif
float Script::PropProto::GetFacing() { return ((ScriptProp *)this)->ScriptProp::GetFacing(); }
bool Script::PropProto::SetFacing(float v, int flag) { return ((ScriptProp *)this)->ScriptProp::SetFacing(v, flag); }
