#include <math.h>
#include <string.h>
#include "tfb/audio/voice.h"
#include "framework/toolkits/audio/audio.h"
#include "script/actor.h"
#include "rw/rwcore.h"

// GLOBAL: 0x617bf8 g_audioMixerFlag
// GLOBAL: 0x617fc4 g_musicVolume
// GLOBAL: 0x617fc8 g_effectsVolume
// GLOBAL: 0x62a499 g_cutSceneVoicesActive
extern int g_musicVolume, g_effectsVolume;
extern unsigned char g_cutSceneVoicesActive;                // set while a cut scene plays

#define TARGET ((::Script::PositionMeasurement *)m_target)

// 0x417f20
bool RWS::TFB::Voice::Update()
{
	StreamLoadB sl;
	RwMatrix mat;
	RwMatrix inv;
	if (m_owner->m_streamed) {
		if (!m_mix || m_mix->m_request == 1)
			return false;
		int streamState = FUN_005080e0(m_mix->m_stream);
		switch (m_mix->m_state) {
		case Audio::MIXER_SLOT_RESERVED:
			if (!(char)CtlIsBusy()) {   // the game tests only al
				g_audioMixerFlag = 1;
				m_mix->SetState(Audio::MIXER_SLOT_STARTING);
				StartStream();
				return true;
			}
			return true;
		case Audio::MIXER_SLOT_STARTING: {
			if (streamState != RWA_STREAM_READY)
				return true;
			g_audioMixerFlag = 0;
			sl.name = m_owner->m_name;
			sl.type = 1;
			sl.unit = 1;
			sl.arg4 = 0;
			FUN_00508200(m_mix->m_stream, &sl);
			m_len = sl.length;
			_rwaStreamSetLoopSegment(m_mix->m_stream, m_owner->m_loop);
			RwaStream *stream = m_mix->m_stream;
			_rwaStreamSetPitch(stream, RandomizeAndSync(1));
			m_mix->SetState(streamState);
		}
		case Audio::MIXER_SLOT_READY:
			if (m_mix->m_request != Audio::MIXER_REQUEST_PLAY)
				return true;
			m_mix->SetState((streamState == RWA_STREAM_PAUSED) + (int)Audio::MIXER_SLOT_PLAYING);   // PLAYING_PENDING while the stream still reports PAUSED
			_rwaStreamSetPlayState(m_mix->m_stream, m_mix->m_request);
		case Audio::MIXER_SLOT_PLAYING:
		case Audio::MIXER_SLOT_PLAYING_PENDING: {
			if (streamState == RWA_STREAM_PAUSED || (streamState == RWA_STREAM_FINISHED && m_owner->m_loop == 0)) {
				if (m_mix->m_state == Audio::MIXER_SLOT_PLAYING)
					return false;
			} else {
				m_mix->SetState(Audio::MIXER_SLOT_PLAYING);
			}
			float vol = m_volume;
			if (m_mix->m_request == Audio::MIXER_REQUEST_PAUSE) {
				_rwaStreamSetPlayState(m_mix->m_stream, RWA_STREAM_PAUSED);
				m_mix->SetState(Audio::MIXER_SLOT_READY);
				vol = 0.0f;
			}
			if (m_positional) {
				// distance to the target in inches: the listener is in metres
				RwaListener *lis = RwsAudio::GetListener();
				RwV3d lisPos = lis->position;
				lisPos.x = RwsAudio::MetresToInches(lisPos.x);
				lisPos.y = RwsAudio::MetresToInches(lisPos.y);
				lisPos.z = RwsAudio::MetresToInches(lisPos.z);
				Vec3 emitPos = *TARGET->GetVector(TARGET);
				RwV3d delta;
				delta.x = emitPos.x - lisPos.x;
				delta.y = emitPos.y - lisPos.y;
				delta.z = emitPos.z - lisPos.z;
				float range = m_owner->m_maxDistance - RwV3dLength(&delta);
				float atten;
				if (range < 0.0f)
					atten = 0.0f;
				else
					atten = (range * range) / (m_owner->m_maxDistance * m_owner->m_maxDistance);
				vol = atten * vol;
			}
			int volSetting;
			if (m_owner->m_music)
				volSetting = g_musicVolume;
			else
				volSetting = g_effectsVolume;
			vol = (float)volSetting * vol * 0.01f;
			if (vol == m_gain && (!m_owner->m_positional || !m_target))
				return true;
			m_gain = vol;
			float pan;
			if (!m_owner->m_positional) {
				pan = (m_owner->m_pan + 1.0f) * 0.5f;
			} else if (!m_target) {
				pan = 0.5f;
			} else {
				// the target's position in the listener's space gives the pan angle
				mat.right.x = 1.0f; mat.right.y = 0.0f; mat.right.z = 0.0f;
				mat.up.x = 0.0f; mat.up.y = 1.0f; mat.up.z = 0.0f;
				mat.at.x = 0.0f; mat.at.y = 0.0f; mat.at.z = 1.0f;
				mat.pos.x = 0.0f; mat.pos.y = 0.0f; mat.pos.z = 0.0f;
				mat.flags |= 0x20003;
				mat.up = RwsAudio::GetListener()->up;
				mat.at = RwsAudio::GetListener()->at;
				mat.right = RwsAudio::GetListener()->right;
				RwMatrixUpdate(&mat);
				RwMatrixInvert(&inv, &mat);
				Vec3 *emitPos = TARGET->GetVector(TARGET);
				RwV3d src = *(RwV3d *)emitPos;
				RwV3d rel;
				RwV3dTransformPoint(&rel, &src, &inv);
				float angle = 0.0f;
				if (!(rel.z < 0.001f && rel.z > -0.001f)) {
					angle = (float)atan2(rel.x, rel.z) * 57.295776f;
					if (angle < 0.0f)
						angle = -angle;
				}
				pan = angle * 0.0055555557f;
			}
			float right = pan * vol * 2.0f;
			float left = vol * 2.0f - right;
			if (right < 0.0f)
				right = 0.0f;
			else if (right > 1.0f)
				right = 1.0f;
			if (left < 0.0f)
				left = 0.0f;
			else if (left > 1.0f)
				left = 1.0f;
			SetStreamGains(m_mix->m_stream, left, right, m_owner->m_is3D);
			return true;
		}
		default:
			return true;
		}
	}
	if (!m_slot)
		return false;
	if (((unsigned)m_pausePos >> 31) & 1)
		return true;
	if (m_pausePos != 0) {
		m_slot->wavePos = m_pausePos;
		if (m_slot->handle)
			FUN_00509ca0(m_slot->handle, RWA_VOICE_WAVE_POSITION, 1, (int)m_pausePos);
		_rwaVirtualVoiceSetTrigger(m_slot, 1);
		m_pausePos = 0;
		RwaWaveFormat fmt = m_slot->wave->targetFormat;
		float rate = (float)fmt.sampleRate;
		m_slot->frequency = rate * m_pitch;
		if (m_slot->handle)
			FUN_00509ca0(m_slot->handle, RWA_VOICE_FREQUENCY, 1, rate * m_pitch);
		m_gain = 0.0f;
	}
	if (m_slot->flags & RWA_VVOICE_PLAYING) {
		if (m_positional)
			Update3DPosition();
		int volSetting;
		if (m_owner->m_music)
			volSetting = g_musicVolume;
		else
			volSetting = g_effectsVolume;
		float vol = (float)volSetting * m_volume * 0.01f;
		if (vol != m_gain) {
			m_gain = vol;
			m_slot->gain = vol;
			if (m_slot->handle)
				FUN_00509ca0(m_slot->handle, RWA_VOICE_GAIN, 1, *(int *)&vol);   // the float's bits as an int: the game's call (int overload)
		}
		// the bee's buzz is halved during cut scenes (hard-coded on the sound's name)
		if (g_cutSceneVoicesActive && strcmp(m_owner->m_name, "bee buzz") == 0) {
			m_slot->gain = 0.5f;
			if (m_slot->handle)
				FUN_00509ca0(m_slot->handle, RWA_VOICE_GAIN, 1, 0x3F000000);
		}
		return true;
	}
	if (m_owner->m_loop) {
		Play();
		return true;
	}
	return false;
}
#undef TARGET
