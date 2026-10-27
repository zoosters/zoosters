#include "framework/startup/win32/systemhook.h"
#include "zoosters/os.h"
static inline int QueryPerformanceFrequency(__int64 *freq) { *freq = (__int64)zoosters::os::PerfFrequency(); return *freq != 0; }
static inline int QueryPerformanceCounter(__int64 *count) { *count = (__int64)zoosters::os::PerfCounter(); return 1; }
static inline unsigned long GetTickCount() { return zoosters::os::TickMs(); }

extern const float g_floatZero;    // GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatOne;     // GLOBAL: 0x5ca668 g_floatOne

// 0x464e80
// GLOBAL: 0x464e80 ??0TFBSystemHook@@QAE@MM@Z
TFBSystemHook::TFBSystemHook(float maxRate, float minRate)
{
	int q = QueryPerformanceFrequency(&m_freq);
	m_minStep = g_floatOne / maxRate;
	float fa = maxRate;
	float fb = minRate;
	m_hasQpc = q;
	m_lastCounter = 0;
	m_pauseStart = 0;
	m_lastTick = 0;
	m_pauseTick = 0;
	m_realTime = 0;
	m_stepTime = 0;
	m_steps = 0;
	m_stepsPrev = 0;
	m_waits = 0;
	m_waitsPrev = 0;
	m_tickDiv = 1000;
	m_maxRate = fa;
	m_waitsPrev = 0;
	m_minRate = fb;
	m_waited = 1;
	m_maxStep = g_floatOne / minRate;
}

// 0x464f00
void TFBSystemHook::Notify()
{
	if (m_hasQpc) {
		QueryPerformanceCounter(&m_lastCounter);
		m_realTime = 0;
		m_stepTime = 0;
		m_steps = 0;
		m_waits = 0;
	} else {
		unsigned t = GetTickCount();
		m_realTime = 0;
		m_stepTime = 0;
		m_steps = 0;
		m_waits = 0;
		m_lastTick = t;
	}
}

// 0x464f40
float TFBSystemHook::Tick()
{
	if (m_hasQpc) {
		if (m_pauseStart != 0)
			return g_floatZero;
	} else if (m_pauseTick != 0) {
		return g_floatZero;
	}
	double dt;
	if (m_hasQpc) {
		__int64 now;
		QueryPerformanceCounter(&now);
		__int64 d = now - m_lastCounter;
		m_lastCounter += d;
		dt = (double)d / (double)m_freq;
	} else {
		unsigned d = GetTickCount();
		d -= m_lastTick;
		m_lastTick += d;
		dt = (double)d / (double)m_tickDiv;
	}
	m_realTime = (float)(dt + m_realTime);
	if (m_realTime >= g_floatOne) {
		m_realTime -= g_floatOne;
		m_stepsPrev = m_steps;
		m_steps = 0;
		m_waitsPrev = m_waits;
		m_waits = 0;
		m_stepTime -= g_floatOne;
	}
	if (m_maxStep + m_stepTime <= m_realTime) {
		m_stepTime = m_maxStep + m_stepTime;
		m_steps++;
		m_waited = 0;
		return m_maxStep;
	}
	if (m_minStep + m_stepTime <= m_realTime) {
		float d = m_realTime - m_stepTime;
		m_steps++;
		m_waited = 0;
		m_stepTime = d + m_stepTime;
		return d;
	}
	if (!m_waited) {
		m_waited = 1;
		m_waits++;
		return -1.0f;
	}
	return g_floatZero;
}

// 0x465090
void TFBSystemHook::Pause()
{
	if (m_hasQpc) {
		if (m_pauseStart == 0)
			QueryPerformanceCounter(&m_pauseStart);
	} else if (m_pauseTick == 0) {
		m_pauseTick = GetTickCount();
	}
}

// 0x4650d0
void TFBSystemHook::Resume()
{
	if (m_hasQpc) {
		if (m_pauseStart != 0) {
			__int64 now = 0;
			QueryPerformanceCounter(&now);
			m_lastCounter += now - m_pauseStart;
			m_pauseStart = 0;
		}
	} else if (m_pauseTick != 0) {
		m_lastTick += GetTickCount() - m_pauseTick;
		m_pauseTick = 0;
	}
}
