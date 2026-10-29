// OpenAL Soft backend (contract: rwaudio_al.h). AL 1.1 / EFX entry points are declared here and loaded at run time (OpenAL32.dll,
// libopenal.dylib or libopenal.so.1).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rwaudio_al.h"
#include "rwaudio_wavedict.h"
#include "../os/native_path.h"   // PORT_NATIVE_PATH

extern "C" FILE *port_log;
#define ALLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); fflush(port_log); } } while (0)
#include "zoosters/os.h"
#if defined(_WIN32)
#define AL_LIBRARY "OpenAL32.dll"
#elif defined(__APPLE__)
#define AL_LIBRARY "libopenal.dylib"
#else
#define AL_LIBRARY "libopenal.so.1"
#endif
static void *AlOpenLibrary(const char *name) { return zoosters::os::LibraryOpen(name); }
static void *AlSymbol(void *lib, const char *name) { return zoosters::os::LibrarySymbol(lib, name); }

// ---------------------------------------------------------------- AL / ALC / EFX (OpenAL 1.1 and EFX 1.0 specifications)
typedef unsigned ALuint; typedef int ALint; typedef int ALenum; typedef int ALsizei; typedef float ALfloat; typedef char ALboolean;
struct ALCdevice; struct ALCcontext;
enum {
	AL_NONE = 0, AL_SOURCE_RELATIVE = 0x202, AL_PITCH = 0x1003, AL_POSITION = 0x1004, AL_VELOCITY = 0x1006, AL_LOOPING = 0x1007,
	AL_BUFFER = 0x1009, AL_GAIN = 0x100A, AL_ORIENTATION = 0x100F, AL_SOURCE_STATE = 0x1010, AL_PLAYING = 0x1012, AL_PAUSED = 0x1013,
	AL_STOPPED = 0x1014, AL_BUFFERS_QUEUED = 0x1015, AL_BUFFERS_PROCESSED = 0x1016, AL_REFERENCE_DISTANCE = 0x1020, AL_ROLLOFF_FACTOR = 0x1021,
	AL_MAX_DISTANCE = 0x1023, AL_SAMPLE_OFFSET = 0x1025, AL_FORMAT_MONO8 = 0x1100, AL_FORMAT_MONO16 = 0x1101, AL_FORMAT_STEREO8 = 0x1102,
	AL_FORMAT_STEREO16 = 0x1103, AL_DOPPLER_FACTOR = 0xC000, AL_DISTANCE_MODEL = 0xD000, AL_INVERSE_DISTANCE_CLAMPED = 0xD002,
	ALC_MAX_AUXILIARY_SENDS = 0x20003, AL_AUXILIARY_SEND_FILTER = 0x20006, AL_FILTER_NULL = 0,
	AL_EFFECT_TYPE = 0x8001, AL_EFFECT_REVERB = 1, AL_EFFECTSLOT_EFFECT = 1, AL_FILTER_TYPE = 0x8001, AL_FILTER_LOWPASS = 1, AL_LOWPASS_GAIN = 1,
	AL_REVERB_DENSITY = 1, AL_REVERB_DIFFUSION, AL_REVERB_GAIN, AL_REVERB_GAINHF, AL_REVERB_DECAY_TIME, AL_REVERB_DECAY_HFRATIO,
	AL_REVERB_REFLECTIONS_GAIN, AL_REVERB_REFLECTIONS_DELAY, AL_REVERB_LATE_REVERB_GAIN, AL_REVERB_LATE_REVERB_DELAY,
	AL_REVERB_AIR_ABSORPTION_GAINHF, AL_REVERB_ROOM_ROLLOFF_FACTOR,
};
#define AL_FUNCS(X) \
	X(ALCdevice *, alcOpenDevice, (const char *)) X(ALboolean, alcCloseDevice, (ALCdevice *)) \
	X(ALCcontext *, alcCreateContext, (ALCdevice *, const ALint *)) X(ALboolean, alcMakeContextCurrent, (ALCcontext *)) \
	X(void, alcDestroyContext, (ALCcontext *)) X(ALboolean, alcIsExtensionPresent, (ALCdevice *, const char *)) \
	X(void, alcGetIntegerv, (ALCdevice *, ALenum, ALsizei, ALint *)) X(const char *, alcGetString, (ALCdevice *, ALenum)) \
	X(void *, alGetProcAddress, (const char *)) X(void *, alcGetProcAddress, (ALCdevice *, const char *)) X(ALenum, alGetError, (void)) X(const char *, alGetString, (ALenum)) \
	X(void, alDistanceModel, (ALenum)) X(void, alDopplerFactor, (ALfloat)) \
	X(void, alListenerf, (ALenum, ALfloat)) X(void, alListener3f, (ALenum, ALfloat, ALfloat, ALfloat)) X(void, alListenerfv, (ALenum, const ALfloat *)) \
	X(void, alGenSources, (ALsizei, ALuint *)) X(void, alDeleteSources, (ALsizei, const ALuint *)) \
	X(void, alSourcei, (ALuint, ALenum, ALint)) X(void, alSourcef, (ALuint, ALenum, ALfloat)) X(void, alSource3f, (ALuint, ALenum, ALfloat, ALfloat, ALfloat)) \
	X(void, alSource3i, (ALuint, ALenum, ALint, ALint, ALint)) X(void, alGetSourcei, (ALuint, ALenum, ALint *)) \
	X(void, alSourcePlay, (ALuint)) X(void, alSourcePause, (ALuint)) X(void, alSourceStop, (ALuint)) X(void, alSourceRewind, (ALuint)) \
	X(void, alSourceQueueBuffers, (ALuint, ALsizei, const ALuint *)) X(void, alSourceUnqueueBuffers, (ALuint, ALsizei, ALuint *)) \
	X(void, alGenBuffers, (ALsizei, ALuint *)) X(void, alDeleteBuffers, (ALsizei, const ALuint *)) \
	X(void, alBufferData, (ALuint, ALenum, const void *, ALsizei, ALsizei))
#define AL_DECL(r, n, a) typedef r (*n##_t) a; static n##_t n;
AL_FUNCS(AL_DECL)
#define EFX_FUNCS(X) \
	X(void, alGenEffects, (ALsizei, ALuint *)) X(void, alEffecti, (ALuint, ALenum, ALint)) X(void, alEffectf, (ALuint, ALenum, ALfloat)) \
	X(void, alGenAuxiliaryEffectSlots, (ALsizei, ALuint *)) X(void, alAuxiliaryEffectSloti, (ALuint, ALenum, ALint)) \
	X(void, alGenFilters, (ALsizei, ALuint *)) X(void, alFilteri, (ALuint, ALenum, ALint)) X(void, alFilterf, (ALuint, ALenum, ALfloat))
EFX_FUNCS(AL_DECL)

static ALCdevice *g_dev;
// PORT_AUDIO_WAV=<file>: no speakers; render the mix through an ALC_SOFT_loopback device (48 kHz stereo s16) up to the audio clock
// each tick (the frame clock under PORT_FIXEDTIME).
typedef ALCdevice *(*alcLoopbackOpenDeviceSOFT_t)(const char *);
typedef void (*alcRenderSamplesSOFT_t)(ALCdevice *, void *, ALsizei);
static alcRenderSamplesSOFT_t alcRenderSamplesSOFT;
static FILE *g_wav;
static long long g_wavSamples;
static double g_wavT0 = -1;
static int g_frame;
static FILE *g_altrace;                      // PORT_ALTRACE=<file>: frame, wave, final L / R gain, Hz of every audible voice per tick
enum { kWavRate = 48000 };
static void wavHeader()
{
	unsigned data = (unsigned)(g_wavSamples * 4), v[11] = { 0x46464952, 36 + data, 0x45564157, 0x20746d66, 16, 0x00020001, kWavRate, kWavRate * 4, 0x00100004, 0x61746164, data };
	fseek(g_wav, 0, SEEK_SET); fwrite(v, 4, 11, g_wav); fseek(g_wav, 0, SEEK_END);
}
static ALCcontext *g_ctx;
static bool g_on;
// Focus lost/gained (window_sdl.cpp). The game's DirectSound buffers lack DSBCAPS_GLOBALFOCUS, so on Windows the game was muted while
// unfocused; OpenAL keeps looping ambiences, so the listener gain is 0 while unfocused. PORT_MUTE=1 (default on with PORT_HIDDEN=1):
// the mix runs as usual with listener gain 0, so timing is unchanged but nothing is audible. A PORT_AUDIO_WAV capture is never muted.
static bool muted()
{
	static int m = -1;
	if (m < 0) {
		const char *v = getenv("PORT_MUTE");
		if (!v || !*v)
			v = getenv("PORT_HIDDEN");
		m = v && *v == '1';
	}
	return m && !g_wav;
}
extern "C" void port_audio_focus(int focused)
{
	if (g_on)
		alListenerf(AL_GAIN, focused && !muted() ? 1.0f : 0.0f);
}
static ALuint g_reverbSlot, g_reverbFx;      // 0 = no EFX
// Reverb as RWA's soft path: every voice and stream sends at its own post-distance, post-gain level (send 1.0, 0x5be5a0; send bus
// 255/256 of dry, 0x5bf430). Off unless PORT_REVERB=<scale> scales the sends: the wet level is unverified against the DMO.
static bool g_reverbOn;
static ALuint g_sendFilter;                  // AL_FILTER_NULL at scale 1, else a flat low-pass (gain = scale)
static AlaListener g_lis = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 1, 0, 0 }, 10.0f, 1.0f, 1.0f };   // RWA listener defaults (0x50bdd0)
static float g_master = 1.0f;                // output +0x38
static float clampf(float x, float lo, float hi) { return x < lo ? lo : x > hi ? hi : x; }
static float mB(int v) { return powf(10.0f, (float)v / 2000.0f); }   // millibels -> linear amplitude

bool alaActive(void) { return g_on; }

// The 12-dword DSFXI3DL2Reverb preset (output +0x44) -> EFX reverb (same values as the DMO, SetAllParameters 0x56d2e0)
static void applyEnvironment(const int *env)
{
	if (!g_reverbSlot) return;
	// DSFXI3DL2Reverb {lRoom, lRoomHF, flRoomRolloffFactor, flDecayTime, flDecayHFRatio, lReflections, flReflectionsDelay, lReverb,
	// flReverbDelay, flDiffusion (%), flDensity (%), flHFReference} -> EFX standard reverb (the I3DL2 -> EFX mapping of the EFX guide)
	const float *f = (const float *)env;
	alEffectf(g_reverbFx, AL_REVERB_GAIN, clampf(mB(env[0]), 0.0f, 1.0f));
	alEffectf(g_reverbFx, AL_REVERB_GAINHF, clampf(mB(env[1]), 0.0f, 1.0f));
	alEffectf(g_reverbFx, AL_REVERB_ROOM_ROLLOFF_FACTOR, clampf(f[2], 0.0f, 10.0f));
	alEffectf(g_reverbFx, AL_REVERB_DECAY_TIME, clampf(f[3], 0.1f, 20.0f));
	alEffectf(g_reverbFx, AL_REVERB_DECAY_HFRATIO, clampf(f[4], 0.1f, 2.0f));
	alEffectf(g_reverbFx, AL_REVERB_REFLECTIONS_GAIN, clampf(mB(env[5]), 0.0f, 3.16f));
	alEffectf(g_reverbFx, AL_REVERB_REFLECTIONS_DELAY, clampf(f[6], 0.0f, 0.3f));
	alEffectf(g_reverbFx, AL_REVERB_LATE_REVERB_GAIN, clampf(mB(env[7]), 0.0f, 10.0f));
	alEffectf(g_reverbFx, AL_REVERB_LATE_REVERB_DELAY, clampf(f[8], 0.0f, 0.1f));
	alEffectf(g_reverbFx, AL_REVERB_DIFFUSION, clampf(f[9] / 100.0f, 0.0f, 1.0f));
	alEffectf(g_reverbFx, AL_REVERB_DENSITY, clampf(f[10] / 100.0f, 0.0f, 1.0f));
	ALenum e1 = alGetError();
	alAuxiliaryEffectSloti(g_reverbSlot, AL_EFFECTSLOT_EFFECT, (ALint)g_reverbFx);   // re-attach = apply the new parameters
	ALLOG("rwaudio_al: environment room %d mB, decay %.2f s, sends %s (err %x %x, slot %u fx %u)\n", env[0], (double)f[3], g_reverbOn ? "on" : "off", e1, alGetError(), g_reverbSlot, g_reverbFx);
}
void alaOutputSync(float masterGain, const int *env, bool envChanged)
{
	if (!g_on) return;
	g_master = masterGain;
	if (envChanged && env) applyEnvironment(env);
}

bool alaOpen(void)
{
	if (g_on) return true;
	const char *e = getenv("PORT_AUDIO");
	if (e && atoi(e) == 0) { ALLOG("rwaudio_al: PORT_AUDIO=0: silent\n"); return false; }
	const char *dll = getenv("PORT_OPENAL");
	void *lib = AlOpenLibrary(dll && dll[0] ? dll : AL_LIBRARY);
	if (!lib) { ALLOG("rwaudio_al: %s not found: silent\n", dll && dll[0] ? dll : AL_LIBRARY); return false; }
	bool ok = true;
#define AL_LOAD(r, n, a) n = (n##_t)AlSymbol(lib, #n); if (!n) { ALLOG("rwaudio_al: missing %s\n", #n); ok = false; }
	AL_FUNCS(AL_LOAD)
	if (!ok) return false;
	const char *wav = getenv("PORT_AUDIO_WAV");
	if (wav && wav[0]) {
		alcLoopbackOpenDeviceSOFT_t open = (alcLoopbackOpenDeviceSOFT_t)alcGetProcAddress(0, "alcLoopbackOpenDeviceSOFT");
		alcRenderSamplesSOFT = (alcRenderSamplesSOFT_t)alcGetProcAddress(0, "alcRenderSamplesSOFT");
		g_dev = open && alcRenderSamplesSOFT ? open(0) : 0;
		g_wav = g_dev ? fopen(wav, "wb") : 0;
		if (!g_wav) { ALLOG("rwaudio_al: PORT_AUDIO_WAV: no loopback device or cannot write %s: silent\n", wav); return false; }
		wavHeader();
	} else g_dev = alcOpenDevice(0);
	if (!g_dev) { ALLOG("rwaudio_al: no playback device: silent\n"); return false; }
	ALint attrs[] = { ALC_MAX_AUXILIARY_SENDS, 1, 0x1990 /*ALC_FORMAT_CHANNELS_SOFT*/, 0x1501 /*ALC_STEREO_SOFT*/,
		0x1991 /*ALC_FORMAT_TYPE_SOFT*/, 0x1402 /*ALC_SHORT_SOFT*/, 0x1007 /*ALC_FREQUENCY*/, kWavRate, 0 };
	if (!g_wav) attrs[2] = 0;                   // a real device keeps its own format
	g_ctx = alcCreateContext(g_dev, attrs);
	if (!g_ctx || !alcMakeContextCurrent(g_ctx)) { ALLOG("rwaudio_al: no context: silent\n"); alcCloseDevice(g_dev); g_dev = 0; return false; }
	alDistanceModel(AL_NONE);                       // RWA's soft mixer does its own distance law (alaVoiceSync)
	if (muted())
		alListenerf(AL_GAIN, 0.0f);
	if (alcIsExtensionPresent(g_dev, "ALC_EXT_EFX")) {
#define EFX_LOAD(r, n, a) n = (n##_t)alGetProcAddress(#n);
		EFX_FUNCS(EFX_LOAD)
		if (alGenEffects && alEffecti && alEffectf && alGenAuxiliaryEffectSlots && alAuxiliaryEffectSloti) {
			alGenEffects(1, &g_reverbFx); alEffecti(g_reverbFx, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
			alGenAuxiliaryEffectSlots(1, &g_reverbSlot);
			if (alGetError()) g_reverbSlot = g_reverbFx = 0;
			const char *rv = getenv("PORT_REVERB");
			float scale = rv && rv[0] ? (float)atof(rv) : 0.0f;
			g_reverbOn = g_reverbSlot && scale > 0;
			if (g_reverbOn && scale != 1.0f && alGenFilters && alFilteri && alFilterf) {
				alGenFilters(1, &g_sendFilter); alFilteri(g_sendFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
				alFilterf(g_sendFilter, AL_LOWPASS_GAIN, clampf(scale, 0.0f, 1.0f)); alFilterf(g_sendFilter, 2 /*AL_LOWPASS_GAINHF*/, 1.0f);
				if (alGetError()) g_sendFilter = 0;
			}
			// Until the game applies one the DMO runs the I3DL2 "generic" preset (g_envPresets[1]); each level's AudioGlobalMixer ctor (0x406850)
			// then sets room/roomHF -10000 (no reverb) until a script calls SetEnvironment.
			static const float kGeneric[12] = { 0, 0, 0.0f, 1.49f, 0.83f, 0, 0.007f, 0, 0.011f, 100.0f, 100.0f, 5000.0f };
			int env[12]; memcpy(env, kGeneric, sizeof env);
			env[0] = -1000; env[1] = -100; env[5] = -2602; env[7] = 200;
			applyEnvironment(env);
		}
	}
	const char *tr = getenv("PORT_ALTRACE");
	if (tr && tr[0] && (g_altrace = fopen(tr, "w")) != 0) fputs("frame\twave\tgl\tgr\thz\n", g_altrace);
	g_on = true;
	static bool exitHook;                        // close the device before the process exits: OpenAL Soft's mixer threads still running at
	if (!exitHook) { exitHook = true; atexit(alaClose); }   // DLL unload abort (mingw condvar assertion, exit code 3)
	ALLOG("rwaudio_al: OpenAL on '%s' (%s %s), EFX reverb %s\n", alcGetString(g_dev, 0x1005 /*ALC_DEVICE_SPECIFIER*/), alGetString(0xB001 /*AL_RENDERER*/),
		alGetString(0xB002 /*AL_VERSION*/), g_reverbSlot ? "yes" : "no");
	return true;
}

// ---------------------------------------------------------------- wave buffers (one AL buffer per RwaWave, made on first play)
static struct WaveBuf { const void *wave; ALuint buf; } g_waves[4096];
static ALuint waveBuffer(const AlaVoice &v)
{
	unsigned h = (unsigned)(size_t)v.wave / 8 % 4096;
	for (unsigned n = 0; n < 4096; n++, h = (h + 1) % 4096) {
		if (g_waves[h].wave == v.wave) return g_waves[h].buf;
		if (!g_waves[h].wave) {
			if (!v.data || !v.dataLen || !v.rate || (v.bits != 8 && v.bits != 16) || (v.channels != 1 && v.channels != 2)) return 0;
			ALenum fmt = v.channels == 1 ? (v.bits == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) : (v.bits == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);
			unsigned frame = v.bits / 8 * v.channels;
			ALuint b = 0;
			alGenBuffers(1, &b);
			alBufferData(b, fmt, v.data, (ALsizei)(v.dataLen / frame * frame), (ALsizei)v.rate);
			if (alGetError()) { ALLOG("rwaudio_al: buffer for wave %p failed\n", v.wave); return 0; }
			g_waves[h].wave = v.wave; g_waves[h].buf = b;
			return b;
		}
	}
	return 0;
}

// ---------------------------------------------------------------- voices: one AL source per virtual voice
static struct VoiceSrc { void *key; ALuint src; const void *wave; unsigned stamp; bool started; } g_vs[128];
static VoiceSrc *voiceSrc(void *key, bool make)
{
	VoiceSrc *free_ = 0;
	for (int i = 0; i < 128; i++) { if (g_vs[i].key == key) return &g_vs[i]; if (!g_vs[i].key && !free_) free_ = &g_vs[i]; }
	if (!make || !free_) return 0;
	memset(free_, 0, sizeof *free_);
	alGenSources(1, &free_->src);
	if (alGetError()) return 0;
	free_->key = key;
	return free_;
}
void alaVoiceDrop(void *key)
{
	if (!g_on) return;
	VoiceSrc *s = voiceSrc(key, false);
	if (!s) return;
	alSourceStop(s->src); alSourcei(s->src, AL_BUFFER, 0); alDeleteSources(1, &s->src);
	memset(s, 0, sizeof *s);
}
void alaWaveDrop(const void *wave)
{
	if (!g_on || !wave) return;
	for (int i = 0; i < 128; i++)
		if (g_vs[i].key && g_vs[i].wave == wave) { alSourceStop(g_vs[i].src); alSourcei(g_vs[i].src, AL_BUFFER, 0); g_vs[i].wave = 0; g_vs[i].started = false; }
	unsigned h = (unsigned)(size_t)wave / 8 % 4096;
	for (unsigned n = 0; n < 4096 && g_waves[h].wave; n++, h = (h + 1) % 4096)
		if (g_waves[h].wave == wave) {
			alDeleteBuffers(1, &g_waves[h].buf);
			// keep the probe chain intact: re-insert the entries that follow
			g_waves[h].wave = 0; g_waves[h].buf = 0;
			for (unsigned k = (h + 1) % 4096; g_waves[k].wave; k = (k + 1) % 4096) {
				WaveBuf w = g_waves[k]; g_waves[k].wave = 0; g_waves[k].buf = 0;
				unsigned j = (unsigned)(size_t)w.wave / 8 % 4096;
				while (g_waves[j].wave) j = (j + 1) % 4096;
				g_waves[j] = w;
			}
			return;
		}
}
void alaVoiceSync(void *key, const AlaVoice &v)
{
	if (!g_on) return;
	VoiceSrc *s = voiceSrc(key, v.playing);
	if (!s) return;
	ALuint src = s->src;
	if (!v.playing) {
		if (s->started) { alSourceStop(src); s->started = false; }
		return;
	}
	bool restart = !s->started || s->wave != v.wave || s->stamp != v.stamp;
	if (restart) {
		ALuint b = waveBuffer(v);
		alSourceStop(src);
		alSourcei(src, AL_BUFFER, (ALint)b);
		s->wave = v.wave; s->stamp = v.stamp; s->started = true;
		if (!b) return;
	}
// Parameters per tick as RWA's software mixer computes them (_rwaListenerCalculateVoiceAttributes 0x50bea0): output gain x distance
// attenuation x voice gain, constant-power pan, Doppler on the pitch. The source is a listener-relative 2D source; OpenAL's distance
// model is off.
	float atten = 1.0f, pan = clampf(v.pan, -1.0f, 1.0f), dop = 1.0f;
	if (v.is3d && v.channels == 1) {
		float d3[3] = { v.pos[0] - g_lis.pos[0], v.pos[1] - g_lis.pos[1], v.pos[2] - g_lis.pos[2] };
		float d = sqrtf(d3[0] * d3[0] + d3[1] * d3[1] + d3[2] * d3[2]);
		float mn = v.minDist, mx = v.maxDist, r = g_lis.distanceFactor;
		if (mx > 0) {                              // 0 if d > max, 1 if d <= min, else (R - x) / (x r + R), x = d - min, R = max - min
			if (d > mx) atten = 0.0f;
			else if (d > mn) { float x = d - mn, R = mx - mn; atten = R > 0 ? (R - x) / (x * r + R) : 0.0f; }
		}
		pan = 0.0f;                                // pan = 2 o - 1, o = clamp(0.5 - 0.5 dot(dir, listener right)) (RW's right points to screen left)
		if (d > 1e-6f) {
			float o = clampf(0.5f - 0.5f * (d3[0] * g_lis.right[0] + d3[1] * g_lis.right[1] + d3[2] * g_lis.right[2]) / d, 0.0f, 1.0f);
			pan = 2.0f * o - 1.0f;
		}
		if (v.vel[0] || v.vel[1] || v.vel[2]) {   // Doppler: v = clamp(d - |(P - V) - L|, +-150) m/s, dop = (340 S - v D) / (340 S)
			float q[3] = { d3[0] - v.vel[0], d3[1] - v.vel[1], d3[2] - v.vel[2] };
			float sp = clampf(d - sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2]), -150.0f, 150.0f), S = g_lis.rolloff, D = g_lis.doppler;
			if (S > 0) dop = clampf((340.0f * S - sp * D) / (340.0f * S), 0.1f, 4.0f);
		}
	}
	alSourcei(src, AL_LOOPING, v.loop ? 1 : 0);
	alSourcef(src, AL_PITCH, v.rate ? clampf(v.freq * dop / (float)v.rate, 0.01f, 16.0f) : 1.0f);
	alSourcef(src, AL_GAIN, clampf(g_master * atten * v.gain, 0.0f, 4.0f));
	if (g_altrace && s->started && s->wave) {
		float sp = pan * 0.5f + 0.5f, g = g_master * atten * v.gain;
		fprintf(g_altrace, "%d\t%s\t%.9g\t%.9g\t%.9g\n", g_frame, v.name ? v.name : "?", (double)(g * sqrtf(1.0f - sp)), (double)(g * sqrtf(sp)),
			(double)(v.freq * dop));
	}
	alSourcei(src, AL_SOURCE_RELATIVE, 1);       // pan as a direction in front of the listener: OpenAL's stereo law then gives L = sqrt(1-s), R = sqrt(s)
	alSource3f(src, AL_POSITION, pan, 0.0f, -sqrtf(1.0f - pan * pan));
	if (g_reverbSlot) alSource3i(src, AL_AUXILIARY_SEND_FILTER, g_reverbOn ? (ALint)g_reverbSlot : 0, 0, (ALint)g_sendFilter);
	if (restart) {
		double total = v.data && v.bits && v.channels ? (double)(v.dataLen / (v.bits / 8 * v.channels)) : 0;
		double off = v.posSamples;
		if (total > 0 && off >= total) off = v.loop ? fmod(off, total) : total - 1;
		alSourcei(src, AL_SAMPLE_OFFSET, off > 0 ? (ALint)off : 0);
		alSourcePlay(src);
		ALenum err = alGetError();
		static int n;
		if (err || n++ < 60) ALLOG("rwaudio_al: voice %p start wave %p at %.0f, %u Hz x %.3f, gain %.2f, %s%s, err %x\n", key, v.wave, off, v.rate,
			v.rate ? (double)(v.freq / (float)v.rate) : 0.0, (double)v.gain, v.is3d ? "3D" : "2D", v.loop ? " loop" : "", err);
	}
}

// ---------------------------------------------------------------- listener and output
void alaListenerSync(const AlaListener &l) { g_lis = l; }   // the AL listener stays at the origin: RWA's math runs in alaVoiceSync

// ---------------------------------------------------------------- streams: .rws segment, Xbox IMA ADPCM decoded here, queued to a source
enum { kStreamBufs = 4, kChunkFrames = 96 };       // 96 frames x 64 = 6144 samples (0.28 s at 22 kHz) per buffer, 4 queued
static struct StreamSrc {
	void *key; ALuint src, bufs[kStreamBufs];
	FILE *f; char path[260]; const WdRws *rws; int seg; unsigned gen;
	unsigned cursor, totalFrames;                  // next frame of the segment to decode
	bool loop, active, ended;
	float gain[2][2];
} g_ss[4];
static StreamSrc *streamSrc(void *key, bool make)
{
	StreamSrc *free_ = 0;
	for (int i = 0; i < 4; i++) { if (g_ss[i].key == key) return &g_ss[i]; if (!g_ss[i].key && !free_) free_ = &g_ss[i]; }
	if (!make || !free_) return 0;
	memset(free_, 0, sizeof *free_);
	alGenSources(1, &free_->src); alGenBuffers(kStreamBufs, free_->bufs);
	if (alGetError()) return 0;
	alSourcei(free_->src, AL_SOURCE_RELATIVE, 1); alSourcef(free_->src, AL_ROLLOFF_FACTOR, 0.0f);
	free_->key = key; free_->seg = -1;
	return free_;
}
static void streamHalt(StreamSrc *s)
{
	alSourceStop(s->src);
	ALint q = 0; alGetSourcei(s->src, AL_BUFFERS_QUEUED, &q);
	ALuint tmp[kStreamBufs];
	if (q > 0) alSourceUnqueueBuffers(s->src, q > kStreamBufs ? kStreamBufs : q, tmp);
	s->active = false;
}
void alaStreamDrop(void *key)
{
	if (!g_on) return;
	StreamSrc *s = streamSrc(key, false);
	if (!s) return;
	streamHalt(s);
	alDeleteSources(1, &s->src); alDeleteBuffers(kStreamBufs, s->bufs);
	if (s->f) fclose(s->f);
	memset(s, 0, sizeof *s);
}
// decode the next chunk into `buf`; false at the end of a non-looping segment
static bool streamFill(StreamSrc *s, ALuint buf)
{
	const WdRws *r = s->rws;
	int ch = r->fmt.channels ? r->fmt.channels : 1, isPcm = 0;
	unsigned fbytes = wdFrameBytes(r, &isPcm);
	if (!s->f || !s->totalFrames || !fbytes || fbytes > 256) return false;
	static short pcm[kChunkFrames * 64 * 2];
	unsigned char fr[256];
	unsigned n = 0;
	while (n < kChunkFrames) {
		if (s->cursor >= s->totalFrames) { if (!s->loop) break; s->cursor = 0; }
		long off = wdFrameOffset(r, s->seg, s->cursor);
		if (off < 0 || fseek(s->f, off, SEEK_SET) || fread(fr, 1, fbytes, s->f) != fbytes) { s->totalFrames = s->cursor; break; }
		if (isPcm) memcpy(pcm + n * 64 * ch, fr, fbytes);
		else wdDecodeXboxIma(fr, ch, pcm + n * 64 * ch);
		n++; s->cursor++;
	}
	if (!n) return false;
	if (ch == 2) {                                 // per-channel gains of a stereo stream: left = side 0, right = side 1 of pair 0
		for (unsigned i = 0; i < n * 64; i++) {
			pcm[2 * i] = (short)(pcm[2 * i] * clampf(s->gain[0][0], 0, 1));
			pcm[2 * i + 1] = (short)(pcm[2 * i + 1] * clampf(s->gain[0][1], 0, 1));
		}
	}
	alBufferData(buf, ch == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pcm, (ALsizei)(n * 64 * ch * 2), (ALsizei)r->fmt.rate);
	return !alGetError();
}
static void streamStart(StreamSrc *s, double posSec)
{
	streamHalt(s);
	const WdRws *r = s->rws;
	s->totalFrames = wdSegmentFrames(r, s->seg);
	s->cursor = r->fmt.rate ? (unsigned)(posSec * r->fmt.rate / 64.0) : 0;
	if (s->cursor >= s->totalFrames) s->cursor = s->loop && s->totalFrames ? s->cursor % s->totalFrames : s->totalFrames;
	int q = 0;
	for (; q < kStreamBufs; q++) if (!streamFill(s, s->bufs[q])) break;
	if (q) { alSourceQueueBuffers(s->src, q, s->bufs); alSourcePlay(s->src); }
	s->active = true; s->ended = q == 0;
	ALLOG("rwaudio_al: stream %p start %s segment %d at %.2f s (frame %u/%u), %d buffers, err %x\n", s->key, s->path, s->seg, posSec, s->cursor,
		s->totalFrames, q, alGetError());
}
void alaStreamSync(void *key, const AlaStream &st)
{
	if (!g_on) return;
	bool want = (st.state == 6 || st.state == 5) && st.rws && st.seg >= 0 && (unsigned)st.seg < st.rws->nseg;
	StreamSrc *s = streamSrc(key, want);
	if (!s) return;
	if (!want) { if (s->active) streamHalt(s); return; }
	if (!s->f || strcmp(s->path, st.path)) {
		if (s->f) fclose(s->f);
		char native[1024];
		s->f = fopen(PORT_NATIVE_PATH(st.path, native), "rb");
		strncpy(s->path, st.path, sizeof s->path - 1);
		if (!s->f) { ALLOG("rwaudio_al: cannot open stream %s\n", st.path); return; }
	}
	memcpy(s->gain, st.gain, sizeof s->gain);
	s->loop = st.loop;
	if (!s->active || s->gen != st.gen || s->seg != st.seg || s->rws != st.rws) {
		s->rws = st.rws; s->seg = st.seg; s->gen = st.gen;
		streamStart(s, st.posSec);
	}
	int ch = st.rws->fmt.channels ? st.rws->fmt.channels : 1;
	float l = st.gain[0][0], r = st.gain[0][1];
	if (ch == 1) {                                 // mono: the two side gains as a level and a pan
		float g = l > r ? l : r, p = (l + r) > 0 ? clampf((r - l) / (l + r), -1, 1) : 0;
		alSourcef(s->src, AL_GAIN, clampf(g_master * g, 0, 4));
		alSource3f(s->src, AL_POSITION, p, 0.0f, -sqrtf(1.0f - p * p));
	} else alSourcef(s->src, AL_GAIN, clampf(g_master, 0, 4));
	alSourcef(s->src, AL_PITCH, clampf(st.pitch > 0 ? st.pitch : 1.0f, 0.01f, 16.0f));
	if (g_reverbSlot) alSource3i(s->src, AL_AUXILIARY_SEND_FILTER, g_reverbOn ? (ALint)g_reverbSlot : 0, 0, (ALint)g_sendFilter);   // streams send too
	ALint state = 0; alGetSourcei(s->src, AL_SOURCE_STATE, &state);
	if (st.state == 5 && state == AL_PLAYING) alSourcePause(s->src);
	else if (st.state == 6 && state == AL_PAUSED) alSourcePlay(s->src);
}
void alaTickBegin(int frame) { g_frame = frame; }
void alaTickEnd(double nowSec)
{
	if (!g_on) return;
	if (g_wav) {                                 // loopback: render the mix up to the audio clock
		if (g_wavT0 < 0) { g_wavT0 = nowSec; ALLOG("rwaudio_al: PORT_AUDIO_WAV capture starts at frame %d (audio clock %.4f s)\n", g_frame, nowSec); }
		long long want = (long long)((nowSec - g_wavT0) * kWavRate);
		static short buf[kWavRate * 2];
		while (g_wavSamples < want) {
			ALsizei n = (ALsizei)(want - g_wavSamples > kWavRate ? kWavRate : want - g_wavSamples);
			alcRenderSamplesSOFT(g_dev, buf, n);
			fwrite(buf, 4, (size_t)n, g_wav);
			g_wavSamples += n;
		}
		wavHeader();
	}
	if (g_altrace) fflush(g_altrace);
	for (int i = 0; i < 4; i++) {
		StreamSrc *s = &g_ss[i];
		if (!s->key || !s->active || s->ended) continue;
		ALint done = 0; alGetSourcei(s->src, AL_BUFFERS_PROCESSED, &done);
		while (done-- > 0) {
			ALuint b; alSourceUnqueueBuffers(s->src, 1, &b);
			if (streamFill(s, b)) alSourceQueueBuffers(s->src, 1, &b); else { s->ended = true; break; }
		}
		ALint state = 0, q = 0; alGetSourcei(s->src, AL_SOURCE_STATE, &state); alGetSourcei(s->src, AL_BUFFERS_QUEUED, &q);
		if (state == AL_STOPPED && q > 0) alSourcePlay(s->src);    // underrun (a long frame): keep going
	}
}
enum { kPcmBufs = 32 };
static struct PcmSrc {
	ALuint src, bufs[kPcmBufs];
	unsigned frames[kPcmBufs];
	int head, count;                               // queued buffers: bufs[head .. head + count), oldest first
	unsigned rate, channels;
	unsigned long long done, pushed;               // frames of the unqueued (played) buffers, frames pushed
	bool open, started;
} g_pcm;
static void pcmReclaim()
{
	ALint n = 0; alGetSourcei(g_pcm.src, AL_BUFFERS_PROCESSED, &n);
	for (; n > 0 && g_pcm.count > 0; n--) {
		ALuint b; alSourceUnqueueBuffers(g_pcm.src, 1, &b);
		g_pcm.done += g_pcm.frames[g_pcm.head];
		g_pcm.head = (g_pcm.head + 1) % kPcmBufs; g_pcm.count--;
	}
}
bool alaPcmOpen(unsigned rate, unsigned channels)
{
	alaPcmClose();
	if (!g_on || g_wav || !rate || channels < 1 || channels > 2) return false;
	memset(&g_pcm, 0, sizeof g_pcm);
	alGenSources(1, &g_pcm.src); alGenBuffers(kPcmBufs, g_pcm.bufs);
	if (alGetError()) { ALLOG("rwaudio_al: movie sound: no source\n"); return false; }
	alSourcei(g_pcm.src, AL_SOURCE_RELATIVE, 1); alSourcef(g_pcm.src, AL_ROLLOFF_FACTOR, 0.0f);
	g_pcm.rate = rate; g_pcm.channels = channels; g_pcm.open = true;
	return true;
}
bool alaPcmPush(const short *pcm, unsigned frames)
{
	if (!g_pcm.open || !frames) return g_pcm.open;
	pcmReclaim();
	if (g_pcm.count == kPcmBufs) return false;
	int i = (g_pcm.head + g_pcm.count) % kPcmBufs;
	alBufferData(g_pcm.bufs[i], g_pcm.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pcm, (ALsizei)(frames * g_pcm.channels * 2),
		(ALsizei)g_pcm.rate);
	alSourceQueueBuffers(g_pcm.src, 1, &g_pcm.bufs[i]);
	g_pcm.frames[i] = frames; g_pcm.count++; g_pcm.pushed += frames;
	return !alGetError();
}
void alaPcmPlay()
{
	if (!g_pcm.open) return;
	pcmReclaim();
	ALint state = 0; alGetSourcei(g_pcm.src, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING && g_pcm.count > 0) { alSourcePlay(g_pcm.src); g_pcm.started = true; }
}
unsigned long long alaPcmPlayedFrames()
{
	if (!g_pcm.open) return 0;
	pcmReclaim();
	ALint off = 0, state = 0; alGetSourcei(g_pcm.src, AL_SOURCE_STATE, &state);
	if (g_pcm.count > 0 && (state == AL_PLAYING || state == AL_PAUSED)) alGetSourcei(g_pcm.src, AL_SAMPLE_OFFSET, &off);
	return g_pcm.done + (unsigned long long)(off > 0 ? off : 0);
}
double alaPcmQueuedSec() { return g_pcm.open ? (double)(g_pcm.pushed - alaPcmPlayedFrames()) / g_pcm.rate : 0.0; }
void alaPcmClose()
{
	if (!g_pcm.open) return;
	alSourceStop(g_pcm.src);
	pcmReclaim();
	alSourcei(g_pcm.src, AL_BUFFER, 0);
	alDeleteSources(1, &g_pcm.src); alDeleteBuffers(kPcmBufs, g_pcm.bufs);
	memset(&g_pcm, 0, sizeof g_pcm);
}
void alaClose(void)
{
	if (!g_on) return;
	alaPcmClose();
	for (int i = 0; i < 4; i++) if (g_ss[i].key) alaStreamDrop(g_ss[i].key);
	for (int i = 0; i < 128; i++) if (g_vs[i].key) alaVoiceDrop(g_vs[i].key);
	for (int i = 0; i < 4096; i++) if (g_waves[i].wave) { alDeleteBuffers(1, &g_waves[i].buf); g_waves[i].wave = 0; }
	if (g_wav) { wavHeader(); fclose(g_wav); g_wav = 0; }
	if (g_altrace) { fclose(g_altrace); g_altrace = 0; }
	alcMakeContextCurrent(0); alcDestroyContext(g_ctx); alcCloseDevice(g_dev);
	g_ctx = 0; g_dev = 0; g_on = false;
}
