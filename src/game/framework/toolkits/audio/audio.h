// Audio toolkit over RWA: output, listener, real and virtual voices, wave dictionary slots. `anon` is the framework's unnamed namespace.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_AUDIO_AUDIO_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_AUDIO_AUDIO_H

#include "rw/rwaudio.h"

namespace RWS { namespace RwsAudio {

const unsigned MAXVOICES = 15;    // real voices
const unsigned MAXVVOICES = 50;   // virtual voices
const unsigned NUMGROUPS = 16;    // wave dictionary slots
const unsigned NUMSTREAMS = 4;    // game addition

// RWA works in metres, the game in inches.
static inline float InchesToMetres(float inches) { return inches * 0.0254f; }
static inline float MetresToInches(float metres) { return metres * 39.37f; }

struct VirtualVoiceSlot {
	RwaVirtualVoice *pVirtualVoice;
	RwBool VoiceAlloc;
};

namespace anon {
RwBool AudioInit();
void CreateAllVoices();   // virtual voices start at gain 1, pan 0
void CreateAllStreams();
void DestroyAllStreams();
void DestroyAllVoices();
}   // namespace anon

RwBool RegisterAudioObjects();
void UnRegisterAudioObjects();
void Open();   // reference counted
void Close();
RwaOsOutput *GetOutputObject();
RwBool FadeOutputObject(float destination, float step);   // true once at `destination`
RwaEnvironment *GetEnvironment();
RwaListener *GetListener();
unsigned AddDictionary(RwaWaveDict *dict);   // the slot index, 16 when full
void RemoveDictionary(RwaWaveDict *dict);
RwaWave *FindWave(const char *name);
RwaVirtualVoice *AllocateVirtualVoice();   // 0 when all are in use
void FreeVirtualVoice(RwaVirtualVoice *voice);
void StopAllVirtualVoices();
RwaStream *GetStream(int index);

}}   // namespace RWS::RwsAudio

// The game defines the toolkit's tables as globals.
extern RwaStream *g_audioStreams[RWS::RwsAudio::NUMSTREAMS];
extern RwaOsVoice *g_audioVoices[RWS::RwsAudio::MAXVOICES];
extern RWS::RwsAudio::VirtualVoiceSlot g_audioVirtualVoices[RWS::RwsAudio::MAXVVOICES];
extern RwaOsOutput *g_audioOutput;
extern RwaListener *g_audioListener;
extern RwaWaveDict *g_audioWaveDicts[RWS::RwsAudio::NUMGROUPS];
extern unsigned g_audioRefCount;
extern RwBool g_audioCoreOpen;
extern RwaCore *g_audioCore;

#endif
