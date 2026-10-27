#include "script/membertable.h"
#include <math.h>
#include "script/actor.h"
#include "script/bearings.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include "script/types/scriptactor.h"
#include "script/runtime.h"
#include "rw/rpworld.h"
#include "rw/rwcore.h"
#include "zoosters/memberrow.h"
#include <float.h>

extern int g_currentLevel;      // GLOBAL: 0x606a38 g_currentLevel
Vec3 *GetZeroVec3();               // returns a static zero vector (0x43fd10)
int *CutScene_StackSlot(int delta);   // 0x436c00
int RotationFromTo(const RwV3d *from, const RwV3d *to, RwMatrix *out);   // 0x449020 (cdecl): rotation from -> to

namespace Script {

static inline RWS::TFB::GameActor *Actor(ScriptReference *owner)
{
	return static_cast<RWS::TFB::GameActor *>(owner);
}

}  // namespace Script

using namespace Script;

// The callers declare their own mirror classes (ScriptActorBs in script/scriptactors.cpp, PaActor in modules/tfb/cprotoactor_attributes.cpp):
// GLOBAL: 0x429ca0 ?GetActivationRange@ScriptActorBs@Script@@QAEMXZ
// GLOBAL: 0x429ca0 ?GetActivationRange@PaActor@@QAEMXZ
// GLOBAL: 0x429d10 ?GetDeactivationRange@ScriptActorBs@Script@@QAEMXZ
// GLOBAL: 0x42ab60 ?SetScriptClockHold@ScriptActorBs@Script@@QAEXH@Z
// 0x429ca0
float ScriptActor::GetActivationRange()
{
	const RWS::TFB::ActorParams &params = Actor(this)->m_params;
	if (params.m_activationRange == 0.0f) {
		if (params.m_deactivationRange == 0.0f)
			return FLT_MAX;
		return 0.0f;
	}
	if (*CutScene_StackSlot(0) != 2)
		return g_floatZero;
	return params.m_activationRange * 12.0f;
}

// 0x429d10
float ScriptActor::GetDeactivationRange()
{
	const RWS::TFB::ActorParams &params = Actor(this)->m_params;
	float sum = params.m_deactivationRange + params.m_activationRange;
	if (sum != 0.0f && *CutScene_StackSlot(0) == 2)
		return sum * 12.0f;
	return FLT_MAX;
}

// 0x429dc0
float ScriptActor::ExposeHeadingObsolete::GetFloat(ScriptReference *owner)
{
	float h;
	float *heading = &h;
	const RwV3d &velocity = Actor(owner)->m_velocity;
	if (!HeadingFromXZ(velocity.x, velocity.z, heading)) {
		float f = Actor(owner)->m_heading + Actor(owner)->m_facing;
		if (f < 0.0f)
			return f + 360.0f;
		if (f >= 360.0f)
			return f - 360.0f;
		return f;
	}
	return *heading;
}

// 0x429e30
float ScriptActor::ExposeSpeed::ExposePitch::GetFloat(ScriptReference *owner)
{
	float p;
	float *pitch = &p;
	PitchFromXYZ(Actor(owner)->m_velocity.x, Actor(owner)->m_velocity.y, Actor(owner)->m_velocity.z, pitch);
	return *pitch;
}

// 0x429eb0
float ScriptActor::ExposeSpeed::GetFloat(ScriptReference *owner)
{
	Corpus *body = Actor(owner)->m_corpus;
	if (body)
		return RwV3dLength(reinterpret_cast<const RwV3d *>(&body->m_owner->m_velocity)) * (1.0f / 12.0f);
	return g_floatZero;
}

// 0x429ef0
Vec3 *ScriptActor::ExposeSpeed::GetVector(ScriptReference *owner)
{
	Corpus *body = Actor(owner)->m_corpus;
	if (body)
		return (Vec3 *)&body->m_owner->m_velocity;
	return GetZeroVec3();
}

// 0x429f20 Script::ScriptActor::ExposeSpeed::SetVector
// Level-12 hack (two fixed input vectors are remapped), then the speed vector is stored in the corpus body and, unless the corpus has flag 0x200,
// projected against the current velocity (LeaveGround(0,0) when the dot product is <= 0.1).
void ScriptActor::ExposeSpeed::SetVector(const Vec3 *v, ScriptReference *owner)
{
	// the game build's raw offsets: actor +0x2c8 m_corpus (asserted in gameactor.h), corpus +0x100 / +0x114 / +0x160 / +0x164 (corpus.h),
	// actor +0x234 / +0x238 = m_params (+0x1ac) .m_groundAccel / .m_groundDecel (+0x88 / +0x8c), actor +0x154 = Collider (+0x98) .m_velocity (+0xbc)
	static_assert(sizeof(void *) != 4 || 0x1ac + offsetof(RWS::TFB::ActorParams, m_groundAccel) == 0x234, "m_groundAccel is not at actor +0x234");
	static_assert(sizeof(void *) != 4 || 0x1ac + offsetof(RWS::TFB::ActorParams, m_groundDecel) == 0x238, "m_groundDecel is not at actor +0x238");
	static_assert(sizeof(void *) != 4 || 0x98 + offsetof(RWS::TFB::Collider, m_velocity) == 0x154, "Collider::m_velocity is not at actor +0x154");
	static_assert(sizeof(void *) != 4 || (sizeof(Script::ScriptActor) == 0x98 && offsetof(RWS::TFB::GameActor, m_matrix) == 0x98 + sizeof(RWS::TFB::Collider)), "Collider is not at +0x98 of the GameActor");
	Corpus *corpus = Actor(owner)->m_corpus;
	float w[3];
	w[0] = v->x;
	w[1] = v->y;
	w[2] = v->z;
	if (g_currentLevel == 12) {
		if (v->x != 0.0f || v->y != 0.0f || v->z != 0.0f) {
			if (v->z == 886.19995f && v->y == 0.0f && v->x == 0.0f) {
				corpus->m_owner->m_params.m_groundDecel = 10.0f;
				corpus->m_owner->m_params.m_groundAccel = 0.0f;
				w[2] = 905.0f;
			}
			if ((int)v->x == -100 && (int)v->y == -11 && (int)v->z == 65) {
				w[0] = -100.0f;
				w[1] = -60.0f;
				w[2] = 280.0f;
			}
		}
	}
	if (corpus) {
		if (!(corpus->m_flags & 0x200)) {
			const float *ground = (const float *)&corpus->m_groundNormal;
			float dot = (w[1] * ground[1] + w[0] * ground[0]) + w[2] * ground[2];
			if (dot > 0.1f) {
				corpus->LeaveGround(0, false);
			} else {   // the game projects when dot <= 0.1 (or NaN): jne after test ah,41h
				float t = -dot;
				float r[3];
				// game 0x429f20+0x190..0x1b9: t*vel1 and t*vel2 go through float temps ([esp+0Ch]/[esp+10h]) before the add; t*vel0 stays in the register
				float p1 = t * ground[1];
				float p2 = t * ground[2];
				r[0] = t * ground[0] + w[0];
				r[1] = p1 + w[1];
				r[2] = p2 + w[2];
				if ((r[1] * r[1] + r[0] * r[0]) + r[2] * r[2] <= 0.35999998f) {
					r[0] = 0.0f;
					r[1] = 0.0f;
					r[2] = 0.0f;
				}
				RWS::TFB::GameActor *actor = corpus->m_owner;
				*(Vec3 *)&static_cast<RWS::TFB::Collider &>(*actor).m_velocity = *(Vec3 *)r;
				char mat[0x40];   // RwMatrix: RotationFromTo writes 0x40 bytes
				float up[3];
				up[0] = 0.0f;
				up[1] = 1.0f;
				up[2] = 0.0f;
				if (RotationFromTo((const RwV3d *)ground, (const RwV3d *)up, (RwMatrix *)mat))
					RwV3dTransformVectors(reinterpret_cast<RwV3d *>(r), reinterpret_cast<const RwV3d *>(r), 1, reinterpret_cast<const RwMatrix *>(mat));
				corpus->m_velocity.x = r[0];
				corpus->m_velocity.z = r[2];
				return;
			}
		}
		RWS::TFB::GameActor *actor = corpus->m_owner;
		Vec3 *av = (Vec3 *)&static_cast<RWS::TFB::Collider &>(*actor).m_velocity;
		*av = *(Vec3 *)w;
		*(Vec3 *)&corpus->m_velocity = *av;
	}
}

// 0x42a1a0
float ScriptActor::ExposeAltitude::GetFloat(ScriptReference *owner)
{
	Corpus *body = Actor(owner)->m_corpus;
	if (body) {
		unsigned fl = body->m_flags;
		if ((fl & 0xa00) == 0x200) {
			if (!(fl & 0x400))
				return 9999.0f;
			float h = body->m_lastFrame.pos.y - body->m_contactPoint.y;
			if (h < 0.25f)
				h = 0.25f;
			return h * (1.0f / 12.0f);
		}
	}
	return g_floatZero;
}

// 0x42a660 Script::ScriptActor::ExposeConeColor::ExposeB::GetInt: layout only: game movsx+and 0FFh, cl folds to movzx; same value
// dead end: the game does `movsx; and eax,0FFh`, cl folds the mask into a movzx
int ScriptActor::ExposeConeColor::ExposeB::GetInt(ScriptReference *owner)
{
	int v = ((signed char *)&Actor(owner)->m_params.m_coneColor)[2];
	return v & 0xff;
}

// 0x42a7a0 Script::ScriptActor::ExposeBlipColor::ExposeB::GetInt: layout only: game movsx+and 0FFh, cl folds to movzx; same value
int ScriptActor::ExposeBlipColor::ExposeB::GetInt(ScriptReference *owner)
{
	return ((signed char *)&Actor(owner)->m_params.m_blipColor)[2] & 0xff;
}

// 0x42ab60
void ScriptActor::SetScriptClockHold(unsigned char v)
{
	Actor(this)->m_scriptClockHold = v;
}

// 0x42ad50
float ScriptActor::ExposeOrigin::ExposeVertComponent::GetFloat(ScriptReference *owner)
{
	ScriptReference *self = g_scriptSelf;
	if (!self->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *selfPos = ((VectorMeasurement *)g_scriptSelf)->GetVector(g_scriptSelf);
	return fabsf(Actor(owner)->m_origin.y - selfPos->y) * (1.0f / 12.0f);
}

// 0x42af40
float ScriptActor::ExposeDestination::ExposeVertComponent::GetFloat(ScriptReference *owner)
{
	ScriptReference *self = g_scriptSelf;
	if (!self->IsKindOf(TFBType_Position()))
		return g_floatZero;
	Vec3 *selfPos = ((VectorMeasurement *)g_scriptSelf)->GetVector(g_scriptSelf);
	return fabsf(Actor(owner)->m_destination.y - selfPos->y) * (1.0f / 12.0f);
}

namespace Script {
// GLOBAL: 0x5cff98 Script::g_vt_ScriptActor_ExposeBlipColor_ExposeA
extern char g_vt_ScriptActor_ExposeBlipColor_ExposeA[];
// GLOBAL: 0x5cffe0 Script::g_vt_ScriptActor_ExposeBlipColor_ExposeB
extern char g_vt_ScriptActor_ExposeBlipColor_ExposeB[];
// GLOBAL: 0x5d0028 Script::g_vt_ScriptActor_ExposeBlipColor_ExposeG
extern char g_vt_ScriptActor_ExposeBlipColor_ExposeG[];
// GLOBAL: 0x5d0070 Script::g_vt_ScriptActor_ExposeBlipColor_ExposeR
extern char g_vt_ScriptActor_ExposeBlipColor_ExposeR[];

// 0x4367a0
void *ScriptActor::ExposeBlipColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptActor::ExposeBlipColor::ExposeR, ScriptActor::ExposeBlipColor::ExposeG, ScriptActor::ExposeBlipColor::ExposeB, ScriptActor::ExposeBlipColor::ExposeA> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5d00b8 Script::g_vt_ScriptActor_ExposeConeColor_ExposeA
extern char g_vt_ScriptActor_ExposeConeColor_ExposeA[];
// GLOBAL: 0x5d0100 Script::g_vt_ScriptActor_ExposeConeColor_ExposeB
extern char g_vt_ScriptActor_ExposeConeColor_ExposeB[];
// GLOBAL: 0x5d0148 Script::g_vt_ScriptActor_ExposeConeColor_ExposeG
extern char g_vt_ScriptActor_ExposeConeColor_ExposeG[];
// GLOBAL: 0x5d0190 Script::g_vt_ScriptActor_ExposeConeColor_ExposeR
extern char g_vt_ScriptActor_ExposeConeColor_ExposeR[];

// 0x436820
void *ScriptActor::ExposeConeColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptActor::ExposeConeColor::ExposeR, ScriptActor::ExposeConeColor::ExposeG, ScriptActor::ExposeConeColor::ExposeB, ScriptActor::ExposeConeColor::ExposeA> row;
	*count = row.count;
	return row.data();
}
// GLOBAL: 0x5d01d8 Script::g_vt_ScriptActor_ExposeTintColor_ExposeA
extern char g_vt_ScriptActor_ExposeTintColor_ExposeA[];
// GLOBAL: 0x5d0220 Script::g_vt_ScriptActor_ExposeTintColor_ExposeB
extern char g_vt_ScriptActor_ExposeTintColor_ExposeB[];
// GLOBAL: 0x5d0268 Script::g_vt_ScriptActor_ExposeTintColor_ExposeG
extern char g_vt_ScriptActor_ExposeTintColor_ExposeG[];
// GLOBAL: 0x5d02b0 Script::g_vt_ScriptActor_ExposeTintColor_ExposeR
extern char g_vt_ScriptActor_ExposeTintColor_ExposeR[];

// 0x4368a0
void *ScriptActor::ExposeTintColor::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptActor::ExposeTintColor::ExposeR, ScriptActor::ExposeTintColor::ExposeG, ScriptActor::ExposeTintColor::ExposeB, ScriptActor::ExposeTintColor::ExposeA> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

Script::ScriptReference *TFBType_Actor();

namespace Script {

// 0x429d60
Vec3 *ScriptActor::GetVector(ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	if (a->m_corpus)
		return (Vec3 *)&a->m_corpus->m_lastFrame.pos;
	return (Vec3 *)&a->m_matrix.pos; // Vec3 = RwV3d layout (until the vector types are unified)
}

// 0x429d80
Vec3 *ScriptActor::GetVelocity(ScriptReference *owner)
{
	return (Vec3 *)&Actor(owner)->m_velocity; // Vec3 = RwV3d layout
}

}  // namespace Script

// 0x43aa60
Script::ScriptReference *Script::ScriptActor::GetType()
{
	return TFBType_Actor();
}


extern int DAT_0061f464;

namespace Script {

// 0x42cdc0
const char *ScriptActor::ExposeWayPoints::GetName()
{
	return "waypoints";
}

// 0x42cde0
const char *ScriptActor::ExposeAttachmentPoints::GetName()
{
	return "attach points";
}

// 0x42ce00
const char *ScriptActor::ExposeTurrets::GetName()
{
	return "turrets";
}

// 0x435de0
const char *ScriptActor::ExposeGroundTurnRate::GetName()
{
	return "ground turn rate";
}

// 0x435df0
const char *ScriptActor::ExposeGroundAccel::GetName()
{
	return "ground acceleration";
}

// 0x435e00
const char *ScriptActor::ExposeGroundDecel::GetName()
{
	return "ground deceleration";
}

// 0x435e20
const char *ScriptActor::ExposeAirSpeed::GetName()
{
	return "air top speed";
}

// 0x435e30
const char *ScriptActor::ExposeAirTurnRate::GetName()
{
	return "air turn rate";
}

// 0x435e40
const char *ScriptActor::ExposeAirAccel::GetName()
{
	return "air acceleration";
}

// 0x435e50
const char *ScriptActor::ExposeAirDecel::GetName()
{
	return "air deceleration";
}

// 0x435e60
const char *ScriptActor::ExposeJumpDelay::GetName()
{
	return "jump delay";
}

// 0x435e70
const char *ScriptActor::ExposeInitialJumpVelocity::GetName()
{
	return "initial jump velocity";
}

// 0x435e80
const char *ScriptActor::ExposeCoastFrames::GetName()
{
	return "jump coast frames";
}

// 0x435e90
const char *ScriptActor::ExposeUpSpeed::GetName()
{
	return "up top speed";
}

// 0x435ea0
const char *ScriptActor::ExposeUpAccel::GetName()
{
	return "up acceleration";
}

// 0x435eb0
const char *ScriptActor::ExposeDownSpeed::GetName()
{
	return "down top speed";
}

// 0x435ec0
const char *ScriptActor::ExposeDownAccel::GetName()
{
	return "down acceleration";
}

// 0x435ed0
const char *ScriptActor::ExposeAnimationRate::GetName()
{
	return "animation rate multiplier";
}

// 0x435ef0
const char *ScriptActor::ExposeBounceRestitution::GetName()
{
	return "bounce restitution";
}

// 0x435f10
const char *ScriptActor::ExposeYOffset::GetName()
{
	return "Y offset";
}

// 0x435f20
const char *ScriptActor::ExposeZOffset::GetName()
{
	return "Z offset";
}

// 0x435f30
const char *ScriptActor::ExposeXExtent::GetName()
{
	return "W extent";
}

// 0x435f40
const char *ScriptActor::ExposeYExtent::GetName()
{
	return "L extent";
}

// 0x435f50
const char *ScriptActor::ExposeZExtent::GetName()
{
	return "H extent";
}

// 0x435f70
const char *ScriptActor::ExposeBlipColor::GetName()
{
	return "blip color";
}

// 0x435f80
const char *ScriptActor::ExposeConeColor::GetName()
{
	return "cone color";
}

// 0x435f90
const char *ScriptActor::ExposeSolidCollider::GetName()
{
	return "solid collision";
}

// 0x435fa0
const char *ScriptActor::ExposeWallCollision::GetName()
{
	return "wall collision";
}

// 0x435fb0
const char *ScriptActor::ExposeVisibleOnRadar::GetName()
{
	return "visible on radar";
}

// 0x435fc0
const char *ScriptActor::ExposeWallClimber::GetName()
{
	return "wall climber";
}

// 0x435fe0
const char *ScriptActor::ExposeIgnoresGroundColor::GetName()
{
	return "ignores ground color";
}

// 0x435ff0
const char *ScriptActor::ExposeGroundTilter::GetName()
{
	return "tilts with ground";
}

// 0x436000
const char *ScriptActor::ExposeNoShadow::GetName()
{
	return "no shadow";
}

// 0x436020
const char *ScriptActor::ExposeMeshCollider::GetName()
{
	return "mesh collider";
}

// 0x436030
const char *ScriptActor::ExposeMeshCollidee::GetName()
{
	return "triangles collide";
}

// 0x436ac0
const char *ScriptActor::ExposeDestination::GetName()
{
	return "destination";
}

// 0x436ad0
const char *ScriptActor::ExposeHeadingObsolete::GetName()
{
	return "heading (OBSOLETE)";
}

// 0x436ae0
const char *ScriptActor::ExposeFacingObsolete::GetName()
{
	return "facing (OBSOLETE)";
}

}  // namespace Script

// 0x42d5f0
const char *ScriptActor_GetName()
{
	return "actor";
}

// 0x42f360
void *GetActorSerialCounter()
{
	return &DAT_0061f464;
}

// 0x435db0
const char *ExposeActivationRange_GetName()
{
	return "activation range";
}

// 0x435dc0
const char *ExposeDeactivationRange_GetName()
{
	return "deactivation range";
}

// 0x435fd0
const char *ExposeIgnoreCutScenes_GetName()
{
	return "cut-scene ignore";
}

// 0x436b00
int ExposeConeAngle_GetMax()
{
	return 360;
}


struct RwV3dW { float x, y, z; };
RpClump *ClumpSetIsVisible(RpClump *clump, bool flag, float from, float to);        // 0x44eca0 (tfb/render/atomicflags.cpp)

namespace Script {

// the offsets and extents (x 12 = game units) feed the collider's local box: half extents (x, z, y), offsets (x, z, y) as the
// matrix's pos, copied to the world box's extents (RwMatrix flags: 0xfffdffff clears rwMATRIXINTERNALIDENTITY 0x20000)
template<int VOL>
static __forceinline void RecomputeBox(RWS::TFB::GameActor *a)
{
	float hx = a->m_params.m_xExtent * 0.5f;
	a->m_localBox.m_extent[0] = hx * 12.0f;
	float hz = (VOL == 2 ? *(volatile float *)&a->m_params.m_zExtent : a->m_params.m_zExtent) * 0.5f;
	a->m_localBox.m_extent[1] = hz * 12.0f;
	float hy = (VOL == 3 ? *(volatile float *)&a->m_params.m_yExtent : a->m_params.m_yExtent) * 0.5f;
	a->m_localBox.m_extent[2] = hy * 12.0f;
	RwV3dW t;
	t.x = (VOL == 0 ? *(volatile float *)&a->m_params.m_xOffset : a->m_params.m_xOffset) * 12.0f;
	t.y = (VOL == 0 ? *(volatile float *)&a->m_params.m_zOffset : a->m_params.m_zOffset) * 12.0f;
	t.z = (VOL == 0 ? *(volatile float *)&a->m_params.m_yOffset : a->m_params.m_yOffset) * 12.0f;
	RwMatrixTranslate(reinterpret_cast<RwMatrix *>(&a->m_localBox.m_frame), reinterpret_cast<const RwV3d *>(&t), rwCOMBINEREPLACE);
	a->m_localBox.m_frame.flags &= 0xfffdffff;
	*(RwV3dW *)a->m_worldBox.m_extent = *(RwV3dW *)a->m_localBox.m_extent;
	if (a->m_controlMode < 2)
		a->UpdateBounds(&a->m_matrix, 0);
}

// the product is stored straight (redundant parentheses `(v * 12.0f)`, no local `t`); was with the local
// 0x42c170
void ScriptActor::ExposeXOffset::SetValue(float v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_xOffset = v;
	Corpus *b = a->m_corpus;
	a->m_localBox.m_frame.pos.x = (v * 12.0f);
	if (b) {
		b->m_flags &= ~0x4000;
		return;
	}
	RecomputeBox<0>(a);
}


// 0x42c290
void ScriptActor::ExposeYOffset::SetValue(float v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_yOffset = v;
	Corpus *b = a->m_corpus;
	a->m_localBox.m_frame.pos.z = (v * 12.0f);
	if (b) {
		b->m_flags &= ~0x4000;
		return;
	}
	RecomputeBox<0>(a);
}

// 0x42c3b0
void ScriptActor::ExposeZOffset::SetValue(float v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_zOffset = v;
	Corpus *b = a->m_corpus;
	a->m_localBox.m_frame.pos.y = (v * 12.0f);
	if (b) {
		b->m_flags &= ~0x4000;
		return;
	}
	RecomputeBox<0>(a);
}


// 0x42c4d0
void ScriptActor::ExposeXExtent::SetValue(float v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_xExtent = v;
	Corpus *b = a->m_corpus;
	float h = v * 0.5f;
	a->m_localBox.m_extent[0] = h * 12.0f;
	if (b) {
		b->m_flags &= ~0x4000;
		return;
	}
	RecomputeBox<1>(a);
}

// 0x42c5f0
void ScriptActor::ExposeYExtent::SetValue(float v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_yExtent = v;
	Corpus *b = a->m_corpus;
	float h = v * 0.5f;
	a->m_localBox.m_extent[2] = h * 12.0f;
	if (b) {
		b->m_flags &= ~0x4000;
		return;
	}
	RecomputeBox<3>(a);
}

// 0x42c720
void ScriptActor::ExposeZExtent::SetValue(float v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_zExtent = v;
	Corpus *b = a->m_corpus;
	float h = v * 0.5f;
	a->m_localBox.m_extent[1] = h * 12.0f;
	if (b) {
		b->m_flags &= ~0x4000;
		return;
	}
	RecomputeBox<2>(a);
}


// 0x42c090
void ScriptActor::ExposeFadePercent::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_fadePercent = v;
	RpClump *clump = (RpClump *)a->m_clump;
	if (clump) {
		float r = a->m_params.m_activationRange;
		ClumpSetIsVisible(clump, false, (r - (float)(unsigned)v * r * 0.01f) * 12.0f, a->m_params.m_activationRange * 12.0f);
	}
}

// 0x42c100
void ScriptActor::ExposeActivationRange::SetValue(float v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_activationRange = v;
	RpClump *clump = (RpClump *)a->m_clump;
	if (clump) {
		ClumpSetIsVisible(clump, false, (v - (float)(unsigned)a->m_params.m_fadePercent * v * 0.01f) * 12.0f, v * 12.0f);
	}
}

}  // namespace Script

namespace Script {
// 0x42cdd0
ScriptReference *ScriptActor::ExposeWayPoints::Deref(ScriptReference *owner, int flags)
{
	return (ScriptReference *)((ScriptActor *)owner)->m_wayPoints;
}

// 0x42cdf0
ScriptReference *ScriptActor::ExposeAttachmentPoints::Deref(ScriptReference *owner, int flags)
{
	return (ScriptReference *)((ScriptActor *)owner)->m_attachmentPoints;
}

// 0x42ce10
ScriptReference *ScriptActor::ExposeTurrets::Deref(ScriptReference *owner, int flags)
{
	return (ScriptReference *)((ScriptActor *)owner)->m_turrets;
}

}  // namespace Script


namespace Script {

// 0x42be70
// Destroys the three member sets (reverse order), then Placement's destructor. The sets are typed ReferenceSet storage in the header
// until their users (shadow_map.cpp, helpers_42c.cpp) switch to typed members; the call is the set's own destructor (0x402b20, no
// virtual dispatch: the vtables of the three sets differ, their destructor does not).
ScriptActor::~ScriptActor()
{
	reinterpret_cast<ReferenceSet *>(m_turrets)->ReferenceSet::~ReferenceSet();
	reinterpret_cast<ReferenceSet *>(m_attachmentPoints)->ReferenceSet::~ReferenceSet();
	reinterpret_cast<ReferenceSet *>(m_wayPoints)->ReferenceSet::~ReferenceSet();
}

}  // namespace Script

// The game shares these bodies with Placement_GetType 0x4039e0, ScriptActor_GetName 0x42d5f0 and ScriptActor_Reset 0x4386c0; each ScriptActor virtual gets its own.
// vf10 (slot 10) is 0x4386c0 itself, bound to the free function.

// GetName (slot 0) is 0x42d5f0 itself (ScriptActor_GetName above): a linker alias, not a body of its own
#if defined(_MSC_VER)
#endif
