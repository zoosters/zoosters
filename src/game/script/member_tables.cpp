// GetMembers (script/types/ classes) whose static array mixes inline-constructed prototypes (vptr store) and prototypes built by an
// out-of-line constructor and copied.
#include "script/types/anglemeasurement.h"
#include "script/types/colormeasurement.h"
#include "script/types/twodmeasurement.h"
#include "script/types/actorattachment.h"
#include "script/types/actorwaypoint.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptmemcard.h"
#include "script/types/scriptshadow.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptmotionblur.h"
#include "zoosters/memberrow.h"
#include "script/types/scriptsprite.h"
#include "script/types/scriptprop.h"
#include "script/types/scriptcontroller.h"
#include "script/types/scriptparticle.h"
#include "script/types/scriptactor.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "script/types/actorturret.h"
#include "script/actor.h"
#include "script/types/scriptactor.h"
#include "script/types/scriptmemcard.h"
#include "script/types/scriptshadow.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptcontroller.h"
#include "script/types/scriptcamera.h"
#include "script/types/scriptmathops.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptmotionblur.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptparticle.h"
#include "script/types/animmap.h"
#include "script/csset.h"
#include "script/types/scriptvertanim.h"

namespace Script {

struct GmSlot {
	void *m_vptr;
	GmSlot(void *vt) : m_vptr(vt) {}
	template<class T> GmSlot(const T &, void *vt) : m_vptr(vt) {}   // temp built by an out-of-line base constructor, then the derived vptr
	GmSlot(const Measurement &m) { new ((void *)this) ScriptReference; m_vptr = *(void **)&m; }
	~GmSlot();   // 0x43b3d0 (ScriptReference::~ScriptReference)
};
// GLOBAL: 0x43b3d0 ??1GmSlot@Script@@QAE@XZ

// ColorMeasurement is 0x20 bytes: a real temporary would change the game's stack frame, so its constructor call (0x416510, a vptr-only write)
// goes through a 4-byte stand-in.
struct GmTmpColor : Measurement { GmTmpColor(); };
// GLOBAL: 0x416510 ??0GmTmpColor@Script@@QAE@XZ
// The temporary is discarded at once (GmSlot keeps only the vtable it is given), so an empty constructor is the same
// FOLDED: 0x416510 Script::GmTmpColor::GmTmpColor
inline GmTmpColor::GmTmpColor() {}

// GLOBAL: 0x5d4b40 Script::g_vtMBAmount
extern char g_vtMBAmount[];
// GLOBAL: 0x5d4af8 Script::g_vtMBOffX
extern char g_vtMBOffX[];
// GLOBAL: 0x5d4ab0 Script::g_vtMBOffY
extern char g_vtMBOffY[];
// GLOBAL: 0x5d4a68 Script::g_vtMBScaleX
extern char g_vtMBScaleX[];
// GLOBAL: 0x5d4310 Script::g_vtShArea
extern char g_vtShArea[];
// GLOBAL: 0x5d42c8 Script::g_vtShPitch
extern char g_vtShPitch[];
// GLOBAL: 0x5d4280 Script::g_vtShHeading
extern char g_vtShHeading[];
// GLOBAL: 0x5d4238 Script::g_vtShVertOffset
extern char g_vtShVertOffset[];
// GLOBAL: 0x5d41f0 Script::g_vtShSoftness
extern char g_vtShSoftness[];
// GLOBAL: 0x5d4548 Script::g_vtFgVertDist
extern char g_vtFgVertDist[];
// GLOBAL: 0x5d4500 Script::g_vtFgVertColor
extern char g_vtFgVertColor[];
// GLOBAL: 0x5d44b8 Script::g_vtFgPixDist
extern char g_vtFgPixDist[];
// GLOBAL: 0x5d4470 Script::g_vtFgPixCutoff
extern char g_vtFgPixCutoff[];
// GLOBAL: 0x5d4428 Script::g_vtFgPixEnable
extern char g_vtFgPixEnable[];

// Temporaries: constructors that live out of line in the game (vptr-only ctors, expose_ctors_*.cpp).


// 0x43f470
void *ScriptMotionBlur::GetMembers(int *count)
{
	static zoosters::MemberRow<ExposeAmount, ExposeOffsetX, ExposeOffsetY, ExposeScaleX, ExposeScaleY, ExposeIsoScale, ExposeField40> row;
	*count = row.count;
	return row.data();
}

// 0x43eea0
void *ScriptShadow::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptShadow::ExposeAreaRadius, ScriptShadow::ExposePitch, ScriptShadow::ExposeHeading, ScriptShadow::ExposeVerticalOffset, ScriptShadow::ExposeSoftness, ScriptShadow::ExposeEnableFading, ScriptShadow::ExposeFadeRadius> row;
	*count = row.count;
	return row.data();
}

// 0x43f030
void *ScriptFog::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptFog::ExposeVertFogDistance, ScriptFog::ExposeVertFogColor, ScriptFog::ExposePixelFogDistance, ScriptFog::ExposePixelCutoffIntensity, ScriptFog::ExposePixelFogEnable, ScriptFog::ExposeDOFBluriness, ScriptFog::ExposePixelFogColor, ScriptFog::ExposeBloomEnable, ScriptFog::ExposeBloomAddFactor, ScriptFog::ExposeBloomHaloSize> row;
	*count = row.count;
	return row.data();
}

// A script object set built inline as a temporary (PS2 ReferenceSet and subclasses): fields, then its destructor
// (set vptr 0x5ca608, Clear 0x43cca0, base vptr).
// GLOBAL: 0x5ca608 Script::GmSetTmp::vftable
// NOVTABLE: Script::GmSetTmp
// the real set class (the game's vtable 0x5ca608 has 18 slots; ReferenceSet declares them; its destructor clears the set)
struct GmSetTmp : ReferenceSet {
	GmSetTmp() {}
};
// GLOBAL: 0x43cca0 ?Clear@GmSetTmp@Script@@QAEXXZ
// NOVTABLE: Script::GmSetTmpD
struct GmSetTmpD : GmSetTmp {
	GmSetTmpD() {}
};

// GLOBAL: 0x5cd598 Script::g_vtAaIndex
extern char g_vtAaIndex[];
// GLOBAL: 0x5cd8a0 Script::g_vtAaLocalOffset
extern char g_vtAaLocalOffset[];
// GLOBAL: 0x5cd858 Script::g_vtAaLocalFacing
extern char g_vtAaLocalFacing[];
// GLOBAL: 0x5cd810 Script::g_vtAaLocalTilt
extern char g_vtAaLocalTilt[];
// GLOBAL: 0x5cd7c8 Script::g_vtAaLocalRoll
extern char g_vtAaLocalRoll[];
// GLOBAL: 0x5cd328 Script::g_vtAaAttacherSet
extern char g_vtAaAttacherSet[];


// 0x42db40
void *ActorAttachment::GetMembers(int *count)
{
	static zoosters::MemberRow<ActorWayPoint::ExposeIndex, ActorAttachment::ExposeLocalOffset, ActorAttachment::ExposeLocalFacing, ActorAttachment::ExposeLocalTilt, ActorAttachment::ExposeLocalRoll, zoosters::VptrOnly<ActorAttachment::AttacherSet>> row;
	*count = row.count;
	return row.data();
}

// Slot type with the out-of-line copy constructor of the game (0x403bb0: base vptr, then the source's vptr).
struct GmSlotX {
	void *m_vptr;
	GmSlotX(void *vt) : m_vptr(vt) {}
	template<class T> GmSlotX(const T &, void *vt) : m_vptr(vt) {}
	GmSlotX(const ScriptReference &o);   // 0x403bb0
	~GmSlotX();                          // 0x43b3d0
};
// GLOBAL: 0x403bb0 ??0GmSlotX@Script@@QAE@ABVScriptReference@1@@Z
// GLOBAL: 0x43b3d0 ??1GmSlotX@Script@@QAE@XZ


// GLOBAL: 0x5cc290 Script::g_vtSpData
extern char g_vtSpData[];
// GLOBAL: 0x5cc248 Script::g_vtSpLocation
extern char g_vtSpLocation[];
// GLOBAL: 0x5cc200 Script::g_vtSpScales
extern char g_vtSpScales[];
// GLOBAL: 0x5cc1b8 Script::g_vtSpRotation
extern char g_vtSpRotation[];
// GLOBAL: 0x5cc170 Script::g_vtSpTint
extern char g_vtSpTint[];
// GLOBAL: 0x5cc128 Script::g_vtSpJustification
extern char g_vtSpJustification[];
// GLOBAL: 0x5cc0e0 Script::g_vtSpVisible
extern char g_vtSpVisible[];

// GLOBAL: 0x5cc098 Script::g_vtSpClones
extern char g_vtSpClones[];
// The ExposeClones temporary: a set with an inline constructor and the out-of-line set destructor (0x402b20).
// NOVTABLE: Script::GmTmpClones
struct GmTmpClones : ScriptReference {
	unsigned char m_sorted;   // ReferenceSet fields as script/referenceset.h ReferenceSet (head/tail = first/last SetNode)
	int m_count, m_version;
	void *m_head, *m_tail;   // pointers (the real ReferenceSet is destroyed through them)
	GmTmpClones() : m_sorted(1), m_count(0), m_version(0), m_head(0), m_tail(0) { *(void **)this = g_vtSpClones; }
	~GmTmpClones();   // 0x402b20
};
// GLOBAL: 0x402b20 ??1GmTmpClones@Script@@UAE@XZ


// 0x416270
void *ScriptSprite::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptSprite::ExposeData, ScriptSprite::ExposeLocation, ScriptSprite::ExposeScales, ScriptSprite::ExposeRotation, ScriptSprite::ExposeTint, ScriptSprite::ExposeJustification, ScriptSprite::ExposeVisible, zoosters::VptrOnly<ScriptSprite::ExposeClones>, ScriptSprite::ExposePriority, ScriptSprite::ExposePixelExtents> row;
	*count = row.count;
	return row.data();
}


// GLOBAL: 0x5d1f78 Script::g_vtPt_ExposeOutputRate
extern char g_vtPt_ExposeOutputRate[];
// GLOBAL: 0x5d1fc8 Script::g_vtPt_ExposeOutputRateSpread
extern char g_vtPt_ExposeOutputRateSpread[];
// GLOBAL: 0x5d2020 Script::g_vtPt_ExposeLifetime
extern char g_vtPt_ExposeLifetime[];
// GLOBAL: 0x5d2070 Script::g_vtPt_ExposeLifetimeSpread
extern char g_vtPt_ExposeLifetimeSpread[];
// GLOBAL: 0x5d20c8 Script::g_vtPt_ExposeClones
extern char g_vtPt_ExposeClones[];
// GLOBAL: 0x5d2110 Script::g_vtPt_ExposeActivationRange
extern char g_vtPt_ExposeActivationRange[];
// GLOBAL: 0x5d2158 Script::g_vtPt_ExposeDeactivationRange
extern char g_vtPt_ExposeDeactivationRange[];
// GLOBAL: 0x5d21a0 Script::g_vtPt_ExposePrimarySVelocity
extern char g_vtPt_ExposePrimarySVelocity[];
// GLOBAL: 0x5d2200 Script::g_vtPt_ExposePrimarySVelocitySpread
extern char g_vtPt_ExposePrimarySVelocitySpread[];
// GLOBAL: 0x5d2268 Script::g_vtPt_ExposePrimarySGravity
extern char g_vtPt_ExposePrimarySGravity[];
// GLOBAL: 0x5d22c8 Script::g_vtPt_ExposePrimarySDrag
extern char g_vtPt_ExposePrimarySDrag[];
// GLOBAL: 0x5d2320 Script::g_vtPt_ExposePrimarySColor
extern char g_vtPt_ExposePrimarySColor[];
// GLOBAL: 0x5d2378 Script::g_vtPt_ExposePrimarySColorSpread
extern char g_vtPt_ExposePrimarySColorSpread[];
// GLOBAL: 0x5d23d8 Script::g_vtPt_ExposePrimarySSize
extern char g_vtPt_ExposePrimarySSize[];
// GLOBAL: 0x5d2430 Script::g_vtPt_ExposePrimarySSizeSpread
extern char g_vtPt_ExposePrimarySSizeSpread[];
// GLOBAL: 0x5d2490 Script::g_vtPt_ExposePrimaryEVelocity
extern char g_vtPt_ExposePrimaryEVelocity[];
// GLOBAL: 0x5d24f0 Script::g_vtPt_ExposePrimaryEVelocitySpread
extern char g_vtPt_ExposePrimaryEVelocitySpread[];
// GLOBAL: 0x5d2550 Script::g_vtPt_ExposePrimaryEGravity
extern char g_vtPt_ExposePrimaryEGravity[];
// GLOBAL: 0x5d25a8 Script::g_vtPt_ExposePrimaryEDrag
extern char g_vtPt_ExposePrimaryEDrag[];
// GLOBAL: 0x5d2600 Script::g_vtPt_ExposePrimaryEColor
extern char g_vtPt_ExposePrimaryEColor[];
// GLOBAL: 0x5d2658 Script::g_vtPt_ExposePrimaryEColorSpread
extern char g_vtPt_ExposePrimaryEColorSpread[];
// GLOBAL: 0x5d26b8 Script::g_vtPt_ExposePrimaryESize
extern char g_vtPt_ExposePrimaryESize[];
// GLOBAL: 0x5d2710 Script::g_vtPt_ExposePrimaryESizeSpread
extern char g_vtPt_ExposePrimaryESizeSpread[];
// GLOBAL: 0x5d2770 Script::g_vtPt_ExposeEmitterSpreadX
extern char g_vtPt_ExposeEmitterSpreadX[];
// GLOBAL: 0x5d27c8 Script::g_vtPt_ExposeEmitterSpreadY
extern char g_vtPt_ExposeEmitterSpreadY[];
// GLOBAL: 0x5d2820 Script::g_vtPt_ExposeEmitterSpreadZ
extern char g_vtPt_ExposeEmitterSpreadZ[];
// GLOBAL: 0x5d2e38 Script::g_vtPt_ExposeAngularSpreadX
extern char g_vtPt_ExposeAngularSpreadX[];
// GLOBAL: 0x5d2e90 Script::g_vtPt_ExposeAngularSpreadZ
extern char g_vtPt_ExposeAngularSpreadZ[];
// GLOBAL: 0x5d2878 Script::g_vtPt_ExposeVisualRadius
extern char g_vtPt_ExposeVisualRadius[];
// GLOBAL: 0x5d2ee8 Script::g_vtPt_ExposeIgnoreCutScenes
extern char g_vtPt_ExposeIgnoreCutScenes[];


// 0x43b8b0
void *ScriptParticle::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptParticle::ExposeOutputRate, ScriptParticle::ExposeOutputRateSpread, ScriptParticle::ExposeLifetime, ScriptParticle::ExposeLifetimeSpread, zoosters::VptrOnly<ScriptParticle::ExposeClones>, ScriptParticle::ExposeActivationRange, ScriptParticle::ExposeDeactivationRange, ScriptParticle::ExposePrimarySVelocity, ScriptParticle::ExposePrimarySVelocitySpread, ScriptParticle::ExposePrimarySGravity, ScriptParticle::ExposePrimarySDrag, ScriptParticle::ExposePrimarySColor, ScriptParticle::ExposePrimarySColorSpread, ScriptParticle::ExposePrimarySSize, ScriptParticle::ExposePrimarySSizeSpread, ScriptParticle::ExposePrimaryEVelocity, ScriptParticle::ExposePrimaryEVelocitySpread, ScriptParticle::ExposePrimaryEGravity, ScriptParticle::ExposePrimaryEDrag, ScriptParticle::ExposePrimaryEColor, ScriptParticle::ExposePrimaryEColorSpread, ScriptParticle::ExposePrimaryESize, ScriptParticle::ExposePrimaryESizeSpread, ScriptParticle::ExposeEmitterSpreadX, ScriptParticle::ExposeEmitterSpreadY, ScriptParticle::ExposeEmitterSpreadZ, ScriptParticle::ExposeAngularSpreadX, ScriptParticle::ExposeAngularSpreadZ, ScriptParticle::ExposeVisualRadius, ScriptParticle::ExposeIgnoreCutScenes> row;
	*count = row.count;
	return row.data();
}


// GLOBAL: 0x5d07b8 Script::g_vtSpeedHeading
extern char g_vtSpeedHeading[];
// GLOBAL: 0x5d0770 Script::g_vtSpeedPitch
extern char g_vtSpeedPitch[];
// GLOBAL: 0x5d0728 Script::g_vtSpeedHorz
extern char g_vtSpeedHorz[];
// GLOBAL: 0x5d06e0 Script::g_vtSpeedVert
extern char g_vtSpeedVert[];
// GLOBAL: 0x5d08d8 Script::g_vtOrgHeading
extern char g_vtOrgHeading[];
// GLOBAL: 0x5d0890 Script::g_vtOrgPitch
extern char g_vtOrgPitch[];
// GLOBAL: 0x5d0848 Script::g_vtOrgHorz
extern char g_vtOrgHorz[];
// GLOBAL: 0x5d0800 Script::g_vtOrgVert
extern char g_vtOrgVert[];
// GLOBAL: 0x5d09b0 Script::g_vtDstHeading
extern char g_vtDstHeading[];
// GLOBAL: 0x5d0968 Script::g_vtDstPitch
extern char g_vtDstPitch[];
// GLOBAL: 0x5d0920 Script::g_vtDstHorz
extern char g_vtDstHorz[];


// 0x4382f0
void *ScriptActor::ExposeSpeed::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptActor::ExposeSpeedHeading, ScriptActor::ExposeSpeed::ExposePitch, ScriptActor::ExposeSpeed::ExposeHorzComponent, ScriptActor::ExposeSpeed::ExposeVertComponent> row;
	*count = row.count;
	return row.data();
}

// 0x438790
void *ScriptActor::ExposeOrigin::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptActor::ExposeOrigin::ExposeHeading, ScriptActor::ExposeOrigin::ExposePitch, ScriptActor::ExposeOrigin::ExposeHorzComponent, ScriptActor::ExposeOrigin::ExposeVertComponent> row;
	*count = row.count;
	return row.data();
}

// 0x438840
void *ScriptActor::ExposeDestination::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptActor::ExposeDestination::ExposeHeading, ScriptActor::ExposeDestination::ExposePitch, ScriptActor::ExposeDestination::ExposeHorzComponent, ScriptActor::ExposeDestination::ExposeVertComponent> row;
	*count = row.count;
	return row.data();
}

// GLOBAL: 0x5d3ea0 Script::g_vtMcUnplugged
extern char g_vtMcUnplugged[];
// GLOBAL: 0x5d3e58 Script::g_vtMcFreeBlocks
extern char g_vtMcFreeBlocks[];
// GLOBAL: 0x5d3e10 Script::g_vtMcStatus
extern char g_vtMcStatus[];
// GLOBAL: 0x5d3dc8 Script::g_vtMcAutoSave
extern char g_vtMcAutoSave[];
// GLOBAL: 0x5d3d80 Script::g_vtMcAutoLoad
extern char g_vtMcAutoLoad[];
// GLOBAL: 0x5d3d38 Script::g_vtMcCurrentSlot
extern char g_vtMcCurrentSlot[];
// The ExposeMemCardCurrentSlot temporary: inline vptr-only constructor, then copied by the out-of-line copy constructor.
// NOVTABLE: Script::GmTmpMcCurrentSlot
struct GmTmpMcCurrentSlot : ScriptReference {
	GmTmpMcCurrentSlot() { *(void **)this = g_vtMcCurrentSlot; }
};


// 0x43e860
void *ScriptMemCard::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptMemCard::ExposeMemCardUnplugged, ScriptMemCard::ExposeMemCardFreeBlocks, ScriptMemCard::ExposeMemCardStatus, ScriptMemCard::ExposeMemCardAutoSave, ScriptMemCard::ExposeMemCardAutoLoad, ScriptMemCard::ExposeMemCardCurrentSlot, ScriptMemCard::ExposeMemCardSlotSize, ScriptMemCard::ExposeMemCardAutoFormat, ScriptMemCard::ExposeMemCardAutoDelete, ScriptMemCard::ExposeMemCardLanguage, ScriptMemCard::ExposeMemCardSlotPercent, ScriptMemCard::ExposeMemCardAutoCreate, ScriptMemCard::ExposeMemCardSlotGameTime, ScriptMemCard::ExposeMemCardChangedState> row;
	*count = row.count;
	return row.data();
}

// GLOBAL: 0x5cefd0 Script::g_vtTa_ExposeHitPoints
extern char g_vtTa_ExposeHitPoints[];
// GLOBAL: 0x5cd380 Script::g_vtTa_ExposeWayPoints
extern char g_vtTa_ExposeWayPoints[];
// GLOBAL: 0x5d0350 Script::g_vtTa_ExposeOrigin
extern char g_vtTa_ExposeOrigin[];
// GLOBAL: 0x5d03c8 Script::g_vtTa_ExposeDestination
extern char g_vtTa_ExposeDestination[];
// GLOBAL: 0x5d0440 Script::g_vtTa_ExposeHeadingObsolete
extern char g_vtTa_ExposeHeadingObsolete[];
// GLOBAL: 0x5d0498 Script::g_vtTa_ExposeFacingObsolete
extern char g_vtTa_ExposeFacingObsolete[];
// GLOBAL: 0x5cf020 Script::g_vtTa_ExposeSpeed
extern char g_vtTa_ExposeSpeed[];
// GLOBAL: 0x5cf080 Script::g_vtTa_ExposeAltitude
extern char g_vtTa_ExposeAltitude[];
// GLOBAL: 0x5cd3d8 Script::g_vtTa_ExposeAttachmentPoints
extern char g_vtTa_ExposeAttachmentPoints[];
// GLOBAL: 0x5cf0d0 Script::g_vtTa_ExposeFadePercent
extern char g_vtTa_ExposeFadePercent[];
// GLOBAL: 0x5cf138 Script::g_vtTa_ExposeActivationRange
extern char g_vtTa_ExposeActivationRange[];
// GLOBAL: 0x5cf190 Script::g_vtTa_ExposeDeactivationRange
extern char g_vtTa_ExposeDeactivationRange[];
// GLOBAL: 0x5cf1e8 Script::g_vtTa_ExposeClones
extern char g_vtTa_ExposeClones[];
// GLOBAL: 0x5cf230 Script::g_vtTa_ExposeGroundSpeed
extern char g_vtTa_ExposeGroundSpeed[];
// GLOBAL: 0x5cf288 Script::g_vtTa_ExposeGroundTurnRate
extern char g_vtTa_ExposeGroundTurnRate[];
// GLOBAL: 0x5cf2e0 Script::g_vtTa_ExposeGroundAccel
extern char g_vtTa_ExposeGroundAccel[];
// GLOBAL: 0x5cf338 Script::g_vtTa_ExposeGroundDecel
extern char g_vtTa_ExposeGroundDecel[];
// GLOBAL: 0x5cf390 Script::g_vtTa_ExposeMoveType
extern char g_vtTa_ExposeMoveType[];
// GLOBAL: 0x5cf3e0 Script::g_vtTa_ExposeAirSpeed
extern char g_vtTa_ExposeAirSpeed[];
// GLOBAL: 0x5cf438 Script::g_vtTa_ExposeAirTurnRate
extern char g_vtTa_ExposeAirTurnRate[];
// GLOBAL: 0x5cf490 Script::g_vtTa_ExposeAirAccel
extern char g_vtTa_ExposeAirAccel[];
// GLOBAL: 0x5cf4e8 Script::g_vtTa_ExposeAirDecel
extern char g_vtTa_ExposeAirDecel[];
// GLOBAL: 0x5cf540 Script::g_vtTa_ExposeJumpDelay
extern char g_vtTa_ExposeJumpDelay[];
// GLOBAL: 0x5cf590 Script::g_vtTa_ExposeInitialJumpVelocity
extern char g_vtTa_ExposeInitialJumpVelocity[];
// GLOBAL: 0x5cf5f0 Script::g_vtTa_ExposeCoastFrames
extern char g_vtTa_ExposeCoastFrames[];
// GLOBAL: 0x5cf648 Script::g_vtTa_ExposeUpSpeed
extern char g_vtTa_ExposeUpSpeed[];
// GLOBAL: 0x5cf6a0 Script::g_vtTa_ExposeUpAccel
extern char g_vtTa_ExposeUpAccel[];
// GLOBAL: 0x5cf6f8 Script::g_vtTa_ExposeDownSpeed
extern char g_vtTa_ExposeDownSpeed[];
// GLOBAL: 0x5cf750 Script::g_vtTa_ExposeDownAccel
extern char g_vtTa_ExposeDownAccel[];
// GLOBAL: 0x5cf7a8 Script::g_vtTa_ExposeAnimationRate
extern char g_vtTa_ExposeAnimationRate[];
// GLOBAL: 0x5cf808 Script::g_vtTa_ExposeWeight
extern char g_vtTa_ExposeWeight[];
// GLOBAL: 0x5cf858 Script::g_vtTa_ExposeBounceRestitution
extern char g_vtTa_ExposeBounceRestitution[];
// GLOBAL: 0x5cf8b0 Script::g_vtTa_ExposeXOffset
extern char g_vtTa_ExposeXOffset[];
// GLOBAL: 0x5cf900 Script::g_vtTa_ExposeYOffset
extern char g_vtTa_ExposeYOffset[];
// GLOBAL: 0x5cf950 Script::g_vtTa_ExposeZOffset
extern char g_vtTa_ExposeZOffset[];
// GLOBAL: 0x5cf9a0 Script::g_vtTa_ExposeXExtent
extern char g_vtTa_ExposeXExtent[];
// GLOBAL: 0x5cf9f0 Script::g_vtTa_ExposeYExtent
extern char g_vtTa_ExposeYExtent[];
// GLOBAL: 0x5cfa40 Script::g_vtTa_ExposeZExtent
extern char g_vtTa_ExposeZExtent[];
// GLOBAL: 0x5d04f0 Script::g_vtTa_ExposeConeAngle
extern char g_vtTa_ExposeConeAngle[];
// GLOBAL: 0x5cfa90 Script::g_vtTa_ExposeConeLength
extern char g_vtTa_ExposeConeLength[];
// GLOBAL: 0x5d0540 Script::g_vtTa_ExposeSweepAngle
extern char g_vtTa_ExposeSweepAngle[];
// GLOBAL: 0x5cfae0 Script::g_vtTa_ExposeBlipColor
extern char g_vtTa_ExposeBlipColor[];
// GLOBAL: 0x5cfb30 Script::g_vtTa_ExposeConeColor
extern char g_vtTa_ExposeConeColor[];
// GLOBAL: 0x5cfb80 Script::g_vtTa_ExposeSolidCollider
extern char g_vtTa_ExposeSolidCollider[];
// GLOBAL: 0x5cfbd8 Script::g_vtTa_ExposeWallCollision
extern char g_vtTa_ExposeWallCollision[];
// GLOBAL: 0x5cfc30 Script::g_vtTa_ExposeVisibleOnRadar
extern char g_vtTa_ExposeVisibleOnRadar[];
// GLOBAL: 0x5cfc88 Script::g_vtTa_ExposeWallClimber
extern char g_vtTa_ExposeWallClimber[];
// GLOBAL: 0x5cfce0 Script::g_vtTa_ExposeIgnoresCutScenes
extern char g_vtTa_ExposeIgnoresCutScenes[];
// GLOBAL: 0x5cfd38 Script::g_vtTa_ExposeIgnoresGroundColor
extern char g_vtTa_ExposeIgnoresGroundColor[];
// GLOBAL: 0x5cfd98 Script::g_vtTa_ExposeGroundTilter
extern char g_vtTa_ExposeGroundTilter[];
// GLOBAL: 0x5cfdf0 Script::g_vtTa_ExposeNoShadow
extern char g_vtTa_ExposeNoShadow[];
// GLOBAL: 0x5cfe40 Script::g_vtTa_ExposeID
extern char g_vtTa_ExposeID[];
// GLOBAL: 0x5cfe88 Script::g_vtTa_ExposeMeshCollider
extern char g_vtTa_ExposeMeshCollider[];
// GLOBAL: 0x5cfee0 Script::g_vtTa_ExposeMeshCollidee
extern char g_vtTa_ExposeMeshCollidee[];
// GLOBAL: 0x5cff38 Script::g_vtTa_ExposeTintColor
extern char g_vtTa_ExposeTintColor[];
// GLOBAL: 0x5cd430 Script::g_vtTa_ExposeTurrets
extern char g_vtTa_ExposeTurrets[];


// 0x436f80
void *ScriptActor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptActor::ExposeHitPoints, zoosters::VptrOnly<CsSetWayPoints>, ScriptActor::ExposeOrigin, ScriptActor::ExposeDestination, ScriptActor::ExposeHeadingObsolete, ScriptActor::ExposeFacingObsolete, ScriptActor::ExposeSpeed, ScriptActor::ExposeAltitude, zoosters::VptrOnly<CsSetAttachPoints>, ScriptActor::ExposeFadePercent, ScriptActor::ExposeActivationRange, ScriptActor::ExposeDeactivationRange, zoosters::VptrOnly<ScriptActor::ExposeClones>, ScriptActor::ExposeGroundSpeed, ScriptActor::ExposeGroundTurnRate, ScriptActor::ExposeGroundAccel, ScriptActor::ExposeGroundDecel, ScriptActor::ExposeMoveType, ScriptActor::ExposeAirSpeed, ScriptActor::ExposeAirTurnRate, ScriptActor::ExposeAirAccel, ScriptActor::ExposeAirDecel, ScriptActor::ExposeJumpDelay, ScriptActor::ExposeInitialJumpVelocity, ScriptActor::ExposeCoastFrames, ScriptActor::ExposeUpSpeed, ScriptActor::ExposeUpAccel, ScriptActor::ExposeDownSpeed, ScriptActor::ExposeDownAccel, ScriptActor::ExposeAnimationRate, ScriptActor::ExposeWeight, ScriptActor::ExposeBounceRestitution, ScriptActor::ExposeXOffset, ScriptActor::ExposeYOffset, ScriptActor::ExposeZOffset, ScriptActor::ExposeXExtent, ScriptActor::ExposeYExtent, ScriptActor::ExposeZExtent, ScriptActor::ExposeConeAngle, ScriptActor::ExposeConeLength, ScriptActor::ExposeSweepAngle, ScriptActor::ExposeBlipColor, ScriptActor::ExposeConeColor, ScriptActor::ExposeSolidCollider, ScriptActor::ExposeWallCollision, ScriptActor::ExposeVisibleOnRadar, ScriptActor::ExposeWallClimber, ScriptActor::ExposeIgnoresCutScenes, ScriptActor::ExposeIgnoresGroundColor, ScriptActor::ExposeGroundTilter, ScriptActor::ExposeNoShadow, ScriptActor::ExposeID, ScriptActor::ExposeMeshCollider, ScriptActor::ExposeMeshCollidee, ScriptActor::ExposeTintColor, zoosters::VptrOnly<CsSetTurrets>> row;
	*count = row.count;
	return row.data();
}


// ScriptProp (script/types/scriptprop.h): three Bool members built by the out-of-line constructor 0x4165b0, ExposeLoopStart / ExposeLoopCount copied.
// GLOBAL: 0x5cce70 Script::g_vtPropMem0
extern char g_vtPropMem0[];
// GLOBAL: 0x5cce28 Script::g_vtPropMem1
extern char g_vtPropMem1[];
// GLOBAL: 0x5ccde0 Script::g_vtPropMem2
extern char g_vtPropMem2[];
// GLOBAL: 0x5ccd98 Script::g_vtPropMem3
extern char g_vtPropMem3[];
// GLOBAL: 0x5ccd50 Script::g_vtPropMem4
extern char g_vtPropMem4[];
// GLOBAL: 0x5ccd08 Script::g_vtPropMem5
extern char g_vtPropMem5[];

// 0x41cc50
void *ScriptProp::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptProp::ExposeVisibility, ScriptProp::ExposeAnimationDirection, ScriptProp::ExposeAnimationRate, ScriptProp::ExposeCurrentFrame, ScriptProp::ExposeTotalFrames, ScriptProp::ExposeLoopEnable, ScriptProp::ExposeLoopStart, ScriptProp::ExposeLoopCount> row;
	*count = row.count;
	return row.data();
}


// ScriptController::ScriptRumble (the six Expose* prototypes; see the near-miss note)
// GLOBAL: 0x5caa40 Script::g_vtRbAct2
extern char g_vtRbAct2[];
// GLOBAL: 0x5ca9f8 Script::g_vtRbAct1
extern char g_vtRbAct1[];
// GLOBAL: 0x5ca9b0 Script::g_vtRbDuration
extern char g_vtRbDuration[];
// GLOBAL: 0x5ca968 Script::g_vtRbRadius
extern char g_vtRbRadius[];
// GLOBAL: 0x5ca920 Script::g_vtRbAtten
extern char g_vtRbAtten[];

struct GmTmpAtten : ScriptController::ScriptRumble::ExposePercent5cab20 {
	GmTmpAtten() { *(void **)this = g_vtRbAtten; }
};

// Near-miss 0x403a70 (3 variants): the game builds element 0, then the Attenuation temporary (0x403be0 call at EH state 1), THEN the plain vptr stores of elements 1-3, then copies the temporary into element 4 (state 5). Ours does elements 0-3 first (temp call at state 4).
// Tried: GmSlotX(GmTmpAtten) in the list, a local `GmTmpAtten a;` before the array (guard test moves after its ctor)
// GLOBAL: 0x6179c4 ?arr@?1??GetMembers@ScriptRumble@ScriptController@Script@@UAEPAXPAH@Z@4PAUGmSlotX@4@A
// GLOBAL: 0x6179dc ?$S1@?1??GetMembers@ScriptRumble@ScriptController@Script@@UAEPAXPAH@Z@4IA
// 0x403a70 Script::ScriptController::ScriptRumble::GetMembers
void *ScriptController::ScriptRumble::GetMembers(int *count)
{
	return Members(count);
}

void *ScriptController::ScriptRumble::Members(int *count)
{
	static zoosters::MemberRow<ScriptController::ScriptRumble::ExposeActuator2Level, ScriptController::ScriptRumble::ExposeActuator1Level, ScriptController::ScriptRumble::ExposeDuration, ScriptController::ScriptRumble::ExposeRadius, ScriptController::ScriptRumble::ExposeAttenuation, ScriptController::ScriptRumble::ExposeTimeRemaining> row;
	*count = row.count;
	return row.data();
}

}

namespace Script {

struct W2Slot {
	void *m_vptr;
	W2Slot(void *vt) : m_vptr(vt) {}
	W2Slot(const ScriptReference &m) { new ((void *)this) ScriptReference; m_vptr = *(void **)&m; }
	~W2Slot();   // 0x43b3d0 (ScriptReference::~ScriptReference)
};
// GLOBAL: 0x43b3d0 ??1W2Slot@Script@@QAE@XZ

// GLOBAL: 0x5cd598 Script::g_vtW2Index
extern char g_vtW2Index[];
// GLOBAL: 0x5cd8a0 Script::g_vtW2LocalOffset
extern char g_vtW2LocalOffset[];
// GLOBAL: 0x5cd858 Script::g_vtW2LocalFacing
extern char g_vtW2LocalFacing[];
// GLOBAL: 0x5cd810 Script::g_vtW2LocalTilt
extern char g_vtW2LocalTilt[];


// 0x42dd50
void *ActorTurret::GetMembers(int *count)
{
	return ActorTurret::Members(count);
}

// the member table without an object (GetMembers does not read `this`): AttachProtoB::GetMembers, the turret prototype, folds into 0x42dd50 and calls it
void *ActorTurret::Members(int *count)
{
	static zoosters::MemberRow<ActorWayPoint::ExposeIndex, ActorAttachment::ExposeLocalOffset, ActorAttachment::ExposeLocalFacing, ActorAttachment::ExposeLocalTilt, ActorAttachment::ExposeLocalRoll, ActorTurret::ExposeLookAtPointer, ActorTurret::ExposeLookAtHorzOffs, ActorTurret::ExposeLookAtVertOffs, ActorTurret::ExposeLookAtHeadingOffs, ActorTurret::ExposeLookAtHeadingRel, ActorTurret::ExposeFOV> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

namespace Script {

struct MemberSlotW {
	void *m_vptr;
	~MemberSlotW();   // 0x43b3d0 (ScriptReference::~ScriptReference)
};
// GLOBAL: 0x43b3d0 ??1MemberSlotW@Script@@QAE@XZ

// Vtables of the three member slots of the anim script type (SurfaceAnimW::GetMembers): 0 = probably ScriptVertAnim::ExposePercentComplete,
// 1 = unidentified, 2 = ScriptVertAnim::ExposePlaybackRate.
// GLOBAL: 0x5d3188 Script::g_vtAnimMember0
extern char g_vtAnimMember0[];
// GLOBAL: 0x5d3140 Script::g_vtAnimMember1
extern char g_vtAnimMember1[];
// GLOBAL: 0x5d30f8 Script::g_vtAnimMember2
extern char g_vtAnimMember2[];

// NOVTABLE: Script::SurfaceAnimW
class SurfaceAnimW : public ScriptReference { public: void *GetMembers(int *count); };

// 0x43c220
void *SurfaceAnimW::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptVertAnim::ExposePercentComplete, ScriptVertAnim::ExposeTimeRemaining, ScriptVertAnim::ExposePlaybackRate> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

// 0x403bb0 = ScriptReference's copy constructor (the source's vptr)
namespace Script { inline GmSlotX::GmSlotX(const ScriptReference &o) : m_vptr(*(void *const *)&o) {} }

#include "script/folded.h"
void *Folded_SurfaceAnimW_GetMembers(void *self, int *count) { return ((Script::SurfaceAnimW *)self)->GetMembers(count); }

// FOLDED: 0x43b3d0 Script::GmSlot::~GmSlot
Script::GmSlot::~GmSlot() {}   // the game folded it into Script::RootRefDtor::~RootRefDtor
// FOLDED: 0x43b3d0 Script::GmSlotX::~GmSlotX
Script::GmSlotX::~GmSlotX() {}   // the game folded it into Script::RootRefDtor::~RootRefDtor
// FOLDED: 0x402b20 Script::GmTmpClones::~GmTmpClones
Script::GmTmpClones::~GmTmpClones() {}   // the game folded it into Script::ReferenceSet::~ReferenceSet
// FOLDED: 0x43b3d0 Script::MemberSlotW::~MemberSlotW
Script::MemberSlotW::~MemberSlotW() {}   // the game folded it into Script::RootRefDtor::~RootRefDtor
// FOLDED: 0x43b3d0 Script::W2Slot::~W2Slot
Script::W2Slot::~W2Slot() {}   // the game folded it into Script::RootRefDtor::~RootRefDtor
