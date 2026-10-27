#include "tfb/anim/tfbmodelstate.h"
#include "framework/core/cresourcemanager.h"
#include "rw/rphanim.h"
#include "rw/rtanim.h"
#include "tfb/anim/animmouth.h"
#include "tfb/anim/eyescontroller.h"
#include "modules/tfb/ctfbmodel.h"
#include <string.h>
#include "rw/rwcore.h"
#include "tfb/save/language.h"

using namespace RWS;


// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x5d5494 g_hudNamesA ("<DEFAULT>")
extern const float g_floatZero;
extern const char g_hudNamesA[12];

// 0x450000
TFBModelState::TFBModelState()
{
	m_updateInterval = 0;
	m_timeSinceUpdate = 0;
	m_lastSlot = 0;
	AnimData emptySlot;
	emptySlot.pRawData_ = 0;
	emptySlot.rateMultiplier_ = 1.0f;
	emptySlot.endBehavior_ = ANIM_END_LOOP;
	emptySlot.flags_ = 0;
	emptySlot.blendFrames_ = 10;
	emptySlot.pName_ = "";
	ResizeAnimData(0x50, emptySlot);
	for (unsigned i = 0; i < kNumSlots; i++)
		m_slotAnim[i] = (RwUInt8)i;
}

// 0x44f820
void TFBModelState::SetSlotRate(int slot, float rate)
{
	if (slot < 0)
		slot = dstAnimIndex_;
	else
		slot = m_slotAnim[slot];
	SetRateMultiplier(slot, rate);
}

// 0x44f850
void TFBModelState::SetUpdateInterval(float seconds)
{
	m_updateInterval = seconds;
}

// Speed 1 for language index 0, 9 and 8, else the keys' length / duration.
// 0x44f860
void TFBModelState::PlayMouthAnim(TFB::KFSet *keys, float (*timeSource)(void *), void *timeSourceArg, float duration)
{
	if (pMouthController_) {
		if (!keys) {
			pMouthController_->Stop();
			return;
		}
		if ((char)IsCurrentLanguage(0) || (char)IsCurrentLanguage(9) || (char)IsCurrentLanguage(8))   // the game tests only al
			pMouthController_->Play(keys, timeSource, timeSourceArg, 1.0f);
		else
			pMouthController_->Play(keys, timeSource, timeSourceArg, keys->m_length / duration);
	}
}

// Entry `index` gets the slot's animation resource; null if missing or not a hierarchy animation.
// 0x44f9f0
void TFBModelState::SetSlotAnimation(int index, const RWSGUID &id)
{
	const char *type;
	const void *res = CResourceManager::FindById(&id, &type, 0, &animData_[index].pName_);
	if (res && !rwstrcmp("rwID_HANIMANIMATION", type))
		animData_[index].pRawData_ = static_cast<const RtAnimAnimation *>(res);
	else
		animData_[index].pRawData_ = 0;
}

// flags bit 0: hold at the end.
// the ice block's SB_ice_stop.anm always holds and stands still (rate 0)
// 0x44fa60
void TFBModelState::SetSlot(int index, const RWSGUID &id, RwUInt32 flags, int blendFrames)
{
	SetSlotAnimation(index, id);
	animData_[index].flags_ = flags;
	animData_[index].blendFrames_ = blendFrames;
	animData_[index].endBehavior_ = (AnimEndBehavior)(flags & ANIMDATA_HOLD);
	if (!strcmp(animData_[index].pName_, "SB_ice_stop.anm")) {
		animData_[index].endBehavior_ = ANIM_END_HOLD;
		animData_[index].rateMultiplier_ = 0.0f;
	}
}

// Default entry (no animation, blend 10 frames, "<DEFAULT>") for every empty slot, once the set has an animation. True if it did.
// 0x44f940
bool TFBModelState::FillEmptySlots()
{
	unsigned n = animData_.size();
	unsigned i = 0;
	AnimData *p = &animData_[0];
	for (; i < n; i++, p++) {
		if (p->pRawData_ != 0)
			break;
	}
	if (i >= n)
		return false;
	if (p->pRawData_ == 0)
		return false;
	AnimData def;
	def.pRawData_ = 0;
	def.rateMultiplier_ = 1.0f;
	def.endBehavior_ = ANIM_END_LOOP;
	def.flags_ = 0;
	def.blendFrames_ = 10;
	def.pName_ = g_hudNamesA;
	unsigned off = 0;
	unsigned k = n;
	do {
		AnimData *e = (AnimData *)((char *)&animData_[0] + off);
		if (!e->pRawData_)
			*e = def;
		off += sizeof(AnimData);
	} while (--k != 0);
	return true;
}

// Copies the model's animations and builds the face controllers it has.
// 0x44fad0
void TFBModelState::InitFromModel(CTFBModel *src, RpHAnimHierarchy *hierarchy)
{
	unsigned n = src->m_modelState.animData_.size();
	for (unsigned i = 0; i < n; i++)
		animData_[i] = src->m_modelState.animData_[i];
	if (src->m_haveMouthAnims && src->m_mouthBoneId >= 0) {
		TFB::AnimMouth *mouth = new TFB::AnimMouth;
		pMouthController_ = mouth;
		if (!mouth->SetAnimations(hierarchy, src->m_mouthBoneId, (const RtAnimAnimation **)src->m_mouthAnims)) {
			delete pMouthController_;
			pMouthController_ = 0;
		}
	}
	bool allEyeAnims = true;
	for (int i = 0; i < 4; i++) {
		if (src->m_eyeAnims[i] == 0) {
			allEyeAnims = false;
			break;
		}
	}
	if (src->m_haveEyeAnims && src->m_eyeBoneIds[0] >= 0 && allEyeAnims) {
		TFB::EyesController *eyes = new TFB::EyesController;
		pEyeController_ = eyes;
		if (!eyes->Create(hierarchy, &src->m_eyeBoneIds[0], (const RtAnimAnimation **)src->m_eyeAnims)) {
			delete pEyeController_;
			pEyeController_ = 0;
		}
	}
}

// Not empty, and not slot 0's animation again.
// 0x44fc70
bool TFBModelState::HasSlotAnimation(int slot)
{
	int idx = m_slotAnim[slot];
	if (!animData_[idx].pRawData_)
		return false;
	if (idx > 0 && animData_[idx].pRawData_ == animData_[0].pRawData_)
		return false;
	return true;
}

// 0x44fca0
bool TFBModelState::IsSlotPlaying(int slot)
{
	int idx;
	if (slot < 0)
		idx = dstAnimIndex_;
	else
		idx = m_slotAnim[slot];
	if (animData_[idx].pRawData_ && idx == dstAnimIndex_ && isAnimating_)
		return true;
	return false;
}

// 0x44fce0
bool TFBModelState::WillFinish(float dt)
{
	AnimData *d = &animData_[dstAnimIndex_];
	float t = animData_[dstAnimIndex_].rateMultiplier_ * rateScale_ * dt;
	if (t >= g_floatZero) {
		t += m_playTime;
		return t >= d->pRawData_->duration;
	}
	t += m_playTime;
	return t < g_floatZero;
}

// weight 0 switches the aux animation off. False if the aux slot is empty or the base slot is neither blend source nor destination.
// 0x44fd30
bool TFBModelState::SetAuxSlot(int baseSlot, int auxSlot, float weight)
{
	int idx = m_slotAnim[auxSlot];
	if (!animData_[idx].pRawData_)
		return false;
	if (m_auxBaseSlot != baseSlot || auxSlot != m_auxSlot || idx != GetAuxAnimation()) {
		int cur = m_slotAnim[baseSlot];
		if (cur == GetSrcAnimation()) {
			m_auxSlot = auxSlot;
			m_auxBaseSlot = baseSlot;
			SetAuxAnimationA(idx);
		} else if (cur == dstAnimIndex_) {
			m_auxSlot = auxSlot;
			m_auxBaseSlot = baseSlot;
			SetAuxAnimationB(idx);
		} else {
			return false;
		}
	}
	// the weight is copied as an integer, as in the game
	*(int *)&auxBlend_ = *(int *)&weight;
	if (g_floatZero == weight) {
		auxBActive_ = false;
		auxAActive_ = false;
		auxAnimIndex_ = 0;
	}
	return true;
}

// Keeps the play position (wrapping at both ends); runs CAnimSet::Update with the gathered time once m_updateInterval has gathered. False without an animation.
// 0x44fe00
bool TFBModelState::Advance(float dt)
{
	const RtAnimAnimation *raw = animData_[dstAnimIndex_].pRawData_;
	if (raw) {
		float t = animData_[dstAnimIndex_].rateMultiplier_ * rateScale_ * dt + m_playTime;
		m_playTime = t;
		float dur = raw->duration;
		if (t < g_floatZero)
			m_playTime = t + dur;
		else if (t >= dur)
			m_playTime = t - dur;
		dt += m_timeSinceUpdate;
		if (dt > m_updateInterval) {
			m_timeSinceUpdate = 0;
			return Update(dt);
		}
		m_timeSinceUpdate = dt;
		return true;
	}
	return false;
}

// Already playing: WillFinish(dt). Else true when it could not start.
// 0x44feb0
bool TFBModelState::PlaySlot(int slot, float dt)
{
	unsigned idx;
	if (slot < 0)
		idx = dstAnimIndex_;
	else
		idx = m_slotAnim[slot];
	AnimData *d = &animData_[idx];
	if (!d->pRawData_)
		return false;
	if (idx == dstAnimIndex_ && isAnimating_)
		return WillFinish(dt);
	m_playTime = 0;
	SetBlendDuration((float)(unsigned)d->blendFrames_ * 0.016666668f); // blend time in frames at 60 fps
	m_lastSlot = slot;
	return !SetDstAnimation(idx);
}
