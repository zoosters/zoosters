// RenderWare Audio (RWA) entry points the game calls, and the RWA object layouts it reads or writes directly.
// No RWA header is available (the game links a PC build statically), so the declarations are ABI-level: handles are opaque structs
// and arguments of unknown meaning are plain ints named `argN` (the game always passes constants). Type names follow RWA's (Rwa*).
#ifndef ZOOSTERS_RW_RWAUDIO_H
#define ZOOSTERS_RW_RWAUDIO_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
extern "C" {

// ---------------------------------------------------------------------------------------------------------------------------
// Object layouts. Partial: only the members the game's code touches; never allocate or copy one of these.
// ---------------------------------------------------------------------------------------------------------------------------

struct RwaWaveDict;   // a loaded wave dictionary (opaque: the game keeps the pointer, 16 slots in RwsAudio)
struct RwaCore;       // the core object _rwaCoreOpen returns (the game only tests it for non-zero)
struct RwaOsVoice;    // a real (DirectSound) voice; RwsAudio creates 15 and hands them to the voice manager
struct RwaStream;     // a stream object (>= 0xd70 bytes; the game reads its play state through _rwaStreamGetPlayState)
// A backing object handle ({object, interface} pair, RwaObjHandle in RWA). The game calls the generic parameter functions below with
// it. `void` (not a struct): the port's shim defines FUN_00509ca0 & co under mangled names that spell the handle as `void *`.
typedef void RwaObjHandle;

// Format block of a wave (0x1c bytes with 4-byte pointers; the v1.0 PS2 header has 0x14). The game reads the rate and the byte length.
struct RwaWaveFormat {
	unsigned sampleRate;  // Hz
	const unsigned char *dataType;  // UUID of the sample format (16 bytes: PCM, IMA ADPCM)
	unsigned length;  // data length in bytes (the second argument of RwaWaveFormatGetSamples)
	unsigned char bitsPerSample;
	unsigned char numChannels;
	unsigned short m_unk0e;
	const void *miscData;  // codec-specific data, miscSize bytes (never set in this game)
	unsigned miscSize;
	unsigned char flags, flags2;
	unsigned short m_unk1a;
};
ZOOSTERS_ASSERT_SIZE(RwaWaveFormat, 0x1c);
ZOOSTERS_ASSERT_OFFSET(RwaWaveFormat, dataType, 0x04);
ZOOSTERS_ASSERT_OFFSET(RwaWaveFormat, length, 0x08);
ZOOSTERS_ASSERT_OFFSET(RwaWaveFormat, bitsPerSample, 0x0c);
ZOOSTERS_ASSERT_OFFSET(RwaWaveFormat, numChannels, 0x0d);
ZOOSTERS_ASSERT_OFFSET(RwaWaveFormat, miscData, 0x10);
ZOOSTERS_ASSERT_OFFSET(RwaWaveFormat, miscSize, 0x14);
ZOOSTERS_ASSERT_OFFSET(RwaWaveFormat, flags, 0x18);

// A wave of a dictionary (RwaWave): the source format at +0x10, the device ("target") format at +0x2c whose rate Voice::Play uses.
struct RwaWave {
	char m_pad00[0x10];
	RwaWaveFormat format;  // source format: sampleRate, length
	RwaWaveFormat targetFormat;  // format on the output device: sampleRate
};
ZOOSTERS_ASSERT_OFFSET(RwaWave, format, 0x10);
ZOOSTERS_ASSERT_OFFSET(RwaWave, targetFormat, 0x2c);

// Flag bits of RwaVirtualVoice::flags.
enum RwaVirtualVoiceFlags {
	RWA_VVOICE_PLAYING = 2,  // triggered and not finished: the wave position lives on the backing handle (FUN_00509cf0)
	RWA_VVOICE_LOOP = 4
};

// A virtual voice (RwaVirtualVoice, handed out by RwsAudio::AllocateVirtualVoice; the voice manager maps them onto the 15 real voices).
struct RwaVirtualVoice {
	char m_pad00[0x30];  // voice-manager node (RWA internal)
	RwaObjHandle *handle;  // backing object handle; 0 = none (every setting below is mirrored to it when set)
	unsigned flags;  // RwaVirtualVoiceFlags
	int m_unk38;
	float frequency;  // Hz = wave rate x pitch
	float gain;
	float pan;
	int wavePos;  // last known wave position (samples)
	int m_unk4c;
	RwaWave *wave;
	unsigned startStamp;  // start time stamp (written by _rwaVirtualVoiceSetTrigger; not read by game code)
	RwV3d position;  // metres
	RwV3d velocity;  // metres per second
	float minDistance, maxDistance;  // metres
	char m_pad78[0xc];
	int handleOwnsRate;  // != 0: the backing handle owns the sample rate (RwaVirtualVoiceSetWave leaves +0x3c alone); not written by game code
};
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, handle, 0x30);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, flags, 0x34);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, gain, 0x40);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, pan, 0x44);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, wavePos, 0x48);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, startStamp, 0x54);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, velocity, 0x64);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, minDistance, 0x70);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, handleOwnsRate, 0x84);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, frequency, 0x3c);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, wave, 0x50);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, position, 0x58);
ZOOSTERS_ASSERT_OFFSET(RwaVirtualVoice, maxDistance, 0x74);

// The environment (reverb) block of the output: the parameters of DirectSound's I3DL2 reverb effect (DSFXI3DL2Reverb order), 12 words.
struct RwaEnvironment {
	int lRoom, lRoomHF;
	float flRoomRolloffFactor, flDecayTime, flDecayHFRatio;
	int lReflections;
	float flReflectionsDelay;
	int lReverb;
	float flReverbDelay, flDiffusion, flDensity, flHFReference;
};
ZOOSTERS_ASSERT_SIZE(RwaEnvironment, 0x30);

// A UUID (GUID layout): the key of a RWA object definition (_rwaObjDefFindByUUID); g_audioOutputUuid is the DirectSound output's.
struct RwaUuid {
	unsigned m_data1;
	unsigned short m_data2, m_data3;
	unsigned char m_data4[8];
};
ZOOSTERS_ASSERT_SIZE(RwaUuid, 16);

// Dirty-flag bits of the output object's change word (RwaOsOutput::dirtyFlags).
enum RwaOutputDirty { RWA_OUTPUT_ENVIRONMENT = 0x80 };

// The audio output (DirectSound output, RwaOsOutput).
struct RwaOsOutput {
	char m_pad00[0x38];
	float gain;  // master gain (_rwaDXAudioOutputSetGain writes it, RwsAudio::FadeOutputObject reads it)
	char m_pad3c[8];
	RwaEnvironment environment;
	int m_unk74;  // not read by game code
	unsigned dirtyFlags;  // RwaOutputDirty
};
ZOOSTERS_ASSERT_OFFSET(RwaOsOutput, gain, 0x38);
ZOOSTERS_ASSERT_OFFSET(RwaOsOutput, environment, 0x44);
ZOOSTERS_ASSERT_OFFSET(RwaOsOutput, dirtyFlags, 0x78);

// Dirty-flag bits of the listener's change word (RwaListener::dirtyFlags).
enum RwaListenerDirty {
	RWA_LISTENER_POSITION = 0x01,
	RWA_LISTENER_ORIENTATION = 0x0c,
	RWA_LISTENER_ROLLOFF = 0x10,
	RWA_LISTENER_DOPPLER = 0x20,
	RWA_LISTENER_DISTANCE = 0x40
};

// The 3D listener (RwaListener). The game writes it directly each frame (AudioGlobalMixer::UpdateListener) and when the mixer's
// attributes change.
struct RwaListener {
	char m_pad00[0x28];
	RwV3d position;  // metres
	char m_pad34[0xc];
	RwV3d up;
	RwV3d at;
	RwV3d right;
	float rolloffFactor;
	float dopplerScale;
	float distanceFactor;
	unsigned dirtyFlags;  // RwaListenerDirty
};
ZOOSTERS_ASSERT_OFFSET(RwaListener, position, 0x28);
ZOOSTERS_ASSERT_OFFSET(RwaListener, up, 0x40);
ZOOSTERS_ASSERT_OFFSET(RwaListener, at, 0x4c);
ZOOSTERS_ASSERT_OFFSET(RwaListener, right, 0x58);
ZOOSTERS_ASSERT_OFFSET(RwaListener, rolloffFactor, 0x64);
ZOOSTERS_ASSERT_OFFSET(RwaListener, dopplerScale, 0x68);
ZOOSTERS_ASSERT_OFFSET(RwaListener, distanceFactor, 0x6c);
ZOOSTERS_ASSERT_OFFSET(RwaListener, dirtyFlags, 0x70);

// Parameter ids of the voice objects' generic parameter functions.
enum RwaVoiceParam {
	RWA_VOICE_POSITION = 0,         // input, data = RwV3d (RwaObjHandleSetParamData)
	RWA_VOICE_VELOCITY = 1,         // input, data = RwV3d
	RWA_VOICE_MIN_DISTANCE = 2,     // float
	RWA_VOICE_MAX_DISTANCE = 3,     // float
	RWA_VOICE_FREQUENCY = 4,        // float Hz
	RWA_VOICE_GAIN = 5,             // float
	RWA_VOICE_WAVE_POSITION = 7,    // set: samples; get (io type 2): the current position in samples
	RWA_VOICE_LOOP = 8,             // 0 / 1
	RWA_VOICE_PAN = 10              // float
};

// Block of the stream position query (_rwaStreamGetStreamPos). The reply is written asynchronously into `seconds`.
struct RwaStreamPosQuery {
	int arg0;                       // -1
	int arg1;                       // 3
	float seconds;                  // out: seconds played
	int arg3;                       // 1
};

// Media of a stream (_rwaStreamSetMedia).
struct RwaStreamMedia {
	const char *path;               // path of the .rws stream (set by LevelHub: speech 0x617fe8, music 0x617fec)
	int mediaType;                  // 1 = file name
	int arg2, arg3, arg4, arg5;     // 0
	int arg6, arg7;  // speech 2 / 2, music 8 / 4
	int loop;
};

// One item of a stream's item list (FUN_00507de0): the segment to play.
struct RwaStreamItem {
	const char *name;               // segment name (the sound's name)
	int type;                       // 1 = by name
	float start;                    // seconds
	int arg3;                       // 1
};
struct RwaStreamItems {
	RwaStreamItem *items;
	int count;
};

// Segment query (FUN_00508200): resolves a segment by name and reports its length. The struct keeps its name `StreamLoadB`: the port
// shim binds the mangled name of FUN_00508200 (`?FUN_00508200@@YAXPAXPAUStreamLoadB@@@Z`).
struct StreamLoadB {
	const char *name;               // in: segment name
	int type;                       // in: 1 = by name; out: state (3 = found)
	float length;                   // out: length in seconds (when arg3 == 1)
	int unit;                       // in: 1 = length as float seconds
	int arg4;                       // 0
};

// Gain of one channel of a stream (_rwaStreamSetStreamGain).
struct RwaStreamGain {
	int arg0;                       // 0
	int pair;                       // channel pair 0 / 1 (surround streams use both)
	int side;                       // 0 = left, 1 = right
	float gain;
};

// Play states of a stream: the values the game passes to _rwaStreamSetPlayState (1, 2, 3) and reads back from FUN_005080e0 (3, 5, 8, 9).
// Names are inferred from how AudioGlobalMixer's state machine and Voice::Update use them.
enum RwaStreamPlayState {
	RWA_STREAM_STOP = 1,       // request: stop (the mixer waits for PAUSED or IDLE afterwards)
	RWA_STREAM_PLAY = 2,       // request: play
	RWA_STREAM_PAUSED = 3,     // request: start paused (Voice::StartStream); observed: stopped / paused
	RWA_STREAM_READY = 5,      // observed: media opened, waiting for the play request
	RWA_STREAM_FINISHED = 8,   // observed: reached the end of the segment (a looping voice keeps going)
	RWA_STREAM_IDLE = 9        // observed: no media
};

// --- Core: open, register the object definitions, close
RwaCore *_rwaCoreOpen(void *memoryFunctions, int arg1); // the engine's memory function table; returns the core object
void RwaCoreRegisterDefaultObjects(void);
void RwaCoreUnregisterDefaultObjects(void);
void RwaCoreClose(void);
void RwaDXAudioRegisterUsingInterface(void); // the DirectSound object definitions: output, voice, wave, codec
void RwaDXAudioUnregister(void);
void FUN_005074c0(void);  // 0x5074c0 RwaCoreUpdate: one core tick, drives voice start / stop and the voice manager

// --- Object definitions and handles
void *_rwaObjDefFindByUUID(int arg0, const void *uuid, void **arg2);
void RwaObjDefGetDefaultCreationParams(void *objectDefinition, void *creationParams);
void RwaObjDestroy(void *object, int arg1, int arg2);
void RwaObjHandleSetParamData(RwaObjHandle *handle, int paramId, int arg2, void *data);
}
// C++ linkage: the port shim defines these as overloads
// 0x509ca0 RwaObjHandleSetParamReal (the shim binds the four mangled spellings): set a parameter through the handle.
void FUN_00509ca0(RwaObjHandle *handle, int paramId, int arg2, int value);
void FUN_00509ca0(RwaObjHandle *handle, int paramId, int arg2, float value);
// 0x509cf0 RwaObjHandleGetParamReal: read a parameter (io type 2); returns the 4-byte value.
int FUN_00509cf0(RwaObjHandle *handle, int paramId, int ioType, int arg3);
extern "C" {

// --- Output, listener, voices
RwaOsOutput *RwaDXAudioOutputCreate(int arg0, void *creationParams, int flags, int arg3, int arg4);
void _rwaDXAudioOutputSetGain(RwaOsOutput *output, float gain);
void _rwaDXAudioOutputSetListener(RwaOsOutput *output, RwaListener *listener);
void _rwaDXAudioOutputSetSpeakerConfig(RwaOsOutput *output, int speakerConfig);
RwaListener *RwaListenerCreate(int flags, int arg1, int arg2);
RwaOsVoice *RwaDXAudioVoiceCreate(RwaOsOutput *output, int arg1, int flags, int arg3, int arg4);
void RwaVoiceManagerRegister(RwaOsOutput *output, RwaOsVoice **voices, int numVoices);
void RwaVoiceManagerUnregister(void);
RwaVirtualVoice *RwaVirtualVoiceCreate(int arg0, int arg1, int arg2, int arg3, int arg4);
void _rwaVirtualVoiceSetTrigger(RwaVirtualVoice *virtualVoice, int trigger);
void _rwaVirtualVoiceSetWave(RwaVirtualVoice *virtualVoice, RwaWave *wave);

// --- Streams
RwaStream *RwaStreamCreate(RwaOsOutput *output, int arg1, int arg2, int arg3, int arg4);
void _rwaStreamSetMedia(RwaStream *stream, const RwaStreamMedia *media); // 0 clears
void FUN_00507de0(RwaStream *stream, const RwaStreamItems *items);  // 0x507de0 _rwaStreamSetItems: the segment to play
void _rwaStreamSetPlayState(RwaStream *stream, int playState);
int FUN_005080e0(RwaStream *stream);  // 0x5080e0 _rwaStreamGetPlayState: the observed play state
void _rwaStreamSetStreamGain(RwaStream *stream, const RwaStreamGain *gain);
void _rwaStreamSetPitch(RwaStream *stream, float pitch);
void _rwaStreamSetLoopSegment(RwaStream *stream, int loop);
void _rwaStreamGetStreamPos(RwaStream *stream, RwaStreamPosQuery *query); // asynchronous: the reply is written to the query block
}
// 0x508200 _rwaStreamGetSegmentInfo: the length of the segment named in the query. The first parameter stays `void *`: the
// shim binds the mangled name.
void FUN_00508200(void *stream, StreamLoadB *query);
extern "C" {

// --- Waves and wave dictionaries
RwaWaveDict *_rwaWaveDictStreamRead(int arg0, RwStream *stream, RwaOsOutput *output, int arg3, int arg4, int arg5);
RwaWave *RwaWaveDictFindWaveByName(RwaWaveDict *waveDict, const char *name);
void RwaWaveDictDestroy(RwaWaveDict *waveDict);
int RwaWaveDestroy(RwaWave *wave);
unsigned RwaWaveFormatGetSamples(const RwaWaveFormat *format, int numBytes);

}

#endif
