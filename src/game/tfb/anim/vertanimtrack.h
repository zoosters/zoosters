// Vertex animation of a geometry or world sector: which vertices follow a MasterVertAnimation curve, at what phase offset.
#ifndef ZOOSTERS_TFB_ANIM_VERTANIMTRACK_H
#define ZOOSTERS_TFB_ANIM_VERTANIMTRACK_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "tfb/anim/mastervertanimation.h"
#include "tfb/render/flags32.h"

// 0x1C bytes in memory; the stream stores the first 8 (index, phase offset). Sort fills in the rest.
struct VertAnimEntry {
	int m_vertex;
	union {
		float m_phaseOffset; // added to the master clock's phase
		int m_twinVertex;    // after Sort, for the entries it dropped as duplicates: the vertex whose colour they copy
	};
	float m_baseColour[4];        // the vertex's colour when the track was sorted
	unsigned char *m_colour;      // the vertex's colour in the pre-light array
	int Read(RwStream *stream);  // vertex index and phase offset; returns 8, or 0 on failure
};
ZOOSTERS_ASSERT_SIZE(VertAnimEntry, 0x1c);
ZOOSTERS_ASSERT_OFFSET(VertAnimEntry, m_baseColour, 0x08);
ZOOSTERS_ASSERT_OFFSET(VertAnimEntry, m_colour, 0x18);

enum VertAnimTrackFlags {
	VERTTRACK_REVERSED = 1 // the master curve runs reversed (set by Sort from MasterVertAnimation::m_reversed)
};

class VertAnimTrack {
public:
	VertAnimTrack();
	~VertAnimTrack();

	int Read(RwStream *stream);
	int SerialSize();
	VertAnimTrack *Assign(const VertAnimTrack *from);
	// Binds to the master curve, sorts by phase offset and colour, drops entries equal to their predecessor (Apply copies them). False without a usable master (missing, no keys, curve not spanning 0..1) or entries.
	bool Sort(RwRGBA *colours);

	int m_format;                // stream format id; starts as g_animDefault
	Flags32 m_flags;
	union {
		int m_masterId;                  // the stream's id of the master, until Sort
		TFB::MasterVertAnimation *m_master; // after Sort
	};
	unsigned m_count;            // entries that follow the curve
	unsigned m_dupCount;         // entries Sort dropped as duplicates (behind the m_count ones)
	VertAnimEntry *m_entries;
	unsigned char m_sorted;
	char m_pad19[3];
};
ZOOSTERS_ASSERT_SIZE(VertAnimTrack, 0x1c);
ZOOSTERS_ASSERT_OFFSET(VertAnimTrack, m_flags, 0x04);
ZOOSTERS_ASSERT_OFFSET(VertAnimTrack, m_count, 0x0c);
ZOOSTERS_ASSERT_OFFSET(VertAnimTrack, m_entries, 0x14);
ZOOSTERS_ASSERT_OFFSET(VertAnimTrack, m_sorted, 0x18);

// Evaluates the master curves into the pre-light colours; sorts each track first if needed. count >= 1.
void ApplyVertAnimTracks(VertAnimTrack *tracks, int count, RwRGBA *colours);

#endif
