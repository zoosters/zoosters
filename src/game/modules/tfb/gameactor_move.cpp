#define ZOOSTERS_RWS_SENDMSG 1
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "script/types/scriptsystem.h"

float Math_EaseInOut(float t, float easeIn, float easeOut);

namespace RWS { namespace TFB {

// Per frame for every player actor (ScriptActors_Update). A timed move eases the actor between m_timedMoveFrom and
// m_destination; when time is up it stops, velocity cleared, Corpus re-enabled. The Corpus contact point follows the last
// frame's position and the actor counts as moved (m_scriptClockHold). Returns true for the frame after a move; the script
// clock stamps in m_script (m_startTime, m_0c) shift by the time the actor stood still.
// Near-match of 0x429a80: the push of the constant 0 is hoisted above the branch; the three-word copy of the contact point schedules
// ld,st,ld,st,ld,st where the game has ld,ld,st,ld,st,st.
bool GameActor::UpdateTimedMove()
{
	if (m_timedMoveMs != 0) {
		unsigned now = *Script::ScriptSystem::GetScriptTimePtr();
		unsigned dur = m_timedMoveMs;
		unsigned start = m_timedMoveStart;
		m_scriptClockHold = 1;
		if (now >= start + dur) {
			m_timedMoveMs = 0;
			m_velocity.z = 0;
			m_velocity.y = 0;
			m_velocity.x = 0;
			ClearLinks();
			PlaceAt(&m_destination);
			if (m_corpus) {
				m_corpus->m_velocity.z = 0;
				m_corpus->m_velocity.y = 0;
				m_corpus->m_velocity.x = 0;
				m_corpus->SetEnabled(1);
				m_corpus->m_flags &= ~CORPUS_STEP_DONE;
			}
			if (m_corpus)
				UpdateBounds(&m_corpus->m_frame->modelling, 0.0f);
			else
				UpdateBounds(&m_matrix, 0.0f);
		} else {
			float t = Math_EaseInOut((float)(now - start) / (float)dur, 0.15f, 0.15f);
			RwV3d d;
			d.x = m_destination.x - m_timedMoveFrom.x;
			d.y = m_destination.y - m_timedMoveFrom.y;
			d.z = m_destination.z - m_timedMoveFrom.z;
			RwV3d p;
			p.x = d.x * t + m_timedMoveFrom.x;
			p.y = d.y * t + m_timedMoveFrom.y;
			p.z = d.z * t + m_timedMoveFrom.z;
			PlaceAt((const Vec3 *)&p);
		}
		Corpus *c = m_corpus;
		if (c) {
			// (the game copies the three words one by one)
			const int *last = (const int *)&c->m_lastFrame.pos;
			int *contact = (int *)&c->m_contactPoint;
			contact[0] = last[0];
			contact[1] = last[1];
			contact[2] = last[2];
		}
	}
	if (m_scriptClockHold) {
		m_scriptClockHold = 0;
		if (!m_script.m_0c)
			m_script.m_0c = *Script::ScriptSystem::GetScriptTimePtr();
		return true;
	}
	if (m_script.m_0c) {
		m_script.m_startTime += *Script::ScriptSystem::GetScriptTimePtr() - m_script.m_0c;
		m_script.m_0c = 0;
	}
	return false;
}

}}
