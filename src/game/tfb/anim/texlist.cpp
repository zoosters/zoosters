#include <string.h>
#include "rw/rwcore.h"
#include "tfb/anim/texlist.h"

void operator_delete(void *ptr); // 0x5500f0
// no unwind state: the game's ~TexList treats the free as nothrow
void *MemAlloc(size_t size);           // 0x550170

enum { kTextureChunk = 6 };

// 0x45af00
TexList::TexList()
{
	m_format = 0x0F0F0001;
	m_duration = 0;
	m_modeFlags.Or(1);
	m_modeFlags.Get(); // the result is unused; the game calls the accessor
	m_count = 0;
	m_textures = 0;
}

// 0x45a3e0
TexList::~TexList()
{
	for (int i = 0; i < (int)m_count; i++)
		RwTextureDestroy(m_textures[i]);
	if (m_textures)
		operator_delete(m_textures);
}

// 0x45a4b0
int TexList::Read(RwStream *stream)
{
	int flags;
	if (!RwStreamReadReal(stream, &m_duration, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &flags, 4))
		return 0;
	m_modeFlags.Set(flags);
	if (!RwStreamReadInt32(stream, (int *)&m_count, 4))
		return 0;
	if ((int)m_count > 0) {
		m_textures = (RwTexture **)MemAlloc(m_count * sizeof(RwTexture *));
		memset(m_textures, 0, m_count * sizeof(RwTexture *));
		for (int i = 0; i < (int)m_count; i++) {
			if (RwStreamFindChunk(stream, kTextureChunk, 0, 0))
				m_textures[i] = RwTextureStreamRead(stream);
		}
	}
	return 0xc;
}

// 0x45abb0
int TexList::Load(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_format, 4))
		return 0;
	int size = 4;
	if (m_format == 0x0F0F0001)
		size = Read(stream) + 4;
	TFB::Clock *clock = &m_clock;
	int two = TFB::CLOCK_ENABLED; // CLOCK_ENABLED is also the loop mode (2); the game shares the register
	clock->m_flags.Or(two);
	float *duration = &m_duration;
	switch (m_modeFlags.Get() & 7) {
	case 1:
		clock->m_duration = *duration;
		clock->m_playMode = TFB::CLOCK_MODE_CLAMP;
		return size;
	case 3:
		clock->m_duration = *duration;
		clock->m_playMode = TFB::CLOCK_MODE_PINGPONG;
		return size;
	case 4:
		clock->m_duration = *duration;
		clock->m_playMode = TFB::CLOCK_MODE_RANDOM;
		return size;
	case 2:
		clock->m_duration = *duration;
		clock->m_playMode = two;
		return size;
	}
	clock->m_duration = *duration;
	clock->m_playMode = two;
	return size;
}

// 0x45ab60
void TexList::Update(float t)
{
	ClockSetTimeInline(m_clock, t);
}

// 0x45a430
RwTexture *TexList::Pick()
{
	int count = m_count;
	int index = (int)((float)count * m_clock.m_phase);
	if (index == count)
		return m_textures[count - 1]; // the phase 1.0 shows the last texture
	return m_textures[index];
}

// 0x45a470
int TexList::StreamSize()
{
	int size = 0x10;
	for (int i = 0; i < (int)m_count; i++)
		size += RwTextureStreamGetSize(m_textures[i]) + 0xc;
	return size;
}

