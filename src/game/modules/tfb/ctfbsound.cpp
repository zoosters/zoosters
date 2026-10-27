// The constructor inlines the separately allocated Script::ScriptSoundObj's, so that class's vtable (0x5cca48) and scalar deleting
// destructor are emitted here.
#include "modules/tfb/ctfbsound.h"
#include "tfb/audio/voice.h"

// GLOBAL: 0x617fd0 g_voiceList

namespace RWS { namespace TFB {
// GLOBAL: 0x5cca84 RWS::TFB::CTFBSound::vftable
// 0x4197f0
// GLOBAL: 0x5cca48 Script::ScriptSoundObj::vftable
// GLOBAL: 0x418ec0 ?GetName@ScriptSoundObj@Script@@UAEPBDXZ
// GLOBAL: 0x418ed0 ?GetType@ScriptSoundObj@Script@@UAEPAVScriptReference@2@XZ
// GLOBAL: 0x418b70 ?GetBaseType@ScriptSoundObj@Script@@UAEPAVScriptReference@2@XZ
// GLOBAL: 0x43e0a0 ?GetMembers@ScriptSoundObj@Script@@UAEPAXPAH@Z

__forceinline SoundLink::SoundLink()
{
	::Script::ScriptSoundObj *p = new ::Script::ScriptSoundObj;
	m_obj = p;
}

// Near-match of 0x418ee0 (198 B vs 201 B): cl keeps &m_obj in edi across the allocation (one extra push/pop, `lea` before the call) where
// the game computes it after,
// and reloads m_obj through eax. Not annotated; acts as the scaffold.
CTFBSound::CTFBSound(const CAttributePacket &)
	: m_name(0), m_subtitle(0), m_wave(0), m_waveCopy(0), m_resolved(0), m_volumePct(100), m_pitchPct(100), m_maxDistance(10000.0f), m_pan(0),
	  m_streamed(0), m_loop(0), m_music(0), m_is3D(0), m_doppler(0), m_positional(0), m_pitchVarPct(0), m_volumeVarPct(0), m_mouthAnim(0)
{
	m_obj->m_link = this;
	m_obj->AdjustRefCount(1);
}

// 0x4192d0
CTFBSound::~CTFBSound()
{
	Voice *v = g_voiceList;
	if (v) {
		Voice **link = &g_voiceList;
		do {
			if (v->m_owner != this) {
				link = &v->m_next;
			} else {
				*link = v->m_next;
				v->Stop();
				::operator delete(v);
			}
			v = *link;
		} while (v);
	}
	m_obj->m_link = 0;
	m_obj->AdjustRefCount(-1);
	if (m_waveCopy) {
		RwaWaveDestroy(m_waveCopy);
		m_waveCopy = 0;
	}
}
}}   // namespace RWS::TFB
