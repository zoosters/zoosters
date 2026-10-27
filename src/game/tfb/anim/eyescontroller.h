// Makes a character blink: four Eyelids after a random wait (2 to 5 s) at a random rate (0.3 to 1).
#ifndef ZOOSTERS_TFB_ANIM_EYESCONTROLLER_H
#define ZOOSTERS_TFB_ANIM_EYESCONTROLLER_H

#include "tfb/anim/clock.h"
#include "tfb/anim/eyelid.h"

void *MemAlloc(size_t size); // 0x550170

namespace TFB {

enum EyesState {
	EYES_IDLE = 0,
	EYES_WAITING = 1,  // the wait clock runs, then the eyelids blink
	EYES_BLINKING = 2, // until the first eyelid is idle again
	EYES_BLINKING_3 = 3 // like EYES_BLINKING but never ends; nothing sets it
};

enum EyesFlags {
	EYES_NO_BLINK = 1 // Blink does nothing (nothing sets it)
};

class EyesController {
public:
	static void *operator new(size_t size) { return MemAlloc((unsigned)size); }
	EyesController();
	~EyesController();

	// nodeIds[0..3]: the model's eye bones. False if an eyelid could not be created or bound.
	virtual bool Create(RpHAnimHierarchy *hierarchy, const int *nodeIds, const RtAnimAnimation **anims);
	virtual void Update(float t, bool absoluteTime);

	void Blink(float rate, float startTime);
	void Reset();

	Flags32 m_flags;        // EyesFlags
	Eyelid *m_eyelids[4];   // owned
	int m_state;            // EyesState
	Clock m_waitClock;      // between blinks
	float m_minWait;
	float m_maxWait;
	float m_minBlinkRate;
	float m_maxBlinkRate;
};
ZOOSTERS_ASSERT_SIZE(EyesController, 0x54);
ZOOSTERS_ASSERT_OFFSET(EyesController, m_eyelids, 0x08);
ZOOSTERS_ASSERT_OFFSET(EyesController, m_state, 0x18);
ZOOSTERS_ASSERT_OFFSET(EyesController, m_waitClock, 0x1c);
ZOOSTERS_ASSERT_OFFSET(EyesController, m_maxBlinkRate, 0x50);

} // namespace TFB

#endif
