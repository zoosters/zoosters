// Own TU: the constructor and the vtable are in cprotoactor.cpp; this defines the destructor the scalar deleting destructor calls.
#include "modules/tfb/cprotoactor.h"
#include "modules/tfb/gameactor.h"
#include "script/referenceset.h"
#include "rw/rpplugins.h"

using RWS::TFB::GameActor;

// A view, not corpus.h: that header (through <vector> / <new>) makes `delete[]` below bind to operator delete[] by name instead of the
// game's operator delete 0x5500f0.
#include "tfb/corpus/corpus.h"
typedef Corpus CorpusPinView;
ZOOSTERS_ASSERT_OFFSET(Corpus, m_controller, 0x104);
// GLOBAL: 0x424500 ?SetPlayerControl@CorpusPinView@@QAEX_NH@Z

struct RpSpline;
void operator delete(void *p);

// Script::ActorWayPoint as `delete[]` needs it (virtual destructor; element size from the vector deleting destructor 0x42cae0).
// Declared three times in src/game/script; any of them changes the operator delete relocation here
#include "script/types/actorwaypoint.h"
typedef Script::ActorWayPoint WayPointRef;

// Script::Placement::SetControlMode (0x42e530), inlined into the destructor (the game keeps the out-of-line copy too).
static __forceinline void SetControlMode(GameActor *actor, int mode, int v)
{
	int old = actor->m_controlMode;
	actor->m_controlMode = mode;
	if (old >= 2) {
		if (mode < 2)
			actor->CreateBody();
	} else if (mode >= 2) {
		if (actor->m_spawnSource != 0) {
			actor->m_controlMode = 3;
			((Script::ReferenceSet *)actor->m_wayPoints)->Clear();
		}
		actor->ReleaseBody();
		actor->m_embedded = 0;
		actor->m_velocity.z = 0.0f;
		actor->m_velocity.y = 0.0f;
		actor->m_velocity.x = 0.0f;
	}
	if (actor->m_corpus) {
		((CorpusPinView *)actor->m_corpus)->m_controller = v;
		((CorpusPinView *)actor->m_corpus)->SetPlayerControl(mode == 0, v);
	}
}

namespace RWS { namespace TFB {
// Near-match of 0x42e5c0: the game keeps the redundant second `cmp eax,2` of the inlined SetControlMode(2,-1) body (cl folds it here); layout only.
CProtoActor::~CProtoActor()
{
	GameActor *a = m_actor;
	if (a->m_controlMode < 2) {
		SetControlMode(a, 2, -1);
	}
	GameActor *b = m_actor;
	for (Script::SetNode *n = b->m_20.m_head; n; n = n->m_next)
		((GameActor *)n->m_object)->DetachProto();
	((Script::ReferenceSet *)b->m_wayPoints)->Clear();
	b->m_proto = 0;
	m_actor->AdjustRefCount(-1);
	if (m_spline) {
		RpSplineDestroy(static_cast<RpSpline *>(m_spline));
		m_spline = 0;
	}
	delete[] (WayPointRef *)m_wayPoints;
}
}}
