// RWA (RenderWare Audio) stand-ins: the API the game uses, with the object layouts it reads directly. No sound here, but the state
// game logic depends on is real: parsed wave dictionaries, voice and stream play clocks, sample counts, segment lengths.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <rw.h>
#include "rwaudio_layouts.h"
#include "rwaudio_al.h"
#define PORT_HEAPCHK(where) do { static int bad; static const bool on = getenv("PORT_HEAPCHK") != 0; if (on && !bad && _heapchk() != _HEAPOK) { bad = 1; if (port_log) { fprintf(port_log, "HEAP CHECK FAILED in %s\n", where); fflush(port_log); } } } while (0)
#define CLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)

extern "C" FILE *port_log;
extern "C" void port_realign(void *stream, const char *who);   // rwshim.cpp

// ---------------------------------------------------------------- RWA audio (every object is an opaque zeroed block)
static void *audioObj(const char *what) { void *p = calloc(1, kAudioBlockBytes); CLOG("rwcore: audio %s -> stub %p\n", what, p); return p; }
// Our bookkeeping is in the tail of the 0x1000-byte block (game-visible fields are below 0x100; streams are >= 0xd70 in RWA). Audio
// clock: under PORT_FIXEDTIME it is the virtual clock's per-frame base (port_run.cpp vtNowUs without the per-read microseconds), read
// directly so queries neither follow wall time nor advance the game's clock; ends and positions then land on the same frame every run.
// Otherwise the wall clock (QueryPerformanceCounter).
#include "zoosters/os.h"
#include "../os/native_path.h"   // PORT_NATIVE_PATH
extern "C" int port_frames_presented(void);
static double nowSec()
{
	static int fixedUs = -1;
	if (fixedUs < 0) {
		const char *e = getenv("PORT_FIXEDTIME");
		fixedUs = !e ? 0 : atoi(e) ? atoi(e) * 1000 : 16667;
	}
	if (fixedUs)
		return 1.0 + (double)port_frames_presented() * fixedUs / 1e6;
	static long long freq;
	if (!freq) { freq = (long long)zoosters::os::PerfFrequency(); if (freq <= 0) freq = 1; }
	return (double)(long long)zoosters::os::PerfCounter() / (double)freq;
}
// PORT_AUDIOLOG=1: one `audiolog: f<frame> ...` line per voice / stream event (create, wave / media / segment, trigger and play-state
// requests, clock end, destroy).
static bool alogOn() { static int on = -1; if (on < 0) on = getenv("PORT_AUDIOLOG") != 0; return on && port_log; }
#define ALOG(...) do { if (alogOn()) { fprintf(port_log, "audiolog: f%d ", port_frames_presented()); fprintf(port_log, __VA_ARGS__); fputc('\n', port_log); } } while (0)
static const RwaWaveObj *waveObj(const RwaWave *w) { const RwaWaveObj *o = (const RwaWaveObj *)w; return o && o->magic == 0x45564157 ? o : 0; }   // kWaveMagic
static const char *waveName(const RwaWave *w) { const RwaWaveObj *o = waveObj(w); return o && o->name ? o->name : "?"; }
// every live voice / stream, so RwaCoreUpdate can advance them (all queries also advance lazily)
static RwaVoiceObj *g_voiceReg[128];
static RwaStream *g_streamReg[16];
template <class T> static void regAdd(T **tab, int n, T *o) { for (int i = 0; i < n; i++) if (!tab[i]) { tab[i] = o; return; } }
template <class T> static void regDel(T **tab, int n, const void *o) { for (int i = 0; i < n; i++) if (tab[i] == o) tab[i] = 0; }
static char g_coreToken[0x40];   // what _rwaCoreOpen hands back: the game only stores it in g_audioCore and tests it for non-zero
extern "C" RwaCore *_rwaCoreOpen(void *memfuncs, int n) { PORT_HEAPCHK("_rwaCoreOpen"); CLOG("rwcore: _rwaCoreOpen(%p, %d) -> stub core\n", memfuncs, n); return (RwaCore *)g_coreToken; }
extern "C" void RwaCoreClose(void) { PORT_HEAPCHK("RwaCoreClose");}
extern "C" void RwaCoreRegisterDefaultObjects(void) { PORT_HEAPCHK("RwaCoreRegisterDefaultObjects");}
extern "C" void RwaCoreUnregisterDefaultObjects(void) { PORT_HEAPCHK("RwaCoreUnregisterDefaultObjects");}
extern "C" void RwaDXAudioRegisterUsingInterface(void) { PORT_HEAPCHK("RwaDXAudioRegisterUsingInterface");}
extern "C" void RwaDXAudioUnregister(void) { PORT_HEAPCHK("RwaDXAudioUnregister");}
// The output opens the OpenAL device (rwaudio_al.cpp); listener and output gain are read from here every tick.
static RwaOsOutput *g_output;
static RwaListener *g_listener;
extern "C" RwaOsOutput *RwaDXAudioOutputCreate(int a, void *params, int b, int c, int d) { PORT_HEAPCHK("RwaDXAudioOutputCreate"); (void)a; (void)params; (void)b; (void)c; (void)d; g_output = (RwaOsOutput *)audioObj("RwaDXAudioOutputCreate"); if (g_output) g_output->gain = 1.0f; alaOpen(); return g_output; }   // master gain 1.0 at open (0x569470)
extern "C" void _rwaDXAudioOutputSetListener(RwaOsOutput *out, RwaListener *l) { PORT_HEAPCHK("_rwaDXAudioOutputSetListener"); (void)out; g_listener = l; }
extern "C" void _rwaDXAudioOutputSetGain(RwaOsOutput *out, float g) { PORT_HEAPCHK("_rwaDXAudioOutputSetGain"); if (out) out->gain = g; }   // 0x56a1d0 stores it at +0x38 (FadeOutputObject 0x44af20 reads it)
extern "C" void _rwaDXAudioOutputSetSpeakerConfig(RwaOsOutput *out, int c) { PORT_HEAPCHK("_rwaDXAudioOutputSetSpeakerConfig"); (void)out; (void)c; }
extern "C" RwaOsVoice *RwaDXAudioVoiceCreate(RwaOsOutput *out, int a, int b, int c, int d) { PORT_HEAPCHK("RwaDXAudioVoiceCreate"); (void)out; (void)a; (void)b; (void)c; (void)d; return (RwaOsVoice *)audioObj("RwaDXAudioVoiceCreate"); }
extern "C" RwaListener *RwaListenerCreate(int a, int b, int c)
{ PORT_HEAPCHK("RwaListenerCreate"); (void)a; (void)b; (void)c;
	RwaListener *l = (RwaListener *)audioObj("RwaListenerCreate");
	if (l) { l->rolloffFactor = 10.0f; l->dopplerScale = 1.0f; l->distanceFactor = 1.0f; }   // RWA defaults (0x50bdd0), in the game's member order (names after the setters)
	return l;
}
extern "C" void RwaObjDefGetDefaultCreationParams(void *def, void *params) { PORT_HEAPCHK("RwaObjDefGetDefaultCreationParams"); (void)def; memset(params, 0, 0x28); }   // RwaOutputParams is 0x28 bytes (r440/audio.cpp: 10 ints)
extern "C" void *_rwaObjDefFindByUUID(int a, const void *uuid, void **out) { PORT_HEAPCHK("_rwaObjDefFindByUUID"); (void)a; (void)uuid; void *d = audioObj("_rwaObjDefFindByUUID"); if (out) *out = d; return d; }
extern "C" void RwaObjDestroy(void *obj, int a, int b) { PORT_HEAPCHK("RwaObjDestroy"); (void)a; (void)b; ALOG("destroy %p", obj); alaVoiceDrop(obj); alaStreamDrop(obj); if (obj == g_output) { alaClose(); g_output = 0; } if (obj == g_listener) g_listener = 0; regDel(g_voiceReg, 128, obj); regDel(g_streamReg, 16, obj); free(obj); }
extern "C" void RwaObjHandleSetParamData(void *obj, int a, int b, void *data) { PORT_HEAPCHK("RwaObjHandleSetParamData"); (void)obj; (void)a; (void)b; (void)data; }
extern "C" void RwaVoiceManagerRegister(RwaOsOutput *out, RwaOsVoice **voices, int n) { PORT_HEAPCHK("RwaVoiceManagerRegister"); (void)out; (void)voices; CLOG("rwcore: RwaVoiceManagerRegister(%d voices)\n", n); }
extern "C" void RwaVoiceManagerUnregister(void) { PORT_HEAPCHK("RwaVoiceManagerUnregister");}
// 0x50a5b0: samples in `bytes` of data of an in-memory RwaWaveFormat (+4 data type UUID pointer, +0xc bits, +0xd channels): PCM bits/8
// per channel frame, IMA / Xbox ADPCM 36 bytes -> 64 samples per channel. The game feeds this into every sound duration.
extern "C" unsigned RwaWaveFormatGetSamples(const RwaWaveFormat *fmt, int bytes)
{ PORT_HEAPCHK("RwaWaveFormatGetSamples");
	if (!fmt || bytes <= 0) return 0;
	return wdSamples(fmt->dataType, fmt->bitsPerSample, fmt->numChannels, (unsigned)bytes);
}
// ---------------------------------------------------------------- waves and wave dictionaries
// RwaWaveObj (rwaudio_layouts.h) = the game's RwaWave (+0x10 source RwaWaveFormat: rate, +4 data type UUID pointer, +8 length in
// bytes, +0xc bits, +0xd channels; +0x2c the format the wave was created as) and our tail.
static const unsigned kWaveMagic = 0x45564157, kDictMagic = 0x54434944;   // 'WAVE', 'DICT'

static void setFmt(RwaWaveFormat *m, const WdFormat &f, unsigned char *uuidStore)
{
	memset(m, 0, sizeof *m);
	m->sampleRate = f.rate; m->length = f.length; m->bitsPerSample = f.bits; m->numChannels = f.channels; m->miscSize = f.miscSize; m->flags = f.flags; m->flags2 = f.flags2;
	if (f.hasType) { memcpy(uuidStore, f.type, 16); m->dataType = uuidStore; }
}
static void freeWave(RwaWaveObj *w) { if (!w) return; alaWaveDrop(w); free(w->data); free(w->name); w->magic = 0; free(w); }
static unsigned streamRead_(void *ctx, void *dst, unsigned n) { return RwStreamRead((RwStream *)ctx, dst, n); }

// 0x50d3d0: read the dictionary chunk (the 0x809 header was consumed by RwStreamFindChunk) into a dictionary object with its waves.
extern "C" RwaWaveDict *_rwaWaveDictStreamRead(int a, RwStream *stream, RwaOsOutput *out, int b, int c, int d)
{ PORT_HEAPCHK("_rwaWaveDictStreamRead");
	(void)a; (void)out; (void)b; (void)c; (void)d;
	char err[200] = "";
	WdSource src = { streamRead_, stream };
	WdDict *wd = wdReadDict(&src, err, sizeof err);
	port_realign(stream, "_rwaWaveDictStreamRead");
	if (!wd) { CLOG("rwaudio: _rwaWaveDictStreamRead(stream %p) failed: %s\n", stream, err); return 0; }
	RwaWaveDict *dict = (RwaWaveDict *)calloc(1, sizeof *dict);
	if (dict) dict->waves = (RwaWaveObj **)calloc(wd->count ? wd->count : 1, sizeof(RwaWaveObj *));
	if (!dict || !dict->waves) { free(dict); wdFreeDict(wd); return 0; }
	dict->magic = kDictMagic; dict->name = wd->name; wd->name = 0;
	unsigned long bytes = 0;
	for (unsigned i = 0; i < wd->count; i++) {
		WdWave *s = &wd->waves[i];
		RwaWaveObj *w = (RwaWaveObj *)calloc(1, sizeof *w);
		if (!w) break;
		w->dict = dict; w->magic = kWaveMagic;
		setFmt(&w->pub.format, s->fmt, w->typeFmt); setFmt(&w->pub.targetFormat, s->target, w->typeTarget);
		w->data = s->data; s->data = 0; w->dataLen = s->dataLen; w->create = s->create; w->name = s->name; s->name = 0;
		bytes += w->dataLen;
		dict->waves[dict->count++] = w;
	}
	wdFreeDict(wd);
	CLOG("rwaudio: wave dictionary '%s' read: %u waves, %lu bytes of sample data\n", dict->name ? dict->name : "-", dict->count, bytes);
	return dict;
}
extern "C" void RwaWaveDictDestroy(RwaWaveDict *dict)
{ PORT_HEAPCHK("RwaWaveDictDestroy");
	if (!dict || dict->magic != kDictMagic) return;
	for (unsigned i = 0; i < dict->count; i++) freeWave(dict->waves[i]);
	CLOG("rwaudio: wave dictionary '%s' destroyed (%u waves)\n", dict->name ? dict->name : "-", dict->count);
	free(dict->waves); free(dict->name); dict->magic = 0; free(dict);
}
static int sameName(const char *a, const char *b)
{
	for (; *a && *b; a++, b++) {
		int x = *a, y = *b;
		if (x >= 'A' && x <= 'Z') x += 32;
		if (y >= 'A' && y <= 'Z') y += 32;
		if (x != y) return 0;
	}
	return !*a && !*b;
}
// 0x50d320: lookup by (lower-case) name; the game tries every loaded dictionary, so a miss is normal and not logged
extern "C" RwaWave *RwaWaveDictFindWaveByName(RwaWaveDict *dict, const char *name)
{ PORT_HEAPCHK("RwaWaveDictFindWaveByName");
	if (!dict || dict->magic != kDictMagic || !name) return 0;
	for (unsigned i = 0; i < dict->count; i++)
		if (dict->waves[i]->name && sameName(dict->waves[i]->name, name)) return &dict->waves[i]->pub;
	return 0;
}
// 0x5088d0: a wave that belongs to a dictionary dies with it
extern "C" int RwaWaveDestroy(RwaWave *w)
{ PORT_HEAPCHK("RwaWaveDestroy");
	RwaWaveObj *wave = (RwaWaveObj *)w;
	if (wave && wave->magic == kWaveMagic && !wave->dict) freeWave(wave);
	return 1;
}
// ---------------------------------------------------------------- virtual voices
// RwaVirtualVoice: handle (the voice itself), flags (2 = valid, 4 = loop), frequency in Hz (rate x pitch, written by the game),
// wavePos, wave, startStamp, handleOwnsRate. Ours (RwaVoiceObj::clock): position in samples advances by (time x frequency) while
// triggered; a one-shot stops at the end, a loop wraps.
static const unsigned kVoiceMagic = 0x43494f56;
static VoiceClock *vc(RwaVirtualVoice *v) { VoiceClock *c = v ? &((RwaVoiceObj *)v)->clock : 0; return c && c->magic == kVoiceMagic ? c : 0; }
static unsigned voiceTotal(RwaVirtualVoice *v)      // samples in the voice's wave
{
	const RwaWave *w = v->wave;
	return w ? RwaWaveFormatGetSamples(&w->format, (int)w->format.length) : 0;
}
static void voiceAdvance(RwaVirtualVoice *v)
{
	VoiceClock *c = vc(v);
	if (!c || !c->playing) return;
	double now = nowSec(), f = v->frequency;
	if (f < 0) f = 0;
	c->base += (now - c->t0) * f;
	c->t0 = now;
	double total = (double)voiceTotal(v);
	if (total <= 0) { c->playing = 0; c->base = 0; return; }
	if (c->base >= total) {
		if (v->flags & RWA_VVOICE_LOOP) c->base -= total * (double)(long long)(c->base / total);
		else {
			c->base = total; c->playing = 0;
			v->flags &= ~(unsigned)RWA_VVOICE_PLAYING;   // RWA clears the playing bit at a one-shot's end: Voice::Update 0x417f20 then drops the voice
			ALOG("voice %p end (one-shot finished) wave %s", v, waveName(v->wave));
		}
	}
}
static int voicePos(RwaVirtualVoice *v) { VoiceClock *c = vc(v); if (!c) return 0; voiceAdvance(v); return (int)c->base; }
extern "C" RwaVirtualVoice *RwaVirtualVoiceCreate(int a, int b, int c, int d, int e)
{ PORT_HEAPCHK("RwaVirtualVoiceCreate"); (void)a; (void)b; (void)c; (void)d; (void)e;
	RwaVoiceObj *o = (RwaVoiceObj *)audioObj("RwaVirtualVoiceCreate");
	if (!o) return 0;
	RwaVirtualVoice *v = &o->pub;
	v->handle = v;
	v->flags = RWA_VVOICE_PLAYING;
	o->clock.magic = kVoiceMagic;
	regAdd(g_voiceReg, 128, o);
	ALOG("voice %p create", v);
	return v;
}
// 0x5095c0: the wave pointer and, unless the handle owns the rate, its sample rate (target format) as the float frequency
extern "C" void _rwaVirtualVoiceSetWave(RwaVirtualVoice *v, RwaWave *w)
{ PORT_HEAPCHK("_rwaVirtualVoiceSetWave");
	if (!v) return;
	v->wave = w;
	if (w && v->handleOwnsRate == 0) v->frequency = (float)w->targetFormat.sampleRate;
	if (VoiceClock *c = vc(v)) { c->playing = 0; c->base = 0; }
	ALOG("voice %p wave %s (%u samples, %.0f Hz)", v, waveName(w), w ? voiceTotal(v) : 0u, (double)v->frequency);
}
// 0x509580: trigger on (start; stamps startStamp) or off (stop, the position stays); the game resumes with a handle position set (id
// 7) first
extern "C" void _rwaVirtualVoiceSetTrigger(RwaVirtualVoice *v, int t)
{ PORT_HEAPCHK("_rwaVirtualVoiceSetTrigger");
	VoiceClock *c = vc(v);
	if (!c) return;
	voiceAdvance(v);
	if (t) {
		static unsigned stamp;
		unsigned total = voiceTotal(v);
		if (c->base >= total && !(v->flags & RWA_VVOICE_LOOP)) c->base = 0;     // a finished one-shot starts again
		c->playing = total != 0;
		if (c->playing) v->flags |= RWA_VVOICE_PLAYING;
		c->t0 = nowSec();
		v->startStamp = ++stamp;
		ALOG("voice %p trigger on at %.0f/%u%s", v, c->base, total, (v->flags & RWA_VVOICE_LOOP) ? " loop" : "");
	} else {
		ALOG("voice %p trigger off at %.0f", v, c->base);
		c->playing = 0;
	}
}
// ---------------------------------------------------------------- streams (.rws music and speech)
// Play state at +0xd6c (FUN_005080e0): 9 idle, 3 stopped, 2 cueing, 4 buffering, 5 cued/paused (Voice::Update 0x417f20 waits for 5),
// 6 playing, 8 played to the end (Voice::Update ends the voice on 3, or 8 when not looping; the soft mixer shows 3).
// Requests (_rwaStreamSetPlayState): 1 stop -> 3; 3 cue -> 2, then 4 after kCueBuffer, 5 after kCueReady; 3 while playing = pause -> 5;
// 2 on a cued stream -> 6 (ignored during the cue: stock shows 5 first, 6 a frame later when Voice::Update re-requests).
// The end (3) comes after the segment's whole blocks have played, block count x (codec bytes per block / frame bytes x 64 samples /
// rate = 0.6617 s at 22052 Hz), plus kEndLatency counted from the port's 6, which comes ~1 frame before stock's. RWA does this on its
// stream thread; here every query and RwaCoreUpdate advances the state on nowSec.
// RwaStream::clock (+0xe00): media path, segment and clock. Only the .rws header is read (segment names and lengths).
static const unsigned kStreamMagic = 0x4d525453;
static StreamClock *sc(RwaStream *s) { StreamClock *c = s ? &s->clock : 0; return c && c->magic == kStreamMagic ? c : 0; }
static int &observed(RwaStream *s) { return s->playState; }

static struct RwsCacheEntry { char path[260]; WdRws *r; } g_rwsCache[40];
static int g_rwsCacheN;
static unsigned fileRead_(void *ctx, void *dst, unsigned n) { return (unsigned)fread(dst, 1, n, (FILE *)ctx); }
static int pathEq(const char *a, const char *b) { return sameName(a, b); }
static WdRws *rwsFor(const char *path)       // header of the .rws at `path` (relative to the game directory), cached; 0 if unreadable
{
	if (!path || !path[0]) return 0;
	for (int i = 0; i < g_rwsCacheN; i++) if (pathEq(g_rwsCache[i].path, path)) return g_rwsCache[i].r;
	char err[200] = "";
	WdRws *r = 0;
	char native[1024];
	FILE *f = fopen(PORT_NATIVE_PATH(path, native), "rb");
	if (f) { WdSource src = { fileRead_, f }; r = wdReadRws(&src, err, sizeof err); fclose(f); } else strcpy(err, "cannot open");
	CLOG("rwaudio: stream header %s: %s", path, r ? "" : err);
	if (r) CLOG("%u segments, block %u, %u Hz\n", r->nseg, r->blockSize, r->fmt.rate); else CLOG("\n");
	if (g_rwsCacheN < 40 && strlen(path) < sizeof g_rwsCache[0].path) { strcpy(g_rwsCache[g_rwsCacheN].path, path); g_rwsCache[g_rwsCacheN++].r = r; }
	return r;
}
static const double kCueBuffer = 5.0 / 60.0, kCueReady = 12.5 / 60.0, kEndLatency = 0.1;   // seconds, fitted to stock
// Seconds after play until the soft path reports a segment's end: its whole blocks of audio + kEndLatency
static float blockEnd(const WdRws *r, int seg)
{
	unsigned ch = r && r->fmt.channels ? r->fmt.channels : 1;
	if (!r || !r->blockSize || !r->codecPerBlock || !r->fmt.rate || !r->samplesPerFrame) return 0.0f;
	unsigned blocks = (r->segs[seg].size + r->blockSize - 1) / r->blockSize;
	double block = (double)(r->codecPerBlock / ((r->frameSize ? r->frameSize : 36u) * ch)) * r->samplesPerFrame / r->fmt.rate;
	return (float)(blocks * block + kEndLatency);
}
static float segSeconds(const WdRws *r, int seg) { return r && r->fmt.rate ? (float)wdSegmentSamples(r, seg) / (float)(int)r->fmt.rate : 0.0f; }
// Called from RwaCoreUpdate once per frame: only then does a cue progress, like RWA's stream thread (stock: 5 shows for a frame before
// Voice::Update makes it 6). Queries only advance playback.
static void streamAdvance(RwaStream *s, bool tick = false)
{
	StreamClock *c = sc(s);
	if (!c) return;
	double now = nowSec();
	if (c->cueing) {
		if (!tick) return;
		double el = now - c->cueT0;
		int &o = observed(s);
		if (el >= kCueReady) {
			c->cueing = 0;
			o = 5;
			ALOG("stream %p cued (state 5) '%s'", s, c->segName);
			if (c->playPending) { c->playPending = 0; c->playing = 1; c->t0 = now; o = 6; ALOG("stream %p play (held request) -> 6 '%s'", s, c->segName); }
		} else if (el >= kCueBuffer && o == 2) o = 4;
		return;
	}
	if (!c->playing) return;
	double p = c->pitch > 0 ? c->pitch : 1.0;
	c->base += (now - c->t0) * p;
	c->t0 = now;
	if (c->loop && c->length > 0) {
		if (c->base >= c->length) c->base -= (double)c->length * (double)(long long)(c->base / c->length);
	} else if (c->length <= 0 || c->base >= (c->endAt > c->length ? c->endAt : c->length)) {
		c->base = c->length > 0 ? c->length : 0; c->playing = 0; observed(s) = 3;
		ALOG("stream %p end (state 3) %s '%s' %.2f s", s, c->path, c->segName, (double)c->length);
	}
}
static double streamPos(const StreamClock *c) { return c->length > 0 && c->base > c->length ? c->length : c->base; }   // position as RWA reports it
extern "C" RwaStream *RwaStreamCreate(RwaOsOutput *out, int a, int b, int c, int d)
{ PORT_HEAPCHK("RwaStreamCreate"); (void)out; (void)a; (void)b; (void)c; (void)d;
	RwaStream *s = (RwaStream *)audioObj("RwaStreamCreate");
	if (!s) return 0;
	StreamClock *k = &s->clock;
	k->magic = kStreamMagic; k->pitch = 1.0f; k->seg = -1;
	k->gain[0][0] = k->gain[0][1] = k->gain[1][0] = k->gain[1][1] = 1.0f;
	observed(s) = 9;
	regAdd(g_streamReg, 16, s);
	ALOG("stream %p create", s);
	return s;
}
// 0x507fd0: media = RwaStreamMedia {path, type (1 = file name), ..., loop}; null media = release (idle)
extern "C" void _rwaStreamSetMedia(RwaStream *s, const RwaStreamMedia *mm)
{ PORT_HEAPCHK("_rwaStreamSetMedia");
	StreamClock *c = sc(s);
	if (!c) return;
	c->playing = 0; c->base = 0; c->length = 0; c->seg = -1; c->rws = 0; c->path[0] = 0; c->segName[0] = 0; c->cueing = c->playPending = 0; c->gen++;
	if (!mm) { observed(s) = 9; return; }
	const char *path = mm->mediaType == 1 ? mm->path : 0;
	if (path) { strncpy(c->path, path, sizeof c->path - 1); c->path[sizeof c->path - 1] = 0; }
	c->loop = mm->loop != 0;
	c->rws = rwsFor(c->path);
	observed(s) = 3;
	ALOG("stream %p media %s%s", s, c->path, c->loop ? " loop" : "");
}
// 0x507de0: items = RwaStreamItems {item*, count}, item = {name ptr, type (1 = by name), start, count}: select the segment, position
// 0, stopped
extern "C" void FUN_00507de0(RwaStream *s, const RwaStreamItems *items)
{ PORT_HEAPCHK("FUN_00507de0");
	StreamClock *c = sc(s);
	if (!c || !items) return;
	const RwaStreamItem *it = items->items;
	c->playing = 0; c->base = 0; c->length = 0; c->seg = -1; c->segName[0] = 0; c->cueing = c->playPending = 0; c->gen++;
	if (it && it->type == 1 && it->name) {
		strncpy(c->segName, it->name, sizeof c->segName - 1); c->segName[sizeof c->segName - 1] = 0;
		c->seg = wdFindSegment(c->rws, c->segName);
		c->length = c->seg >= 0 ? segSeconds(c->rws, c->seg) : 0.0f;
		c->endAt = c->seg >= 0 ? blockEnd(c->rws, c->seg) : 0.0f;
		ALOG("stream %p segment '%s' -> %d, %.2f s", (void *)s, c->segName, c->seg, (double)c->length);
		static int logged;
		if (logged++ < 40) CLOG("rwaudio: stream %p segment '%s' of %s -> %d, %.2f s\n", (void *)s, c->segName, c->path, c->seg, c->length);
	}
	if (observed(s) != 9) observed(s) = 3;
}
extern "C" void _rwaStreamSetLoopSegment(RwaStream *s, int a) { PORT_HEAPCHK("_rwaStreamSetLoopSegment"); if (StreamClock *c = sc(s)) c->loop = a != 0; }
extern "C" void _rwaStreamSetPitch(RwaStream *s, float p) { PORT_HEAPCHK("_rwaStreamSetPitch"); if (StreamClock *c = sc(s)) { streamAdvance(s); c->pitch = p; } }
extern "C" void _rwaStreamSetPlayState(RwaStream *s, int st)
{ PORT_HEAPCHK("_rwaStreamSetPlayState");
	StreamClock *c = sc(s);
	if (!c) { if (s) observed(s) = (st == 1 || st == 3) ? 3 : st; return; }
	streamAdvance(s);
	int &o = observed(s);
	int was = o;
	if (st == 1) { c->playing = 0; c->base = 0; c->cueing = c->playPending = 0; o = 3; c->gen++; }   // stop: back to the start
	else if (st == 3) {                                                                    // cue, or pause while playing
		if (o == 6) { c->playing = 0; o = 5; }
		else if (o == 3 || o == 8) { c->playing = 0; c->base = 0; c->cueing = 1; c->playPending = 0; c->cueT0 = nowSec(); o = 2; c->gen++; }
	} else if (st == 2 && o != 9) {                                                        // play
		if (o == 5) { c->playing = 1; c->t0 = nowSec(); o = 6; }
		// o == 2 / 4 (cueing): ignored, Voice::Update asks again once it sees 5
		else if (o == 3 || o == 8) { c->base = 0; c->cueing = 1; c->playPending = 1; c->cueT0 = nowSec(); o = 2; c->gen++; }
	} else o = st;
	ALOG("stream %p request %d: state %d -> %d at %.2f s '%s'", s, st, was, o, c->base, c->segName);
}
// 0x5080e0
extern "C" int FUN_005080e0(RwaStream *obj) { PORT_HEAPCHK("FUN_005080e0"); streamAdvance(obj); return obj->playState; }
// 0x507e90: query RwaStreamPosQuery {arg0 out segment index, arg1 mode (3), seconds out position as float seconds when arg3 == 1, arg3
// 1}; on failure arg0 = seconds = 0
extern "C" void _rwaStreamGetStreamPos(RwaStream *s, RwaStreamPosQuery *q)
{ PORT_HEAPCHK("_rwaStreamGetStreamPos");
	StreamClock *c = sc(s);
	if (!c || c->seg < 0 || observed(s) == 9) { q->arg0 = 0; q->seconds = 0.0f; return; }
	streamAdvance(s);
	if (q->arg1 == 3) q->arg0 = c->seg;
	if (q->arg3 == 1) q->seconds = (float)streamPos(c);
	else if (q->arg3 == 2) { int samples = (int)(streamPos(c) * (c->rws ? c->rws->fmt.rate : 0)); memcpy(&q->seconds, &samples, sizeof samples); }   // sample count in the same word
}
// Once per tick after the clocks advance, the device follows the model (rwaudio_al.h). Read-only towards the game.
static void deviceSync()
{
	if (!alaActive()) return;
	alaTickBegin(port_frames_presented());
	if (const RwaListener *l = g_listener) {   // position, up, at, right (metres), distance factor, Doppler, roll-off
		static_assert(sizeof(RwV3d) == 12, "RwV3d is three floats");
		AlaListener a;
		memcpy(a.pos, &l->position, 12); memcpy(a.up, &l->up, 12); memcpy(a.at, &l->at, 12); memcpy(a.right, &l->right, 12);
		a.distanceFactor = l->rolloffFactor; a.doppler = l->dopplerScale; a.rolloff = l->distanceFactor;   // AlaListener keeps the order of the game's stores (+0x64, +0x68, +0x6c)
		alaListenerSync(a);
	}
	if (const RwaOsOutput *o = g_output) {      // environment: 12-dword preset, applied like RWA's output update: only once
		static_assert(sizeof(RwaEnvironment) == 12 * sizeof(int), "the environment is 12 words");
		static int last[12]; static bool seen;            // SetEnvironment 0x4177a0 has set the apply bit (RWA_OUTPUT_ENVIRONMENT) in the change word (the game also copies
		bool apply = (o->dirtyFlags & RWA_OUTPUT_ENVIRONMENT) && (!seen || memcmp(last, &o->environment, sizeof last) != 0);   // preset 0 there without it)
		if (apply) { memcpy(last, &o->environment, sizeof last); seen = true; }
		alaOutputSync(o->gain, last, apply);   // the stand-in leaves the bit set (RWA clears it): game-visible state unchanged
	}
	for (int i = 0; i < 128; i++) {
		RwaVoiceObj *vo = g_voiceReg[i];
		RwaVirtualVoice *v = vo ? &vo->pub : 0;
		VoiceClock *c = vc(v);
		if (!c) continue;
		const RwaWaveObj *w = waveObj(v->wave);
		AlaVoice a;
		memset(&a, 0, sizeof a);
		if (w) {
			a.wave = w; a.name = w->name; a.data = w->data; a.dataLen = w->dataLen;
			a.rate = w->pub.format.sampleRate; a.bits = w->pub.format.bitsPerSample; a.channels = w->pub.format.numChannels;
			a.is3d = (w->create & 1) != 0;
		}
		a.playing = c->playing != 0 && a.wave;
		a.stamp = v->startStamp;
		a.posSamples = c->base;
		a.loop = (v->flags & RWA_VVOICE_LOOP) != 0;
		a.freq = v->frequency; a.gain = v->gain; a.pan = v->pan;
		memcpy(a.pos, &v->position, 12); memcpy(a.vel, &v->velocity, 12);
		a.minDist = v->minDistance; a.maxDist = v->maxDistance;
		alaVoiceSync(vo, a);
	}
	for (int i = 0; i < 16; i++) {
		RwaStream *s = g_streamReg[i];
		StreamClock *c = sc(s);
		if (!c) continue;
		AlaStream a;
		a.path = c->path; a.rws = c->rws; a.seg = c->seg; a.state = observed(s); a.gen = c->gen;
		a.posSec = streamPos(c); a.loop = c->loop != 0; a.pitch = c->pitch;
		memcpy(a.gain, c->gain, sizeof a.gain);
		alaStreamSync(s, a);
	}
	alaTickEnd(nowSec());
}
// 0x5074c0 RwaCoreUpdate (-> 0x5074d0, RwaCoreUpdateFinish): one core tick; here it advances every voice and stream clock and drives
// the device
extern "C" void FUN_005074c0(void)
{ PORT_HEAPCHK("FUN_005074c0");
	for (int i = 0; i < 128; i++) if (g_voiceReg[i]) voiceAdvance(&g_voiceReg[i]->pub);
	for (int i = 0; i < 16; i++) if (g_streamReg[i]) streamAdvance(g_streamReg[i], true);
	deviceSync();
}
// 0x508110: g = {0, channel pair 0/1, side 0/1, float gain} (SetStreamGains 0x4176d0); only the device uses it
extern "C" void _rwaStreamSetStreamGain(RwaStream *s, const RwaStreamGain *g)
{ PORT_HEAPCHK("_rwaStreamSetStreamGain");
	StreamClock *c = sc(s);
	if (c && g && (unsigned)g->pair < 2 && (unsigned)g->side < 2) c->gain[g->pair][g->side] = g->gain;
}
// unnamed RWA entry points the game calls by address (no names are known for them yet)
extern "C" void FUN_005074b0(void) { PORT_HEAPCHK("FUN_005074b0");}                                     // 0x5074b0: `ret`
// 0x5724c0: the DirectSoundCreate import thunk (DSOUND.DLL ordinal 11): no sound device
extern "C" long __stdcall FUN_005724c0(void *guid, void **out, void *outer) { PORT_HEAPCHK("FUN_005724c0"); (void)guid; (void)outer; *out = 0; CLOG("rwcore: DirectSoundCreate -> DSERR_NODRIVER\n"); return (long)(int)0x88780078; }
// ---- the RWA entry points the game calls by their C++ names (the declarations of src/game/include/rw/rwaudio.h; RwaObjHandle is void)
// 0x509ca0: set a parameter through an object handle (int variant; the float variant is a no-op). Ids: 4 frequency, 5 gain, 8 loop, 10
// pan (the game keeps its own copies, which the clock reads); 7 with ioType 1 = wave position in samples (restores a paused sound),
// the only one that changes the clock.
static void handleSetInt(RwaObjHandle *h, int id, int io, int v)
{
	RwaVirtualVoice *vv = (RwaVirtualVoice *)h;   // RwaVirtualVoiceCreate: the handle is the voice
	VoiceClock *c = vc(vv);
	if (!c || id != 7 || io != 1) return;
	voiceAdvance(vv);
	c->base = v < 0 ? 0 : v;
	if (c->playing) c->t0 = nowSec();
}
void FUN_00509ca0(RwaObjHandle *h, int a, int b, int v) { PORT_HEAPCHK("FUN_00509ca0"); handleSetInt(h, a, b, v); }
void FUN_00509ca0(RwaObjHandle *h, int a, int b, float v) { PORT_HEAPCHK("FUN_00509ca0"); (void)h; (void)a; (void)b; (void)v; }
// 0x509cf0: get a parameter through an object handle; id 7 (ioType 2) = wave position in samples from the voice clock, others 0.
// The 4th argument is an optional out flag (real RWA: *flag = result != 0); the game passes 0.
int FUN_00509cf0(RwaObjHandle *h, int a, int b, int v)
{ PORT_HEAPCHK("FUN_00509cf0");
	int r = (a == 7 && b == 2) ? voicePos((RwaVirtualVoice *)h) : 0;   // the handle is the voice
	if (v) *(int *)(size_t)v = r != 0;
	return r;
}
// 0x508200: segment lookup in the stream's header. info = {[0] name (in) / segment index (out), [1] type (1 = by name, 3 = by index; 2
// = UUID unsupported) in / 3 = found out, [2] length out: samples, or float seconds when [3] == 1, [3] flag in, [4] substream count
// out}. Voice::Update 0x417f20 reads the length from [2] (stock Voice m_len = 1.8632 for a 1.86 s line). A miss zeroes [1], [3] and
// [4].
void FUN_00508200(void *stream, StreamLoadB *info)
{ PORT_HEAPCHK("FUN_00508200");
	RwaStream *s = (RwaStream *)stream;
	StreamClock *c = sc(s);
	int seg = -1;
	if (c && c->rws) {
		if (info->type == 1) seg = wdFindSegment(c->rws, info->name);
		else if (info->type == 3) {   // by index: the first word (the name pointer's slot) holds the index
			intptr_t idx = (intptr_t)info->name;
			seg = (idx >= 0 && (uintptr_t)idx < c->rws->nseg) ? (int)idx : -1;
		}
	}
	if (seg < 0) { info->type = 0; info->unit = 0; info->arg4 = 0; return; }
	unsigned samples = wdSegmentSamples(c->rws, seg);
	info->name = (const char *)(intptr_t)seg; info->type = 3; info->arg4 = (int)c->rws->nsub;
	if (info->unit == 1) info->length = c->rws->fmt.rate ? (float)samples / (float)(int)c->rws->fmt.rate : 0.0f;
	else memcpy(&info->length, &samples, sizeof samples);   // the same word holds the sample count
}
