#include "modules/audio/audioglobalmixer.h"
#include "framework/core/ceventhandler.h"
#include "tfb/audio/voice.h"
#include "framework/toolkits/audio/audio.h"

// GLOBAL: 0x617c00 g_audioGlobalMixer
// GLOBAL: 0x617bf8 g_audioMixerFlag
// GLOBAL: 0x62a344 RWS::iMsgRunningPostTick
// GLOBAL: 0x62a518 RWS::iMsgStopSystem

namespace RWS { namespace Audio {
// GLOBAL: 0x5caf10 RWS::Audio::AudioGlobalMixer::vftable_for_CAttributeHandler
// GLOBAL: 0x5caf08 RWS::Audio::AudioGlobalMixer::vftable_for_CEventHandler
// 0x406ed0
// 0x406a20
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z
// Near-match of 0x406850: the game stores g_audioGlobalMixer before the first LinkMsg; doing so reshapes the slot zeroing (+1 B).
AudioGlobalMixer::AudioGlobalMixer(const CAttributePacket &)
	: m_Gain(30.0f), m_DopplerScale(1.0f), m_DistanceFactor(1.0f), m_RollOffFactor(1.0f), m_DebugInfo(0), m_FadeInc(0.03f), m_FadeState(0)
{
	m_unk94 = 0;
	m_unk98 = 0;
	m_unk9c = 0;
	LinkMsg(RWS::iMsgRunningPostTick);
	g_audioGlobalMixer = this;
	LinkMsg(RWS::iMsgStopSystem);
	g_audioMixerFlag = 0;
	for (unsigned i = 0; i < RwsAudio::NUMSTREAMS; ++i)
		m_slots[i].m_stream = RwsAudio::GetStream((int)i);
	// No reverb until a script sets an environment.
	RwaEnvironment reverb = { -10000, -10000, 0.0f, 1.49f, 0.83f, -2602, 0.007f, 200, 0.011f, 100.0f, 100.0f, 5000.0f };
	RwsAudio::GetOutputObject()->environment = reverb;
	RwsAudio::GetOutputObject()->dirtyFlags |= RWA_OUTPUT_ENVIRONMENT;
}

// 0x406a30
AudioGlobalMixer::~AudioGlobalMixer()
{
	FreeAllVoices();
	RwsAudio::StopAllVirtualVoices();
	g_audioGlobalMixer = 0;

	// Only linked by the constructor; registered elsewhere.
	UnLinkMsg(RWS::iMsgRunningPostTick);
	UnLinkMsg(RWS::iMsgStopSystem);

	// Unlink before unregistering (unregistering may free the registration).
	CEventId *const ownMsgs[] = { &m_RenderIn, &m_FadeUp, &m_FadeDown, &m_EndFade };
	for (CEventId *pMsg : ownMsgs) {
		UnLinkMsg(*pMsg);
		UnRegisterMsg(*pMsg);
	}
}

}}   // namespace RWS::Audio
