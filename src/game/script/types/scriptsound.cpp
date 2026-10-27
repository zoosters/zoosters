// Script::ScriptSound: the Expose* members of the audio settings (stream count, effects / music volume, environment type and level) and
// of one playing sound (volume, pitch, percent complete, time remaining) (Game.exe 0x418600-0x418a40, name 0x418ec0). The audio settings
// work on game globals; a sound's members take its script object (ScriptSoundObj) as the `owner` and look for its voice in g_voiceList
// (the one playing for the running script, g_scriptSelf).
#include "script/membertable.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "framework/startup/win32/cfgobj.h"
#include "framework/startup/win32/appstate.h"
#include "modules/tfb/ctfbsound.h"
#include "tfb/audio/voice.h"
#include "tfb/audio/audioenv.h"
#include "script/types/scriptsound.h"
#include "script/bearings.h"
#include "zoosters/memberrow.h"
#include "script/ops_media.h"

// GLOBAL: 0x617fc0 g_soundStreamCount
// GLOBAL: 0x617fc8 g_effectsVolume
// GLOBAL: 0x617fc4 g_musicVolume
// GLOBAL: 0x62a5a8 g_audioDevice
// GLOBAL: 0x519c90 ?SetOption@CfgObj@@QAEXPBDH@Z
// GLOBAL: 0x617fcc g_envType
// GLOBAL: 0x602a08 g_envLevel
// GLOBAL: 0x617fd0 g_voiceList
// GLOBAL: 0x621f14 g_scriptSelf
// GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatZero;   // 0x5ca5a8

namespace Script {

// 0x418ec0
const char *ScriptSound::GetName()
{
	return "sound";
}

// 0x418600
int ScriptSound::ExposeStreamCount::GetInt(ScriptReference *owner)
{
	return g_soundStreamCount;
}

// 0x418610
int ScriptSound::ExposeEffectsVolume::GetInt(ScriptReference *owner)
{
	return g_effectsVolume;
}

// 0x418620
void ScriptSound::ExposeEffectsVolume::SetValue(int value, ScriptReference *owner)
{
	g_effectsVolume = value;
	if (g_audioDevice)
		g_audioDevice->SetOption("Settings\\Options\\Effect", value);
}

// 0x418650
int ScriptSound::ExposeMusicVolume::GetInt(ScriptReference *owner)
{
	return g_musicVolume;
}

// 0x418660
void ScriptSound::ExposeMusicVolume::SetValue(int value, ScriptReference *owner)
{
	g_musicVolume = value;
	if (g_audioDevice)
		g_audioDevice->SetOption("Settings\\Options\\Music", value);
}

// 0x418690
int ScriptSound::ExposeEnvironmentLevel::GetInt(ScriptReference *owner)
{
	return (int)(g_envLevel * 100.0f);
}

// 0x4186b0
void ScriptSound::ExposeEnvironmentLevel::SetValue(int value, ScriptReference *owner)
{
	if (value > 100)
		value = 100;
	g_envLevel = (float)value * 0.01f;
	SetEnvironment(g_envType, g_envLevel);
}

// 0x4186f0
int ScriptSound::ExposeEnvironmentType::GetInt(ScriptReference *owner)
{
	return g_envType;
}

// 0x418700
void ScriptSound::ExposeEnvironmentType::SetValue(int value, ScriptReference *owner)
{
	g_envType = value;
	SetEnvironment(value, g_envLevel);
}

// 0x4189b0
int ScriptSound::ExposeVolume::GetInt(ScriptReference *owner)
{
	RWS::TFB::CTFBSound *s = static_cast<ScriptSoundObj *>(owner)->GetSound();
	return s->m_volumePct;
}

// 0x4189d0
void ScriptSound::ExposeVolume::SetValue(int value, ScriptReference *owner)
{
	RWS::TFB::CTFBSound *s = static_cast<ScriptSoundObj *>(owner)->GetSound();
	s->m_volumePct = value;
	for (RWS::TFB::Voice *v = g_voiceList; v; v = v->m_next) {
		if (s == v->m_owner && (v->m_target == g_scriptSelf || v->m_scriptRef == g_scriptSelf)) {
			v->m_volume = (float)value * 0.01f;
			return;
		}
	}
}

// 0x418a20
int ScriptSound::ExposePitch::GetInt(ScriptReference *owner)
{
	RWS::TFB::CTFBSound *s = static_cast<ScriptSoundObj *>(owner)->GetSound();
	return s->m_pitchPct;
}

// 0x418750
int ScriptSound::ExposePercentComplete::GetInt(ScriptReference *owner)
{
	RWS::TFB::CTFBSound *s = static_cast<ScriptSoundObj *>(owner)->GetSound();
	for (RWS::TFB::Voice *v = g_voiceList; v; v = v->m_next) {
		if (s == v->m_owner && (v->m_target == g_scriptSelf || v->m_scriptRef == g_scriptSelf)) {
			if (v->m_mix) {
				if (v->m_mix->m_state == RWS::Audio::MIXER_SLOT_PLAYING || v->m_mix->m_state == RWS::Audio::MIXER_SLOT_PLAYING_PENDING ||
				    v->m_mix->m_state == RWS::Audio::MIXER_SLOT_READY) {
					RwaStreamPosQuery q;
					q.arg3 = 1;
					q.arg1 = 3;
					q.arg0 = -1;
					_rwaStreamGetStreamPos(v->m_mix->m_stream, &q);
					double pct = q.seconds * 100.0f;
					return (int)(pct / v->m_len);
				}
				return 0;
			}
			RwaVirtualVoice *vs = v->m_slot;
			if (vs && (vs->flags & RWA_VVOICE_PLAYING)) {
				int total = RwaWaveFormatGetSamples(&vs->wave->format, vs->wave->format.length);
				int cur;
				if (vs->handle && (vs->flags & RWA_VVOICE_PLAYING))
					cur = FUN_00509cf0(vs->handle, RWA_VOICE_WAVE_POSITION, 2, 0);
				else
					cur = vs->wavePos;
				vs->wavePos = cur;
				if (cur >= total)
					return 100;
				return cur * 100 / total;
			}
			return 100;
		}
	}
	return 100;
}

// 0x418860
float ScriptSound::ExposeTimeRemaining::GetFloat(ScriptReference *owner)
{
	RWS::TFB::CTFBSound *s = static_cast<ScriptSoundObj *>(owner)->GetSound();
	for (RWS::TFB::Voice *v = g_voiceList; v; v = v->m_next) {
		if (s == v->m_owner && (v->m_target == g_scriptSelf || v->m_scriptRef == g_scriptSelf)) {
			if (v->m_mix) {
				if (v->m_mix->m_state == RWS::Audio::MIXER_SLOT_PLAYING || v->m_mix->m_state == RWS::Audio::MIXER_SLOT_PLAYING_PENDING ||
				    v->m_mix->m_state == RWS::Audio::MIXER_SLOT_READY) {
					RwaStreamPosQuery q;
					q.arg3 = 1;
					q.arg1 = 3;
					q.arg0 = -1;
					_rwaStreamGetStreamPos(v->m_mix->m_stream, &q);
					float rem = v->m_len - q.seconds;
					if (rem <= 0.01f)
						return 0.01f;
					return rem;
				}
				return 99999.0f;
			}
			RwaVirtualVoice *vs = v->m_slot;
			if (vs && (vs->flags & RWA_VVOICE_PLAYING)) {
				int total = RwaWaveFormatGetSamples(&vs->wave->format, vs->wave->format.length);
				int cur;
				if (vs->handle && (vs->flags & RWA_VVOICE_PLAYING))
					cur = FUN_00509cf0(vs->handle, RWA_VOICE_WAVE_POSITION, 2, 0);
				else
					cur = vs->wavePos;
				vs->wavePos = cur;
				if (cur >= total)
					return g_floatZero;
				float rem = (float)(total - cur) / vs->frequency;
				if (rem <= 0.01f)
					return 0.01f;
				return rem;
			}
			return g_floatZero;
		}
	}
	return g_floatZero;
}

// 0x418a40
void ScriptSound::ExposePitch::SetValue(int value, ScriptReference *owner)
{
	RWS::TFB::CTFBSound *s = static_cast<ScriptSoundObj *>(owner)->GetSound();
	s->m_pitchPct = value;
	for (RWS::TFB::Voice *v = g_voiceList; v; v = v->m_next) {
		if (s == v->m_owner && (v->m_target == g_scriptSelf || v->m_scriptRef == g_scriptSelf)) {
			float pitch = (float)value * 0.01f;
			if (pitch < 0.01f)
				pitch = 0.01f;
			v->m_pitch = pitch;
			if (v->m_mix) {
				_rwaStreamSetPitch(v->m_mix->m_stream, pitch);
			} else if (v->m_slot) {
				RwaVirtualVoice *vs = v->m_slot;
				RwaWaveFormat fmt = vs->wave->targetFormat;
				float rate = (float)fmt.sampleRate * pitch;
				if (rate < 100.0f)
					rate = 100.0f;
				else if (rate > 100000.0f)
					rate = 100000.0f;
				vs->frequency = rate;
				if (vs->handle)
					FUN_00509ca0(vs->handle, RWA_VOICE_FREQUENCY, 1, rate);
			}
			return;
		}
	}
}


}   // namespace Script

namespace Script {
// GLOBAL: 0x5d3470 Script::g_vt_ScriptSound_ExposePercentComplete
extern char g_vt_ScriptSound_ExposePercentComplete[];
// GLOBAL: 0x5d3428 Script::g_vt_ScriptSound_ExposePitch
extern char g_vt_ScriptSound_ExposePitch[];
// GLOBAL: 0x5d34b8 Script::g_vt_ScriptSound_ExposeTimeRemaining
extern char g_vt_ScriptSound_ExposeTimeRemaining[];
// GLOBAL: 0x5d33d8 Script::g_vt_ScriptSound_ExposeVolume
extern char g_vt_ScriptSound_ExposeVolume[];

// 0x43e0a0
void *ScriptSoundObj::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptSound::ExposeVolume, ScriptSound::ExposePitch, ScriptSound::ExposePercentComplete, ScriptSound::ExposeTimeRemaining> row;
	*count = row.count;
	return row.data();
}

// The game has one function for each of these bodies (0x418ec0 / 0x418ed0 / 0x418b70, shared by ScriptSound and ScriptSoundObj);
// the ScriptSoundObj virtuals are defined here.

const char *ScriptSoundObj::GetName()
{
	return "sound";
}

ScriptReference *ScriptSoundObj::GetType()
{
	return TFBType_Sound();
}

ScriptReference *ScriptSoundObj::GetBaseType()
{
	return TFBType_Media();
}

}  // namespace Script

namespace Script {

// 0x43dbe0
const char *ScriptSound::ExposeVolume::GetName()
{
	return "volume";
}

// 0x43dc90
const char *ScriptSound::ExposeStreamCount::GetName()
{
	return "active stream count";
}

// 0x43dd00
const char *ScriptSound::ExposeEffectsVolume::GetName()
{
	return "effects volume";
}

}  // namespace Script


namespace Script {

// 0x43dd70
const char *ScriptSound::ExposeMusicVolume::GetName()
{
	return "music volume";
}

// 0x43ddd0
const char *ScriptSound::ExposeEnvironmentType::GetName()
{
	return "environment reverb type";
}

// 0x43de30
const char *ScriptSound::ExposeEnvironmentLevel::GetName()
{
	return "environment reverb level";
}

}  // namespace Script

// 0x43dbf0
const char *ExposePitch_GetName()
{
	return "pitch";
}
