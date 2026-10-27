// Actor prototypes: op prototype getters, the actor type (TFBType_Actor / WaActor), attachment and turret types,
// the cut-scene record stack and the temporary "subset" constructor.
#include "script/script.h"
#include "script/ops_actor.h"
#include "script/script.h"
#include "script/referenceset.h"
#include "script/csset.h"
#include "script/actor.h"
#include "script/types/scriptactor.h"
#include "script/types/actorattachment.h"
#include "script/types/scriptsystem.h"
using namespace Script;

// GLOBAL: 0x5ced68 Script::ActorMoveTo::vftable
// 0x4354d0
ScriptReference *OpProto_ActorMoveTo()
{
	static ActorMoveTo s;
	return &s;
}

// GLOBAL: 0x5cedcc Script::ActorMoveFrom::vftable
// 0x435540
ScriptReference *OpProto_ActorMoveFrom()
{
	static ActorMoveFrom s;
	return &s;
}

// GLOBAL: 0x5cecd4 Script::ActorCutScene::vftable
// 0x4355b0
ScriptReference *OpProto_ActorCutScene()
{
	static ActorCutScene s;
	return &s;
}

// GLOBAL: 0x5cee14 Script::ActorTeleportTo::vftable
// 0x4365e0
ScriptReference *OpProto_ActorTeleportTo()
{
	static ActorTeleportTo s;
	return &s;
}

// GLOBAL: 0x5cee5c Script::ActorTurnTo::vftable
// 0x436660
ScriptReference *OpProto_ActorTurnTo()
{
	static ActorTurnTo s;
	return &s;
}

// GLOBAL: 0x5ceeec Script::CheckFOV::vftable
// 0x436700
ScriptReference *OpProto_CheckFOV()
{
	static CheckFOV s;
	return &s;
}

namespace Script {
}  // namespace Script

ScriptReference *TFBType_Actor();     // 0x436e30
ScriptReference *OpProto_ActorPlayAnim();
ScriptReference *OpProto_ActorRunAsPlayer();

struct ActorOpRegistrar {
	void RegisterOps();
};

// 0x436cd0
void ActorOpRegistrar::RegisterOps()
{
	((ScriptReference *)OpProto_ActorPlayAnim())->ResolveSymbol(TFBType_Actor(), 0, 0);
	((ScriptReference *)OpProto_ActorMoveTo())->ResolveSymbol(TFBType_Actor(), 0, 0);
	((ScriptReference *)OpProto_ActorTeleportTo())->ResolveSymbol(TFBType_Actor(), 0, 0);
	((ScriptReference *)OpProto_ActorTurnTo())->ResolveSymbol(TFBType_Actor(), 0, 0);
	((ScriptReference *)OpProto_ActorRunAsPlayer())->ResolveSymbol(TFBType_Actor(), 0, 0);
	((ScriptReference *)OpProto_ActorMoveFrom())->ResolveSymbol(TFBType_Actor(), 0, 0);
	((ScriptReference *)OpProto_CheckFOV())->ResolveSymbol(TFBType_Actor(), 0, 0);
	((ScriptReference *)OpProto_ActorCutScene())->ResolveSymbol(TFBType_Actor(), 0, 0);
	TFBType_Actor()->vf10();
}

extern int g_cutSceneStackIndex;   // 0x61fc00
// GLOBAL: 0x61fc00 g_cutSceneStackIndex

namespace Script {

// A script object set (PS2 ReferenceSet): fields, then the destructor clears it.
// GLOBAL: 0x5ca608 Script::CsSet::vftable
// NOVTABLE: Script::CsSet

// vtable 0x5cef40 ("set", 35 slots)
// GLOBAL: 0x5cef40 Script::CsSetD::vftable
// NOVTABLE: Script::CsSetD
struct CsSetD : CsSet {
	CsSetD() {}
};

struct CutSceneRec {
	int m_state;       // 2 = none (ExposeCutSceneActive: != 2); CutScene_Begin stores its argument
	int m_serial;      // g_cutSceneCounter at CutScene_Begin
	int m_startTime;   // script time at CutScene_Begin
	CsSetD m_actors;   // the cut scene's actors (CutScene_End passes each to 0x42cfc0)
	CutSceneRec();
	~CutSceneRec();
};

// 0x436a00
CutSceneRec::CutSceneRec() : m_state(2)
{
}

// 0x435430
CutSceneRec::~CutSceneRec()
{
}

}  // namespace Script

// 0x436c00
int *CutScene_StackSlot(int delta)
{
	static Script::CutSceneRec stack[5];
	int i = g_cutSceneStackIndex;
	g_cutSceneStackIndex += delta;
	if (g_cutSceneStackIndex < 0)
		g_cutSceneStackIndex = 0;
	return &stack[i].m_state;
}

namespace Script {

// GLOBAL: 0x5ca710 Script::CsSetE::vftable
// NOVTABLE: Script::CsSetE
struct CsSetE : CsSet {
	CsSetE() {}
	ScriptReference *GetBaseType();   // 5: 0x4034C0
};

// NOVTABLE: Script::WaPlacement
struct WaPlacement : ScriptReference {
	int m_04;
	CsSet m_08;
	CsSetE m_20;
	WaPlacement *m_38;
	int m_3c;
	WaPlacement() : m_04(-1), m_3c(0) { m_38 = this; }
};

// The prototype's type is a real ScriptActor (the game's vtable 0x5cd5e8 has the 40 slots the compiler builds for it).
// ScriptActor::ScriptActor (gameactor.cpp) runs the member-set initialisation of 0x42d540; the prototype constructor 0x436960 does not need to skip it.
struct WaActor : ScriptActor {
	WaActor() {}
	WaActor(char dummy);
	WaActor(ActorOpRegistrar *r);   // 0x436960
};
WaActor::WaActor(char dummy) {}

// ScriptActor::ScriptActor (gameactor.cpp) is only a scaffold that lets cl infer nothrow for GameActor's constructor. The rest of 0x42d540 (m_4c = 2 and
// the three member sets: vptrs, empty lists) is done here on the actor's own storage (sized by ReferenceSet, also on x64), so CProtoActor /
// GameActor::DetachProto can Clear the waypoint set. A 32-bit layout mirror did this before: on x64 its sets ran past the object and cleared
// TFBType_Actor's init guard: endless recursion in the actor type's static init.
void p4b_ScriptActor_init_sets(void *actor)
{
	ScriptActor *a = (ScriptActor *)actor;
	static_assert(sizeof(CsSetWayPoints) <= sizeof(a->m_wayPoints) && sizeof(CsSetAttachPoints) <= sizeof(a->m_attachmentPoints) &&
	              sizeof(CsSetTurrets) <= sizeof(a->m_turrets), "member set storage");
	a->m_controlMode = 2;
	::new (static_cast<void *>(a->m_wayPoints)) CsSetWayPoints;
	::new (static_cast<void *>(a->m_attachmentPoints)) CsSetAttachPoints;
	::new (static_cast<void *>(a->m_turrets)) CsSetTurrets;
}

}  // namespace Script

// NEARMISS 0x42cb40 CsSubset named ctor (same behaviour): the game stores the vptr 0x5cd2e0 and m_18 BEFORE m_name (+0x1c); ours puts the
// m_name store first (init-list orders A/B/C tried; with the plain `ScriptReference m_type` member + vptr assignment in the body the
// store order matches but the EH state store 0 comes before the m_type vptr store).
namespace Script {

// GLOBAL: 0x5ca608 Script::CsSubsetBase::vftable
// NOVTABLE: Script::CsSubsetBase
struct CsSubsetBase : ScriptReference {
	unsigned char m_sorted;   // +4; +8 count, +0xc version, +0x10 head node, +0x14 tail (Script::ReferenceSet, script/referenceset.h)
	int m_count, m_version;
	void *m_head, *m_tail;
	CsSubsetBase() : m_sorted(1), m_count(0), m_version(0), m_head(0), m_tail(0) {}
	~CsSubsetBase() { Clear(); }
	void Clear();                          // 0x43cca0
};
// GLOBAL: 0x43cca0 ?Clear@CsSubsetBase@Script@@QAEXXZ

// a ScriptReference-shaped member that is a copy of another object: base vptr store, then the source's vptr
struct __declspec(novtable) CsRefCopy : ScriptReference {
	CsRefCopy(const ScriptReference &src) { *(void **)this = *(void **)&src; }
};

// GLOBAL: 0x5cd2e0 Script::CsSubset::vftable
// NOVTABLE: Script::CsSubset
struct CsSubset : CsSubsetBase {
	int m_18;
	const char *m_name;        // +0x1c
	CsRefCopy m_type;          // +0x20: copy of the type object (vptr)
	CsSubset(const char *name, ScriptReference *type, OpContext *script);
	int AdjustRefCount(int delta);   // 2: 0x42CBC0
};

// 0x42cb40 ??0CsSubset@Script@@QAE@PBDPAVScriptReference@1@PAVOpContext@1@@Z
CsSubset::CsSubset(const char *name, ScriptReference *type, OpContext *script)
	: m_18(0), m_name(name), m_type(*type)
{
	if (script != 0)
		((ScriptReference *)this)->ResolveSymbol(script, 0, 0);
}

}  // namespace Script

extern int g_cutSceneCounter;   // GLOBAL: 0x61f9bc g_cutSceneCounter
void CutScene_ApplyState();      // 0x429a40
void CutScene_RemoveActor(void *actor);          // 0x42cfc0 (cdecl: the caller cleans the argument)

// 0x436c90
void CutScene_Begin(int state)
{
	Script::CutSceneRec *r = (Script::CutSceneRec *)CutScene_StackSlot(0);
	if (r->m_state == 2) {
		g_cutSceneCounter++;
		r->m_serial = g_cutSceneCounter;
		r->m_startTime = *Script::ScriptSystem::GetScriptTimePtr();
		r->m_state = state;
	} else {
		r->m_state = state;
	}
}

struct CsNodeW { CsNodeW *m_prev, *m_next; float m_key; void *m_obj; };   // pointer-sized links (the object is at +0x18 on x64)

// Near-miss: 0x436de0 CutScene_End: cl emits `mov [esi],2` after the `test eax,eax` (game: right after `mov esi,eax`)
// 0x436de0 CutScene_End
void CutScene_End()
{
	int *st = CutScene_StackSlot(0);
	*st = 2;
	Script::CutSceneRec *r = (Script::CutSceneRec *)st;
	CsNodeW *n = (CsNodeW *)r->m_actors.m_head;
	// game 0x10-0x24: loop (jmp back to the `mov eax,[esi+1Ch]`): drains the actor list until it is empty, then Clear
	while (n) {
		CutScene_RemoveActor(n->m_obj);
		n = (CsNodeW *)r->m_actors.m_head;
	}
	r->m_actors.Clear();
	r->m_serial = 0;
	CutScene_StackSlot(-1);
	CutScene_ApplyState();
}

namespace Script {
// NOVTABLE: Script::AttachBaseP
// The two prototypes are ActorAttachments: its vtable of 35 slots and members; the game's constructors set +4 to -1 and the attacher set,
// the rest is static storage (zero).
#define ATTACH_PROTO_OVERRIDES
struct AttachBaseP : ActorAttachment {
	AttachBaseP() { m_index = -1; }
};
// GLOBAL: 0x5cd690 Script::AttachProtoA::vftable
// NOVTABLE: Script::AttachProtoA
struct AttachProtoA : AttachBaseP {
	AttachProtoA() {}
	const char *GetName();                // 0: 0x42D640
	ScriptReference *GetType();           // 4: 0x42D650
	ScriptReference *GetBaseType();       // 5: 0x4039F0
	int GetKind();                           // 6: 0x43E6F0
	void *GetMembers(int *count);         // 7: 0x42DB40
	ATTACH_PROTO_OVERRIDES
};
// NOVTABLE: Script::AttachMidP
struct AttachMidP : AttachBaseP {
	AttachMidP() {}
};
// GLOBAL: 0x5cdb30 Script::AttachProtoB::vftable
// NOVTABLE: Script::AttachProtoB
struct AttachProtoB : AttachMidP {
	AttachProtoB() {}
	const char *GetName();                // 0: 0x42CEC0
	ScriptReference *GetType();           // 4: 0x42CED0
	ScriptReference *GetBaseType();       // 5: 0x4039F0
	int GetKind();                           // 6: 0x43E6F0
	void *GetMembers(int *count);         // 7: 0x42DD50
	ATTACH_PROTO_OVERRIDES
};
}  // namespace Script

// 0x436ea0
Script::ScriptReference *TFBType_Attachment()
{
	static Script::AttachProtoA s;
	return &s;
}

// 0x436f10
Script::ScriptReference *TFBType_Turret()
{
	static Script::AttachProtoB s;
	return &s;
}

namespace Script {
// the set member at +8 stores its vptr BEFORE its fields (an ordinary base-first constructor)
// GLOBAL: 0x5ca608 Script::CsSetF::vftable
struct CsSetF : ReferenceSet {
	CsSetF() {}
};
// The real Placement (the game's vtable 0x5ca7d8 has 40 slots; same layout and constructor).
struct PlacementProtoW3a : Placement {
	PlacementProtoW3a() {}
};
}  // namespace Script

// 0x42f5c0
Script::ScriptReference *TFBType_Placement()
{
	static Script::PlacementProtoW3a s;
	return &s;
}

// GLOBAL: 0x436cd0 ?RegisterOps@ActorOpRegistrar@@QAEXXZ
// GLOBAL: 0x436960 ??0WaActor@Script@@QAE@PAUActorOpRegistrar@@@Z

// NOVTABLE: ActorProtoW3a
// __declspec(novtable): the game's static is a plain WaActor (its ctor stores only WaActor::vftable); without it cl stores a
// vtable of its own whose slot 14 is the deleting dtor where the game's has GetFloat: SetFindSub's GetFloat on the Actor type
// destroyed the static in the port (level 0 crash)
struct __declspec(novtable) ActorProtoW3a : Script::WaActor {
	ActorProtoW3a(ActorOpRegistrar *r) : WaActor(r) { }
};

// GLOBAL: 0x61fcc0 ?s@?1??TFBType_Actor@@YAPAVScriptReference@Script@@XZ@4UActorProtoW3a@@A
// GLOBAL: 0x61fd58 ?$S9@?1??TFBType_Actor@@YAPAVScriptReference@Script@@XZ@4IA
// 0x436e30 TFBType_Actor: order guard-or, WaActor ctor, RegisterOps(&reg), atexit(dtor), return &s identical; ours calls the derived ctor out of line (WaActor ctor + RegisterOps inside it), game inlines it; forceinline matches size (108 B) but adds an EH state store + unwind funclet and scores, so kept; differences are EH only, no behaviour difference
Script::ScriptReference *TFBType_Actor()
{
	ActorOpRegistrar reg;
	static ActorProtoW3a s(&reg);
	ZOOSTERS_PROTO_REGISTER_AFTER(reg.RegisterOps());
	return &s;
}

// ---- 0x42def0: slot 6 of the CProtoActor family's vtable (0x5cd908..): forwards to the base type
namespace Script {
class ProtoSlot6 : public ScriptReference {
public:
	int GetKind();
};
// 0x42def0
int ProtoSlot6::GetKind()
{
	return GetBaseType()->GetKind();
}
}  // namespace Script

namespace Script { WaActor::WaActor(ActorOpRegistrar *) {} }

namespace Script {
inline void CsSubsetBase::Clear() { ((ReferenceSet *)this)->Clear(); }
}

#include "script/folded.h"
#include "script/types/actorturret.h"
#include "script/ops_set.h"
// FOLDED: 0x4039f0 Script::AttachProtoA::GetBaseType
Script::ScriptReference * Script::AttachProtoA::GetBaseType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x42d640 Script::AttachProtoA::GetName
char const * Script::AttachProtoA::GetName() { return "attachment"; }
// FOLDED: 0x42d650 Script::AttachProtoA::GetType
Script::ScriptReference * Script::AttachProtoA::GetType() { return ActorAttachment_GetType(); }
// FOLDED: 0x42db40 Script::AttachProtoA::GetMembers
void * Script::AttachProtoA::GetMembers(int * a1) { return Script::ActorAttachment::GetMembers(a1); }
// FOLDED: 0x43e6f0 Script::AttachProtoA::GetKind
int Script::AttachProtoA::GetKind() { return 0; }
// FOLDED: 0x4039f0 Script::AttachProtoB::GetBaseType
Script::ScriptReference * Script::AttachProtoB::GetBaseType() { return PositionMeasurement_GetType(); }
// FOLDED: 0x42cec0 Script::AttachProtoB::GetName
char const * Script::AttachProtoB::GetName() { return "turret"; }
// FOLDED: 0x42ced0 Script::AttachProtoB::GetType
Script::ScriptReference * Script::AttachProtoB::GetType() { return ActorTurret_GetType(); }
// FOLDED: 0x42dd50 Script::AttachProtoB::GetMembers
void * Script::AttachProtoB::GetMembers(int * a1) { return Script::ActorTurret::Members(a1); }   // cross-class fold; the shared body is a static of ActorTurret
// FOLDED: 0x43e6f0 Script::AttachProtoB::GetKind
int Script::AttachProtoB::GetKind() { return 0; }
// FOLDED: 0x42cde0 Script::CsSetAttachPoints::GetName
char const * Script::CsSetAttachPoints::GetName() { return "attach points"; }
// FOLDED: 0x42cdf0 Script::CsSetAttachPoints::Deref
Script::ScriptReference * Script::CsSetAttachPoints::Deref(Script::ScriptReference * a1, int a2) { return (Script::ScriptReference *)((Script::ScriptActor *)a1)->m_attachmentPoints; }   // cross-class fold of ScriptActor::ExposeAttachmentPoints::Deref (0x42cdf0)
// FOLDED: 0x42d650 Script::CsSetAttachPoints::GetBaseType
Script::ScriptReference * Script::CsSetAttachPoints::GetBaseType() { return ActorAttachment_GetType(); }
// FOLDED: 0x42ce00 Script::CsSetTurrets::GetName
char const * Script::CsSetTurrets::GetName() { return "turrets"; }
// FOLDED: 0x42ce10 Script::CsSetTurrets::Deref
Script::ScriptReference * Script::CsSetTurrets::Deref(Script::ScriptReference * a1, int a2) { return (Script::ScriptReference *)((Script::ScriptActor *)a1)->m_turrets; }   // cross-class fold of ScriptActor::ExposeTurrets::Deref (0x42ce10)
// FOLDED: 0x42ced0 Script::CsSetTurrets::GetBaseType
Script::ScriptReference * Script::CsSetTurrets::GetBaseType() { return ActorTurret_GetType(); }
// FOLDED: 0x42cdc0 Script::CsSetWayPoints::GetName
char const * Script::CsSetWayPoints::GetName() { return "waypoints"; }
// FOLDED: 0x42cdd0 Script::CsSetWayPoints::Deref
Script::ScriptReference * Script::CsSetWayPoints::Deref(Script::ScriptReference * a1, int a2) { return (Script::ScriptReference *)((Script::ScriptActor *)a1)->m_wayPoints; }   // cross-class fold of ScriptActor::ExposeWayPoints::Deref (0x42cdd0)
// FOLDED: 0x42d4a0 Script::CsSetWayPoints::GetBaseType
Script::ScriptReference * Script::CsSetWayPoints::GetBaseType() { return ActorWayPoint_GetType(); }
// FOLDED: 0x42cbc0 Script::CsSubset::AdjustRefCount
int Script::CsSubset::AdjustRefCount(int a1) { return ((Script::ScriptSubset *)this)->Script::ScriptSubset::AdjustRefCount(a1); }   // cross-class fold; the classes share no base here
