// Script actor ops (cut-scene, play animation, move to/from, teleport, turn, run as player, check fov): GetName / Clone / CloneToPool.
#include "script/types/scriptsystem.h"
#include "script/ops_actor.h"
#include "script/ops_set.h"
#include "script/actor.h"
#include "script/types/scriptactor.h"
// ActorCursor's SetCursor spans PlacementSet::m_1c and the actor's SetCursorTail: contiguous on x86 and x64
static_assert(offsetof(Script::ScriptActor, m_unk40) == offsetof(Script::ScriptActor, m_20) + offsetof(Script::Placement::PlacementSet, m_1c) +
              sizeof(intptr_t) && sizeof(Script::SetCursor) == sizeof(intptr_t) + sizeof(Script::SetCursorTail) &&
              offsetof(Script::SetCursor, m_node) == sizeof(intptr_t), "actor set cursor layout");

// GLOBAL: 0x603044 g_forEachOrderNames
extern const char *g_forEachOrderNames[4];
// GLOBAL: 0x602f98 g_compareOpNames
extern const char *g_compareOpNames[7];
// GLOBAL: 0x621f1c g_forEachItem
extern Script::ScriptReference *g_forEachItem;
// GLOBAL: 0x622000 g_forEachSet
extern Script::ScriptReference *g_forEachSet;
// GLOBAL: 0x622004 g_scriptSubset
extern Script::ScriptReference *g_scriptSubset;
// GLOBAL: 0x61f514 g_findSubOwner
extern Script::ScriptReference *g_findSubOwner;
// GLOBAL: 0x621f18 g_scriptOwner
extern Script::ScriptReference *g_scriptOwner;
#if defined(_MSC_VER)
extern "C" double __cdecl sin(double);
extern "C" double __cdecl cos(double);
#pragma intrinsic(sin, cos)
#else
#include <math.h>
#endif
// GLOBAL: 0x621f14 g_scriptSelf
extern Script::ScriptReference *g_scriptSelf;

Script::ScriptReference *TFBType_Placement();
Script::ScriptReference *TFBType_Controller();
struct CutSceneSlot {
	int m_state;    // cut-scene state (2, 3, -1)
	int m_counter;
	int m_stamp;    // 0x436c00: state +0, counter +4, stamp +8
};
int *CutScene_StackSlot(int delta);  // returns a CutSceneSlot *; five 0x24-byte slots in a function-local static array; delta 0 = current, 1 = push, -1 = pop
void CutScene_End();
void CutScene_ApplyState();
void CutScene_ProcessActorSet(Script::ReferenceSet *obj, bool *flag);
// GLOBAL: 0x61f9bc g_cutSceneCounter
extern int g_cutSceneCounter;
bool TFB_CompareFloat(float a, int op, float b);
#include "script/types/scriptcontroller.h"
Script::ControllerPlayerRef *TFB_GetPlayerRefs();  // 0x439460 static array of the four player reference objects

namespace Script {

// 0x435300
// Shared with ActorCutScene::Parse: the game's linker folded the two into one body (same bytes, the ops share OpBlock and the byte at +8),
// so the body is written once over the op and its byte and both members call it.
static int ParseOneByteOp(OpCode *op, unsigned char *arg, int size)
{
	op->GetName();
	TFBRead_u8(arg);
	return size;
}
int ActorPlayAnim::Parse() { return ParseOneByteOp(this, &m_anim, sizeof(ActorPlayAnim)); }
// FOLDED: 0x435300 Script::ActorCutScene::Parse
int ActorCutScene::Parse() { return ParseOneByteOp(this, &m_mode, sizeof(ActorCutScene)); }

// 0x435330
int ActorMoveTo::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Position());
	refs.Read(&m_target);
	if (g_scriptLastType->GetType() != TFBType_Set()) {
		m_limit.opArg = 0;
		TFBRead_u8(&m_limit.opArg);
	} else {
		g_scriptLastType = g_scriptLastType->GetBaseType();
		ScriptEnumReader(&m_limit.opArg, g_forEachOrderNames, 0);
	}
	TFBRead_u8(&m_anim);
	ScriptValueReader(&m_limit, g_scriptLastType, 0, 4);
	return sizeof(ActorMoveTo);
}

// 0x4353e0
int ActorMoveFrom::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Position());
	refs.Read(&m_target);
	TFBRead_u8(&m_anim);
	ScriptValueReader(&m_limit, g_scriptLastType, 0, 4);
	return sizeof(ActorMoveFrom);
}

// 0x4358d0
int ActorTeleportTo::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Position());
	refs.Read(&m_target);
	if (g_scriptLastType->GetType() != TFBType_Set()) {
		m_seconds.opArg = 0;
		TFBRead_u8(&m_seconds.opArg);
	} else {
		g_scriptLastType = g_scriptLastType->GetBaseType();
		ScriptEnumReader(&m_seconds.opArg, g_forEachOrderNames, 0);
	}
	ScriptValueReader(&m_facing, TFBType_Angle(), 0, 2), ScriptValueReader(&m_seconds, 0, "seconds", 4);
	return sizeof(ActorTeleportTo);
}

// 0x435990
int ActorTurnTo::Parse()
{
	GetName();
	ScriptValueReader(&m_angle, TFBType_Angle(), 0, 2);
	TFBRead_u8(&m_angle.opArg);
	return sizeof(ActorTurnTo);
}

// 0x4359e0
int ActorRunAsPlayer::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Controller());
	refs.Read(&m_player);
	return sizeof(ActorRunAsPlayer);
}

// 0x435a20
int CheckFOV::Parse()
{
	GetName();
	ScriptRefReader refs(0);
	ScriptValueReader(&m_heading, TFBType_Angle(), 0, 2), ScriptValueReader(&m_halfFov, TFBType_Angle(), 0, 2), TFBType_Position(), refs.Read(&m_target), ScriptEnumReader(&m_range.opArg, g_compareOpNames, 0), ScriptValueReader(&m_range, TFBType_Int(), "feet", 4);
	TFBRead_u8(&m_halfFov.opArg);
	return sizeof(CheckFOV);
}

// 0x435220
const char *ActorCutScene::GetName()
{
	return "cut-scene";
}

// 0x436d90
OpCode *ActorCutScene::Clone()
{
	return CloneToPool(this);
}

// 0x436da0
ActorCutScene *ActorCutScene::CloneToPool(const ActorCutScene *src)
{
	void *mem = g_scriptPool;
	ActorCutScene *r = new (mem) ActorCutScene(*src);
	g_scriptPool += sizeof(ActorCutScene);
	return r;
}

// 0x435290
int ActorPlayAnim::Execute()
{
	ScriptReference *actor = g_scriptSelf;   // not necessarily a placement: checked first
	bool run = false;
	if (actor->IsKindOf(TFBType_Placement())) {
		run = ((Placement *)g_scriptSelf)->PlayAnimation(m_anim - 1);
	}
	if (!run) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}

// 0x437690
int ActorTurnTo::Execute()
{
	ScriptReference *self = g_scriptSelf;
	Placement *actor;
	if (self->IsKindOf(TFBType_Actor())) {
		actor = ((Placement *)g_scriptSelf);
	} else {
		actor = 0;
	}
	bool turning = false;
	if (actor != 0) {
		float angle;
		if (m_angle.EvalAngle(&angle, 0)) {
			turning = actor->SetFacing(angle, 0);
		}
		actor->PlayAnimation(m_angle.opArg - 1);
	}
	if (!turning) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}

// 0x437890
int ActorRunAsPlayer::Execute()
{
	ScriptReference *a = g_scriptSelf;
	if (a->IsKindOf(TFBType_Actor())) {
		Placement *actor = ((Placement *)g_scriptSelf);
		if (actor != 0) {
			ScriptReference *owner;
			ScriptReference *set;
			ScriptReference *r = m_player.Resolve(&owner, &set, true);
			int index;
			if (r != 0) {
				index = (Script::ControllerPlayerRef *)r - TFB_GetPlayerRefs();
				if (index >= 4) {
					index = -1;
				}
			} else {
				index = -1;
			}
			actor->SetControlMode(0, index);
		}
	}
	return 1;
}

// 0x4377b0
int ActorMoveFrom::Execute()
{
	bool stop = false;
	ScriptReference *a = g_scriptSelf;
	if (a->IsKindOf(TFBType_Actor())) {
		Placement *actor = ((Placement *)g_scriptSelf);
		if (actor != 0) {
			ScriptReference *owner;
			ScriptReference *set;
			float limit;
			ScriptReference *target = m_target.Resolve(&owner, &set, true);
			if (target != 0) {
				if (target != g_scriptSelf) {
					if (m_limit.EvalFloat(&limit)) {
						if (limit < ((PositionMeasurement *)target)->GetFloat(owner)) {
							stop = true;
						}
					}
				}
				if (!stop) {
					actor->SetDestination(((PositionMeasurement *)target)->GetVector(owner), 1);
					actor->PlayAnimation(m_anim - 2);
				}
			}
		}
	}
	if (!stop) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}

// Positions the actor's set cursor for a walk over `set`: kept when `res` is the set and the cursor is already on it (same stamp), else
// placed on the first member (`firstKind`) or on `res`. False when there is no such member (cursor cleared).
static bool PrepareCursor(SetCursor *cur, ReferenceSet *set, ScriptReference *res, int firstKind, int kind)
{
	if (res == (ScriptReference *)set) {
		if (cur->m_set == set && cur->m_stamp == set->m_stamp)
			return true;
		set->Next(firstKind, &cur->m_node);
	} else {
		cur->m_node = set->FindNode(res);
	}
	if (cur->m_node == 0) {
		cur->m_set = 0;
		return false;
	}
	cur->m_set = set;
	cur->m_kind = kind;
	cur->m_stamp = set->m_stamp;
	return true;
}

// MoveTo's test of one candidate: true when the target's float (a distance) is within `limitValue`; otherwise the actor heads for the candidate
// (unless an earlier one was already reached) and false is returned.
static bool ArrivedAt(Placement *actor, ScriptValue &limitValue, ScriptReference *res, ScriptReference *owner, bool alreadyArrived)
{
	float limit;
	if (res == 0 || !limitValue.EvalFloat(&limit))
		return false;
	if (((PositionMeasurement *)res)->GetFloat(owner) <= limit)
		return true;
	if (!alreadyArrived)
		actor->SetDestination(((PositionMeasurement *)res)->GetVector(owner), 0);
	return false;
}

// ActorMoveTo::Execute for a placed actor: resolve the target (a position, or a set walked through the actor's cursor in the limit's direction)
// and send the actor to the first candidate not yet reached. True when one is reached (the child block then runs); else plays the op's animation.
static bool MoveToTarget(ActorMoveTo *op, Placement *actor)
{
	ScriptReference *owner;
	ReferenceSet *set;
	ScriptReference *res = op->m_target.Resolve(&owner, (ScriptReference **)&set, true);   // a position, or a set of them
	if (res == 0)
		return false;
	if (res == g_forEachItem && op->m_target.bits == 0xffff4000) {
		set = (ReferenceSet *)g_forEachSet;
	} else {
		if (set == (ReferenceSet *)g_scriptSubset) {
			if (res == g_scriptSubset) {
				SetNode *node;
				set->Next(op->m_limit.opArg + 1, &node);
				res = node->m_object;
				owner = res;
			}
			set = (ReferenceSet *)g_findSubOwner;
		}
	}
	SetCursor *cur = ActorCursor(actor);
	int kind = op->m_limit.opArg + 5;
	int count = 1;
	bool useCursor = set != 0;   // a set: walk its members; otherwise `res` is the only candidate
	if (useCursor) {
		count = set->m_count;
		if (count == 0) {   // empty set: nothing to walk to
			actor->PlayAnimation(op->m_anim - 2);
			return false;
		}
		useCursor = PrepareCursor(cur, set, res, op->m_limit.opArg + 1, kind);
	}
	bool arrived = false;
	for (;;) {
		if (useCursor && cur->m_kind != kind) {
			cur->m_kind = kind;   // the cursor last walked another direction: adopt ours, one more step
			count++;
		} else {
			if (useCursor) {
				res = cur->m_node->m_object;
				owner = res;
				if (res == 0)
					break;
				unsigned sub = op->m_target.bits & 0x3f;
				if (sub != 0) {
					int n;
					ScriptReference *members = (ScriptReference *)res->GetMembers(&n);
					res = members[sub - 1].Deref(owner, 0);
				}
			}
			bool found = ArrivedAt(actor, op->m_limit, res, owner, arrived);
			arrived |= found;
			if (!found)
				break;
		}
		if (!useCursor)
			break;
		if (cur->m_set->Next(cur->m_kind, &cur->m_node) == 0 && cur->m_set->Next(cur->m_kind - 4, &cur->m_node) == 0)
			break;
		if (--count == 0)
			break;
	}
	if (!arrived)
		actor->PlayAnimation(op->m_anim - 2);
	return arrived;
}

// 0x4373a0
int ActorMoveTo::Execute()
{
	bool arrived = false;
	ScriptReference *a = g_scriptSelf;
	if (a->IsKindOf(TFBType_Actor())) {
		Placement *actor = ((Placement *)g_scriptSelf);
		if (actor != 0)
			arrived = MoveToTarget(this, actor);
	}
	if (!arrived) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}

// 0x435610
int ActorTeleportTo::Execute()
{
	ScriptReference *a = g_scriptSelf;
	if (!a->IsKindOf(TFBType_Placement()))
		return 1;
	Placement *actor = ((Placement *)g_scriptSelf);
	if (actor == 0)
		return 1;
	ScriptReference *owner;
	ReferenceSet *set;
	ScriptReference *res = m_target.Resolve(&owner, (ScriptReference **)&set, true);   // a position, or a set of them
	if (res == 0) {
		return 1;
	}
	if (res == g_forEachItem && m_target.bits == 0xffff4000) {
		set = (ReferenceSet *)g_forEachSet;
	} else {
		if (set == (ReferenceSet *)g_scriptSubset) {
			if (res == g_scriptSubset) {
				SetNode *node;
				set->Next(m_seconds.opArg + 1, &node);
				res = node->m_object;
				owner = res;
			}
			set = (ReferenceSet *)g_findSubOwner;
		}
	}
	SetCursor *cur = ActorCursor(actor);
	int kind = m_seconds.opArg + 5;
	int count = 1;
	bool valid = set != 0;
	if (valid) {
		count = set->m_count;
		if (count == 0) {
			return 1;
		}
		valid = PrepareCursor(cur, set, res, m_seconds.opArg + 1, kind);
	}
	do {
		if (!valid) {
			break;
		}
		if (cur->m_kind != kind) {
			count++;
			cur->m_kind = kind;
			res = 0;
		} else {
			ScriptReference *obj = cur->m_node->m_object;
			owner = obj;
			res = obj;
			if (obj != 0) {
				unsigned sub = m_target.bits & 0x3f;
				if (sub != 0) {
					int n;
					ScriptReference *members = (ScriptReference *)obj->GetMembers(&n);
					res = members[sub - 1].Deref(owner, 0);
				}
			}
		}
		if (cur->m_set->Next(cur->m_kind, &cur->m_node) == 0
			&& cur->m_set->Next(cur->m_kind - 4, &cur->m_node) == 0) {
			break;
		}
		if (--count == 0) {
			break;
		}
	} while (res == 0);
	if (res == 0) {
		return 1;
	}
	Vec3 *dst = ((PositionMeasurement *)res)->GetVector(owner);
	Vec3 *here = actor->GetVector(actor);
	Vec3 old = *here;
	bool moved;
	if (old.x == dst->x && old.y == dst->y && old.z == dst->z) {
		moved = false;
	} else {
		moved = true;
		actor->SetVector(dst, actor);
	}
	actor->SetDestination(dst, 0);
	{
	float f;
	if (m_facing.EvalAngle(&f, 0)) {
		actor->SetFacing(f, 1);
	}
	m_seconds.EvalFloat(&f);
	if (f != 0.0f) {
		if (moved) {
			actor->SetVector(&old, actor);
		}
		actor->SetDestinationTimed(dst, f);
	}
	}
	actor->SetAttachTarget(res);
	return 1;
}

// State change one ActorCutScene op (m_mode = `mode`) asks of the cut-scene state machine. True when the child block should run
// (state changed; mode 2: only when the old state was -1).
static bool ApplyCutSceneMode(unsigned char mode)
{
	int oldState = ((CutSceneSlot *)CutScene_StackSlot(0))->m_state;
	if (mode == 2) {
		if (oldState == 2)
			return false;
		CutScene_End();
		return oldState == -1;
	}
	if (oldState == 3 && mode != 3)
		return false;
	if (mode == 3 && oldState != 2) {
		CutScene_StackSlot(1);
		oldState = 2;
	}
	int newState = mode;
	CutSceneSlot *slot = (CutSceneSlot *)CutScene_StackSlot(0);
	if (slot->m_state == 2) {
		g_cutSceneCounter++;
		slot->m_counter = g_cutSceneCounter;
		slot->m_stamp = *Script::ScriptSystem::GetScriptTimePtr();
	}
	slot->m_state = newState;
	bool changed = oldState != mode;
	if (oldState == 2) {
		CutScene_ApplyState();
		bool flag = true;
		CutScene_ProcessActorSet(TFB_GetPlayerSet(), &flag);
		CutScene_ProcessActorSet(TFB_GetNPCSet(), &flag);
		CutScene_ProcessActorSet(TFB_GetInactiveSet(), &flag);
	}
	return changed;
}

// 0x4383b0
int ActorCutScene::Execute()
{
	if (!ApplyCutSceneMode(m_mode)) {
		SkipBlock();
		return 1;
	}
	return OpCode::Execute();
}

// View cone of a CheckFOV: half the opening angle and sin/cos of the heading and of its two edges (P = heading + half angle, M = heading - half angle), radians.
struct FovCone {
	float halfFov;
	float sinA, cosA;
	float sinP, cosP;
	float sinM, cosM;
};

// Is the offset (dx, dz) outside the view cone? `margin` is slack past the edges (18 for objects that are not plain positions).
// Up to 90 degrees half angle the cone is the intersection of three half planes (heading, two edges); wider, their union; from 180 degrees
// (or a NaN angle) nothing is excluded.
static bool OutsideCone(const FovCone &c, float dx, float dz, float margin)
{
	if (c.halfFov <= 1.5707964f) {
		float nm = -margin;
		if (dz * c.cosA + dx * c.sinA < nm)
			return true;
		if (dz * c.sinP - dx * c.cosP < nm)
			return true;
		if (dx * c.cosM - dz * c.sinM < nm)
			return true;
	} else if (c.halfFov < 3.1415927f) {
		if (dz * c.cosA + dx * c.sinA < -margin && dx * c.cosP - dz * c.sinP > margin && dz * c.sinM - dx * c.cosM > margin)
			return true;
	}
	return false;
}

// One CheckFOV candidate: unless it is the observer, inside the cone and passing the distance test (and the line of sight, if asked), it is
// inserted into `subset` keyed by its squared distance.
static void TestFovCandidate(CheckFOV *op, ScriptReference *actor, Vec3 *obs, const FovCone &cone, float range,
                             PositionMeasurement *obj, ReferenceSet &subset)
{
	if (obj == actor)
		return;
	Vec3 *p = obj->GetVector(obj);
	float dx = p->x - obs->x;
	float dz = p->z - obs->z;
	float margin = obj->GetType() != TFBType_Position() ? 18.0f : 0.0f;
	if (OutsideCone(cone, dx, dz, margin))
		return;
	float dy = p->y - obs->y;
	float dist2 = dy * dy + dx * dx + dz * dz;
	if (TFB_CompareFloat(dist2, op->m_range.opArg, range)) {
		if (op->m_halfFov.opArg != 1 || obs->LineOfSightTo(obj)) {
			subset.Insert(obj, dist2);
		}
	}
}

// 0x436050
int CheckFOV::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	ScriptReference *res = m_target.Resolve(&owner, &set, true);
	owner = g_scriptOwner;
	ScriptReference *actor = g_scriptSelf;   // not necessarily a position: checked first
	if (res != 0 && actor->IsKindOf(TFBType_Position())) {
		float heading;
		float halfFov;
		float range;
		if (m_heading.EvalAngle(&heading, 0) && m_halfFov.EvalAngle(&halfFov, 0) && m_range.EvalFloat(&range)) {
			ScriptReference *type;
			if (res != set) {
				type = res->GetType();
			} else {
				res = 0;
				type = set->GetBaseType();
			}
			ScriptSubset subset("subset", type, 0);
			range = range * range * 144.0f;   // feet -> squared distance
			heading = heading * 0.017453292f;   // degrees -> radians
			halfFov = halfFov * 0.008726646f;   // full angle in degrees -> half angle in radians
			FovCone cone;
			cone.halfFov = halfFov;
			cone.sinA = (float)sin(heading);
			cone.cosA = (float)cos(heading);
			cone.cosP = (float)cos(halfFov + heading);
			cone.sinP = (float)sin(halfFov + heading);
			cone.cosM = (float)cos(heading - halfFov);
			cone.sinM = (float)sin(heading - halfFov);
			Vec3 *obs = ((PositionMeasurement *)actor)->GetVector(owner);
			SetNode *iter;
			PositionMeasurement *obj;
			if (res == 0) {
				obj = (PositionMeasurement *)((ReferenceSet *)set)->Next(1, &iter);
			} else {
				iter = 0;
				obj = (PositionMeasurement *)res;
			}
			while (obj != 0) {
				TestFovCandidate(this, actor, obs, cone, range, obj, subset);
				if (iter == 0) {
					break;
				}
				obj = (PositionMeasurement *)((ReferenceSet *)set)->Next(5, &iter);
			}
			ScriptReference *saved = g_scriptSubset;
			g_scriptSubset = &subset;
			int r;
			if (subset.m_count == 0) {
				SkipBlock();
				r = 1;
			} else {
				r = OpCode::Execute();
			}
			g_scriptSubset = saved;
			return r;
		}
	}
	SkipBlock();
	return 1;
}

// 0x435280
const char *ActorPlayAnim::GetName()
{
	return "play animation";
}

// 0x4372d0
OpCode *ActorPlayAnim::Clone()
{
	return CloneToPool(this);
}

// 0x4372e0
ActorPlayAnim *ActorPlayAnim::CloneToPool(const ActorPlayAnim *src)
{
	void *mem = g_scriptPool;
	ActorPlayAnim *r = new (mem) ActorPlayAnim(*src);
	g_scriptPool += sizeof(ActorPlayAnim);
	return r;
}

// 0x435320
const char *ActorMoveTo::GetName()
{
	return "move to";
}

// 0x437320
OpCode *ActorMoveTo::Clone()
{
	return CloneToPool(this);
}

// 0x437330
ActorMoveTo *ActorMoveTo::CloneToPool(const ActorMoveTo *src)
{
	void *mem = g_scriptPool;
	ActorMoveTo *r = new (mem) ActorMoveTo(*src);
	g_scriptPool += sizeof(ActorMoveTo);
	return r;
}

// 0x4353d0
const char *ActorMoveFrom::GetName()
{
	return "move from";
}

// 0x437730
OpCode *ActorMoveFrom::Clone()
{
	return CloneToPool(this);
}

// 0x437740
ActorMoveFrom *ActorMoveFrom::CloneToPool(const ActorMoveFrom *src)
{
	void *mem = g_scriptPool;
	ActorMoveFrom *r = new (mem) ActorMoveFrom(*src);
	g_scriptPool += sizeof(ActorMoveFrom);
	return r;
}

// 0x435600
const char *ActorTeleportTo::GetName()
{
	return "teleport to";
}

// 0x436b20
OpCode *ActorTeleportTo::Clone()
{
	return CloneToPool(this);
}

// 0x436b30
ActorTeleportTo *ActorTeleportTo::CloneToPool(const ActorTeleportTo *src)
{
	void *mem = g_scriptPool;
	ActorTeleportTo *r = new (mem) ActorTeleportTo(*src);
	g_scriptPool += sizeof(ActorTeleportTo);
	return r;
}

// 0x435980
const char *ActorTurnTo::GetName()
{
	return "turn to";
}

// 0x437620
OpCode *ActorTurnTo::Clone()
{
	return CloneToPool(this);
}

// 0x437630
ActorTurnTo *ActorTurnTo::CloneToPool(const ActorTurnTo *src)
{
	void *mem = g_scriptPool;
	ActorTurnTo *r = new (mem) ActorTurnTo(*src);
	g_scriptPool += sizeof(ActorTurnTo);
	return r;
}

// 0x4359d0
const char *ActorRunAsPlayer::GetName()
{
	return "run as player";
}

// 0x436bb0
OpCode *ActorRunAsPlayer::Clone()
{
	return CloneToPool(this);
}

// 0x436bc0
ActorRunAsPlayer *ActorRunAsPlayer::CloneToPool(const ActorRunAsPlayer *src)
{
	void *mem = g_scriptPool;
	ActorRunAsPlayer *r = new (mem) ActorRunAsPlayer(*src);
	g_scriptPool += sizeof(ActorRunAsPlayer);
	return r;
}

// 0x435a10
const char *CheckFOV::GetName()
{
	return "check fov";
}

// 0x43d070
int CheckFOV::IsBlock()
{
	return 3;
}

// 0x437900
OpCode *CheckFOV::Clone()
{
	return CloneToPool(this);
}

// 0x437910
CheckFOV *CheckFOV::CloneToPool(const CheckFOV *src)
{
	void *mem = g_scriptPool;
	CheckFOV *r = new (mem) CheckFOV(*src);
	g_scriptPool += sizeof(CheckFOV);
	return r;
}

}  // namespace Script
