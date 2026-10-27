// One playing instance of a CTFBSound: a sample on a virtual voice or a stream on a mixer slot. Linked in g_voiceList.
#ifndef ZOOSTERS_TFB_AUDIO_VOICE_H
#define ZOOSTERS_TFB_AUDIO_VOICE_H

#include "modules/tfb/ctfbsound.h"
#include "modules/audio/audioglobalmixer.h"

namespace RWS { namespace TFB {

// Bit 31 of Voice::m_pausePos: paused. The low bits keep the wave position to resume at.
const unsigned kVoicePaused = 0x80000000u;

struct Voice {
	Voice *m_next;
	RwaVirtualVoice *m_slot;               // 0 for streamed sounds
	Audio::MixerSlot *m_mix;
	float m_len;                           // stream segment length in seconds
	CTFBSound *m_owner;
	unsigned char m_positional;            // follows its target (distance attenuation, pan, 3D position)
	char m_pad15[3];
	::Script::ScriptReference *m_target;   // the object the sound plays at; its model gets the mouth animation
	::Script::ScriptReference *m_scriptRef;   // script reference current at start (g_voiceDefaultRef); voices are found by either reference
	int m_cutSceneTime;                    // cut-scene time slot the voice was last timed with
	int m_pausePos;                        // bit 31 = kVoicePaused, low bits = wave position to resume at
	float m_volume;                        // owner's percent x 0.01 with the random spread, capped at 1
	float m_gain;                          // -1 forces the first update
	float m_pitch;                         // owner's percent x 0.01 with the random spread

	Voice(CTFBSound *owner, ::Script::ScriptReference *ref);
	float RandomizeAndSync(char useDelay);
	bool Update();  // false when the voice has finished
	void Pause();
	void Play();
	void Update3DPosition();
	void Stop();  // always followed by operator delete
	void StartStream();
	static float GetPlayPosition(Voice *voice);  // + one logic tick; the callback of the target's mouth animation
};
ZOOSTERS_ASSERT_SIZE(Voice, 0x34);
ZOOSTERS_ASSERT_OFFSET(Voice, m_target, 0x18);
ZOOSTERS_ASSERT_OFFSET(Voice, m_pausePos, 0x24);
ZOOSTERS_ASSERT_OFFSET(Voice, m_pitch, 0x30);

}}   // namespace RWS::TFB

// GLOBAL: 0x617fd0 g_voiceList
extern RWS::TFB::Voice *g_voiceList;
// GLOBAL: 0x617fc0 g_soundStreamCount
extern int g_soundStreamCount;  // ScriptSound.StreamCount

void FreeAllVoices();
void UpdateVoices(unsigned limit);
void StopVoicesFor(::Script::ScriptReference *ref, RWS::TFB::CTFBSound *owner, int mode);  // cdecl; 0 = any
void StopVoicesForObject(::Script::ScriptReference *ref, int mode);
void SetStreamGains(RwaStream *stream, float left, float right, char surround);

#endif
