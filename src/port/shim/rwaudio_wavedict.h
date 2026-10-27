// RWA data readers: wave dictionaries (chunk 0x809) and .rws stream headers (0x80D). Plain C++ over a byte source, host-buildable
// (rwaudio_wavedict.cpp).
#ifndef PORT_RWAUDIO_WAVEDICT_H
#define PORT_RWAUDIO_WAVEDICT_H

struct WdSource {                      // sequential byte source: read returns the number of bytes delivered
	unsigned (*read)(void *ctx, void *dst, unsigned n);
	void *ctx;
};

struct WdFormat {                      // RwaWaveFormat as serialised (first 0x1C bytes + data type UUID)
	unsigned rate, length;             // +0 Hz, +8 data length in bytes
	unsigned char bits, channels;      // +0xC, +0xD
	unsigned char flags, flags2;       // +0x18, +0x19
	unsigned miscSize;                 // +0x14
	int hasType;                       // +4 pointer non-zero: `type` valid
	unsigned char type[16];            // data type UUID, GUID byte order
};

struct WdWave {
	char *name;                        // lower-case sound name (may be 0)
	WdFormat fmt, target;              // stored format, format to create as
	unsigned flags, create;            // header flags (bit 1 = name ...), create flags (bit 0 = 3D)
	unsigned char *data;               // sample data (malloc; the caller may take it and clear the field)
	unsigned dataLen;
};

struct WdDict {
	char *name;                        // dictionary name or 0
	unsigned count;
	WdWave *waves;
};

// Reads a dictionary whose 0x809 chunk header is already consumed: 0x80A header, 0x80C wave list. 0 + message on failure.
WdDict *wdReadDict(WdSource *src, char *err, int errSize);
void wdFreeDict(WdDict *d);

struct WdSegment { char *name; unsigned size, offset, codecBytes; };
struct WdRws {
	char *name;
	unsigned nseg;
	WdSegment *segs;
	unsigned nsub;
	unsigned samplesPerFrame, frameSize;   // substream 0 (+0xC, +0x1A)
	WdFormat fmt;                          // substream 0 format
	unsigned blockSize, dataOffset;
	unsigned codecPerBlock;                // substream 0 (+0x20): frame bytes at the start of each block (0x2010)
};
// Reads the header of a stream file from its start (0x80D, 0x80E, prefix, body).
WdRws *wdReadRws(WdSource *src, char *err, int errSize);
void wdFreeRws(WdRws *r);
// Index of the segment called `name` (case-insensitive) or -1.
int wdFindSegment(const WdRws *r, const char *name);
// Samples of a segment as FUN_00508200 (0x508200) computes them: codec bytes * samples per frame / frame size / channels.
unsigned wdSegmentSamples(const WdRws *r, int seg);

// Samples in `bytes` of data of the given layout (RwaWaveFormatGetSamples 0x50A5B0). type = UUID (GUID byte order) or 0 (PCM).
unsigned wdSamples(const unsigned char *type, unsigned bits, unsigned channels, unsigned bytes);

// One 36 x channels byte frame (Xbox IMA ADPCM: per channel s16 first sample, u8 step index, u8 0, then u32 nibble words L/R in turn)
// -> 64 samples per channel, interleaved (_rwaWaveConvXADPCMDecodeMono/Stereo 0x517910/0x517ac0).
void wdDecodeXboxIma(const unsigned char *frame, int channels, short *out);
// Codec frames (36 x channels bytes) of segment `seg`, and the file offset of frame `k` (blocks of blockSize, the first codecPerBlock
// bytes of each are frames; seek as 0x514490: dataOffset + segment offset + block x blockSize). -1 / 0 if out of range.
unsigned wdSegmentFrames(const WdRws *r, int seg);
long wdFrameOffset(const WdRws *r, int seg, unsigned k);

#endif
