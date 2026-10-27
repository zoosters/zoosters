#include "modules/audio/audioglobalmixer.h"
#include "tfb/audio/voice.h"
#include "framework/toolkits/audio/audio.h"
#include "rw/rwaudio.h"
#include "rw/rttoolkits.h"

// GLOBAL: 0x617c00 g_audioGlobalMixer
// GLOBAL: 0x617bf8 g_audioMixerFlag
// GLOBAL: 0x617bfc g_ctlCounter
// GLOBAL: 0x617c18 g_audioNextUpdate

unsigned GetTimeMs();                              // 0x44d2c0 millisecond clock
int DebugMessageHandler(const char *fmt, ...);   // 0x4635d0 debug print

namespace RWS { namespace Audio {

// 0x4064d0
void MixerSlot::SetState(int state)
{
	m_state = state;
	if (state) {
		if (state == MIXER_SLOT_STOPPED)
			++g_ctlCounter;
	} else
		--g_ctlCounter;
}

// 0x406790
void MixerSlot::Release()
{
	int state = m_state;
	m_request = MIXER_REQUEST_STOP;
	if (state != MIXER_SLOT_STARTING && state != MIXER_SLOT_FREE) {
		if (state != MIXER_SLOT_RESERVED)
			m_state = MIXER_SLOT_STOPPING;
		else {
			SetState(MIXER_SLOT_STOPPED);
			SetState(MIXER_SLOT_FREE);
		}
	}
}

static inline int FindFreeSlot(MixerSlot *slots)
{
	for (unsigned i = 0; i < RwsAudio::NUMSTREAMS; i++) {
		if (slots[i].m_state == MIXER_SLOT_FREE)
			return i;
	}
	return -1;
}

// 0x4067d0
MixerSlot *AudioGlobalMixer::AllocSlot()
{
	AudioGlobalMixer *mixer = g_audioGlobalMixer;
	if (mixer) {
		int i = FindFreeSlot(mixer->m_slots);
		if (i >= 0) {
			mixer->m_slots[i].m_state = MIXER_SLOT_RESERVED;
			mixer->m_slots[i].m_request = MIXER_REQUEST_PLAY;
			return &mixer->m_slots[i];
		}
	}
	return 0;
}

// 0x406500
// Runs at most every 10 ms. Modes 1 and 2 are for shutdown (2 forces a slot through STARTING and STOPPED).
// Returns mode when there is no mixer.
int AudioGlobalMixer::UpdateStreams(int mode)
{
	if (!g_audioGlobalMixer)
		return mode;
	if (GetTimeMs() > g_audioNextUpdate) {
		int result = mode;
		for (unsigned i = 0; i < RwsAudio::NUMSTREAMS; i++) {
			MixerSlot *slot = &g_audioGlobalMixer->m_slots[i];
			RwaStream *stream = slot->m_stream;
			int playState = FUN_005080e0(stream);
			switch (slot->m_state) {
			case MIXER_SLOT_STARTING:
				if (result != 2 && (slot->m_request != MIXER_REQUEST_STOP || playState != RWA_STREAM_READY)) {
					result = 0;
					break;
				}
				g_audioMixerFlag = 0;
				slot->m_state = MIXER_SLOT_STOPPING;
				result = 0;
				// fall through
			case MIXER_SLOT_STOPPING:
				if (playState != RWA_STREAM_PAUSED && playState != RWA_STREAM_IDLE) {
					_rwaStreamSetPlayState(slot->m_stream, RWA_STREAM_STOP);
					result = 0;
					break;
				}
				slot->SetState(MIXER_SLOT_STOPPED);
				// fall through
			case MIXER_SLOT_STOPPED:
				if (result != 2 && playState != RWA_STREAM_PAUSED && playState != RWA_STREAM_IDLE) {
					result = 0;
					break;
				}
				slot->SetState(MIXER_SLOT_FREE);
				// fall through
			case MIXER_SLOT_FREE:
				if (result == 0 || g_audioMixerFlag || g_ctlCounter) {
					result = 0;
					break;
				}
				if (slot->m_request) {
					slot->m_request = MIXER_REQUEST_NONE;
					_rwaStreamSetMedia(slot->m_stream, 0);
					result = 0;
					break;
				}
				if (result != 2 && playState != RWA_STREAM_IDLE)
					result = 0;
				break;
			default:
				result = 0;
			}
		}
		FUN_005074c0();
		g_audioNextUpdate = GetTimeMs() + 10;
		return result;
	}
	return 0;
}

// 0x406640
void AudioGlobalMixer::StopAllStreams(bool wait)
{
	FreeAllVoices();
	RwsAudio::StopAllVirtualVoices();
	if (wait) {
		unsigned deadline = GetTimeMs() + 500;
		do {
			if (UpdateStreams(1))
				return;
			RtFSManagerSyncAllFiles();
		} while (GetTimeMs() < deadline);
		DebugMessageHandler("ABORTING STREAMS!\n");
		if (UpdateStreams(2))
			return;
		do {
			RtFSManagerSyncAllFiles();
		} while (!UpdateStreams(2));
	}
}

}}   // namespace RWS::Audio
