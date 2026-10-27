#include "tfb/anim/animmouth.h"

int FUN_0043e6f0(const char *msg);  // a no-op in the release build

namespace TFB {

enum { kEmoteEntry = 8 }; // the ninth animation: the emote

// GLOBAL: 0x5d5764 TFB::AnimMouth::vftable

// 0x45b690
AnimMouth::AnimMouth()
{
	m_timeSource = 0;
	m_timeSourceArg = 0;
	m_state = MIXER_IDLE;
	m_flags.Set(0);
	m_blended = 0;
	m_keys = 0;
}

// 0x45b6f0
AnimMouth::~AnimMouth() throw()
{
	if (m_blended)
		RtAnimInterpolatorDestroy(m_blended);
}

// 0x45b4a0
bool AnimMouth::SetAnimations(RpHAnimHierarchy *hierarchy, int nodeId, const RtAnimAnimation **anims)
{
	if (!SubHierarchyMixer::Init(hierarchy, nodeId, 9, anims))
		return false;
	RtAnimInterpolator *skeleton = m_hierarchy->currentAnim;
	m_blended = RtAnimInterpolatorCreateSubInterpolator(skeleton, m_firstNode, m_nodeCount, skeleton->maxInterpKeyFrameSize);
	RtAnimInterpolatorSetCurrentAnim(m_blended, const_cast<RtAnimAnimation *>(m_entries->m_animation));
	return true;
}

// 0x45b500
void AnimMouth::Play(KFSet *keys, MouthTimeSource timeSource, void *timeSourceArg, float rate)
{
	m_keys = keys;
	m_clock.m_duration = keys->m_length;
	m_clock.m_rate = rate;
	m_clock.Rewind(0);
	m_clock.Start(0);
	m_state = MIXER_PLAYING;
	m_timeSource = timeSource;
	m_timeSourceArg = timeSourceArg;
}

// 0x45b550
void AnimMouth::Stop()
{
	m_keys = 0;
	m_state = MIXER_IDLE;
	m_timeSource = 0;
	m_timeSourceArg = 0;
}

// 0x45b720
void AnimMouth::Update(float t, bool absoluteTime)
{
	int state = m_state;
	if (state == MIXER_IDLE || state == 1 || state != MIXER_PLAYING) // sic: the game tests all three
		return;
	if (m_keys == 0)
		return;
	if (m_clock.GetPhase() >= 1.0f) {
		m_keys = 0;
		m_state = MIXER_IDLE;
		m_timeSource = 0;
		m_timeSourceArg = 0;
		return;
	}
	if (m_timeSource != 0) {
		float time = m_timeSource(m_timeSourceArg);
		m_clock.SetTime(time);
	} else if (absoluteTime) {
		m_clock.SetTime(t);
	} else {
		m_clock.AdvanceBy(t);
	}
	KeyValue shapes;
	m_keys->GetValue(m_clock.GetPhase(), &shapes);
	float weight = shapes.v[2];
	RtAnimInterpolator *first = m_entries[(int)shapes.v[0]].m_interpolator;
	RtAnimInterpolator *second = m_entries[(int)shapes.v[1]].m_interpolator;
	if (!first)
		first = m_entries->m_interpolator;
	if (!second)
		second = m_entries->m_interpolator;
	RtAnimInterpolatorAddAnimTime(first, 0);
	RtAnimInterpolatorAddAnimTime(second, 0);
	RtAnimInterpolatorBlend(m_blended, first, second, weight);
	if (m_flags.Test(MIXER_EMOTING)) {
		FUN_0043e6f0("EMOTING!!\n");
		RtAnimInterpolator *emote = m_entries[kEmoteEntry].m_interpolator;
		RtAnimInterpolatorSetCurrentTime(emote, 0);
		RtAnimInterpolatorBlend(m_blended, m_blended, emote, 0.5f);
	}
	// jaw root keeps the skeleton's pose
	for (int i = 1; i < m_blended->numNodes; i++) {
		RtAnimInterpolator *skeleton = m_hierarchy->currentAnim;
		RpHAnimInterpFrame *to = (RpHAnimInterpFrame *)(RtAnimInterpolatorGetInterpFrames(skeleton) +
		                                                (m_firstNode + i) * skeleton->currentInterpKeyFrameSize);
		RpHAnimInterpFrame *from = (RpHAnimInterpFrame *)(RtAnimInterpolatorGetInterpFrames(m_blended) +
		                                                  m_blended->currentInterpKeyFrameSize * i);
		to->orientation = from->orientation;
		to->translation = from->translation;
	}
}

} // namespace TFB
