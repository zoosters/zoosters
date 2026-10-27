// A Clock that plays a key set: the colour (alpha, red, green, blue) or the alpha (first float) of a material.
#ifndef ZOOSTERS_TFB_ANIM_KFANIMATION_H
#define ZOOSTERS_TFB_ANIM_KFANIMATION_H

#include "tfb/anim/clock.h"
#include "tfb/anim/kfset.h"

extern int g_animDefaultC;

namespace TFB {

// Novtable base: its inline constructor stores no vptr, so the vptr stores come after the bases and m_clipFormat, as in the game.
// The key set has a vtable of its own (0x5D569C, a copy of KFSet's).
class KFAnimationKeys : public KFSet {};

class __declspec(novtable) KFAnimationCore : public Clock {
public:
	KFAnimationKeys m_keys; // the PS2 has it as a base
	int m_clipFormat; // stream format id; starts as g_animDefaultC
	__forceinline KFAnimationCore() : m_clipFormat(g_animDefaultC) {}
};

class KFAnimation : public KFAnimationCore {
public:
	KFAnimation();
	~KFAnimation();

	// The raw phase, not GetPhase: a reversed clock is not honoured.
	bool GetCurrentValue(KeyValue *out);

	// Reads format id, clock, key set; starts the clock at its initial time. Returns the bytes read.
	int Read(RwStream *stream);
	int ReadFormat1(RwStream *stream);
	int ReadFormat2(RwStream *stream);
	int SerialSize();
};
ZOOSTERS_ASSERT_SIZE(KFAnimation, 0x50);
ZOOSTERS_ASSERT_OFFSET(KFAnimation, m_keys, 0x28);
ZOOSTERS_ASSERT_OFFSET(KFAnimation, m_clipFormat, 0x4c);

} // namespace TFB

#endif
