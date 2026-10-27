#include <math.h>
#include "tfb/anim/clock.h"
#include "tfb/math/random.h"

extern const float g_floatOne;  // GLOBAL: 0x5ca668 g_floatOne

namespace TFB {

// GLOBAL: 0x5d56fc TFB::Clock::vftable
// GLOBAL: 0x43b3d0 ??1Clock@TFB@@UAE@XZ

// 0x45acd0
__declspec(noinline) Clock::Clock()
{
	m_unk04 = 0;
	m_flags.Set(0);
	m_duration = 0;
	m_initialTime = 0;
	m_time = 0;
	m_phase = 0;
	m_timeBase = 0;
	m_playMode = CLOCK_MODE_CLAMP;
	m_rate = 1.0f;
}

// 0x4597d0
void Clock::Start(float timeBase)
{
	m_flags.Or(CLOCK_ENABLED);
	m_flags.AndNot(CLOCK_DONE);
	m_phase = m_initialTime / m_duration;
	m_timeBase = timeBase;
	m_time = m_initialTime;
}

// 0x459810
void Clock::Rewind(float timeBase)
{
	m_phase = m_initialTime / m_duration;
	m_time = m_initialTime;
	m_flags.AndNot(CLOCK_DONE);
	m_timeBase = timeBase;
}

// 0x459790
float Clock::GetPhase()
{
	if (!m_flags.Test(CLOCK_REVERSED))
		return m_phase;
	return g_floatOne - m_phase;
}

// 0x4597c0
float Clock::GetTime()
{
	return m_time;
}

// 0x459670
void Clock::Wrap(float *value, float limit)
{
	switch (m_playMode) {
	case CLOCK_MODE_CLAMP:
		if (*value > limit) {
			*value = limit;
			m_flags.Or(CLOCK_DONE);
		}
		if (*value < g_floatZero) {
			*value = 0;
			m_flags.Or(CLOCK_DONE);
		}
		break;
	case CLOCK_MODE_PINGPONG: {
		float period = limit + limit;
		*value = (float)fmod((double)*value, (double)period);
		if (*value <= limit) {
			m_flags.AndNot(CLOCK_REVERSED);
		} else if (!m_flags.Test(CLOCK_NO_MIRROR)) {
			m_flags.Or(CLOCK_REVERSED);
			*value = period - *value;
		}
		break;
	}
	case CLOCK_MODE_RANDOM:
		*value = RandRange(0, limit);
		break;
	case CLOCK_MODE_LOOP:
	default: // 5, 6, 7 loop as well
		if (*value > limit)
			*value = (float)fmod((double)*value, (double)limit);
		break;
	}
}

// 0x45a720
void Clock::AdvanceBy(float dt)
{
	if (m_flags.Test(CLOCK_ENABLED) && !(m_duration <= g_floatZero)) {
		m_time = dt * m_rate + m_time;
		m_phase = (m_time - m_timeBase) / m_duration;
		Wrap(&m_phase, 1.0f);
	}
}

// 0x45a770
void Clock::SetTime(float t)
{
	if (m_flags.Test(CLOCK_ENABLED) && !(m_duration <= g_floatZero)) {
		m_time = (t - m_timeBase) * m_rate;
		m_phase = m_time / m_duration;
		Wrap(&m_phase, 1.0f);
	}
}

} // namespace TFB
