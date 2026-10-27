#ifndef ZOOSTERS_MODULES_AUDIO_AUDIOGLOBALMIXER_H
#define ZOOSTERS_MODULES_AUDIO_AUDIOGLOBALMIXER_H

#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "rw/rwaudio.h"

namespace RWS { namespace Audio {

enum MixerSlotState {
	MIXER_SLOT_FREE = 0,
	MIXER_SLOT_STARTING = 2,          // waiting for the stream to report READY
	MIXER_SLOT_STOPPED = 3,           // stopped, waiting to be freed (counted in g_ctlCounter)
	MIXER_SLOT_READY = 5,
	MIXER_SLOT_PLAYING = 6,
	MIXER_SLOT_PLAYING_PENDING = 7,   // play requested while the stream still reported PAUSED
	MIXER_SLOT_STOPPING = 8,
	MIXER_SLOT_RESERVED = 9
};

enum MixerSlotRequest {
	MIXER_REQUEST_NONE = 0,
	MIXER_REQUEST_STOP = RWA_STREAM_STOP,
	MIXER_REQUEST_PLAY = RWA_STREAM_PLAY,
	MIXER_REQUEST_PAUSE = RWA_STREAM_PAUSED
};

struct MixerSlot {
	MixerSlot() : m_state(0), m_request(0) {}
	int m_state;
	int m_request;                     // a nonzero request on a free slot clears the stream's media
	RwaStream *m_stream;
	void SetState(int state);
	void Release();
};
ZOOSTERS_ASSERT_SIZE(MixerSlot, 0xc);

// PS2 base order: AE
class AudioGlobalMixer : public CSystemCommands, public CAttributeHandler, public CEventHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(AudioGlobalMixer);
	AudioGlobalMixer(const CAttributePacket &attr);
	~AudioGlobalMixer();
	virtual void HandleEvents(CMsg &msg);
	virtual void HandleAttributes(const CAttributePacket &attr);
	void UpdateListener(CMsg &msg);
	static MixerSlot *AllocSlot();
	// Mode 0 is the normal tick, 1 / 2 are shutting down. Returns nonzero when every slot is free and idle.
	static int UpdateStreams(int mode);
	// Waits up to 500 ms for the streams to stop ("ABORTING STREAMS!"), then until done.
	static void StopAllStreams(bool wait);
protected:
	float m_Gain;
	float m_DopplerScale, m_DistanceFactor, m_RollOffFactor;
	CEventId m_RenderIn;
	int m_DebugInfo;
	float m_FadeInc;
	CEventId m_FadeDown;
	CEventId m_FadeUp;
	int m_FadeState;                                       // 0 stopped, 1 down, 2 up
	CEventId m_EndFade;
	MixerSlot m_slots[4];
	int m_unk94, m_unk98, m_unk9c;

	// Inside the class: the members are protected.
	static void CheckLayout() { ZOOSTERS_ASSERT_OFFSET(AudioGlobalMixer, m_slots, 0x64); }
};
ZOOSTERS_ASSERT_SIZE(AudioGlobalMixer, 0xa0);

}}   // namespace RWS::Audio

extern RWS::Audio::AudioGlobalMixer *g_audioGlobalMixer;
extern unsigned char g_audioMixerFlag;   // set while a stream is being started (CtlIsBusy waits on it)
extern int g_ctlCounter;   // number of slots in STOPPED: nonzero = streams busy
extern unsigned g_audioNextUpdate;
int CtlIsBusy();

#endif
