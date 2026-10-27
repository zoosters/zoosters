// Animation timer: maps time to a phase in [0, 1] by play mode.
#ifndef ZOOSTERS_TFB_ANIM_CLOCK_H
#define ZOOSTERS_TFB_ANIM_CLOCK_H

#include "zoosters/base.h"
#include "script/types/scriptsurfaceanim.h"
#include "tfb/render/flags32.h"

extern const float g_floatZero; // GLOBAL: 0x5ca5a8 (clock.cpp)

namespace TFB {

enum ClockFlags {
	CLOCK_REVERSED = 1,   // phase counts down (ping-pong on its way back, or set by the stream)
	CLOCK_ENABLED = 2,    // AdvanceBy / SetTime only move an enabled clock
	CLOCK_DONE = 4,       // a clamping clock reached an end (Start / Rewind clear it)
	CLOCK_NO_MIRROR = 8   // ping-pong does not fold back past the end (MasterVertAnimation::Read)
};

enum ClockMode {
	CLOCK_MODE_CLAMP = 1,     // stops at the ends, sets CLOCK_DONE
	CLOCK_MODE_LOOP = 2,      // fmod
	CLOCK_MODE_PINGPONG = 3,
	CLOCK_MODE_RANDOM = 4
	// 5, 6 and 7 (MasterVertAnimation::Read) behave like LOOP.
};

class Clock : public Script::ScriptSurfaceAnim {
public:
	Clock();

	// timeBase: the origin SetTime measures from.
	void Start(float timeBase);
	void Rewind(float timeBase);
	// 1 - phase while CLOCK_REVERSED.
	float GetPhase();
	float GetTime();
	void AdvanceBy(float dt);
	void SetTime(float t);
	// Folds *value into [0, limit] by play mode; may set CLOCK_DONE / CLOCK_REVERSED.
	void Wrap(float *value, float limit);

	int m_unk04;         // unused
	Flags32 m_flags;     // ClockFlags
	int m_playMode;      // ClockMode
	float m_duration;    // <= 0: never moves
	float m_initialTime; // Start / Rewind restart from here
	float m_rate;
	float m_time;
	float m_phase;
	float m_timeBase;    // origin of AdvanceBy / SetTime
};
ZOOSTERS_ASSERT_SIZE(Clock, 0x28);
ZOOSTERS_ASSERT_OFFSET(Clock, m_flags, 0x08);
ZOOSTERS_ASSERT_OFFSET(Clock, m_duration, 0x10);
ZOOSTERS_ASSERT_OFFSET(Clock, m_phase, 0x20);
ZOOSTERS_ASSERT_OFFSET(Clock, m_timeBase, 0x24);

// SetTime as the game inlined it in MasterVertAnimation::Update and TexList::Update.
static inline void ClockSetTimeInline(Clock &c, float t)
{
	if (c.m_flags.Test(CLOCK_ENABLED) && !(c.m_duration <= g_floatZero)) {
		c.m_time = (t - c.m_timeBase) * c.m_rate;
		c.m_phase = c.m_time / c.m_duration;
		c.Wrap(&c.m_phase, 1.0f);
	}
}

} // namespace TFB

#endif
