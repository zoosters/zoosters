// Animation state of a TFB model instance: a CAnimSet, the slot -> animation table, play position and skeleton update timer.
#ifndef ZOOSTERS_TFB_ANIM_TFBMODELSTATE_H
#define ZOOSTERS_TFB_ANIM_TFBMODELSTATE_H

#include "framework/toolkits/animation/canimset.h"

class CTFBModel;    // rws/ctfbmodel.h
namespace TFB {
class KFSet; // tfb/anim/kfset.h
}

class TFBModelState : public RWS::CAnimSet {
public:
	enum { kNumSlots = 0x80 };

	TFBModelState();

	// Slots map through m_slotAnim to a set index; slot < 0 means the current animation.
	void SetSlotRate(int slot, float rate);
	void SetUpdateInterval(float seconds);
	void PlayMouthAnim(TFB::KFSet *keys, float (*timeSource)(void *), void *timeSourceArg, float duration);
	bool FillEmptySlots();
	void SetSlotAnimation(int index, const RWS::RWSGUID &id);
	void SetSlot(int index, const RWS::RWSGUID &id, RwUInt32 flags, int blendFrames);
	void InitFromModel(CTFBModel *src, RpHAnimHierarchy *hierarchy);
	bool HasSlotAnimation(int slot);  // an animation of its own, not slot 0's again
	bool IsSlotPlaying(int slot);
	bool WillFinish(float dt);
	bool SetAuxSlot(int baseSlot, int auxSlot, float weight);
	bool Advance(float dt);
	bool PlaySlot(int slot, float dt);

	// Corpus accesses the game does through member offsets.
	int GetLastSlot() const { return m_lastSlot; }
	bool IsAuxActive() const { return auxAActive_ || auxBActive_; }   // Corpus +0x40 / +0x41
	void ClearAux()   // Corpus +0x44 (auxBlend_), +0x41, +0x40, +0x48 (auxAnimIndex_)
	{
		auxBlend_ = 0;
		auxBActive_ = false;
		auxAActive_ = false;
		auxAnimIndex_ = 0;
	}
	int GetDstAnimIndex() const { return (int)dstAnimIndex_; }   // not in the cl 13.10 build: a new inline here moves MenuPointer_Update's code
	const RwUInt8 &GetSlotIndex(int slot) const { return m_slotAnim[slot]; }
	// AccessBehavior (GameActor, GameSprite): returns the slot entry, replaces it for value >= 0.
	int ExchangeSlotIndex(int slot, int value)
	{
		int old = m_slotAnim[slot];
		if (value >= 0)
			m_slotAnim[slot] = (RwUInt8)value;
		return old;
	}
	RwReal GetRateScale() const { return rateScale_; }
	void SetRateScale(RwReal rate) { rateScale_ = rate; }
	RwReal GetAuxWeight() const { return auxBlend_; }
	void SetUpdateMatricesCallback(void *callback, void *data)
	{
		pUpdateMatricesCB_ = callback;
		pUpdateMatricesCBData_ = data;
	}

private:
	static void CheckLayout()
	{
		ZOOSTERS_ASSERT_OFFSET(TFBModelState, m_slotAnim, 0x58);
		ZOOSTERS_ASSERT_OFFSET(TFBModelState, m_updateInterval, 0xd8);
		ZOOSTERS_ASSERT_OFFSET(TFBModelState, m_lastSlot, 0xe0);
		ZOOSTERS_ASSERT_OFFSET(TFBModelState, m_auxBaseSlot, 0xec);
	}

	RwUInt8 m_slotAnim[kNumSlots]; // slot -> index into animData_ (identity after construction)
	RwReal m_updateInterval;       // Advance runs UpdateAnim only after this much time has gathered
	RwReal m_timeSinceUpdate;
	int m_lastSlot;                // slot of the last PlaySlot
	RwReal m_playTime;             // current animation, 0 at PlaySlot
	int m_auxSlot;                 // second slot of the last SetAuxSlot
	int m_auxBaseSlot;             // first slot of the last SetAuxSlot
};
ZOOSTERS_ASSERT_SIZE(TFBModelState, 0xf0);

#endif
