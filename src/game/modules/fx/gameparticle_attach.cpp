#include "modules/fx/gameparticle.h"
#include "script/actor.h"
#include "modules/tfb/gameactor.h"

namespace RWS { namespace FX {

// 0x40a9e0
void GameParticle::SetAttachTarget(Script::ScriptReference *target)
{
	m_link.Release();
	Script::ScriptReference *carrier = (Script::ScriptReference *)((Script::PositionMeasurement *)target)->vf26((intptr_t)this);
	if (carrier) {
		carrier->AdjustRefCount(1);
		m_link.m_index = -1;
		AttachPoint *point = (AttachPoint *)target;
		m_link.m_attachPoint = point;
		point->m_riders.Insert(this, 3.4028235e+38f);
		m_link.m_frame = m_link.m_attachPoint->m_frame;
		m_link.m_id = static_cast<TFB::GameActor *>(carrier)->m_corpus->m_id;
	}
	m_link.m_carrier = carrier ? static_cast<TFB::GameActor *>(carrier) : 0;   // the GameActor's collider base
}
}}
