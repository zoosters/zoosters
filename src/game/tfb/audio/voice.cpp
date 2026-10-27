#include <stdlib.h>
#include "tfb/audio/voice.h"
#include "framework/toolkits/audio/audio.h"
#include "script/actor.h"
#include "modules/tfb/gameactor.h"
#include "modules/tfb/sprites/gamesprite.h"
#include "modules/tfb/sprites/spriteextra.h"
#include "tfb/corpus/corpus.h"
#include "tfb/anim/kfset.h"

// GLOBAL: 0x617fd0 g_voiceList
// GLOBAL: 0x621f34 g_voiceDefaultRef
// GLOBAL: 0x62a2f8 g_nextLevel
// GLOBAL: 0x606a38 g_currentLevel
// GLOBAL: 0x617fc4 g_musicVolume
// GLOBAL: 0x617fc8 g_effectsVolume
// GLOBAL: 0x606c3c RWS::MainLoop::Logic::Rate
// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x5cc9ac g_randScale
extern ::Script::ScriptReference *g_voiceDefaultRef;  // the script reference current when a voice starts
extern int g_nextLevel;
extern int g_currentLevel;  // (a sound starts only while no level change is pending)
extern int g_musicVolume;  // percent
extern int g_effectsVolume;  // percent
namespace RWS { namespace MainLoop { namespace Logic { extern unsigned Rate; } } }
extern const float g_floatZero;
extern const float g_randScale;  // 1 / (RAND_MAX + 1)

int *CutScene_StackSlot(int delta);                    // 0x436c00 {state (-1 = none), time slot}
::Script::ScriptReference *TFBType_Position();         // 0x430300
::Script::ScriptReference *TFBType_Actor();            // 0x436e30
::Script::ScriptReference *TFBType_Sprite();           // 0x43e330
void ShowCaption(const char *name);                    // 0x414720
void HideCaption(const char *name);                    // 0x414750
const char *GetSoundStreamPath();                      // 0x419820 the speech stream's path (set by LevelHub)
const char *GetMusicStreamPath();                      // 0x419830 the music stream's path

// Gain of a stream's two (stereo: left, right) or four (surround: first pair left, second right) channels.
// 0x4176d0
void SetStreamGains(RwaStream *stream, float left, float right, char surround)
{
	RwaStreamGain gain;
	gain.arg0 = 0;
	gain.side = 0;
	gain.pair = 0;
	if (!surround) {
		gain.gain = left;
		_rwaStreamSetStreamGain(stream, &gain);
		gain.gain = right;
		gain.side = 1;
		_rwaStreamSetStreamGain(stream, &gain);
	} else {
		gain.gain = left;
		_rwaStreamSetStreamGain(stream, &gain);
		gain.gain = left;
		gain.side = 1;
		_rwaStreamSetStreamGain(stream, &gain);
		gain.pair = 1;
		gain.gain = right;
		gain.side = 0;
		_rwaStreamSetStreamGain(stream, &gain);
		gain.gain = right;
		gain.side = 1;
		_rwaStreamSetStreamGain(stream, &gain);
	}
}

namespace RWS { namespace TFB {

// Distance range in inches: 3D sounds attenuate over [1, max]; other positioned sounds are audible up to max only (min = max - 0.5); no range (<= 1): never attenuated.
static const float kMinDistance3D = 1.0f;
static const float kMinRangeSlack = 0.5f;
static const float kUnlimitedDistance = 120000.0f;

// Links the voice into g_voiceList and starts it unless a cut scene is being skipped or a level change is pending: a stream takes a mixer slot, a sample a virtual voice.
// 0x418b80
Voice::Voice(CTFBSound *owner, ::Script::ScriptReference *ref)
{
	m_next = 0;
	m_slot = 0;
	m_mix = 0;
	m_len = 0;
	m_owner = owner;
	m_target = ref;
	m_scriptRef = g_voiceDefaultRef;
	m_cutSceneTime = CutScene_StackSlot(0)[1];
	m_pausePos = 0;
	m_gain = -1.0f;
	m_pitch = 0;
	m_next = g_voiceList;
	g_voiceList = this;
	if (*CutScene_StackSlot(0) != -1 && g_nextLevel == g_currentLevel) {
		if (m_owner->m_subtitle)
			ShowCaption(m_owner->m_subtitle);
		if (m_owner->m_streamed && m_owner->m_name) {
			m_mix = Audio::AudioGlobalMixer::AllocSlot();
			int follows;
			if (m_owner->m_is3D && m_target && m_target->IsKindOf(TFBType_Position()))
				follows = 1;
			else
				follows = 0;
			m_positional = follows;
			return;
		}
		if (m_owner->m_wave) {
			m_slot = RwsAudio::AllocateVirtualVoice();
			if (m_slot) {
				int follows;
				if ((m_owner->m_positional || m_owner->m_doppler) && m_target && m_target->IsKindOf(TFBType_Position()))
					follows = 1;
				else
					follows = 0;
				m_positional = follows;
				_rwaVirtualVoiceSetWave(m_slot, m_owner->m_wave);
				RandomizeAndSync(1);
				int vol = m_owner->m_music ? g_musicVolume : g_effectsVolume;
				m_gain = (float)vol * m_volume * 0.01f;
				Play();
				if (m_positional == 0 && m_owner->m_is3D == 0) {
					// not positioned: the voice sits at the listener
					RwV3d p = RwsAudio::GetListener()->position;
					m_slot->position = p;
					if (m_slot->handle)
						RwaObjHandleSetParamData(m_slot->handle, RWA_VOICE_POSITION, 1, &p);
				} else {
					Update3DPosition();
				}
				CTFBSound *ow = m_owner;
				float minDist, maxDist;
				if (ow->m_is3D) {
					minDist = kMinDistance3D;
					maxDist = ow->m_maxDistance;
				} else if (ow->m_maxDistance > 1.0f) {
					minDist = ow->m_maxDistance - kMinRangeSlack;
					maxDist = ow->m_maxDistance;
				} else {
					minDist = kUnlimitedDistance;
					maxDist = kUnlimitedDistance;
				}
				float minMetres = RwsAudio::InchesToMetres(minDist);
				m_slot->minDistance = minMetres;
				if (m_slot->handle)
					FUN_00509ca0(m_slot->handle, RWA_VOICE_MIN_DISTANCE, 1, minMetres);
				maxDist = RwsAudio::InchesToMetres(maxDist);
				m_slot->maxDistance = maxDist;
				if (m_slot->handle)
					FUN_00509ca0(m_slot->handle, RWA_VOICE_MAX_DISTANCE, 1, maxDist);
			}
		}
	}
}

// Releases the stream slot or stops and frees the virtual voice, clears the mouth animation and the caption. The caller unlinks and deletes.
// 0x418e10
void Voice::Stop()
{
	RandomizeAndSync(0);
	if (m_owner->m_subtitle)
		HideCaption(m_owner->m_subtitle);
	if (m_owner->m_streamed && m_owner->m_name) {
		if (m_mix) {
			m_mix->Release();
			if (m_mix->m_state == Audio::MIXER_SLOT_STOPPING)
				SetStreamGains(m_mix->m_stream, 0, 0, m_owner->m_is3D);
			m_mix = 0;
		}
	} else if (m_slot) {
		m_slot->flags &= ~RWA_VVOICE_LOOP;
		if (m_slot->handle)
			FUN_00509ca0(m_slot->handle, RWA_VOICE_LOOP, 1, 0.0f);   // the float overload, as at the game's call site
		_rwaVirtualVoiceSetTrigger(m_slot, 0);
		RwsAudio::FreeVirtualVoice(m_slot);
	}
}

static __forceinline float ScaledX(const Vec3 *v) { Vec3 t = *v; return RwsAudio::InchesToMetres(t.x); }
static __forceinline float ScaledY(const Vec3 *v) { Vec3 t = *v; return RwsAudio::InchesToMetres(t.y); }
static __forceinline float ScaledZ(const Vec3 *v) { Vec3 t = *v; return RwsAudio::InchesToMetres(t.z); }


// Pushes the target's position (and velocity for Doppler sounds) to the virtual voice, in metres.
// 0x417d70
#define TARGET ((::Script::PositionMeasurement *)m_target)
void Voice::Update3DPosition()
{
	if (m_target && m_target->IsKindOf(TFBType_Position())) {
		RwV3d p;
		p.x = ScaledX(TARGET->GetVector(TARGET));
		p.y = ScaledY(TARGET->GetVector(TARGET));
		RwV3d *slotPos;
		p.z = (ScaledZ(TARGET->GetVector(TARGET)));
		slotPos = &m_slot->position;
		*slotPos = p;
		if (m_slot->handle)
			RwaObjHandleSetParamData(m_slot->handle, RWA_VOICE_POSITION, 1, &p);
		if (m_owner->m_doppler) {
			p.x = ScaledX(TARGET->GetVelocity(TARGET));
			p.y = ScaledY(TARGET->GetVelocity(TARGET));
			Vec3 *vel = TARGET->GetVelocity(TARGET);
			p.z = (ScaledZ(vel));
			*(&m_slot->velocity) = p;
			if (m_slot->handle)
				RwaObjHandleSetParamData(m_slot->handle, RWA_VOICE_VELOCITY, 1, &p);
		}
	}
}
#undef TARGET

// Opens the stream file on the mixer slot, selects the sound's segment by name and starts it paused; Voice::Update plays it once loaded.
// 0x417810
void Voice::StartStream()
{
	RwaStreamMedia media;
	media.mediaType = 1;
	media.arg3 = 0;
	media.arg4 = 0;
	media.arg2 = 0;
	media.arg5 = 0;
	if (m_owner->m_music == 0) {
		media.path = GetSoundStreamPath();
		media.arg6 = 2;
		media.arg7 = 2;
	} else {
		media.path = GetMusicStreamPath();
		media.arg6 = 8;
		media.arg7 = 4;
	}
	media.loop = m_owner->m_loop != 0;
	_rwaStreamSetMedia(m_mix->m_stream, &media);
	RwaStreamItem item;
	item.type = 1;
	item.name = m_owner->m_name;
	RwaStreamItems items;
	item.arg3 = 1;
	item.start = 0.0f;
	items.count = 1;
	items.items = &item;
	FUN_00507de0(m_mix->m_stream, &items);
	_rwaStreamSetPlayState(m_mix->m_stream, RWA_STREAM_PAUSED);
}

static __forceinline float WaveDuration(RwaVirtualVoice *v)
{
	return (float)RwaWaveFormatGetSamples(&v->wave->format, v->wave->format.length) / v->frequency;
}

// Position in seconds plus one logic tick; the mouth animation calls it back with the voice.
// NEARMISS: 0x417a70 RWS::TFB::Voice::GetPlayPosition (DIFF, 239 B vs 244 B)
// 0x417a70
float Voice::GetPlayPosition(Voice *voice)
{
	Voice *a = voice;
	Audio::MixerSlot *st = a->m_mix;
	if (st) {
		if (st->m_state != Audio::MIXER_SLOT_PLAYING && st->m_state != Audio::MIXER_SLOT_PLAYING_PENDING &&
		    st->m_state != Audio::MIXER_SLOT_READY)
			return g_floatZero;
		RwaStreamPosQuery q;
		q.arg3 = 1;
		q.arg1 = 3;
		q.arg0 = -1;
		_rwaStreamGetStreamPos(st->m_stream, &q);
		return q.seconds + 1.0f / (float)RWS::MainLoop::Logic::Rate;
	}
	RwaVirtualVoice *v = a->m_slot;
	if (v) {
		unsigned f = v->flags;
		int pos;
		if (!(f & RWA_VVOICE_PLAYING)) {
			pos = RwaWaveFormatGetSamples(&v->wave->format, v->wave->format.length);
		} else {
			if (v->handle && (f & RWA_VVOICE_PLAYING))
				pos = FUN_00509cf0(v->handle, RWA_VOICE_WAVE_POSITION, 2, 0);
			else
				pos = v->wavePos;
			v->wavePos = pos;
		}
		return (float)pos / v->frequency + 1.0f / (float)RWS::MainLoop::Logic::Rate;
	}
	return g_floatZero;
}

// 0x417960
void Voice::Play()
{
	m_slot->pan = m_owner->m_pan;
	if (m_slot->handle)
		FUN_00509ca0(m_slot->handle, RWA_VOICE_PAN, 1, m_owner->m_pan);
	if (m_owner->m_loop)
		m_slot->flags |= RWA_VVOICE_LOOP;
	else
		m_slot->flags &= ~RWA_VVOICE_LOOP;
	if (m_slot->handle)
		FUN_00509ca0(m_slot->handle, RWA_VOICE_LOOP, 1, (int)m_owner->m_loop);
	_rwaVirtualVoiceSetTrigger(m_slot, 1);
	RwaWaveFormat hdr = m_slot->wave->targetFormat;
	float rate = (float)hdr.sampleRate;
	m_slot->frequency = rate * m_pitch;
	if (m_slot->handle)
		FUN_00509ca0(m_slot->handle, RWA_VOICE_FREQUENCY, 1, rate * m_pitch);
	m_slot->gain = m_gain;
	if (m_slot->handle)
		FUN_00509ca0(m_slot->handle, RWA_VOICE_GAIN, 1, m_gain);
}

// Stream: asks its mixer slot to pause. Sample: remembers the wave position in m_pausePos (at least 1) and is triggered off. Sets kVoicePaused.
// 0x4178e0
void Voice::Pause()
{
	unsigned state = m_pausePos;
	if ((unsigned char)(state >> 31) & 1)
		return;
	Audio::MixerSlot *mix = m_mix;
	if (mix) {
		mix->m_request = Audio::MIXER_REQUEST_PAUSE;
		m_pausePos |= kVoicePaused;
		return;
	}
	RwaVirtualVoice *voice = m_slot;
	if (voice) {
		if (state == 0) {
			int pos;
			if (voice->handle && (voice->flags & RWA_VVOICE_PLAYING))
				pos = FUN_00509cf0(voice->handle, RWA_VOICE_WAVE_POSITION, 2, 0);
			else
				pos = voice->wavePos;
			voice->wavePos = pos;
			m_pausePos = pos;
			if (pos == 0)
				m_pausePos = 1;
		}
		_rwaVirtualVoiceSetTrigger(voice, 0);
	}
	m_pausePos |= kVoicePaused;
}

// With useDelay: volume and pitch from the sound's percentages with a random spread (volume capped at 1). Then hands the mouth animation (or 0) and the duration to the target's model state, with GetPlayPosition as the clock. Returns the pitch.
// NEARMISS: 0x417b70 RWS::TFB::Voice::RandomizeAndSync (DIFF; the residual is the call tail)
// 0x417b70
float Voice::RandomizeAndSync(char useDelay)
{
	if (useDelay) {
		m_pitch = (float)m_owner->m_pitchPct * 0.01f;
		unsigned spread = m_owner->m_pitchVarPct;
		if (spread > 0) {
			float pct = (float)spread * 0.01f;
			float amp = pct * m_pitch;
			float r = (float)rand() * g_randScale;
			m_pitch = (1.0f - (r + r)) * amp + m_pitch;
		}
		m_volume = (float)m_owner->m_volumePct * 0.01f;
		spread = m_owner->m_volumeVarPct;
		if (spread > 0) {
			float pct = (float)spread * 0.01f;
			float amp = pct * m_volume;
			m_volume = m_volume - (float)rand() * g_randScale * amp;
		}
		if (m_volume > 1.0f)
			m_volume = 1.0f;
	}
	if (m_target && m_owner->m_mouthAnim) {
		// an actor's body or a sprite's model; an Actor without a Corpus falls through to the sprite test, as in the game
		TFBModelState *ms = 0;
		if (m_target->GetType() == TFBType_Actor() && static_cast<GameActor *>(m_target)->m_corpus)
			ms = &static_cast<GameActor *>(m_target)->m_corpus->m_model;
		else if (m_target->GetType() == TFBType_Sprite() && static_cast<GameSprite *>(m_target)->m_extra)
			ms = static_cast<GameSprite *>(m_target)->m_extra;
		if (ms) {
			RwaVirtualVoice *v = m_slot;
			float t = v ? WaveDuration(v) : m_len;
			::TFB::KFSet *d = useDelay ? m_owner->m_mouthAnim : 0;
			ms->PlayMouthAnim(d, reinterpret_cast<float (*)(void *)>(GetPlayPosition), this, t);
		}
	}
	return m_pitch;
}

}}   // namespace RWS::TFB
