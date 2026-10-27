// Colour curve of a vertex animation plus its clock. The world plugin reads an array of them; VertAnimTrack finds its master by id.
#ifndef ZOOSTERS_TFB_ANIM_MASTERVERTANIMATION_H
#define ZOOSTERS_TFB_ANIM_MASTERVERTANIMATION_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "tfb/anim/clock.h"
#include "tfb/render/flags32.h"
#include "tfb/render/tfbstring.h"

namespace TFB {

// time 0..1, colour r g b a in 0..1
struct VertAnimKey {
	float m_time;
	float m_colour[4];
};
ZOOSTERS_ASSERT_SIZE(VertAnimKey, 0x14);

enum VertAnimChannels {
	VERTANIM_RGBA = 1,
	VERTANIM_RGB = 2,
	VERTANIM_ALPHA = 3
};

enum VertAnimBlend {
	VERTANIM_REPLACE = 1,
	VERTANIM_ADD = 2,      // the value is added to the colour (clamped to 255)
	VERTANIM_MULTIPLY = 3
};

class MasterVertAnimation : public Script::ScriptSurfaceAnim {
public:
	MasterVertAnimation();
	~MasterVertAnimation();

	// Reads format id (1..5), then sets the clock from m_modeFlags & 7 (1 once, 3 ping-pong, 4..6 loop variants, else loop). Returns the bytes read.
	int Read(RwStream *stream);
	// Formats: 1 the curve; 2 adds four vectors nothing reads; 3, 4, 5 add one parameter, two, two plus the blend mode.
	int ReadFormat1(RwStream *stream);
	int ReadFormat2(RwStream *stream);
	int ReadFormat3(RwStream *stream);
	int ReadFormat4(RwStream *stream);
	int ReadFormat5(RwStream *stream);
	int SerialSize();
	void Update(float t);

	int m_format;               // stream format id; starts as 5 (g_animDefaultNL)
	int m_id;                   // matches VertAnimTrack::m_masterId
	Flags32 m_modeFlags;        // low three bits: play mode (Read)
	int m_channels;             // VertAnimChannels
	TFBString m_name;
	float m_duration;           // seconds per cycle; Read copies it into the clock
	int m_reversed;             // nonzero: the clock starts reversed, and the tracks sorted against this curve are flagged reversed
	int m_keyCount;
	VertAnimKey *m_keys;        // sorted by time
	float m_vectors[4][4];      // format 2 and later; nothing reads them
	float m_paramA;             // format 3 and later; nothing reads it
	float m_paramB;             // format 4 and later; nothing reads it
	int m_blendMode;            // VertAnimBlend (format 5; 1 before)
	Clock m_clock;              // the tracks read its phase
	MasterVertAnimation *m_next; // next of g_masterVertAnimList
};
ZOOSTERS_ASSERT_SIZE(MasterVertAnimation, 0xa0);
ZOOSTERS_ASSERT_OFFSET(MasterVertAnimation, m_modeFlags, 0x0c);
ZOOSTERS_ASSERT_OFFSET(MasterVertAnimation, m_name, 0x14);
ZOOSTERS_ASSERT_OFFSET(MasterVertAnimation, m_keys, 0x24);
ZOOSTERS_ASSERT_OFFSET(MasterVertAnimation, m_vectors, 0x28);
ZOOSTERS_ASSERT_OFFSET(MasterVertAnimation, m_blendMode, 0x70);
ZOOSTERS_ASSERT_OFFSET(MasterVertAnimation, m_clock, 0x74);
ZOOSTERS_ASSERT_OFFSET(MasterVertAnimation, m_next, 0x9c);

} // namespace TFB

// Every MasterVertAnimation of the loaded world.
extern TFB::MasterVertAnimation *g_masterVertAnimList;

#endif
