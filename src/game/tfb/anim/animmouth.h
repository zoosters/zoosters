// Mouth-shape blending for a talking character, driven by a LipSyncKFSet.
#ifndef ZOOSTERS_TFB_ANIM_ANIMMOUTH_H
#define ZOOSTERS_TFB_ANIM_ANIMMOUTH_H

#include "tfb/anim/kfset.h"
#include "tfb/anim/subhierarchymixer.h"

void *MemAlloc(size_t size); // 0x550170

namespace TFB {

// Play position of the sound in seconds (Voice::GetPlayPosition).
typedef float (*MouthTimeSource)(void *arg);

class AnimMouth : public SubHierarchyMixer {
public:
	static void *operator new(size_t size) { return MemAlloc((unsigned)size); }
	AnimMouth();
	~AnimMouth() throw();

	virtual void Update(float t, bool absoluteTime);
	// eight mouth shapes plus an emote
	virtual bool SetAnimations(RpHAnimHierarchy *hierarchy, int nodeId, const RtAnimAnimation **anims);

	// The clock follows timeSource when set, else the time Update gets.
	void Play(KFSet *keys, MouthTimeSource timeSource, void *timeSourceArg, float rate);
	void Stop();

	RtAnimInterpolator *m_blended;  // blend of the two mouth shapes
	KFSet *m_keys;                  // 0 when idle
	MouthTimeSource m_timeSource;   // 0: follow Update's time
	void *m_timeSourceArg;
};
ZOOSTERS_ASSERT_SIZE(AnimMouth, 0x60);
ZOOSTERS_ASSERT_OFFSET(AnimMouth, m_blended, 0x50);
ZOOSTERS_ASSERT_OFFSET(AnimMouth, m_timeSource, 0x58);

} // namespace TFB

#endif
