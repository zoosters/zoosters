// RWA data readers (wave dictionaries, .rws headers). Plain C++ over a byte source, no RenderWare, no pointer-size assumptions.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rwaudio_wavedict.h"
#ifdef _WIN32
#define snprintf _snprintf          // MSVC 7.1 CRT
#endif

static unsigned rd32(const unsigned char *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned)p[3] << 24); }
static unsigned short rd16(const unsigned char *p) { return (unsigned short)(p[0] | (p[1] << 8)); }
static void fail(char *err, int n, const char *msg, unsigned a = 0) { if (err && n > 0) snprintf(err, n, "%s (%#x)", msg, a); }
static int take(WdSource *s, void *dst, unsigned n) { return n == 0 || s->read(s->ctx, dst, n) == n; }

struct Chunk { unsigned type, size, ver; };
static int readChunk(WdSource *s, Chunk *c) { unsigned char h[12]; if (!take(s, h, 12)) return 0; c->type = rd32(h); c->size = rd32(h + 4); c->ver = rd32(h + 8); return 1; }

// A cursor over an in-memory chunk payload.
struct Cur { const unsigned char *b; unsigned n, p; };
static int cTake(Cur *c, unsigned n, const unsigned char **out) { if (n > c->n - c->p) return 0; *out = c->b + c->p; c->p += n; return 1; }
// aligned string: text, NUL, filler up to (strlen + 16) & ~15 bytes (>= 1 NUL)
static int cAstr(Cur *c, char **out)
{
	unsigned e = c->p;
	while (e < c->n && c->b[e]) e++;
	if (e >= c->n) return 0;
	unsigned len = e - c->p, adv = (len + 16) & ~15u;
	if (adv > c->n - c->p) return 0;
	char *s = (char *)malloc(len + 1);
	if (!s) return 0;
	memcpy(s, c->b + c->p, len);
	s[len] = 0;
	*out = s;
	c->p += adv;
	return 1;
}
// RwaWaveFormat: 0x1C bytes (+ UUID if the type pointer is set, + miscSize bytes if the misc pointer is set)
static int cFormat(Cur *c, WdFormat *f)
{
	const unsigned char *b;
	if (!cTake(c, 0x1C, &b)) return 0;
	memset(f, 0, sizeof *f);
	f->rate = rd32(b); f->length = rd32(b + 8); f->bits = b[0xC]; f->channels = b[0xD];
	f->miscSize = rd32(b + 0x14); f->flags = b[0x18]; f->flags2 = b[0x19];
	if (rd32(b + 4)) {
		const unsigned char *u;
		if (!cTake(c, 16, &u)) return 0;
		memcpy(f->type, u, 16);
		f->hasType = 1;
	}
	if (rd32(b + 0x10)) {
		const unsigned char *m;
		if (!cTake(c, f->miscSize, &m)) return 0;
	}
	return 1;
}

// ---------------------------------------------------------------- samples per data format
static const unsigned char kPCM[16]  = { 0x17, 0xd2, 0x1b, 0xd0, 0x87, 0x35, 0xed, 0x4e, 0xb9, 0xd9, 0xb8, 0xe8, 0x6e, 0xa9, 0xb9, 0x95 };
static const unsigned char kIMA[16]  = { 0x93, 0x65, 0x38, 0xef, 0x11, 0xb6, 0x2d, 0x43, 0x95, 0x7f, 0xa7, 0x1a, 0xde, 0x44, 0x22, 0x7a };
static const unsigned char kXADPCM[16] = { 0x2b, 0xa2, 0x2f, 0x63, 0xdd, 0x11, 0x8f, 0x45, 0xaa, 0x27, 0xa5, 0xc3, 0x46, 0xe9, 0x79, 0x0e };
static const unsigned char kVAG[16]  = { 0x98, 0x97, 0xea, 0xd9, 0xbc, 0xbb, 0x7b, 0x44, 0x96, 0xb2, 0x65, 0x47, 0x59, 0x10, 0x2e, 0x16 };
static const unsigned char kGCN[16]  = { 0xb0, 0x15, 0x62, 0xf8, 0xd5, 0x31, 0x29, 0x4c, 0xbd, 0x37, 0xcd, 0xbf, 0x9b, 0xd1, 0x0c, 0x53 };
static const unsigned char kFLOAT[16] = { 0x82, 0x43, 0x1e, 0xda, 0x99, 0x2c, 0x61, 0x4c, 0xad, 0x99, 0x7f, 0x36, 0x4b, 0x21, 0x15, 0x37 };

unsigned wdSamples(const unsigned char *type, unsigned bits, unsigned channels, unsigned bytes)
{
	if (!channels) channels = 1;
	unsigned frameBytes, frameSamples;          // bytes and samples of one frame of ONE channel (RwaWaveFormatGetFrameSize / ...GetSamplesPerAudioFrame)
	if (type && (!memcmp(type, kIMA, 16) || !memcmp(type, kXADPCM, 16))) { frameBytes = 36; frameSamples = 64; }
	else if (type && !memcmp(type, kVAG, 16)) { frameBytes = 16; frameSamples = 28; }
	else if (type && !memcmp(type, kGCN, 16)) { frameBytes = 8; frameSamples = 14; }
	else if (type && !memcmp(type, kFLOAT, 16)) { frameBytes = 4; frameSamples = 1; }
	else { frameBytes = bits / 8; frameSamples = 1; }                           // PCM (kPCM) and unknown types
	if (!frameBytes) return 0;
	return bytes / (frameBytes * channels) * frameSamples;
}

// ---------------------------------------------------------------- wave dictionary
void wdFreeDict(WdDict *d)
{
	if (!d) return;
	for (unsigned i = 0; i < d->count; i++) { free(d->waves[i].name); free(d->waves[i].data); }
	free(d->waves);
	free(d->name);
	free(d);
}

static int readWave(WdSource *s, WdWave *w, char *err, int errn)
{
	Chunk c;
	if (!readChunk(s, &c) || c.type != 0x802) { fail(err, errn, "wave chunk 0x802 expected", c.type); return 0; }
	unsigned wsize = c.size;
	if (!readChunk(s, &c) || c.type != 0x803 || c.size > 4096) { fail(err, errn, "wave header 0x803 expected", c.type); return 0; }
	unsigned hsize = c.size;
	unsigned char hb[4096];
	if (!take(s, hb, hsize)) { fail(err, errn, "short wave header"); return 0; }
	Cur h = { hb, hsize, 0 };
	const unsigned char *b;
	if (!cTake(&h, 4, &b)) return 0;
	w->flags = rd32(b);
	if (w->flags & ~0xFu) { fail(err, errn, "wave header flags", w->flags); return 0; }
	if (!cFormat(&h, &w->fmt) || !cFormat(&h, &w->target) || !cTake(&h, 4, &b)) { fail(err, errn, "wave header formats"); return 0; }
	w->create = rd32(b);
	if ((w->flags & 1) && !cTake(&h, 16, &b)) return 0;
	if ((w->flags & 2) && !cAstr(&h, &w->name)) { fail(err, errn, "wave name"); return 0; }
	if (!readChunk(s, &c) || c.type != 0x804) { fail(err, errn, "wave data 0x804 expected", c.type); return 0; }
	if (c.size > 0x10000000u || wsize != 24 + hsize + c.size) { fail(err, errn, "wave chunk sizes disagree", wsize); return 0; }
	w->dataLen = c.size;
	w->data = (unsigned char *)malloc(c.size ? c.size : 1);
	if (!w->data || !take(s, w->data, c.size)) { fail(err, errn, "wave data"); return 0; }
	return 1;
}

WdDict *wdReadDict(WdSource *s, char *err, int errn)
{
	Chunk c;
	if (!readChunk(s, &c) || c.type != 0x80A || c.size < 0x24 || c.size > 0x10000) { fail(err, errn, "0x80A dictionary header expected", c.type); return 0; }
	unsigned char hb[0x10000];
	if (!take(s, hb, c.size)) { fail(err, errn, "short dictionary header"); return 0; }
	if (hb[0x1B] & 8) { fail(err, errn, "byte-swapped dictionary not supported"); return 0; }
	WdDict *d = (WdDict *)calloc(1, sizeof *d);
	if (!d) return 0;
	Cur h = { hb, c.size, 0x24 };
	const unsigned char *u;
	if (rd32(hb) && !cTake(&h, 16, &u)) { fail(err, errn, "dictionary uuid"); wdFreeDict(d); return 0; }
	if (rd32(hb + 4) && !cAstr(&h, &d->name)) { fail(err, errn, "dictionary name"); wdFreeDict(d); return 0; }
	unsigned char cnt[4];
	if (!readChunk(s, &c) || c.type != 0x80C || !take(s, cnt, 4)) { fail(err, errn, "0x80C wave list expected", c.type); wdFreeDict(d); return 0; }
	unsigned n = rd32(cnt);
	if (n > 100000) { fail(err, errn, "wave count", n); wdFreeDict(d); return 0; }
	d->waves = (WdWave *)calloc(n ? n : 1, sizeof(WdWave));
	if (!d->waves) { wdFreeDict(d); return 0; }
	for (unsigned i = 0; i < n; i++) {
		d->count = i + 1;                         // so a failed wave is freed too
		if (!readWave(s, &d->waves[i], err, errn)) { wdFreeDict(d); return 0; }
	}
	return d;
}

// ---------------------------------------------------------------- .rws stream header
void wdFreeRws(WdRws *r)
{
	if (!r) return;
	for (unsigned i = 0; i < r->nseg; i++) free(r->segs[i].name);
	free(r->segs);
	free(r->name);
	free(r);
}

int wdFindSegment(const WdRws *r, const char *name)
{
	if (!r || !name) return -1;
	for (unsigned i = 0; i < r->nseg; i++) {
		const char *a = r->segs[i].name, *b = name;
		if (!a) continue;
		for (; *a && *b; a++, b++) {
			int x = *a, y = *b;
			if (x >= 'A' && x <= 'Z') x += 32;
			if (y >= 'A' && y <= 'Z') y += 32;
			if (x != y) break;
		}
		if (!*a && !*b) return (int)i;
	}
	return -1;
}

unsigned wdSegmentSamples(const WdRws *r, int seg)
{
	if (!r || seg < 0 || (unsigned)seg >= r->nseg) return 0;
	unsigned ch = (r->fmt.flags & 4) ? 1 : r->fmt.channels;
	if (!r->frameSize || !ch) return 0;
	return (r->segs[seg].codecBytes * r->samplesPerFrame / r->frameSize) / ch;      // 32-bit like the game's imul/div
}

WdRws *wdReadRws(WdSource *s, char *err, int errn)
{
	Chunk c;
	if (!readChunk(s, &c) || c.type != 0x80D) { fail(err, errn, "0x80D stream chunk expected", c.type); return 0; }
	if (!readChunk(s, &c) || c.type != 0x80E) { fail(err, errn, "0x80E header chunk expected", c.type); return 0; }
	unsigned char pre[0x14];
	if (!take(s, pre, 0x14)) { fail(err, errn, "short prefix"); return 0; }
	unsigned bodyN = rd32(pre);
	if (bodyN < 0x2C || bodyN > 0x400000 || bodyN > c.size - 0x14) { fail(err, errn, "header body size", bodyN); return 0; }
	unsigned char *body = (unsigned char *)malloc(bodyN);
	if (!body || !take(s, body, bodyN)) { free(body); fail(err, errn, "short header body"); return 0; }
	WdRws *r = (WdRws *)calloc(1, sizeof *r);
	if (!r) { free(body); return 0; }
	Cur h = { body, bodyN, 0x2C };
	const unsigned char *u;
	unsigned nseg = rd32(body + 0xC), nsub = rd32(body + 0x14);
	r->blockSize = rd32(body + 0x20); r->dataOffset = rd32(body + 0x24);
	if (nseg > 100000 || nsub == 0 || nsub > 64) { fail(err, errn, "segment/substream count", nseg); goto bad; }
	r->nseg = nseg; r->nsub = nsub;
	if (rd32(body) && !cTake(&h, 16, &u)) goto bad;
	if (rd32(body + 4) && !cAstr(&h, &r->name)) goto bad;
	{
		const unsigned char *segRaw;
		if (!cTake(&h, nseg * 0x20, &segRaw)) goto bad;
		r->segs = (WdSegment *)calloc(nseg ? nseg : 1, sizeof(WdSegment));
		if (!r->segs) goto bad;
		for (unsigned i = 0; i < nseg; i++) {
			r->segs[i].size = rd32(segRaw + i * 0x20 + 0x18);
			r->segs[i].offset = rd32(segRaw + i * 0x20 + 0x1C);
			r->segs[i].codecBytes = r->segs[i].size;           // without a sizes table the whole segment
		}
		for (unsigned i = 0; i < nseg; i++)
			if (rd32(segRaw + i * 0x20 + 0xC)) {
				const unsigned char *sz;
				if (!cTake(&h, nsub * 4, &sz)) goto bad;
				r->segs[i].codecBytes = rd32(sz);               // substream 0
			}
		for (unsigned i = 0; i < nseg; i++)
			if (rd32(segRaw + i * 0x20) && !cTake(&h, 16, &u)) goto bad;
		for (unsigned i = 0; i < nseg; i++)
			if (rd32(segRaw + i * 0x20 + 4) && !cAstr(&h, &r->segs[i].name)) { fail(err, errn, "segment name", i); goto bad; }
		// markers (none in the PC files): 0x14 bytes each, then their UUIDs and names
		unsigned totalMarkers = 0;
		for (unsigned i = 0; i < nseg; i++) totalMarkers += rd32(segRaw + i * 0x20 + 0x10);
		if (totalMarkers > 0) {
			const unsigned char *mk;
			if (totalMarkers > bodyN / 0x14 || !cTake(&h, totalMarkers * 0x14, &mk)) goto bad;
			for (unsigned i = 0; i < totalMarkers; i++) if (rd32(mk + i * 0x14) && !cTake(&h, 16, &u)) goto bad;
			for (unsigned i = 0; i < totalMarkers; i++) if (rd32(mk + i * 0x14 + 4)) { char *tmp = 0; if (!cAstr(&h, &tmp)) goto bad; free(tmp); }
		}
	}
	{
		const unsigned char *sub;
		if (!cTake(&h, nsub * 0x28, &sub)) goto bad;
		r->samplesPerFrame = rd32(sub + 0xC);
		r->frameSize = rd16(sub + 0x1A);
		r->codecPerBlock = rd32(sub + 0x20);
		if (rd32(sub + 0x14)) {
			if (!cFormat(&h, &r->fmt)) { fail(err, errn, "substream format"); goto bad; }
		}
	}
	free(body);
	return r;
bad:
	if (err && errn > 0 && !err[0]) fail(err, errn, "stream header body does not parse");
	free(body);
	wdFreeRws(r);
	return 0;
}

// ---------------------------------------------------------------- Xbox IMA ADPCM (streams)
static const int kImaIndex[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
static const short kImaStep[89] = { 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97,
	107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
	1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
	12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 };
void wdDecodeXboxIma(const unsigned char *f, int ch, short *out)
{
	int s[2], idx[2];
	for (int c = 0; c < ch; c++) {
		s[c] = (short)(f[4 * c] | f[4 * c + 1] << 8); idx[c] = f[4 * c + 2] > 88 ? 88 : f[4 * c + 2];
		out[c] = (short)s[c];
	}
	for (int w = 0; w < 8; w++)
		for (int c = 0; c < ch; c++) {
			const unsigned char *p = f + 4 * ch + 4 * (w * ch + c);
			unsigned v = p[0] | p[1] << 8 | p[2] << 16 | (unsigned)p[3] << 24;
			for (int j = 0; j < (w < 7 ? 8 : 7); j++) {
				int nib = (v >> (4 * j)) & 15, d = (kImaStep[idx[c]] * ((nib & 7) * 2 + 1)) >> 3;
				s[c] = nib & 8 ? s[c] - d : s[c] + d;
				if (s[c] < -32768) s[c] = -32768; else if (s[c] > 32767) s[c] = 32767;
				idx[c] += kImaIndex[nib]; if (idx[c] < 0) idx[c] = 0; else if (idx[c] > 88) idx[c] = 88;
				out[(1 + 8 * w + j) * ch + c] = (short)s[c];
			}
		}
}
unsigned wdSegmentFrames(const WdRws *r, int seg)
{
	unsigned fb = 36u * (r && r->fmt.channels ? r->fmt.channels : 1);
	return r && seg >= 0 && (unsigned)seg < r->nseg ? r->segs[seg].codecBytes / fb : 0;
}
long wdFrameOffset(const WdRws *r, int seg, unsigned k)
{
	unsigned fb = 36u * (r && r->fmt.channels ? r->fmt.channels : 1), per = r && r->codecPerBlock ? r->codecPerBlock / fb : 0;
	if (!per || k >= wdSegmentFrames(r, seg)) return -1;
	return (long)(r->dataOffset + r->segs[seg].offset + k / per * r->blockSize + k % per * fb);
}
