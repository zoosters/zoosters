#include <string.h>
#include "tfb/audio/voice.h"

// GLOBAL: 0x617fd0 g_voiceList
// GLOBAL: 0x617fc0 g_soundStreamCount
// GLOBAL: 0x62a499 g_cutSceneVoicesActive
extern int g_soundStreamCount;
extern unsigned char g_cutSceneVoicesActive;  // set while a cut scene's sounds are retimed
int *CutScene_StackSlot(int delta);             // 0x436c00 {state (-1 = none), time slot}

using namespace RWS::TFB;

// 0x419380
void FreeAllVoices()
{
	Voice *v = g_voiceList;
	while (v) {
		g_voiceList = v->m_next;
		v->Stop();
		operator delete(v);
		v = g_voiceList;
	}
}

// Updates each voice; a finished one is deleted. `limit` is the cut-scene time slot (-1: none): voices timed at or after it are retimed to it and resumed if paused; others are paused (mode 3, or looping / music sounds) or deleted.
// 0x419050
void UpdateVoices(unsigned limit)
{
	if (g_cutSceneVoicesActive) {
		int state = *CutScene_StackSlot(0);
		if (state == 2 || state == -1)
			g_cutSceneVoicesActive = 0;
	}
	int mode;
	if (limit == (unsigned)-1)
		mode = 3;
	else
		mode = *CutScene_StackSlot(0);
	Voice *v = g_voiceList;
	Voice **link = &g_voiceList;
	g_soundStreamCount = 0;
	for (; v; v = *link) {
		bool keep = false;   // still playing (or paused)
		if (v->Update()) {
			if ((unsigned)v->m_cutSceneTime >= limit && mode != -1) {
				unsigned flags = v->m_pausePos;
				v->m_cutSceneTime = limit;
				if ((unsigned char)(flags >> 31) & 1) {
					v->m_pausePos = flags & ~kVoicePaused;   // resume
					if (v->m_mix)
						v->m_mix->m_request = RWS::Audio::MIXER_REQUEST_PLAY;
				}
				keep = true;
			} else if (mode == 3 || v->m_owner->m_loop || v->m_owner->m_music) {
				v->Pause();
				keep = true;
			}
		}
		if (keep) {
			if (v->m_mix)
				g_soundStreamCount++;
			link = &v->m_next;
		} else {
			*link = v->m_next;
			v->Stop();
			operator delete(v);
		}
	}
}

// 0x419220
void StopVoicesForObject(::Script::ScriptReference *ref, int mode)
{
	StopVoicesFor(ref, 0, mode);
}

// A plain sound object deletes the voices of `ref` (0 = all); any other kind goes through StopVoicesFor.
// 0x419240
void ::Script::ScriptSoundObj::StopVoices(::Script::ScriptReference *ref)
{
	if (GetKind() == 0) {
		Voice *v = g_voiceList;
		if (v) {
			Voice **link = &g_voiceList;
			do {
				if (ref == 0 || v->m_target == ref || v->m_scriptRef == ref) {
					*link = v->m_next;
					v->Stop();
					operator delete(v);
				} else {
					link = &v->m_next;
				}
				v = *link;
			} while (v);
		}
	} else {
		StopVoicesFor(ref, GetSound(), 0);
	}
}

// Voices of (ref, owner) (0 = any): mode 0 deletes them, -1 retimes them (cut-scene time slot), else they are detached from their target, unless the sound loops (then deleted).
// 0x419180
__declspec(noinline) void StopVoicesFor(::Script::ScriptReference *ref, CTFBSound *owner, int mode)
{
	Voice **link = &g_voiceList;
	Voice *v = g_voiceList;
	while (v) {
		if ((owner == 0 || v->m_owner == owner) && (ref == 0 || v->m_target == ref || v->m_scriptRef == ref)) {
			if (mode == -1) {
				v->m_cutSceneTime = CutScene_StackSlot(0)[1];
				link = &v->m_next;
			} else if (mode != 0 && v->m_owner->m_loop == 0) {
				v->RandomizeAndSync(0);
				v->m_target = 0;
				v->m_positional = 0;
				link = &v->m_next;
			} else {
				*link = v->m_next;
				v->Stop();
				operator delete(v);
			}
		} else {
			link = &v->m_next;
		}
		v = *link;
	}
}

// Starts a voice for `ref`, except the prompt sound of the "press the circle" tutorial.
// 0x419150
void ::Script::ScriptSoundObj::Play(::Script::ScriptReference *ref)
{
	if (m_link) {
		CTFBSound *snd = GetSound();
		if (strcmp(snd->m_name, "throw07_pressthecircle") != 0)
			snd->FindOrCreateVoice(ref);
	}
}
