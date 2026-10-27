// The system timer: high-resolution clock with a tick-count fallback, pause / resume, and per-second frame bookkeeping. Tick hands out the next frame's time step.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_SYSTEMHOOK_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_SYSTEMHOOK_H

#include "zoosters/base.h"

struct TFBSystemHook {
	__int64 m_freq;           // +0x00 performance counter frequency
	__int64 m_lastCounter;    // +0x08 at the last Tick / Notify (Resume adds the paused interval)
	__int64 m_pauseStart;     // +0x10 at Pause (0 = running)
	unsigned m_tickDiv;       // +0x18 1000
	unsigned m_lastTick;      // +0x1c tick-count fallback
	unsigned m_pauseTick;     // +0x20 tick-count fallback: at Pause
	int m_hasQpc;             // +0x24
	float m_maxRate;          // +0x28 ctor arg a: at most this many steps per second
	float m_minRate;          // +0x2c ctor arg b: at least this many steps per second
	float m_minStep;          // +0x30 1/a: shortest step
	float m_maxStep;          // +0x34 1/b: longest step; more lag is paid out in several steps
	unsigned char m_waited;   // +0x38 Tick returned -1 since the last step
	float m_realTime;         // +0x3c real time this second
	float m_stepTime;         // +0x40 time handed out as steps this second
	int m_steps;              // +0x44 steps this second
	int m_stepsPrev;          // +0x48 previous second
	int m_waits;              // +0x4c -1 returns (nothing due) this second
	int m_waitsPrev;          // +0x50 previous second

	TFBSystemHook(float maxRate, float minRate);
	void Notify();
	float Tick();
	void Pause();
	void Resume();
};
ZOOSTERS_ASSERT_SIZE(TFBSystemHook, 0x58);
ZOOSTERS_ASSERT_OFFSET(TFBSystemHook, m_hasQpc, 0x24);
ZOOSTERS_ASSERT_OFFSET(TFBSystemHook, m_waited, 0x38);
ZOOSTERS_ASSERT_OFFSET(TFBSystemHook, m_waitsPrev, 0x50);

extern TFBSystemHook TFBHook;   // GLOBAL: 0x62a4b8 TFBHook

#endif
