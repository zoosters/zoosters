#include "modules/tfb/ctfbsound.h"
#include "tfb/audio/voice.h"

// GLOBAL: 0x617fd0 g_voiceList

// 0x418720
int Script::ScriptSoundObj::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		if (this != GetType())
			delete this;
		return 0;
	}
	return m_refCount;
}

namespace RWS { namespace TFB {

// 0x418fb0
Voice *CTFBSound::FindOrCreateVoice(::Script::ScriptReference *ref)
{
	if (m_loop || !m_multiInstance) {
		for (Voice *v = g_voiceList; v; v = v->m_next) {
			if (this == v->m_owner && (v->m_target == ref || v->m_scriptRef == ref))
				return v;
		}
	}
	return new Voice(this, ref);
}

}}   // namespace RWS::TFB
