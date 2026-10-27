#include <stdlib.h>
#include <string.h>
#include "tfb/anim/vertanimtrack.h"

extern int g_animDefault;                   // GLOBAL: 0x6066ec g_animDefault (the latest track stream format id)
extern const int *g_vertAnimSortColours;    // GLOBAL: 0x62a0b0 g_vertAnimSortColours
extern const float g_floatZero;             // GLOBAL: 0x5ca5a8 g_floatZero
void operator_delete(void *ptr);    // 0x5500f0
void *MemAlloc(size_t size);              // 0x550170

using namespace TFB;

// The sort's comparison: phase offset, then the vertices' colour words (g_vertAnimSortColours, set by Sort), then the vertex index.
// 0x459f10
int CompareVertAnimEntries(const void *pa, const void *pb)
{
	const VertAnimEntry *a = (const VertAnimEntry *)pa;
	const VertAnimEntry *b = (const VertAnimEntry *)pb;
	if (a->m_phaseOffset < b->m_phaseOffset)
		return -1;
	if (a->m_phaseOffset > b->m_phaseOffset)
		return 1;
	int diff = g_vertAnimSortColours[a->m_vertex] - g_vertAnimSortColours[b->m_vertex];
	if (diff)
		return diff;
	return a->m_vertex - b->m_vertex;
}

// 0x459eb0
int VertAnimEntry::Read(RwStream *stream)
{
	if (!RwStreamReadInt32(stream, &m_vertex, 4))
		return 0;
	return RwStreamReadReal(stream, &m_phaseOffset, 4) ? 8 : 0;
}

// 0x459ef0
VertAnimTrack::VertAnimTrack()
{
	m_masterId = 0;
	m_count = 0;
	m_dupCount = 0;
	m_entries = 0;
	m_sorted = 0;
	m_format = g_animDefault;
}

// 0x45ab50
__declspec(noinline) VertAnimTrack::~VertAnimTrack()
{
	if (m_entries)
		operator_delete(m_entries);
}

// 0x45a300
int VertAnimTrack::Read(RwStream *stream)
{
	int flags;
	if (!RwStreamReadInt32(stream, &m_format, 4))
		return 0;
	if (!RwStreamReadInt32(stream, &flags, 4))
		return 0;
	m_flags.Set(flags);
	if (!RwStreamReadInt32(stream, &m_masterId, 4))
		return 0;
	if (!RwStreamReadInt32(stream, (int *)&m_count, 4))
		return 0;
	int size = 0x10;
	if (m_count > 0) {
		m_entries = (VertAnimEntry *)MemAlloc(m_count * sizeof(VertAnimEntry));
		if (!m_entries)
			return 0;
		for (unsigned i = 0; i < m_count; i++)
			size += m_entries[i].Read(stream);
	}
	return size;
}

// 0x45a2e0
int VertAnimTrack::SerialSize()
{
	unsigned size = 0x10;
	if (m_count > 0)
		size = m_count * 8 + 0x10;
	return size;
}

// 0x45a1c0
VertAnimTrack *VertAnimTrack::Assign(const VertAnimTrack *from)
{
	m_count = from->m_count;
	m_dupCount = from->m_dupCount;
	m_masterId = from->m_masterId;
	m_format = from->m_format;
	m_flags.Set(from->m_flags.m_bits);
	unsigned total = m_count + m_dupCount;
	m_entries = (VertAnimEntry *)MemAlloc(total * sizeof(VertAnimEntry));
	for (unsigned i = 0; i < total; i++) {
		m_entries[i].m_vertex = from->m_entries[i].m_vertex;
		m_entries[i].m_twinVertex = from->m_entries[i].m_twinVertex; // the phase offset's bits
	}
	return this;
}

static int ColourWord(const RwRGBA *colours, int vertex)
{
	return *(const int *)(colours + vertex);
}

static void BindEntry(VertAnimEntry *entry, RwRGBA *colours)
{
	unsigned char *colour = (unsigned char *)(colours + entry->m_vertex);
	entry->m_colour = colour;
	entry->m_baseColour[0] = (float)colour[0];
	entry->m_baseColour[1] = (float)colour[1];
	entry->m_baseColour[2] = (float)colour[2];
	entry->m_baseColour[3] = (float)colour[3];
}

// 0x459f60
bool VertAnimTrack::Sort(RwRGBA *colours)
{
	if (m_sorted)
		return true;
	MasterVertAnimation *master = g_masterVertAnimList;
	while (master && master->m_id != m_masterId)
		master = master->m_next;
	if (!master)
		return false;
	int keyCount = master->m_keyCount;
	if (keyCount <= 0 || m_count == 0)
		return false;
	const VertAnimKey *curve = master->m_keys;
	if (curve[0].m_time > 0.0f)
		return false;
	if (curve[keyCount - 1].m_time < 1.0f)
		return false;
	m_master = master;
	if (master->m_reversed)
		m_flags.Or(VERTTRACK_REVERSED);
	m_sorted = 1;
	g_vertAnimSortColours = (const int *)colours;
	qsort(m_entries, m_count, sizeof(VertAnimEntry), CompareVertAnimEntries);
	m_dupCount = m_count;
	// entry 0 is filled before the loop, as in the game; a shifted-down entry is filled again
	const unsigned char *map = (const unsigned char *)colours;
	{
		VertAnimEntry *entry = &m_entries[0];
		entry->m_colour = (unsigned char *)(map + entry->m_vertex * 4);
		entry->m_baseColour[0] = (float)map[entry->m_vertex * 4];
		entry->m_baseColour[1] = (float)map[entry->m_vertex * 4 + 1];
		entry->m_baseColour[2] = (float)map[entry->m_vertex * 4 + 2];
		entry->m_baseColour[3] = (float)map[entry->m_vertex * 4 + 3];
	}
	for (unsigned i = 1; i < m_count; i++) {
		VertAnimEntry *entry = &m_entries[i];
		entry->m_colour = (unsigned char *)(map + entry->m_vertex * 4);
		entry->m_baseColour[0] = (float)map[entry->m_vertex * 4];
		entry->m_baseColour[1] = (float)map[entry->m_vertex * 4 + 1];
		entry->m_baseColour[2] = (float)map[entry->m_vertex * 4 + 2];
		entry->m_baseColour[3] = (float)map[entry->m_vertex * 4 + 3];
		if (m_entries[i - 1].m_phaseOffset == entry->m_phaseOffset &&
		    *(const int *)(map + m_entries[i - 1].m_vertex * 4) == *(const int *)(map + entry->m_vertex * 4)) {
			// Same phase and colour as the previous entry: moved behind the m_count live entries; Apply copies its colour from the twin.
			VertAnimEntry dropped = *entry;
			m_count--;
			memmove(entry, entry + 1, (m_count - i) * sizeof(VertAnimEntry)); // memmove: the ranges overlap; sizeof, not 0x1c (entries are 0x20 on x64)
			// only vertex, colour pointer and the twin word of the vacated last slot are written (its base colour keeps what the slot held)
			m_entries[m_count].m_vertex = dropped.m_vertex;
			m_entries[m_count].m_colour = (unsigned char *)(map + dropped.m_vertex * 4);
			m_entries[m_count].m_twinVertex = m_entries[i - 1].m_vertex;
			i--;
		}
	}
	m_dupCount -= m_count;
	return true;
}

// Folds a phase into [0, period]: reflected (ping-pong) or wrapped beyond 1.
static float WrapPhase(float phase, float period)
{
	if (phase > 1.0f) {
		if (!(phase > period))
			phase = period - phase;
		else
			phase = phase - period;
	}
	return phase;
}

// 0x45a840, the per-track part
static void ApplySortedTrack(VertAnimTrack *track, RwRGBA *colours)
{
	MasterVertAnimation *master = track->m_master;
	float period = (master->m_modeFlags.Get() & 7) == 3 ? 2.0f : 1.0f;
	VertAnimEntry *entries = track->m_entries;
	// `fst` stores the float-rounded sum before the wrap; the loop wraps the unrounded one
	float previous = WrapPhase(entries[0].m_phaseOffset + master->m_clock.m_phase, period);
	// Binary search for the first key after the first entry's phase (bottom-tested, no lo <= hi guard before the first probe).
	int lo = 0;
	int hi = master->m_keyCount - 1;
	VertAnimKey *keys = master->m_keys;
	do {
		int mid = (lo + hi) >> 1;
		if (!(keys[mid].m_time <= previous)) // game: greater or NaN goes left
			hi = mid - 1;
		else
			lo = mid + 1;
	} while (lo <= hi);
	VertAnimKey *key = &keys[lo];
	int channels = master->m_channels;
	int firstChannel;
	int endChannel;
	if (channels == VERTANIM_ALPHA) {
		firstChannel = channels; // the alpha only (channel 3)
		endChannel = 4;
	} else {
		firstChannel = 0;
		endChannel = 3 + (channels != VERTANIM_RGB);
	}
	unsigned r = 0;
	do { // the sort guarantees at least one entry
		float phase = WrapPhase(entries[r].m_phaseOffset + master->m_clock.m_phase, period);
		if (phase < previous) // game: `test ah,5; jp` skips on phase >= previous and on NaN
			key = keys;
		previous = phase;
		while (phase >= key->m_time)
			key++;
		float fraction = (phase - key[-1].m_time) / (key->m_time - key[-1].m_time);
		unsigned char *dst = entries[r].m_colour;
		int ch;
		switch (master->m_blendMode) {
		case VERTANIM_REPLACE:
			ch = firstChannel;
			do {
				float a0 = key[-1].m_colour[ch];
				float value = (key->m_colour[ch] - a0) * fraction + a0;
				dst[ch] = (unsigned char)(int)(value * 255.0f);
			} while (++ch < endChannel);
			break;
		case VERTANIM_ADD:
			ch = firstChannel;
			do {
				float a0 = key[-1].m_colour[ch];
				float value = (key->m_colour[ch] - a0) * fraction + a0;
				int sum = (int)(value * 255.0f + entries[r].m_baseColour[ch]);
				dst[ch] = sum < 0 ? 0 : sum >= 255 ? 255 : (unsigned char)sum;
			} while (++ch < endChannel);
			break;
		case VERTANIM_MULTIPLY:
			ch = firstChannel;
			do {
				float a0 = key[-1].m_colour[ch];
				float value = (key->m_colour[ch] - a0) * fraction + a0;
				int product = (int)(value * entries[r].m_baseColour[ch]);
				dst[ch] = product >= 255 ? 255 : (unsigned char)product;
			} while (++ch < endChannel);
			break;
		}
	} while (++r < track->m_count);
	for (unsigned q = 0; q < track->m_dupCount; q++) {
		const VertAnimEntry &dup = entries[r + q];
		*(int *)dup.m_colour = ColourWord(colours, dup.m_twinVertex);
	}
}

// 0x45a840
void ApplyVertAnimTracks(VertAnimTrack *tracks, int count, RwRGBA *colours)
{
	do {
		if (tracks->m_sorted || tracks->Sort(colours))
			ApplySortedTrack(tracks, colours);
		tracks++;
	} while (--count != 0);
}
