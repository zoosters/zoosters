// RWA objects handed to the game (rwaudio.cpp): the game's own structs (rw/rwaudio.h) first, so both pointer sizes agree, then the
// shim's bookkeeping. Compile-checked by rwviewcheck.cpp.
#ifndef PORT_SHIM_RWAUDIO_LAYOUTS_H
#define PORT_SHIM_RWAUDIO_LAYOUTS_H
#include "rw/rwaudio.h"
#include "rwaudio_wavedict.h"

// Every voice / stream / output / listener is a zeroed block of this size; the game-visible object is at its start, our clock behind it.
enum { kAudioBlockBytes = 0x1000, kVoiceClockOffset = 0x200, kStreamStateOffset = 0xd6c, kStreamClockOffset = 0xe00 };

// A wave dictionary (RwaWaveDict, opaque to the game): the waves of one 0x809 chunk.
struct RwaWaveObj;
struct RwaWaveDict { unsigned magic; char *name; unsigned count; RwaWaveObj **waves; };

// A wave as the game sees it (RwaWave: format at +0x10, target format at +0x2c on x86); the tail is ours.
struct RwaWaveObj {
	RwaWave pub;
	unsigned char *data;            // sample data (PCM 16-bit LE)
	unsigned dataLen;
	unsigned create;                // create flags (bit 0 = 3D)
	char *name;
	unsigned char typeFmt[16], typeTarget[16];   // the UUIDs the formats' dataType pointers point to
	RwaWaveDict *dict;              // owning dictionary (0 = standalone)
	unsigned magic;
};

// Virtual voice: the play clock. The position is in samples and advances by (clock x frequency) while triggered.
struct VoiceClock { unsigned magic; int playing; double base, t0; };      // base = position in samples at t0
struct RwaVoiceObj {
	union { RwaVirtualVoice pub; char block[kVoiceClockOffset]; };
	VoiceClock clock;
};

// Stream: the .rws media, segment and play clock; the observed play state (RWA keeps it at +0xd6c) is what _rwaStreamGetPlayState returns.
struct StreamClock {
	unsigned magic; int playing, loop, seg;
	int cueing, playPending;        // a cue (request 3) in progress since cueT0; a play request waiting for it (stop -> play only)
	double cueT0;
	float endAt;                    // seconds of play after which the stream reports the end
	double base, t0;                // position in seconds at t0 (runs on past the length by up to kEndLatency)
	float length, pitch;            // segment length in seconds (0 = unknown), pitch ratio
	WdRws *rws;
	char path[260], segName[128];
	unsigned gen;                   // bumps when a cue or a new media / segment restarts the stream (the device restarts at the position)
	float gain[2][2];               // _rwaStreamSetStreamGain: [pair][side]
};
struct RwaStream {
	char m_pad00[kStreamStateOffset];
	int playState;                  // +0xd6c observed play state
	char m_pad70[kStreamClockOffset - kStreamStateOffset - 4];
	StreamClock clock;              // +0xe00
};
static_assert(sizeof(RwaVoiceObj) <= kAudioBlockBytes && sizeof(RwaVirtualVoice) <= kVoiceClockOffset, "voice object fits its block");
static_assert(sizeof(RwaStream) <= kAudioBlockBytes, "stream object fits its block");
static_assert(sizeof(RwaOsOutput) <= kAudioBlockBytes && sizeof(RwaListener) <= kAudioBlockBytes, "output / listener fit their blocks");
#endif
