#include "tfb/anim/mastervertanimation.h"

void *MemAlloc(size_t size);           // 0x550170

extern int g_animDefaultNL; // GLOBAL: 0x6066e4 g_animDefaultNL (the latest stream format id, 5)
// GLOBAL: 0x62a0b4 g_masterVertAnimList

namespace TFB {

// GLOBAL: 0x5d56d0 TFB::MasterVertAnimation::vftable

// 0x45ae70
MasterVertAnimation::MasterVertAnimation()
{
	m_format = g_animDefaultNL;
	m_id = 0;
	m_channels = VERTANIM_RGBA;
	m_modeFlags.Set(0);
	m_keyCount = 0;
	m_keys = 0;
	m_paramA = 0;
	m_paramB = 0;
	m_blendMode = VERTANIM_REPLACE;
}

// 0x4598d0
MasterVertAnimation::~MasterVertAnimation()
{
	if (m_keys)
		operator_delete(m_keys);
}

// Makes the compiler emit the vector deleting destructor (slot 3); the plugin deletes the whole array.
// 0x45fbe0
void MasterVertAnimationDeleteArray(int count)
{
	MasterVertAnimation *array = new MasterVertAnimation[count];
	delete[] array;
}

// 0x45a7f0
void MasterVertAnimation::Update(float t)
{
	ClockSetTimeInline(m_clock, t);
}

// 0x459930
int MasterVertAnimation::SerialSize()
{
	// vectors (0x30) and parameters (0xc) add up to three key sizes
	int size = m_name.SerialSize() + 0x1c;
	return size + (m_keyCount + 3) * (int)sizeof(VertAnimKey);
}

// 0x459950
int MasterVertAnimation::ReadFormat1(RwStream *stream)
{
	int flags;
	if (!RwStreamReadInt32(stream, &m_id, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &flags, 4))
		return 0;
	m_modeFlags.Set(flags);
	if (!RwStreamReadInt32(stream, &m_channels, 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_duration, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &m_reversed, 4))
		return 0;
	int size = m_name.Read(stream) + 0x14;
	if (!RwStreamReadInt32(stream, &m_keyCount, 4))
		return 0;
	size += 4;
	if (m_keyCount > 0) {
		m_keys = (VertAnimKey *)MemAlloc(m_keyCount * 0x14);
		for (int i = 0, offset = 0; i < m_keyCount; i++, offset += 0x14) {
			char *key = (char *)m_keys + offset;
			// The stream stores the alpha before the other three channels.
			if (!RwStreamReadReal(stream, (float *)key, 4))
				return 0;
			if (!RwStreamReadReal(stream, (float *)(key + 0x10), 4))
				return 0;
			if (!RwStreamReadReal(stream, (float *)(key + 4), 4))
				return 0;
			if (!RwStreamReadReal(stream, (float *)(key + 8), 4))
				return 0;
			if (!RwStreamReadReal(stream, (float *)(key + 0xc), 4))
				return 0;
			size += 0x14;
		}
	}
	if (m_reversed)
		m_clock.m_flags.Or(CLOCK_REVERSED);
	return size;
}

// 0x459b00
int MasterVertAnimation::ReadFormat2(RwStream *stream)
{
	int size = ReadFormat1(stream);
	float *a = m_vectors[0];
	float *b = m_vectors[1];
	float *c = m_vectors[2];
	float *d = m_vectors[3];
	if (!RwStreamReadReal(stream, a, 4))
		return 0;
	if (!RwStreamReadReal(stream, a + 1, 4))
		return 0;
	if (!RwStreamReadReal(stream, a + 2, 4))
		return 0;
	if (!RwStreamReadReal(stream, b, 4))
		return 0;
	if (!RwStreamReadReal(stream, b + 1, 4))
		return 0;
	if (!RwStreamReadReal(stream, b + 2, 4))
		return 0;
	if (!RwStreamReadReal(stream, c, 4))
		return 0;
	if (!RwStreamReadReal(stream, c + 1, 4))
		return 0;
	if (!RwStreamReadReal(stream, c + 2, 4))
		return 0;
	if (!RwStreamReadReal(stream, d, 4))
		return 0;
	if (!RwStreamReadReal(stream, d + 1, 4))
		return 0;
	if (!RwStreamReadReal(stream, d + 2, 4))
		return 0;
	return size + 0x30;
}

// 0x459c30
int MasterVertAnimation::ReadFormat3(RwStream *stream)
{
	int size = ReadFormat2(stream);
	if (!RwStreamReadReal(stream, &m_paramA, 4))
		return 0;
	return size + 4;
}

// 0x459c70
int MasterVertAnimation::ReadFormat4(RwStream *stream)
{
	int size = ReadFormat2(stream);
	if (!RwStreamReadReal(stream, &m_paramA, 4))
		return 0;
	if (!RwStreamReadReal(stream, &m_paramB, 4))
		return 0;
	return size + 8;
}

// 0x459cc0
int MasterVertAnimation::ReadFormat5(RwStream *stream)
{
	int size = ReadFormat2(stream);
	// after a failed read the later ones are still attempted, as in the game
	if (!RwStreamReadReal(stream, &m_paramA, 4))
		size = 0;
	else if (!RwStreamReadReal(stream, &m_paramB, 4))
		size = 0;
	else
		size += 8;
	if (!RwStreamReadInt32(stream, &m_blendMode, 4))
		return 0;
	return size + 4;
}

// 0x459d30
int MasterVertAnimation::Read(RwStream *stream)
{
	int *format = &m_format;
	if (!RwStreamReadInt32(stream, format, 4))
		return 0;
	int size = 4;
	switch (*format) {
	case 0x0F0F0001: size = ReadFormat1(stream) + 4; break;
	case 0x0F0F0002: size = ReadFormat2(stream) + 4; break;
	case 0x0F0F0003: size = ReadFormat3(stream) + 4; break;
	case 0x0F0F0004: size = ReadFormat4(stream) + 4; break;
	case 0x0F0F0005: size = ReadFormat5(stream) + 4; break;
	}
	m_clock.m_flags.Or(CLOCK_ENABLED);
	m_clock.m_flags.Or(CLOCK_NO_MIRROR);
	float *duration = &m_duration;
	switch (m_modeFlags.Get() & 7) {
	case 1: m_clock.m_duration = *duration; m_clock.m_playMode = CLOCK_MODE_CLAMP; return size;
	case 3: m_clock.m_duration = *duration; m_clock.m_playMode = CLOCK_MODE_PINGPONG; return size;
	case 4: m_clock.m_duration = *duration; m_clock.m_playMode = 5; return size;
	case 5: m_clock.m_duration = *duration; m_clock.m_playMode = 6; return size;
	case 6: m_clock.m_duration = *duration; m_clock.m_playMode = 7; return size;
	}
	m_clock.m_duration = *duration;
	m_clock.m_playMode = CLOCK_MODE_LOOP;
	return size;
}

} // namespace TFB
