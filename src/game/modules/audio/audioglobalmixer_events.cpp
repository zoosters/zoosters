#include "modules/audio/audioglobalmixer.h"
#include "framework/core/ceventhandler.h"
#include "tfb/audio/voice.h"
#include "framework/toolkits/audio/audio.h"
#include "rw/rwcore.h"
#include "modules/tfb/camera_views.h"
#include "rw/rttoolkits.h"

// GLOBAL: 0x62a344 RWS::iMsgRunningPostTick
// GLOBAL: 0x62a518 RWS::iMsgStopSystem
int *CutScene_StackSlot(int delta);           // 0x436c00
namespace RWS { int SendMsgId(CEventId *id); }   // 0x406470 (SendMsg(CMsg(id)), out of line)

namespace RWS { namespace Audio {

// 0x4066b0
// The render event's data is the camera: its world matrix (inches -> metres) becomes the listener pose.
void AudioGlobalMixer::UpdateListener(CMsg &msg)
{
	RwCamera *camera = (RwCamera *)msg.pData;
	if (camera) {
		RwMatrix *ltm = RwFrameGetLTM(CameraView(camera)->frame);
		RwV3d pos;
		pos.x = RwsAudio::InchesToMetres(ltm->pos.x);
		pos.y = RwsAudio::InchesToMetres(ltm->pos.y);
		pos.z = RwsAudio::InchesToMetres(ltm->pos.z);
		RwsAudio::GetListener()->position = pos;
		RwsAudio::GetListener()->dirtyFlags |= RWA_LISTENER_POSITION;
		RwsAudio::GetListener()->up = ltm->up;
		RwsAudio::GetListener()->at = ltm->at;
		RwsAudio::GetListener()->right = ltm->right;
		RwsAudio::GetListener()->dirtyFlags |= RWA_LISTENER_ORIENTATION;
	}
}

// 0x406da0
void AudioGlobalMixer::HandleEvents(CMsg &cmsg)
{
	if (cmsg.Id == RWS::iMsgRunningPostTick.pEventId) {
		UpdateVoices(CutScene_StackSlot(0)[1]);
		switch (m_FadeState) {
		case 1:
			if (RwsAudio::FadeOutputObject(0.0f, m_FadeInc)) {
				m_FadeState = 0;
				SendMsgId(&m_EndFade);
			}
			break;
		case 2:
			if (RwsAudio::FadeOutputObject(m_Gain, m_FadeInc)) {
				m_FadeState = 0;
				SendMsgId(&m_EndFade);
			}
			break;
		}
		UpdateStreams(0);
		RtFSManagerSyncAllFiles();
	} else if (cmsg.Id == m_RenderIn.pEventId) {
		UpdateListener(cmsg);
	} else if (cmsg.Id == m_FadeDown.pEventId) {
		m_FadeState = 1;
	} else if (cmsg.Id == m_FadeUp.pEventId) {
		m_FadeState = 2;
	} else if (cmsg.Id == RWS::iMsgStopSystem.pEventId) {
		StopAllStreams(1);
	}
}

}}   // namespace RWS::Audio
