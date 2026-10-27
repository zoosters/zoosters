#include <string.h>
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "script/referenceset.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "rw/rwcore.h"
#include "modules/tfb/cprotoactor.h"

// GLOBAL: 0x5ca758 g_twelveW3a
extern const float g_twelveW3a;

namespace RWS {
namespace TFB {

// 0x41d410
void *GameActor::GetBodyPosition()
{
	if (m_corpus)
		return &m_corpus->m_contactPoint;
	return &m_matrix.pos;
}

// 0x42bb80
void GameActor::UpdateCollisionExtents(bool pinned)
{
	Corpus *c = m_corpus;
	if (c && pinned) {
		c->m_flags &= ~CORPUS_STEP_DONE;
		return;
	}
	m_localBox.m_extent[0] = m_params.m_xExtent * 0.5f * g_twelveW3a;
	m_localBox.m_extent[1] = m_params.m_zExtent * 0.5f * g_twelveW3a;
	m_localBox.m_extent[2] = m_params.m_yExtent * 0.5f * g_twelveW3a;
	RwV3d t;
	t.x = m_params.m_xOffset * g_twelveW3a;
	t.y = m_params.m_zOffset * g_twelveW3a;
	t.z = m_params.m_yOffset * g_twelveW3a;
	RwMatrixTranslate(reinterpret_cast<RwMatrix *>(&m_localBox.m_frame), &t, rwCOMBINEREPLACE);
	m_localBox.m_frame.flags &= 0xfffdffff;
	*(RwV3d *)m_worldBox.m_extent = *(RwV3d *)m_localBox.m_extent;
	if (pinned && m_controlMode < 2)
		UpdateBounds(&m_matrix, 0);
}

// Deletes everything in a member set (the game's loop: take the head object, unlink its node, delete the object). A macro: the set
// expression is evaluated again on every pass, as in the game (a function with the set as parameter changes the bytes).
#define DELETE_MEMBERS(set) \
	while ((node = (set)->m_head) != 0) { \
		Script::ScriptReference *obj = node->m_object; \
		(set)->RemoveNode(node); \
		if (obj) \
			delete obj; \
	}

// Near-match of 0x42bca0: loops stay rotated with hoisted ecx (game: top-tested, lea ecx per iteration); proto / src register split.
void GameActor::ReleaseBody()
{
	Collider::Reset();
	char *proto = (char *)m_proto;   // CProtoActor: +0x20 the parameter block, +0x148 the frame matrix (cprotoactor.h keeps them protected)
	const void *src;
	if (proto && m_controlMode != 3)
		src = &m_proto->m_frame;
	else if (m_corpus)
		src = (char *)&m_corpus->m_frame->modelling;
	else
		src = &m_matrix;
	memcpy(&m_matrix, src, sizeof(RwMatrix));
	if (proto && m_spawnSource == 0) {
		float activation = m_params.m_activationRange;
		float deactivation = m_params.m_deactivationRange;
		float facing = m_initialFacing;
		memcpy(&m_params, m_proto->m_attributes, sizeof(ActorParams));
		m_params.m_activationRange = activation;
		m_params.m_deactivationRange = deactivation;
		m_heading = 0.0f;
		m_facing = facing;
		UpdateCollisionExtents(false);
	}
	Corpus *c = m_corpus;
	if (c) {
		Script::SetNode *node;
		DELETE_MEMBERS((Script::ReferenceSet *)m_attachmentPoints);
		DELETE_MEMBERS((Script::ReferenceSet *)m_turrets);
		DELETE_MEMBERS(&m_corpus->m_feet);
		c = m_corpus;
		c->LeaveGround(0, false);
		c->SetEnabled(-1);
		c = m_corpus;
		if (c)
			delete c;
		m_corpus = 0;
	}
}

} // namespace TFB
} // namespace RWS
