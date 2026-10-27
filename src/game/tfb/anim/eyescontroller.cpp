#include "tfb/anim/eyescontroller.h"
#include "tfb/math/random.h"

namespace TFB {

// GLOBAL: 0x5d5790 TFB::EyesController::vftable

// 0x45bae0
EyesController::EyesController()
{
	for (int i = 0; i < 4; i++)
		m_eyelids[i] = 0;
	m_minWait = 2.0f;
	m_maxWait = 5.0f;
	m_minBlinkRate = 0.3f;
	m_maxBlinkRate = 1.0f;
	m_state = EYES_WAITING;
	Clock *clock = &m_waitClock;
	m_waitClock.m_duration = RandRange(m_minWait, m_maxWait);
	clock->Rewind(clock->m_timeBase + clock->m_time);
	clock->Start(0);
}

// 0x45bb90
EyesController::~EyesController()
{
	for (int i = 0; i < 4; i++)
		delete m_eyelids[i];
}

// 0x45b9e0
bool EyesController::Create(RpHAnimHierarchy *hierarchy, const int *nodeIds, const RtAnimAnimation **anims)
{
	for (int i = 0; i < 4; i++) {
		Eyelid *eyelid = new Eyelid;
		m_eyelids[i] = eyelid;
		if (!eyelid)
			return false;
		const RtAnimAnimation *animation = anims[i];
		if (!eyelid->SetAnimation(hierarchy, nodeIds[i], &animation))
			return false;
	}
	return true;
}

// 0x45b580
void EyesController::Blink(float rate, float startTime)
{
	if (!m_flags.Test(EYES_NO_BLINK)) {
		m_state = EYES_BLINKING;
		for (int i = 0; i < 4; i++)
			m_eyelids[i]->Play(rate, startTime);
	}
}

// 0x45bab0
void EyesController::Reset()
{
	m_state = EYES_WAITING;
	m_waitClock.m_duration = RandRange(m_minWait, m_maxWait);
	Clock *clock = &m_waitClock;
	clock->Rewind(m_waitClock.m_timeBase + clock->m_time);
}

// 0x45bc10
void EyesController::Update(float t, bool absoluteTime)
{
	if (absoluteTime)
		m_waitClock.SetTime(t);
	else
		m_waitClock.AdvanceBy(t);
	int state = m_state;
	if (state == EYES_IDLE)
		return;
	if (state == EYES_WAITING) {
		if (m_waitClock.m_flags.Test(CLOCK_DONE))
			Blink(RandRange(m_minBlinkRate, m_maxBlinkRate), t);
		return;
	}
	if (state == EYES_BLINKING) {
		int first = m_eyelids[0]->m_state;
		if (first == MIXER_IDLE || first == 1) { // the blink is over
			Reset();
			return;
		}
		Eyelid **eyelid = m_eyelids;
		int n = 4;
		do {
			(*eyelid)->Update(t, absoluteTime);
			eyelid++;
		} while (--n);
	} else if (state == EYES_BLINKING_3) {
		Eyelid **eyelid = m_eyelids;
		int n = 4;
		do {
			(*eyelid)->Update(t, absoluteTime);
			eyelid++;
		} while (--n);
	}
}

} // namespace TFB
