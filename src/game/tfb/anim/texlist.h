// One texture animation slot of a TFB material: a texture list and the clock that picks one.
#ifndef ZOOSTERS_TFB_ANIM_TEXLIST_H
#define ZOOSTERS_TFB_ANIM_TEXLIST_H

#include "rw/rwcore.h"
#include "tfb/anim/clock.h"

class TexList {
public:
	TexList();
	~TexList();

	// Reads the format id (and Read for format 1), then sets the clock from m_modeFlags & 7 (1 once, 3 ping-pong, 4 random, else loop). Returns the bytes read.
	int Load(RwStream *stream);
	int Read(RwStream *stream);
	void Update(float t);
	RwTexture *Pick();
	int StreamSize();

	int m_format;
	float m_duration;       // seconds per cycle; Load copies it into the clock
	Flags32 m_modeFlags;    // low three bits: play mode (Load)
	unsigned m_count;
	RwTexture **m_textures;
	TFB::Clock m_clock;
};
ZOOSTERS_ASSERT_SIZE(TexList, 0x3c);
ZOOSTERS_ASSERT_OFFSET(TexList, m_modeFlags, 0x08);
ZOOSTERS_ASSERT_OFFSET(TexList, m_textures, 0x10);
ZOOSTERS_ASSERT_OFFSET(TexList, m_clock, 0x14);

#endif
