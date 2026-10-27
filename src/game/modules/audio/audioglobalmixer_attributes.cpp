#include "modules/audio/audioglobalmixer.h"
#include "framework/toolkits/audio/audio.h"
#include "script/actor.h"

// GLOBAL: 0x60288c g_audioGlobalMixerClassId
// GLOBAL: 0x550b20 ?ReplaceLinkedMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@PBD1@Z
extern const char *g_audioGlobalMixerClassId;

namespace RWS { namespace Audio {

// 0x406b50
// Commands 7-9 also go straight into the listener; 10 ignores the value and sets 4.
void AudioGlobalMixer::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	for (CAttributeCommandIterator it(attr, g_audioGlobalMixerClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 6: {
			float gain = it->GetAs_RwReal();
			m_Gain = gain;
			_rwaDXAudioOutputSetGain(RwsAudio::GetOutputObject(), gain);
			break;
		}
		case 7:
			*(int *)&m_DopplerScale = it->GetAs_RwInt32();
			RwsAudio::GetListener()->dopplerScale = m_DopplerScale;
			RwsAudio::GetListener()->dirtyFlags |= RWA_LISTENER_DOPPLER;
			break;
		case 8:
			*(int *)&m_DistanceFactor = it->GetAs_RwInt32();
			RwsAudio::GetListener()->distanceFactor = m_DistanceFactor;
			RwsAudio::GetListener()->dirtyFlags |= RWA_LISTENER_DISTANCE;
			break;
		case 9:
			*(int *)&m_RollOffFactor = it->GetAs_RwInt32();
			RwsAudio::GetListener()->rolloffFactor = m_RollOffFactor;
			RwsAudio::GetListener()->dirtyFlags |= RWA_LISTENER_ROLLOFF;
			break;
		case 10:
			it.operator->();
			_rwaDXAudioOutputSetSpeakerConfig(RwsAudio::GetOutputObject(), 4);
			break;
		case 0:
			UnLinkMsg(m_RenderIn);
			UnRegisterMsg(m_RenderIn);
			RegisterMsg(m_RenderIn, it->GetAs_RwChar_ptr(), "RwCamera*");
			LinkMsg(m_RenderIn, "RwCamera*");
			break;
		case 11:
			m_DebugInfo = it->GetAs_RwInt32();
			break;
		case 1:
			RwsAudio::StopAllVirtualVoices();
			break;
		case 2:
			ReplaceLinkedMsg(m_FadeUp, it->GetAs_RwChar_ptr(), 0);
			break;
		case 3:
			ReplaceLinkedMsg(m_FadeDown, it->GetAs_RwChar_ptr(), 0);
			break;
		case 5:
			*(int *)&m_FadeInc = it->GetAs_RwInt32();
			break;
		case 4:
			ReplaceRegisteredMsg(m_EndFade, it->GetAs_RwChar_ptr(), 0);
			break;
		}
	}
}

}}   // namespace RWS::Audio
