// The game's Bink calls (tfb/level/bink.h) on binkdec: one movie at a time, read frame by frame from the file, sound through
// rwaudio_al's movie source. Clock: the sound's play position while it plays, else the steady clock from the first frame on.
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binkdec.h"
#include "tfb/level/bink.h"
#include "../os/native_path.h"
#include "../shim/rwaudio_al.h"

extern "C" FILE *port_log;
#define BLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); fflush(port_log); } } while (0)

namespace {

const double kSoundAhead = 0.5;   // seconds of sound kept queued
const double kExtrapolate = 0.05; // the AL play position moves in mixer periods: interpolate this far past its last change

struct Movie {
	BINK pub;                     // first: the game's BINK * is the Movie *
	FILE *f;
	binkdec::Header h;
	uint32_t *index;              // frames + 1 file offsets
	uint8_t *pkt, *apkt;          // one frame, one audio packet (+ kPadding)
	binkdec::Video *video;
	binkdec::Audio *audio;
	bool sound, soundDone, started, fullRange, flagsLogged;
	uint32_t audioFrame;          // next frame whose sound is read ahead
	double t0, startWall, lastClock, lastPlayedAt;
	unsigned long long lastPlayed;
	unsigned damaged;
	double decodeSec;
};

double wallSec()
{
	return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

uint32_t rl32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

bool readAt(FILE *f, uint32_t off, void *buf, size_t n)
{
	return !fseek(f, (long)off, SEEK_SET) && fread(buf, 1, n, f) == n;
}

void pumpSound(Movie *m)
{
	if (!m->sound) return;
	while (m->audioFrame < m->h.frames && alaPcmQueuedSec() < kSoundAhead) {
		uint32_t i = m->audioFrame++, start = m->index[i], size = m->index[i + 1] - start;
		uint8_t len[4];
		if (size < 4 || !readAt(m->f, start, len, 4)) break;
		uint32_t n = rl32(len);
		if (n > size - 4) break;
		if (n < 4) continue;
		if (!readAt(m->f, start + 4, m->apkt, n)) break;
		memset(m->apkt + n, 0, binkdec::kPadding);
		const int16_t *pcm;
		int frames = binkdec::AudioDecode(m->audio, m->apkt, n, &pcm);
		if (frames > 0 && !alaPcmPush(pcm, (unsigned)frames)) { BLOG("bink: sound queue full at frame %u\n", i); break; }
	}
	alaPcmPlay();
}

double clockSec(Movie *m)
{
	double now = wallSec(), t;
	if (m->sound && !m->soundDone) {
		unsigned long long p = alaPcmPlayedFrames();
		if (p != m->lastPlayed) { m->lastPlayed = p; m->lastPlayedAt = now; }
		double since = now - m->lastPlayedAt;
		t = (double)p / binkdec::AudioRate(m->audio) + (since < kExtrapolate ? since : kExtrapolate);
		if (m->audioFrame >= m->h.frames && alaPcmQueuedSec() <= 0.0) {   // sound over before the picture: steady clock from here
			m->soundDone = true;
			m->t0 = now - t;
		}
	} else {
		t = now - m->t0;
	}
	if (t < m->lastClock) t = m->lastClock;
	m->lastClock = t;
	return t;
}

void destroy(Movie *m)
{
	if (m->sound) alaPcmClose();
	binkdec::VideoClose(m->video);
	binkdec::AudioClose(m->audio);
	if (m->f) fclose(m->f);
	free(m->index);
	free(m->pkt);
	free(m->apkt);
	free(m);
}

inline uint8_t clip8(int v) { return (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v); }

} // namespace

extern "C" {

BINK *__stdcall BinkOpen(const char *name, unsigned flags)
{
	char np[512];
	const char *path = PORT_NATIVE_PATH(name ? name : "", np);
	Movie *m = (Movie *)calloc(1, sizeof(Movie));
	if (!m) return 0;
	uint8_t *hb = (uint8_t *)malloc(binkdec::kHeaderMax);
	size_t hn = 0;
	m->f = fopen(path, "rb");
	if (m->f && hb) hn = fread(hb, 1, binkdec::kHeaderMax, m->f);
	bool ok = hb && binkdec::ParseHeader(hb, hn, &m->h);
	free(hb);
	const binkdec::Header &h = m->h;
	size_t largest = 0;
	if (ok) {
		m->index = (uint32_t *)malloc(((size_t)h.frames + 1) * 4);
		ok = m->index && !fseek(m->f, (long)h.indexOffset, SEEK_SET) && fread(m->index, 4, h.frames, m->f) == h.frames;
		for (uint32_t i = 0; ok && i < h.frames; i++) m->index[i] = rl32((const uint8_t *)&m->index[i]) & ~1u;   // bit 0: key frame
		if (ok) m->index[h.frames] = h.fileSize;
		for (uint32_t i = 0; ok && i < h.frames; i++) {
			ok = m->index[i + 1] > m->index[i];
			if (ok && m->index[i + 1] - m->index[i] > largest) largest = m->index[i + 1] - m->index[i];
		}
	}
	if (ok) {
		m->pkt = (uint8_t *)malloc(largest + binkdec::kPadding);
		m->apkt = (uint8_t *)malloc(largest + binkdec::kPadding);
		m->video = binkdec::VideoOpen(h);
		ok = m->pkt && m->apkt && m->video;
	}
	if (!ok) {
		BLOG("bink: BinkOpen %s (%x): %s -> skipped\n", path, flags, !m->f ? "cannot open" : "not a Bink 1 movie the decoder reads");
		destroy(m);
		return 0;
	}
	if (h.tracks) {
		m->audio = binkdec::AudioOpen(h, 0);
		m->sound = m->audio && alaPcmOpen(binkdec::AudioRate(m->audio), binkdec::AudioChannels(m->audio));
	}
	m->fullRange = binkdec::Revision(h) == 'k';
	m->pub.Width = h.width;
	m->pub.Height = h.height;
	m->pub.Frames = h.frames;
	m->pub.FrameNum = 1;
	m->pub.FrameRate = h.fpsNum;
	m->pub.FrameRateDiv = h.fpsDen;
	BLOG("bink: BinkOpen %s: BIK%c %ux%u, %u frames at %u/%u, %u track(s)%s%s\n", path, binkdec::Revision(h), h.width, h.height, h.frames,
		h.fpsNum, h.fpsDen, h.tracks, h.tracks && !m->audio ? " (sound format not supported: silent)" : "",
		m->sound ? "" : ", no sound device: steady clock");
	return &m->pub;
}

void __stdcall BinkClose(BINK *b)
{
	if (!b) return;
	Movie *m = (Movie *)b;
	BLOG("bink: BinkClose at frame %u of %u after %.2f s (frame time %.2f s, %s clock), %u damaged, decode %.2f ms/frame\n", b->FrameNum,
		b->Frames, m->started ? wallSec() - m->startWall : 0.0, (double)(b->FrameNum - 1) * b->FrameRateDiv / b->FrameRate,
		m->sound && !m->soundDone ? "sound" : "steady", m->damaged, b->LastFrameNum ? 1000.0 * m->decodeSec / b->LastFrameNum : 0.0);
	destroy(m);
}

int __stdcall BinkWait(BINK *b)
{
	Movie *m = (Movie *)b;
	if (!m || !m->started) return 0;
	pumpSound(m);
	double due = (double)(b->FrameNum - 1) * b->FrameRateDiv / b->FrameRate;
	return clockSec(m) < due ? 1 : 0;
}

void __stdcall BinkDoFrame(BINK *b)
{
	Movie *m = (Movie *)b;
	if (!m || b->FrameNum < 1 || b->FrameNum > b->Frames) return;
	uint32_t i = b->FrameNum - 1, start = m->index[i], size = m->index[i + 1] - start;
	if (!readAt(m->f, start, m->pkt, size)) { b->ReadError = 1; return; }
	memset(m->pkt + size, 0, binkdec::kPadding);
	uint32_t o = 0;
	for (uint32_t t = 0; t < m->h.tracks; t++) {   // the sound packets come first; read ahead separately
		if (o + 4 > size || rl32(m->pkt + o) > size - o - 4) { b->ReadError = 1; return; }
		o += 4 + rl32(m->pkt + o);
	}
	double t0 = wallSec();
	if (!binkdec::VideoDecode(m->video, m->pkt + o, size - o) && m->damaged++ < 4) BLOG("bink: frame %u damaged\n", i);
	m->decodeSec += wallSec() - t0;
	if (!m->started) {
		m->started = true;
		pumpSound(m);
		m->t0 = m->startWall = m->lastPlayedAt = wallSec();
	}
	pumpSound(m);
}

void __stdcall BinkNextFrame(BINK *b)
{
	if (!b) return;
	b->LastFrameNum = b->FrameNum;
	if (b->FrameNum < b->Frames) b->FrameNum++;
}

// The game passes 0x80000003 only (32-bit B, G, R, A, the D3D8 camera-texture format). The port's raster is what the renderer makes
// of the camera's depth: 32-bit on d3d9, 24-bit B, G, R on gl3 (RwRasterLock gives the D3D byte order on both); the pitch tells.
int __stdcall BinkCopyToBuffer(BINK *b, void *dest, int destpitch, unsigned destheight, unsigned destx, unsigned desty, unsigned flags)
{
	Movie *m = (Movie *)b;
	if (!m || !dest) return -1;
	if ((flags & 0xf) != 3 && !m->flagsLogged) { m->flagsLogged = true; BLOG("bink: BinkCopyToBuffer flags %x: written as 32-bit\n", flags); }
	const unsigned bpp = destpitch > 0 && (unsigned)destpitch < b->Width * 4 ? 3 : 4;
	if (desty >= destheight || destx * bpp >= (unsigned)(destpitch > 0 ? destpitch : 0)) return -1;
	unsigned w = b->Width, h = b->Height;
	if (h > destheight - desty) h = destheight - desty;
	if (w > (unsigned)destpitch / bpp - destx) w = (unsigned)destpitch / bpp - destx;
	int ys, us, vs;
	const uint8_t *Y = binkdec::VideoPlane(m->video, 0, &ys), *U = binkdec::VideoPlane(m->video, 1, &us),
		*V = binkdec::VideoPlane(m->video, 2, &vs);
	// BT.601 in 8.8 fixed point: limited range ('i' and older), full range ('k')
	const int ky = m->fullRange ? 256 : 298, y0 = m->fullRange ? 0 : 16;
	const int krv = m->fullRange ? 359 : 409, kgu = m->fullRange ? 88 : 100, kgv = m->fullRange ? 183 : 208, kbu = m->fullRange ? 454 : 516;
	for (unsigned y = 0; y < h; y++) {
		uint8_t *d = (uint8_t *)dest + (size_t)(desty + y) * destpitch + destx * bpp;
		const uint8_t *yr = Y + (size_t)y * ys, *ur = U + (size_t)(y >> 1) * us, *vr = V + (size_t)(y >> 1) * vs;
		for (unsigned x = 0; x < w; x++, d += bpp) {
			int c = ky * (yr[x] - y0) + 128, u = ur[x >> 1] - 128, v = vr[x >> 1] - 128;
			d[0] = clip8((c + kbu * u) >> 8);
			d[1] = clip8((c - kgu * u - kgv * v) >> 8);
			d[2] = clip8((c + krv * v) >> 8);
			if (bpp == 4) d[3] = 0xff;
		}
	}
	return 0;
}

// The game hands Bink its DirectSound object; the movie sound goes through OpenAL instead.
void *__stdcall BinkOpenDirectSound(void *) { return 0; }
int __stdcall BinkSetSoundSystem(void *, void *) { return 1; }

}
