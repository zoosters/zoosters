#include "tfb/anim/eyelid.h"

namespace TFB {

// GLOBAL: 0x5d5780 TFB::Eyelid::vftable

// 0x45b560
unsigned char Eyelid::SetAnimation(RpHAnimHierarchy *hierarchy, int nodeId, const RtAnimAnimation **anims)
{
	return SubHierarchyMixer::Init(hierarchy, nodeId, 1, anims) != 0;
}

// 0x45b8f0
void Eyelid::Update(float t, bool absoluteTime)
{
	int state = m_state;
	if (state == MIXER_IDLE || state == 1 || state != MIXER_PLAYING) // sic: the game tests all three
		return;
	if (m_clock.m_flags.Test(CLOCK_DONE)) {
		m_state = MIXER_IDLE;
		return;
	}
	if (absoluteTime)
		m_clock.SetTime(t);
	else
		m_clock.AdvanceBy(t);
	RtAnimInterpolator *blink = m_entries->m_interpolator;
	RtAnimInterpolatorSetCurrentTime(blink, m_clock.GetTime());
	CopyFrames(blink, 0);
}

} // namespace TFB
