#include "tfb/anim/kfanimation.h"

// GLOBAL: 0x6066fc g_animDefaultC (the latest clip stream format id; declared in kfanimation.h)

namespace TFB {

// GLOBAL: 0x5d56a4 TFB::KFAnimation::vftable
// GLOBAL: 0x5d569c TFB::KFAnimationKeys::vftable

// 0x45ad40
KFAnimation::KFAnimation() {}

// 0x459840
KFAnimation::~KFAnimation() {}

// compiler-made (vtable slot 3)
// 0x45a7c0

// 0x4598b0
bool KFAnimation::GetCurrentValue(KeyValue *out)
{
	return m_keys.GetValue(m_phase, out);
}

// 0x45ada0
int KFAnimation::ReadFormat1(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_playMode, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_flags.m_bits, 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_duration, 4))
		return 0;
	return m_keys.Read(stream) + 0xc;
}

// 0x45ae00
int KFAnimation::ReadFormat2(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_playMode, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_flags.m_bits, 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_duration, 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_initialTime, 4))
		return 0;
	return m_keys.Read(stream) + 0x10;
}

// 0x45af70
int KFAnimation::Read(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_clipFormat, 4))
		return 0;
	int size = 4;
	switch (m_clipFormat) {
	case 0x0F0F0001:
		size = ReadFormat1(stream) + 4;
		break;
	case 0x0F0F0002:
		size = ReadFormat2(stream) + 4;
		break;
	}
	// = Clock::Rewind(0)
	m_time = m_initialTime;
	m_phase = m_initialTime / m_duration;
	m_flags.AndNot(CLOCK_DONE);
	m_timeBase = 0;
	return size;
}

// 0x45a7e0
int KFAnimation::SerialSize()
{
	return m_keys.SerialSize() + 0x14;
}

} // namespace TFB
