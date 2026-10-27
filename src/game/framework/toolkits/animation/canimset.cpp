#include "zoosters/base.h"
#include "framework/toolkits/animation/canimset.h"
#include "framework/toolkits/animation/animhelper.h"
#include "framework/toolkits/animation/skinsafe.h"
#include "rw/rpplugins.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "framework/core/memoryhandler.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rwmacros.h"
#include "framework/toolkits/world/smartptrs/catomicptr.h"
#include "framework/toolkits/world/smartptrs/cclumpptr.h"
#include "framework/toolkits/math/maths.h"
#include "tfb/level/level.h"
#include "rw/rpcollis.h"
#include "framework/core/cresourcemanager.h"
#include "tfb/render/atomicflags.h"
#include "framework/toolkits/world/framehelper.h"
#include "framework/toolkits/world/atomichelper.h"
#include "framework/toolkits/world/camerahelper.h"
#include "framework/toolkits/world/lighthelper.h"
#include "framework/toolkits/world/clumphelper.h"
#include "framework/toolkits/world/factory.h"
#include "tfb/anim/animmouth.h"
#include "tfb/anim/eyescontroller.h"

using namespace RWS;

// GLOBAL: 0x44a5b0 ?PushBack@CAnimSet@RWS@@IAEXABUAnimData@2@@Z
// GLOBAL: 0x44ff50 ?ResizeAnimData@CAnimSet@RWS@@IAEXIUAnimData@2@@Z
// The game's own MSVC 7.1 vector instances live at these addresses.
void RWS::CAnimSet::PushBack(const AnimData &animData) { animData_.push_back(animData); }
void RWS::CAnimSet::ResizeAnimData(RwUInt32 count, AnimData fill) { animData_.resize(count, fill); }

// Stores its two arguments in globals that the matrix update reads.
extern "C" {
void FUN_00554ea0(void *callback, void *data);   // 0x554ea0
}
float TFBClock_GetRawSeconds();

// CSystemCommands command that attaches a resource.
enum { CMD_AttachResource = 0 };

// The game's constant is a hair below 60.
static const float kFramesPerSecond = 59.999996f;

namespace {

RpHAnimHierarchy *CloneRpHAnimHierarchy(RpHAnimHierarchy *cloneMe)
{
	return RpHAnimHierarchyCreateFromHierarchy(cloneMe, (RpHAnimHierarchyFlag)cloneMe->flags,
	                                          cloneMe->currentAnim->maxInterpKeyFrameSize);
}

void SetCurrentAnim(RpHAnimHierarchy *hierarchy, const RtAnimAnimation *anim, RwReal startTime)
{
	RtAnimInterpolatorSetCurrentAnim(hierarchy->currentAnim, const_cast<RtAnimAnimation *>(anim));
	RtAnimInterpolatorSetCurrentTime(hierarchy->currentAnim, startTime);
}

} // namespace

namespace anon {
template <class T> const T &Clamp(const T &v, const T &lo, const T &hi)
{
	if (v > hi)
		return hi;
	if (v < lo)
		return lo;
	return v;
}

// Keeps the current time within 0 and the duration.
// 0x449470
__declspec(noinline) float ClampedTimeDelta(float unClampedDeltaTime, float currTime, float duration)
{
	const float demandedTargetTime = currTime + unClampedDeltaTime;
	const float clampedTargetTime = Clamp(demandedTargetTime, 0.0f, duration);
	return clampedTargetTime - currTime;
}
} // namespace anon

// 0x44a550
CAnimSet::CAnimSet()
    : pMainHierarchy_(0), pBlendSrcHierarchy_(0), pBlendDstHierarchy_(0), blendDuration_(0), elapsedBlendTime_(0), dstAnimIndex_(0),
      srcAnimIndex_(0), bBlendIntoNext_(false), isBlending_(false), isAnimating_(false), pAuxBlendHierarchy_(0), pAuxHierarchy_(0),
      auxAActive_(false), auxBActive_(false), auxBlend_(0), auxAnimIndex_(0), rateScale_(1.0f), pMouthController_(0),
      pEyeController_(0), pUpdateMatricesCB_(0), pUpdateMatricesCBData_(0)
{
}

// 0x44a0e0
CAnimSet::~CAnimSet()
{
	if (pBlendSrcHierarchy_)
		RpHAnimHierarchyDestroy(pBlendSrcHierarchy_);
	if (pBlendDstHierarchy_)
		RpHAnimHierarchyDestroy(pBlendDstHierarchy_);
	if (pAuxBlendHierarchy_)
		RpHAnimHierarchyDestroy(pAuxBlendHierarchy_);
	if (pAuxHierarchy_)
		RpHAnimHierarchyDestroy(pAuxHierarchy_);
	delete pMouthController_;
	delete pEyeController_;
}

// 0x4492e0
bool CAnimSet::HasTarget() const
{
	return 0 != pMainHierarchy_;
}

// 0x4492f0
bool CAnimSet::SetTarget(RpClump *target)
{
	pMainHierarchy_ = FindHAnimHierarchy(*target);
	if (pMainHierarchy_) {
		RpSkin *pFirstSkin = (RpSkin *)ClumpFindFirstSkin(target);
		if (pFirstSkin)
			ApplyDefaultPoseToHierarchy(*pMainHierarchy_, *pFirstSkin);
		pMainHierarchy_->flags |= rpHANIMHIERARCHYUPDATELTMS | rpHANIMHIERARCHYUPDATEMODELLINGMATRICES;
		RpHAnimHierarchyAttach(pMainHierarchy_);
		AttachHAnimHierarchy(*target, *pMainHierarchy_);
		pBlendSrcHierarchy_ = CloneRpHAnimHierarchy(pMainHierarchy_);
		pBlendDstHierarchy_ = CloneRpHAnimHierarchy(pMainHierarchy_);
	}
	return HasTarget();
}

// 0x44a6e0
void CAnimSet::HandleSystemCommands(const CAttributePacket &attr)
{
	CAttributeCommandIterator attrIt(attr, CSystemCommands::pCSystemCommandsClassID);
	while (!attrIt.IsFinished()) {
		if (CMD_AttachResource == attrIt->GetCommandId()) {
			const RWSGUID *pResourceId = static_cast<const RWSGUID *>(attrIt->GetAs_void_ptr());
			AddAnimation(pResourceId);
		}
		++attrIt;
	}
}

// Default rate, looping, blend time 10 frames. The game only takes animations that fit the hierarchy, so the target must be set first.
// 0x44a640
void CAnimSet::AddAnimation(const RWSGUID *pResourceId)
{
	const char *resourceType;
	const char *resourceName;
	const void *pObject = CResourceManager::FindById(pResourceId, &resourceType, 0, &resourceName);
	if (pObject) {
		if (!rwstrcmp("rwID_HANIMANIMATION", resourceType)) {
			const RtAnimAnimation *pAnim = static_cast<const RtAnimAnimation *>(pObject);
			if (AnimationIsCompatibleWithHierarchy(*pAnim, *pMainHierarchy_)) {
				AnimData animData;
				animData.pRawData_ = pAnim;
				animData.rateMultiplier_ = 1.0f;
				animData.endBehavior_ = ANIM_END_LOOP;
				animData.flags_ = 0;
				animData.blendFrames_ = 10;
				animData.pName_ = resourceName;
				PushBack(animData);
			}
		}
	}
}

// 0x4493a0
RwUInt32 CAnimSet::GetSrcAnimation() const
{
	return srcAnimIndex_;
}

// 0x4493b0
RwUInt32 CAnimSet::GetAuxAnimation() const
{
	return auxAnimIndex_;
}

// 0x4493c0
void CAnimSet::SetBlendDuration(RwReal duration)
{
	blendDuration_ = duration;
}

// 0x449560
void CAnimSet::SetRateMultiplier(RwUInt32 animIndex, RwReal rate)
{
	animData_[animIndex].rateMultiplier_ = rate;
}

// 0x449580
void CAnimSet::SetEndBehavior(RwUInt32 animIndex, AnimEndBehavior behavior)
{
	animData_[animIndex].endBehavior_ = behavior;
}

// A HOLD animation stops at its duration; the blend source always holds.
// 0x4495a0
bool CAnimSet::UpdateHierarchy(RpHAnimHierarchy *hierarchy, RwUInt32 animIndex, RwReal timeDelta)
{
	const RtAnimAnimation &animData = *animData_[animIndex].pRawData_;
	int behavior = ANIM_END_HOLD;
	isAnimating_ = behavior != 0;
	const RwReal rateAdjustedTimeDelta = animData_[animIndex].rateMultiplier_ * rateScale_ * timeDelta;
	RwReal clampedTimeDelta = rateAdjustedTimeDelta;
	if (hierarchy != pBlendSrcHierarchy_)
		behavior = animData_[animIndex].endBehavior_;
	switch (behavior) {
	case ANIM_END_HOLD:
		clampedTimeDelta = anon::ClampedTimeDelta(rateAdjustedTimeDelta, hierarchy->currentAnim->currentTime, animData.duration);
		if (clampedTimeDelta == 0.0f)
			isAnimating_ = false;
		break;
	}
	if (clampedTimeDelta >= 0.0f)
		RtAnimInterpolatorAddAnimTime(hierarchy->currentAnim, clampedTimeDelta);
	else
		RtAnimInterpolatorSubAnimTime(hierarchy->currentAnim, -clampedTimeDelta);
	return isAnimating_;
}

// 0x449820
int CAnimSet::TotalOf(int animIndex)
{
	return (int)(animData_[animIndex].pRawData_->duration * kFramesPerSecond);
}

// Read from the blend destination while blending into it, else from the main hierarchy.
// 0x449850
int CAnimSet::FrameOf(int animIndex)
{
	if (isBlending_ && bBlendIntoNext_ && dstAnimIndex_ == (RwUInt32)animIndex) {
		float t = pBlendDstHierarchy_->currentAnim->currentTime / animData_[animIndex].pRawData_->duration;
		return (int)(t * (float)(RwUInt32)(-1 - (int)(animData_[animIndex].pRawData_->duration * -kFramesPerSecond)) + 0.5f);
	}
	float frames = (float)(RwUInt32)(-1 - (int)(animData_[animIndex].pRawData_->duration * -kFramesPerSecond));
	float scaled = frames * pMainHierarchy_->currentAnim->currentTime;
	return (int)(scaled / animData_[animIndex].pRawData_->duration + 0.5f);
}

// 0x449a10
int CAnimSet::LoopCountOf(int animIndex)
{
	RpHAnimHierarchy *h;
	if (isBlending_ && bBlendIntoNext_ && dstAnimIndex_ == (RwUInt32)animIndex)
		h = pBlendDstHierarchy_;
	else
		h = pMainHierarchy_;
	float t = h->currentAnim->currentTime;
	return (int)(t / animData_[animIndex].pRawData_->duration);
}

// Past the end the frame wraps (LOOP), holds at the last frame (HOLD) or goes to 0.
// 0x449910
void CAnimSet::SetFrameOf(int animIndex, RwUInt32 frame)
{
	RwUInt32 total = (int)(animData_[animIndex].pRawData_->duration * kFramesPerSecond);
	if (frame >= total) {
		switch (animData_[animIndex].endBehavior_) {
		case ANIM_END_LOOP:
			frame %= total;
			break;
		case ANIM_END_HOLD:
			frame = total - 1;
			break;
		default:
			frame = 0;
			break;
		}
	}
	float t = (float)frame * animData_[animIndex].pRawData_->duration /
	          (float)(RwUInt32)(-1 - (int)(animData_[animIndex].pRawData_->duration * -kFramesPerSecond));
	if (isBlending_ && bBlendIntoNext_ && dstAnimIndex_ == (RwUInt32)animIndex) {
		RtAnimInterpolatorSetCurrentTime(pBlendDstHierarchy_->currentAnim, t);
		UpdateHierarchy(pBlendDstHierarchy_, animIndex, 1e-6f);
	} else {
		RtAnimInterpolatorSetCurrentTime(pMainHierarchy_->currentAnim, t);
		UpdateHierarchy(pMainHierarchy_, animIndex, 1e-6f);
	}
}

// The old destination becomes the blend source; the new one starts at 0, or at the same phase with ANIMDATA_SYNC_PHASE on both.
// 0x449660
void CAnimSet::InitiateBlend(RpHAnimHierarchy *currDstHierarchy, RwUInt32 newDstAnimIndex)
{
	SetCurrentAnim(pBlendSrcHierarchy_, animData_[dstAnimIndex_].pRawData_, currDstHierarchy->currentAnim->currentTime);
	srcAnimIndex_ = dstAnimIndex_;
	isBlending_ = true;
	elapsedBlendTime_ = 0;
	float startTime = 0;
	if ((animData_[srcAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE) && (animData_[newDstAnimIndex].flags_ & ANIMDATA_SYNC_PHASE)) {
		RtAnimInterpolator *i = currDstHierarchy->currentAnim;
		float phase = i->currentTime / i->pCurrentAnim->duration;
		startTime = phase * animData_[newDstAnimIndex].pRawData_->duration;
	}
	SetCurrentAnim(pBlendDstHierarchy_, animData_[newDstAnimIndex].pRawData_, startTime);
	dstAnimIndex_ = newDstAnimIndex;
}

// 0x449720
void CAnimSet::TerminateBlend()
{
	if (isBlending_) {
		SetCurrentAnim(pMainHierarchy_, animData_[dstAnimIndex_].pRawData_, pBlendDstHierarchy_->currentAnim->currentTime);
		isBlending_ = false;
	}
}

// Same phase as srcHierarchy with ANIMDATA_SYNC_PHASE on both.
// 0x449770
void CAnimSet::SyncAux(RpHAnimHierarchy *srcHierarchy, RwUInt32 animIndex)
{
	float startTime = 0;
	if ((animData_[dstAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE) && (animData_[animIndex].flags_ & ANIMDATA_SYNC_PHASE)) {
		RtAnimInterpolator *i = srcHierarchy->currentAnim;
		float phase = i->currentTime / i->pCurrentAnim->duration;
		startTime = phase * animData_[animIndex].pRawData_->duration;
	}
	SetCurrentAnim(pAuxHierarchy_, animData_[animIndex].pRawData_, startTime);
	SetCurrentAnim(pAuxBlendHierarchy_, animData_[animIndex].pRawData_, startTime);
	auxAnimIndex_ = animIndex;
}

// Blends when the set already plays something. Clears the aux animations.
// 0x449a50
bool CAnimSet::SetDstAnimation(RwUInt32 animIndex)
{
	if (bBlendIntoNext_) {
		if (isBlending_)
			InitiateBlend(pBlendDstHierarchy_, animIndex);
		else
			InitiateBlend(pMainHierarchy_, animIndex);
	} else {
		SetCurrentAnim(pMainHierarchy_, animData_[animIndex].pRawData_, 0);
		dstAnimIndex_ = animIndex;
		bBlendIntoNext_ = (pBlendDstHierarchy_ && pBlendSrcHierarchy_) ? true : false;
	}
	auxAActive_ = false;
	auxBActive_ = false;
	auxAnimIndex_ = 0;
	return true;
}

// Over the blend source, or the main animation. The aux hierarchies are cloned on first use.
// 0x449af0
bool CAnimSet::SetAuxAnimationA(RwUInt32 animIndex)
{
	if (!pAuxHierarchy_)
		pAuxHierarchy_ = CloneRpHAnimHierarchy(pMainHierarchy_);
	if (!pAuxBlendHierarchy_)
		pAuxBlendHierarchy_ = CloneRpHAnimHierarchy(pMainHierarchy_);
	if (bBlendIntoNext_) {
		if (isBlending_)
			SyncAux(pBlendSrcHierarchy_, animIndex);
		else
			SyncAux(pMainHierarchy_, animIndex);
	} else
		SetCurrentAnim(pAuxHierarchy_, animData_[animIndex].pRawData_, 0);
	auxAnimIndex_ = animIndex;
	auxAActive_ = true;
	return true;
}

// Over the blend destination, or the main animation.
// 0x449bb0
bool CAnimSet::SetAuxAnimationB(RwUInt32 animIndex)
{
	if (!pAuxHierarchy_)
		pAuxHierarchy_ = CloneRpHAnimHierarchy(pMainHierarchy_);
	if (!pAuxBlendHierarchy_)
		pAuxBlendHierarchy_ = CloneRpHAnimHierarchy(pMainHierarchy_);
	if (bBlendIntoNext_) {
		if (isBlending_)
			SyncAux(pBlendDstHierarchy_, animIndex);
		else
			SyncAux(pMainHierarchy_, animIndex);
	} else
		SetCurrentAnim(pAuxHierarchy_, animData_[animIndex].pRawData_, 0);
	auxAnimIndex_ = animIndex;
	auxBActive_ = true;
	return true;
}

// 0x449c70
bool CAnimSet::UpdateBlended(RwReal timeDelta)
{
	UpdateHierarchy(pBlendSrcHierarchy_, srcAnimIndex_, timeDelta);
	bool ret = UpdateHierarchy(pBlendDstHierarchy_, dstAnimIndex_, timeDelta);
	RtAnimInterpolatorBlend(pMainHierarchy_->currentAnim, pBlendSrcHierarchy_->currentAnim, pBlendDstHierarchy_->currentAnim,
	                        elapsedBlendTime_ / blendDuration_);
	return ret;
}

// Destination with the aux animation.
// 0x449cd0
bool CAnimSet::UpdateBlendedAuxA(RwReal timeDelta)
{
	if ((animData_[dstAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE) && (animData_[auxAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE)) {
		RtAnimInterpolator *i = pBlendDstHierarchy_->currentAnim;
		float phase = i->currentTime / i->pCurrentAnim->duration;
		RtAnimInterpolatorSetCurrentTime(pAuxHierarchy_->currentAnim, phase * animData_[auxAnimIndex_].pRawData_->duration);
	}
	UpdateHierarchy(pAuxHierarchy_, auxAnimIndex_, timeDelta);
	bool ret = UpdateHierarchy(pBlendDstHierarchy_, dstAnimIndex_, timeDelta);
	RtAnimInterpolatorBlend(pMainHierarchy_->currentAnim, pBlendDstHierarchy_->currentAnim, pAuxHierarchy_->currentAnim, auxBlend_);
	return ret;
}

// Source blended into (destination with aux), aux synced to the destination.
// 0x449d70
bool CAnimSet::UpdateBlendedAuxB(RwReal timeDelta)
{
	if ((animData_[dstAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE) && (animData_[auxAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE)) {
		RtAnimInterpolator *i = pBlendDstHierarchy_->currentAnim;
		float phase = i->currentTime / i->pCurrentAnim->duration;
		RtAnimInterpolatorSetCurrentTime(pAuxHierarchy_->currentAnim, phase * animData_[auxAnimIndex_].pRawData_->duration);
	}
	UpdateHierarchy(pBlendSrcHierarchy_, srcAnimIndex_, timeDelta);
	UpdateHierarchy(pAuxHierarchy_, auxAnimIndex_, timeDelta);
	bool ret = UpdateHierarchy(pBlendDstHierarchy_, dstAnimIndex_, timeDelta);
	RtAnimInterpolatorBlend(pAuxBlendHierarchy_->currentAnim, pBlendDstHierarchy_->currentAnim, pAuxHierarchy_->currentAnim, auxBlend_);
	RtAnimInterpolatorBlend(pMainHierarchy_->currentAnim, pBlendSrcHierarchy_->currentAnim, pAuxBlendHierarchy_->currentAnim,
	                        elapsedBlendTime_ / blendDuration_);
	return ret;
}

// (Source with aux) blended into the destination, aux synced to the source.
// 0x449e50
bool CAnimSet::UpdateBlendedAuxC(RwReal timeDelta)
{
	if ((animData_[srcAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE) && (animData_[auxAnimIndex_].flags_ & ANIMDATA_SYNC_PHASE)) {
		RtAnimInterpolator *i = pBlendSrcHierarchy_->currentAnim;
		float phase = i->currentTime / i->pCurrentAnim->duration;
		RtAnimInterpolatorSetCurrentTime(pAuxHierarchy_->currentAnim, phase * animData_[auxAnimIndex_].pRawData_->duration);
	}
	UpdateHierarchy(pBlendSrcHierarchy_, srcAnimIndex_, timeDelta);
	UpdateHierarchy(pAuxHierarchy_, auxAnimIndex_, timeDelta);
	bool ret = UpdateHierarchy(pBlendDstHierarchy_, dstAnimIndex_, timeDelta);
	RtAnimInterpolatorBlend(pAuxBlendHierarchy_->currentAnim, pBlendSrcHierarchy_->currentAnim, pAuxHierarchy_->currentAnim, auxBlend_);
	RtAnimInterpolatorBlend(pMainHierarchy_->currentAnim, pAuxBlendHierarchy_->currentAnim, pBlendDstHierarchy_->currentAnim,
	                        elapsedBlendTime_ / blendDuration_);
	return ret;
}

// The face controllers get the clock time instead of dt when no aux animation is active.
// 0x449f60
bool CAnimSet::Update(RwReal dt)
{
	bool r = false;
	if (pUpdateMatricesCB_)
		FUN_00554ea0(pUpdateMatricesCB_, pUpdateMatricesCBData_);
	else
		FUN_00554ea0(0, 0);
	if (auxAActive_ || auxBActive_) {
		if (bBlendIntoNext_) {
			elapsedBlendTime_ += dt;
			if (elapsedBlendTime_ >= blendDuration_) {
				TerminateBlend();
				if (auxAActive_ || auxBActive_)
					r = UpdateBlendedAuxA(dt);
				else
					r = UpdateHierarchy(pMainHierarchy_, dstAnimIndex_, dt);
			} else {
				if (auxBActive_)
					r = UpdateBlendedAuxB(dt);
				else
					r = UpdateBlendedAuxC(dt);
			}
		} else {
			r = UpdateBlendedAuxA(dt);
		}
		if (pMouthController_)
			pMouthController_->Update(dt, false);
		if (pEyeController_)
			pEyeController_->Update(dt, false);
		RpHAnimHierarchyUpdateMatrices(pMainHierarchy_);
	} else if (bBlendIntoNext_) {
		if (isBlending_) {
			elapsedBlendTime_ += dt;
			if (elapsedBlendTime_ >= blendDuration_) {
				TerminateBlend();
				r = UpdateHierarchy(pMainHierarchy_, dstAnimIndex_, dt);
			} else {
				r = UpdateBlended(dt);
			}
		} else {
			r = UpdateHierarchy(pMainHierarchy_, dstAnimIndex_, dt);
		}
		if (pMouthController_)
			pMouthController_->Update(TFBClock_GetRawSeconds(), true);
		if (pEyeController_)
			pEyeController_->Update(TFBClock_GetRawSeconds(), true);
		RpHAnimHierarchyUpdateMatrices(pMainHierarchy_);
	}
	return r;
}

// The game's table of cleanup callbacks points at it.
// 0x413cc0
void __fastcall thunk_FUN_0044a0e0(RWS::CAnimSet *p)
{
	p->~CAnimSet();
}
