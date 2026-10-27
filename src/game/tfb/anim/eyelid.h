// One blink animation on an eye sub-tree.
#ifndef ZOOSTERS_TFB_ANIM_EYELID_H
#define ZOOSTERS_TFB_ANIM_EYELID_H

#include "tfb/anim/subhierarchymixer.h"

namespace TFB {

class Eyelid : public SubHierarchyMixer {
public:
	Eyelid() { m_state = MIXER_IDLE; }
	~Eyelid() {}

	virtual void Update(float t, bool absoluteTime);
	virtual unsigned char SetAnimation(RpHAnimHierarchy *hierarchy, int nodeId, const RtAnimAnimation **anims);

	void Play(float rate, float startTime)
	{
		m_clock.m_duration = m_entries->m_animation->duration;
		m_clock.m_rate = rate;
		m_clock.Rewind(startTime);
		m_clock.Start(startTime);
		m_state = MIXER_PLAYING;
	}
};
ZOOSTERS_ASSERT_SIZE(Eyelid, 0x50);

} // namespace TFB

#endif
