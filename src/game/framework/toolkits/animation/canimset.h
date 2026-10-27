// Animation set: RpHAnim animations played on one clump's skeleton, blended between two. The game's class adds a second blend
// level driven by two "aux" animations, a rate scale, two owned face controllers, a matrix callback and frame queries for the script.
// 0x58 bytes; TFBModelState and ScriptProp (+0x138) embed or derive from it.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_ANIMATION_CANIMSET_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_ANIMATION_CANIMSET_H

#include <vector> // first: <new> must precede script/script.h, which declares the placement new only if <new> was not included
#include "zoosters/base.h"
#include "zoosters/vec71.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"

// RenderWare types are forward declared: some includers carry their own RW stand-ins, so this must not pull in include/rw.
// The typedefs repeat rw/rwcore.h's.
typedef float RwReal;
typedef uint32_t RwUInt32;
typedef uint8_t RwUInt8;
typedef char RwChar;
struct RpHAnimHierarchy;
struct RtAnimAnimation;
struct RtAnimInterpolator;

struct RpClump;
namespace TFB {
class AnimMouth;
class EyesController;
} // namespace TFB
class WorldObject;

namespace RWS {

enum AnimEndBehavior { ANIM_END_LOOP = 0, ANIM_END_HOLD = 1 };

// AnimData::flags_ bits.
enum AnimDataFlags {
	ANIMDATA_HOLD = 1,       // copied into endBehavior_
	ANIMDATA_SYNC_PHASE = 2  // a blend between two animations with this bit starts the new one at the same normalised time
};

// The game's AnimData has flags_ and blendFrames_ between the base's fields.
struct AnimData {
	const RtAnimAnimation *pRawData_; // +0x00 the animation resource (0 for an empty slot)
	RwReal rateMultiplier_;           // +0x04 playback rate, 1 = recorded speed, negative = backwards
	AnimEndBehavior endBehavior_;     // +0x08
	RwUInt32 flags_;                  // +0x0c AnimDataFlags
	RwUInt32 blendFrames_;            // +0x10 blend-in time in 1/60 s frames (10 by default; TFBModelState::PlaySlot)
	const RwChar *pName_;             // +0x14 resource name, e.g. "SB_ice_stop.anm"
};
ZOOSTERS_ASSERT_SIZE(AnimData, 0x18);
ZOOSTERS_ASSERT_OFFSET(AnimData, endBehavior_, 0x08);
ZOOSTERS_ASSERT_OFFSET(AnimData, flags_, 0x0c);
ZOOSTERS_ASSERT_OFFSET(AnimData, blendFrames_, 0x10);
ZOOSTERS_ASSERT_OFFSET(AnimData, pName_, 0x14);

class CAnimSet : public CSystemCommands {
public:
	CAnimSet();
	~CAnimSet();

	bool HasTarget() const;
	// Clones the clump's hierarchy for the blend levels. Takes a pointer, as the game's callers pass.
	bool SetTarget(RpClump *target);
	void HandleSystemCommands(const CAttributePacket &attr);
	void AddAnimation(const RWSGUID *pResourceId);

	// Blends from the current animation when possible. Always true.
	bool SetDstAnimation(RwUInt32 animIndex);
	RwUInt32 GetSrcAnimation() const;
	RwUInt32 GetAuxAnimation() const;   // game addition
	void SetRateMultiplier(RwUInt32 animIndex, RwReal rate);
	void SetEndBehavior(RwUInt32 animIndex, AnimEndBehavior behavior);
	void SetBlendDuration(RwReal duration);
	RwUInt32 GetAnimationCount() const;   // out-of-line copy, defined with ScriptProp

	// False when it has reached the end.
	bool Update(RwReal timeDelta);

	// Game additions: two aux animations over the main one, and frame queries for the script.
	bool SetAuxAnimationA(RwUInt32 animIndex);
	bool SetAuxAnimationB(RwUInt32 animIndex);
	int TotalOf(int animIndex);   // frames at 60 fps
	int FrameOf(int animIndex);
	int LoopCountOf(int animIndex);
	void SetFrameOf(int animIndex, RwUInt32 frame);

protected:
	// Reach the game's out-of-line push_back / resize by address.
	void PushBack(const AnimData &animData);
	void ResizeAnimData(RwUInt32 count, AnimData fill);

	void InitiateBlend(RpHAnimHierarchy *currDstHierarchy, RwUInt32 newDstAnimIndex);
	void TerminateBlend();
	void SyncAux(RpHAnimHierarchy *srcHierarchy, RwUInt32 animIndex);
	bool UpdateBlended(RwReal timeDelta);
	bool UpdateBlendedAuxA(RwReal timeDelta);
	bool UpdateBlendedAuxB(RwReal timeDelta);
	bool UpdateBlendedAuxC(RwReal timeDelta);
	bool UpdateHierarchy(RpHAnimHierarchy *hierarchy, RwUInt32 animIndex, RwReal timeDelta);

	zoosters::Vec71<AnimData> animData_;         // the 7.1 layout (16 bytes): a modern std::vector is 12 and shifts every later member (Corpus::m_owner +0x160)
	RpHAnimHierarchy *pMainHierarchy_;        // +0x10 the skeleton of the target clump
	RpHAnimHierarchy *pBlendSrcHierarchy_;    // +0x14 clone: the animation blended from
	RpHAnimHierarchy *pBlendDstHierarchy_;    // +0x18 clone: the animation blended to
	RwReal blendDuration_;                    // +0x1c
	RwReal elapsedBlendTime_;                 // +0x20
	RwUInt32 dstAnimIndex_;                   // +0x24
	RwUInt32 srcAnimIndex_;                   // +0x28
	bool bBlendIntoNext_;                     // +0x2c
	bool isBlending_;                         // +0x2d
	bool isAnimating_;                        // +0x2e
	RpHAnimHierarchy *pAuxBlendHierarchy_;    // +0x30 clone: destination blended with the aux animation
	RpHAnimHierarchy *pAuxHierarchy_;         // +0x34 clone: plays the aux animation
	bool auxAActive_;                         // +0x38 aux synced to the blend source; cleared by SetDstAnimation
	bool auxBActive_;                         // +0x39 aux synced to the blend destination; cleared by SetDstAnimation
	RwReal auxBlend_;                         // +0x3c weight of the aux animation
	RwUInt32 auxAnimIndex_;                   // +0x40
	RwReal rateScale_;                        // +0x44 multiplies every animation's rate, 1 initially
	::TFB::AnimMouth *pMouthController_;      // +0x48 owned: speech
	::TFB::EyesController *pEyeController_;   // +0x4c owned: blinking
	void *pUpdateMatricesCB_;                 // +0x50 per-node matrix callback
	void *pUpdateMatricesCBData_;             // +0x54 its data

	// Inside the class: the members are protected.
	static void CheckLayout()
	{
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, pMainHierarchy_, 0x10);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, dstAnimIndex_, 0x24);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, bBlendIntoNext_, 0x2c);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, pAuxBlendHierarchy_, 0x30);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, auxAActive_, 0x38);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, auxBActive_, 0x39);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, auxBlend_, 0x3c);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, auxAnimIndex_, 0x40);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, rateScale_, 0x44);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, pMouthController_, 0x48);
		ZOOSTERS_ASSERT_OFFSET(CAnimSet, pUpdateMatricesCBData_, 0x54);
	}

	friend class ::WorldObject; // reads animData_.size inline
};
ZOOSTERS_ASSERT_SIZE(CAnimSet, 0x58);

} // namespace RWS

#endif
