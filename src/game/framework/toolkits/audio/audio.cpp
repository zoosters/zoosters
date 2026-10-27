// Differs from the stock toolkit: four stream objects, AddDictionary returns the slot index, RemoveDictionary does not recreate
// the voices, StopAllVirtualVoices also clears each voice's wave and runs one core update, and the RWA setters are direct field
// writes (RwaVirtualVoice::gain / pan).
#include <string.h>
#include "framework/toolkits/audio/audio.h"
#include "framework/startup/win32/appstate.h"

// GLOBAL: 0x629470 g_audioStreams
// GLOBAL: 0x629480 g_audioVoices
// GLOBAL: 0x6294c0 g_audioVirtualVoices
// GLOBAL: 0x629650 g_audioOutput
// GLOBAL: 0x629654 g_audioListener
// GLOBAL: 0x629658 g_audioWaveDicts
// GLOBAL: 0x629698 g_audioRefCount
// GLOBAL: 0x62969c g_audioCoreOpen
// GLOBAL: 0x60b8c0 g_audioCore
// GLOBAL: 0x606598 g_audioEnvironment
// GLOBAL: 0x5e1900 g_audioOutputUuid

extern const RwaUuid g_audioOutputUuid;
extern RwaEnvironment g_audioEnvironment;
int FUN_0043e6f0(const char *fmt, ...);   // empty: a debug print

// AudioInit sets display, useSoftware, fast3D and enableReverb; the other words keep the defaults RwaObjDefGetDefaultCreationParams wrote.
struct RwaDXAudioOutputCreationParams {
	void *display;        // +0x00 window handle
	int m_unk04;
	int useSoftware;      // +0x08
	int enableReverb;     // +0x0c
	int fast3D;           // +0x10
	int m_unk14, m_unk18, m_unk1c, m_unk20, m_unk24;
};

// 0x44abe0
void RWS::RwsAudio::anon::CreateAllVoices()
{
	for (unsigned i = 0; i < MAXVOICES; ++i)
		g_audioVoices[i] = RwaDXAudioVoiceCreate(g_audioOutput, 0, 2, 0, 0);
	RwaVoiceManagerRegister(g_audioOutput, g_audioVoices, MAXVOICES);
	for (unsigned j = 0; j < MAXVVOICES; ++j) {
		g_audioVirtualVoices[j].pVirtualVoice = RwaVirtualVoiceCreate(0, 0, 0, 0, 0);
		g_audioVirtualVoices[j].pVirtualVoice->gain = 1.0f;
		if (g_audioVirtualVoices[j].pVirtualVoice->handle)
			FUN_00509ca0(g_audioVirtualVoices[j].pVirtualVoice->handle, RWA_VOICE_GAIN, 1, 1.0f);
		g_audioVirtualVoices[j].pVirtualVoice->pan = 0.0f;
		if (g_audioVirtualVoices[j].pVirtualVoice->handle)
			FUN_00509ca0(g_audioVirtualVoices[j].pVirtualVoice->handle, RWA_VOICE_PAN, 1, 0.0f);
	}
}

// 0x44aca0
void RWS::RwsAudio::anon::DestroyAllVoices()
{
	for (unsigned i = 0; i < MAXVVOICES; ++i) {
		RwaObjDestroy(g_audioVirtualVoices[i].pVirtualVoice, 0, 0);
		g_audioVirtualVoices[i].VoiceAlloc = 0;
	}
	RwaVoiceManagerUnregister();
	for (unsigned j = 0; j < MAXVOICES; ++j)
		RwaObjDestroy(g_audioVoices[j], 0, 0);
}

// 0x44acf0
void RWS::RwsAudio::anon::CreateAllStreams()
{
	for (unsigned i = 0; i < NUMSTREAMS; ++i)
		g_audioStreams[i] = RwaStreamCreate(g_audioOutput, 0, 0, 0, 0);
}

// 0x44ad20
void RWS::RwsAudio::anon::DestroyAllStreams()
{
	for (unsigned i = 0; i < NUMSTREAMS; ++i) {
		RwaObjDestroy(g_audioStreams[i], 0, 0);
		g_audioStreams[i] = 0;
	}
}

// 0x44ad50
RwBool RWS::RwsAudio::anon::AudioInit()
{
	RwaDXAudioOutputCreationParams params;
	memset(g_audioVirtualVoices, 0, sizeof(g_audioVirtualVoices));
	memset(g_audioWaveDicts, 0, sizeof(g_audioWaveDicts));
	memset(g_audioVoices, 0, sizeof(g_audioVoices));
	memset(g_audioStreams, 0, sizeof(g_audioStreams));
	RwaObjDefGetDefaultCreationParams(_rwaObjDefFindByUUID(0, &g_audioOutputUuid, 0), &params);
	params.useSoftware = 0;
	params.fast3D = 0;
	params.enableReverb = 1;
	params.display = RWS::Win::GetMainWindow();
	g_audioOutput = RwaDXAudioOutputCreate(0, &params, 6, 0, 0);
	g_audioListener = RwaListenerCreate(4, 0, 0);
	_rwaDXAudioOutputSetListener(g_audioOutput, g_audioListener);
	return 1;
}

// 0x44ae10
RwaStream *RWS::RwsAudio::GetStream(int index)
{
	return g_audioStreams[index];
}

// 0x44ae20
RwBool RWS::RwsAudio::RegisterAudioObjects()
{
	// no engine block yet; the shim ignores the argument
	g_audioCore = _rwaCoreOpen(RwEngineInstance ? &RwEngineGlobals()->memoryFuncs : 0, 1);
	RwBool ok;
	if (g_audioCore) {
		RwaCoreRegisterDefaultObjects();
		RwaDXAudioRegisterUsingInterface();
		ok = 1;
	} else {
		ok = 0;
	}
	g_audioCoreOpen = ok;
	return ok;
}

// 0x44ae60
void RWS::RwsAudio::UnRegisterAudioObjects()
{
	RwaCoreUnregisterDefaultObjects();
	RwaDXAudioUnregister();
	RwaCoreClose();
	g_audioCoreOpen = 0;
}

// 0x44ae80
void RWS::RwsAudio::Open()
{
	if (g_audioRefCount == 0) {
		if (anon::AudioInit()) {
			anon::CreateAllVoices();
			anon::CreateAllStreams();
		}
	}
	++g_audioRefCount;
}

// 0x44aeb0
void RWS::RwsAudio::Close()
{
	if (g_audioRefCount == 1) {
		anon::DestroyAllStreams();
		anon::DestroyAllVoices();
		RwaObjDestroy(g_audioOutput, 0, 0);
		RwaObjDestroy(g_audioListener, 0, 0);
		g_audioOutput = 0;
		g_audioListener = 0;
	}
	--g_audioRefCount;
}

// 0x44af00
unsigned RWS::RwsAudio::AddDictionary(RwaWaveDict *dict)
{
	unsigned i = 0;
	do {
		if (!g_audioWaveDicts[i]) {
			g_audioWaveDicts[i] = dict;
			return i;
		}
		++i;
	} while (i < NUMGROUPS);
	return i;
}

// 0x44af20
RwBool RWS::RwsAudio::FadeOutputObject(float destination, float step)
{
	RwaOsOutput *out = g_audioOutput;
	float cur = out->gain;
	if (destination < cur) {
		cur = cur - step;
		if (cur < destination)
			cur = destination;
	}
	if (destination > cur) {
		cur = cur + step;
		if (cur >= destination)
			cur = destination;
	}
	_rwaDXAudioOutputSetGain(out, cur);
	return cur == destination;
}

// 0x44afb0
RwaWave *RWS::RwsAudio::FindWave(const char *name)
{
	RwaWave *wave = 0;
	for (unsigned i = 0; i < NUMGROUPS; ++i) {
		if (g_audioWaveDicts[i]) {
			wave = RwaWaveDictFindWaveByName(g_audioWaveDicts[i], name);
			if (wave)
				break;
		}
	}
	if (!wave)
		FUN_0043e6f0("The sound <%s> was not found in the wave dictionary.\n", name);
	return wave;
}

// 0x44b000
RwaVirtualVoice *RWS::RwsAudio::AllocateVirtualVoice()
{
	unsigned i = 0;
	do {
		if (!g_audioVirtualVoices[i].VoiceAlloc) {
			g_audioVirtualVoices[i].VoiceAlloc = 1;
			return g_audioVirtualVoices[i].pVirtualVoice;
		}
		++i;
	} while (i < MAXVVOICES);
	return 0;
}

// 0x44b030
void RWS::RwsAudio::FreeVirtualVoice(RwaVirtualVoice *voice)
{
	unsigned i = 0;
	do {
		if (g_audioVirtualVoices[i].pVirtualVoice == voice) {
			_rwaVirtualVoiceSetTrigger(g_audioVirtualVoices[i].pVirtualVoice, 0);
			g_audioVirtualVoices[i].VoiceAlloc = 0;
			return;
		}
		++i;
	} while (i < MAXVVOICES);
}

// 0x44b070
void RWS::RwsAudio::StopAllVirtualVoices()
{
	for (unsigned i = 0; i < MAXVVOICES; ++i) {
		_rwaVirtualVoiceSetTrigger(g_audioVirtualVoices[i].pVirtualVoice, 0);
		_rwaVirtualVoiceSetWave(g_audioVirtualVoices[i].pVirtualVoice, 0);
	}
	FUN_005074c0();
}

// 0x44b0b0
RwaOsOutput *RWS::RwsAudio::GetOutputObject()
{
	return g_audioOutput;
}

// 0x44b0c0
RwaListener *RWS::RwsAudio::GetListener()
{
	return g_audioListener;
}

// 0x44b0d0
RwaEnvironment *RWS::RwsAudio::GetEnvironment()
{
	return &g_audioEnvironment;
}

// 0x44b0e0
void RWS::RwsAudio::RemoveDictionary(RwaWaveDict *dict)
{
	StopAllVirtualVoices();
	for (unsigned i = 0; i < NUMGROUPS; ++i) {
		if (g_audioWaveDicts[i] == dict) {
			RwaWaveDictDestroy(g_audioWaveDicts[i]);
			g_audioWaveDicts[i] = 0;
		}
	}
}
