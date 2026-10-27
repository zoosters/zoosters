// Commands: 0 actor script object by symbol, 1 attributes block (0x114 B), 2 actor by guid, 3 resource guid, 4 waypoint count, 5 waypoint, 6
// colour bytes.
// Then the CSystemCommands pass (command 1 = frame matrix).
#include <math.h>
#include <string.h>
#include "script/script.h"
#include "modules/tfb/cprotoactor.h"
#include "script/symbolregistry.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "script/runtime.h"
#include "script/referenceset.h"
#include "script/types/actorwaypoint.h"

// GLOBAL: 0x602f94 g_protoActorClassId
extern const char *g_protoActorClassId;
extern const float g_twelveW3a;   // GLOBAL: 0x5ca758 g_twelveW3a
// the linked handler of command 2 is a CTFBModel; +0x118 = CTFBModel::m_clumpId (first word). ctfbmodel.h is not included:
// <vector> would change the allocator relocations here (like tfb/corpus/corpus.h)
#define PA_ATTRS m_attributes                        // +0x20
#define PA_ATTR84 (*(int *)(m_attributes + 0x64))     // +0x84
#include "tfb/corpus/corpus.h"
#include "modules/tfb/ctfbmodel.h"
#define PA_CLUMPID(h) (((CTFBModel *)(h))->m_clumpId.m_Data[0])
ZOOSTERS_ASSERT_OFFSET_AT(PA_clumpId, offsetof(CTFBModel, m_clumpId.m_Data[0]), 0x118);

// The body frame's modelling matrix: RwFrame +0x10 in the 32-bit layout (the frame header holds pointers, so the offset is larger on x64)
#define PA_FRAME_MATRIX(c) ((char *)&((CorpusView *)(c))->m_frame->modelling)
// GLOBAL: 0x424500 ?SetPlayerControl@CorpusView@@QAEXHH@Z
// GLOBAL: 0x4500b0 ?GetGuid@PaResRef@@QBEXPAUPaGuid@@@Z
struct PaV3 { float x, y, z; };
struct PaGuid { unsigned d[4]; };
// A view, not corpus.h: that header brings <new>, so new[] / delete[] would bind to operator new[] / delete[] by name instead of the game's
// allocator calls.
typedef Corpus CorpusView;
ZOOSTERS_ASSERT_OFFSET(Corpus, m_frame, 0xfc);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_flags, 0x100);
ZOOSTERS_ASSERT_OFFSET(Corpus, m_controller, 0x104);
struct PaResRef { void GetGuid(PaGuid *out) const; };
void *GetActorSerialCounter();   // 0x42f360
int ScriptAssets_Add(RWS::RWSGUID *guid);

static __forceinline RWS::TFB::GameActor *PaLookup(int idx)
{
	if (idx >= 0x3fc0d)
		return (RWS::TFB::GameActor *)g_curScript->m_locals[idx - 0x3fc0d];
	SymbolSlot *slot = &(*g_symRegistryY)[idx];
	return (RWS::TFB::GameActor *)(slot->obj & ~(uintptr_t)1);
}

// Placement::SetControlMode, inlined (the game keeps the 0x42e530 copy too)
static __forceinline void PaSetMode(RWS::TFB::GameActor *actor, int mode)
{
	int old = actor->m_controlMode;
	actor->m_controlMode = mode;
	if (old < 2) {
		if (actor->m_spawnSource) {
			actor->m_controlMode = 3;
			((Script::ReferenceSet *)actor->m_wayPoints)->Clear();
		}
		actor->ReleaseBody();
		actor->m_embedded = 0;
		actor->m_velocity.z = 0.0f;
		actor->m_velocity.y = 0.0f;
		actor->m_velocity.x = 0.0f;
	}
}

namespace RWS { namespace TFB {

// 0x42e810
void CProtoActor::HandleAttributes(const CAttributePacket &attr)
{
	unsigned char found = 0;
	CAttributeHandler::HandleAttributes(attr);
	CAttributeCommandIterator it(attr, g_protoActorClassId);
	for (; !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0: {
			const char *data = it->GetAs_RwChar_ptr();
			::Script::ScriptReference *actor = m_actor;
			int idx = ((::Script::ScriptReference *)actor)->ResolveSymbol(actor->GetType(), data, 0);
			if (idx < 0)
				break;
			GameActor *named = PaLookup(idx);
			found = (named == m_actor);
			if (found)
				break;
			delete m_actor;
			m_actor = named;
			named->AdjustRefCount(1);
			named->m_proto = this;
			break;
		}
		case 3: {
			PaGuid guid;
			(static_cast<const PaResRef *>(it->GetAs_void_ptr()))->GetGuid(&guid);
			PaGuid *dst = (PaGuid *)m_scriptGuid;
			*dst = guid;
			if (dst->d[0])
				m_scriptAsset = ScriptAssets_Add((RWS::RWSGUID *)dst);
			break;
		}
		case 2: {
			PaGuid guid;
			(static_cast<const PaResRef *>(it->GetAs_void_ptr()))->GetGuid(&guid);
			PaGuid guidCopy = guid;
			CAttributeHandler *handler = CAttributeHandler::Find(*(const RWSGUID *)&guidCopy);
			m_linkedHandler = (intptr_t)handler;
			if (handler && PA_CLUMPID(handler))
				break;
			m_actor->GetName();
			m_linkedHandler = 0;
			break;
		}
		case 4: {
			int count = it->GetAs_RwInt32();
			if (count == m_numWayPoints)
				break;
			if (m_wayPoints)
				delete[] (::Script::ActorWayPoint *)m_wayPoints;
			::Script::ActorWayPoint *arr = 0;
			if (count)
				arr = new ::Script::ActorWayPoint[count];
			m_wayPoints = arr;
			m_numWayPoints = count;
			break;
		}
		case 5: {
			const CAttributeCommand *cmd = it.operator->();
			int idx = cmd->GetAs_RwInt32();
			if ((unsigned)m_numWayPoints <= (unsigned)idx) {   // unsigned compare
				if (m_wayPoints)
					delete[] (::Script::ActorWayPoint *)m_wayPoints;
				m_numWayPoints = idx + 1;
				m_wayPoints = new ::Script::ActorWayPoint[idx + 1];
			}
			::Script::ActorWayPoint *wp = (::Script::ActorWayPoint *)((char *)m_wayPoints + idx * sizeof(::Script::ActorWayPoint));
			wp->m_pos.x = cmd->GetAs_RwReal_ptr()[1];
			wp->m_pos.y = cmd->GetAs_RwReal_ptr()[2];
			wp->m_index = idx;
			wp->m_pos.z = cmd->GetAs_RwReal_ptr()[3];
			((Script::ReferenceSet *)m_actor->m_wayPoints)->Insert(wp, (float)(unsigned)idx);
			break;
		}
		case 1: {
			memcpy(PA_ATTRS, it->GetAs_RwChar_ptr(), 0x114);
			GameActor *actor = m_actor;
			// [this+0x84] = 0 happens before the second copy, so the actor's copy gets the zero too
			PA_ATTR84 = 0;
			memcpy(&actor->m_params, PA_ATTRS, 0x114);
			Corpus *corpus = actor->m_corpus;
			if (corpus) {
				actor->m_corpus = 0;
				actor->ReleaseBody();
				actor->m_corpus = corpus;
				((::Script::ScriptReference *)corpus)->AdjustRefCount(-1);
				((::Script::ScriptReference *)actor->m_corpus)->AdjustRefCount(1);
				const RwMatrix *src = actor->m_corpus ? (const RwMatrix *)PA_FRAME_MATRIX(actor->m_corpus) : &actor->m_matrix;
				actor->UpdateBounds(src, 0);
			}
			break;
		}
		case 6: {
			unsigned rgba = it->GetAs_RwUInt32();
			m_actor->m_tint[0] = (unsigned char)rgba;
			m_actor->m_tint[1] = (unsigned char)(rgba >> 8);
			m_actor->m_tint[2] = (unsigned char)(rgba >> 16);
			m_actor->m_tint[3] = (unsigned char)(rgba >> 24);
			break;
		}
		}
	}
	CAttributeCommandIterator it2(attr, CSystemCommands::pCSystemCommandsClassID);
	for (; !it2.IsFinished(); ++it2) {
		if (it2->GetCommandId() == 1) {
			memcpy(&m_frame, it2->GetAs_RwChar_ptr(), 0x40);
			RwMatrixOptimize(&m_frame, 0);
			float x = m_frame.at.x;
			float y = m_frame.at.y;
			float z = m_frame.at.z;
			float pitch = (float)atan2((double)y, (double)sqrt(x * x + z * z));
			float yaw;
			if (pitch < 1.5707964f && pitch > -1.5707964f)
				yaw = (float)atan2((double)x, (double)z);
			else
				yaw = 0.0f;
			m_actor->m_initialFacing = yaw * 57.2957763671875f;   // the game's constant is 0x42652ee0, not (float)57.2958
			if (m_actor->m_initialFacing < 0.0f)
				m_actor->m_initialFacing += 360.0f;
			m_actor->m_origin = *(Vec3 *)&m_frame.pos;
			GameActor *actor = m_actor;
			if (found) {
				Corpus *corpus = actor->m_corpus;
				char *dst = corpus ? PA_FRAME_MATRIX(corpus) : (char *)&actor->m_matrix;
				*(PaV3 *)(dst + 0x30) = *(PaV3 *)&actor->m_origin;
			} else {
				m_actor->PlaceAt(&actor->m_origin);
			}
			actor = m_actor;
			actor->m_heading = 0.0f;
			actor->m_facing = actor->m_initialFacing;
			actor = m_actor;
			Corpus *corpus = actor->m_corpus;
			if (!corpus) {
				char *dst = corpus ? PA_FRAME_MATRIX(corpus) : (char *)&actor->m_matrix;
				memcpy(dst, &m_frame, 0x40);
			}
		}
	}
	if (found) {
		GameActor *actor = m_actor;
		int *c = (int *)GetActorSerialCounter();
		actor->m_id = *c;
		++*c;
		m_actor->ReleaseBody();
		if (((PlacementIdW *)m_actor)->IsPlacementRemoved()) {
			actor = m_actor;
			PaSetMode(actor, 3);
			Corpus *corpus = actor->m_corpus;
			if (corpus) {
				((CorpusView *)corpus)->m_controller = -1;
				((CorpusView *)actor->m_corpus)->SetPlayerControl(0, -1);
			}
			return;
		}
		actor = m_actor;
		PaSetMode(actor, 2);
		Corpus *corpus = actor->m_corpus;
		if (corpus) {
			((CorpusView *)corpus)->m_controller = -1;
			((CorpusView *)actor->m_corpus)->SetPlayerControl(0, -1);
		}
		float f = m_actor->GetActivationRange();
		if (f == 3.4028235e38f) {
			m_actor->ActivateActor(true);
			return;
		}
		TFB_GetInactiveSet()->Insert(m_actor, 3.4028235e38f);
		return;
	}
	GameActor *actor = m_actor;
	Corpus *corpus = actor->m_corpus;
	if (corpus) {
		((CorpusView *)corpus)->m_flags &= 0xffffbfff;
		return;
	}
	actor->m_localBox.m_extent[0] = (actor->m_params.m_xExtent * 0.5f) * g_twelveW3a;
	actor->m_localBox.m_extent[1] = (actor->m_params.m_zExtent * 0.5f) * g_twelveW3a;
	actor->m_localBox.m_extent[2] = (actor->m_params.m_yExtent * 0.5f) * g_twelveW3a;
	float offset[3];
	offset[0] = actor->m_params.m_xOffset * 12.0f;
	offset[1] = actor->m_params.m_zOffset * 12.0f;
	offset[2] = actor->m_params.m_yOffset * 12.0f;
	RwMatrixTranslate(&actor->m_localBox.m_frame, (const RwV3d *)offset, rwCOMBINEREPLACE);
	actor->m_localBox.m_frame.flags &= 0xfffdffff;
	actor->m_worldBox.m_extent[0] = actor->m_localBox.m_extent[0];
	actor->m_worldBox.m_extent[1] = actor->m_localBox.m_extent[1];
	actor->m_worldBox.m_extent[2] = actor->m_localBox.m_extent[2];
	if (actor->m_controlMode < 2)
		actor->UpdateBounds(&actor->m_matrix, 0);
}
}}

#include "framework/core/resourceref.h"
inline void PaResRef::GetGuid(PaGuid *out) const { ((const RWS::ResourceRef *)this)->GetGuid((RWS::RWSGUID *)out); }
