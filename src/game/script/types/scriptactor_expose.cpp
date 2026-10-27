// Script::ScriptActor Expose* members: GetName / GetInt / GetFloat / SetValue of the actor properties scripts can read and write.
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus.h"
#include <math.h>

// The three flag setters live on the actor-link class of modules/tfb/gameactor_links.cpp (GameActor itself); the owner is the actor.
typedef RWS::TFB::GameActor ActorLinkView;
// GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatZero;
static inline RWS::TFB::GameActor *Actor(Script::ScriptReference *owner)
{
	return static_cast<RWS::TFB::GameActor *>(owner);
}

namespace Script {

// 0x42a500
float ScriptActor::ExposeAnimationRate::GetFloat(ScriptReference *owner)
{
	Corpus *b = Actor(owner)->m_corpus;
	if (b)
		return b->m_model.GetRateScale();
	return g_floatZero;
}

// 0x42a520
void ScriptActor::ExposeAnimationRate::SetValue(float v, ScriptReference *owner)
{
	Corpus *b = Actor(owner)->m_corpus;
	if (b)
		b->m_model.SetRateScale(v);
}

// 0x42abd0
float ScriptActor::ExposeFacingObsolete::GetFloat(ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	float f = a->m_heading + a->m_facing;
	if (f < 0.0f)
		return f + 360.0f;
	if (f >= 360.0f)
		return f - 360.0f;
	return f;
}

// 0x42ac40
Vec3 *ScriptActor::ExposeOrigin::GetVector(ScriptReference *owner)
{
	return &Actor(owner)->m_origin;
}

// 0x42ae00
Vec3 *ScriptActor::ExposeDestination::GetVector(ScriptReference *owner)
{
	return &Actor(owner)->m_destination;
}

// 0x429e60
float ScriptActor::ExposeSpeed::ExposeVertComponent::GetFloat(ScriptReference *owner)
{
	return fabsf(Actor(owner)->m_velocity.y) * 0.083333336f;
}

// 0x429e80
float ScriptActor::ExposeSpeed::ExposeHorzComponent::GetFloat(ScriptReference *owner)
{
	const RwV3d &v = Actor(owner)->m_velocity;
	return (float)sqrt(v.x * v.x + v.z * v.z) * 0.083333336f;
}

// 0x429d90
int ScriptActor::ExposeHitPoints::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_hitPoints;
}

// 0x429da0
void ScriptActor::ExposeHitPoints::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_hitPoints = v;
}

// 0x42a210
int ScriptActor::ExposeFadePercent::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_fadePercent;
}

// 0x42a220
float ScriptActor::ExposeActivationRange::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_activationRange;
}

// 0x42a230
float ScriptActor::ExposeDeactivationRange::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_deactivationRange;
}

// 0x42a240
void ScriptActor::ExposeDeactivationRange::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_deactivationRange = v;
}

// 0x42a260
float ScriptActor::ExposeGroundAccel::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_groundAccel;
}

// 0x42a270
void ScriptActor::ExposeGroundAccel::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_groundAccel = v;
}

// 0x42a290
float ScriptActor::ExposeGroundDecel::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_groundDecel;
}

// 0x42a2a0
void ScriptActor::ExposeGroundDecel::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_groundDecel = v;
}

// 0x42a2c0
int ScriptActor::ExposeMoveType::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_moveType;
}

// 0x42a2d0
void ScriptActor::ExposeMoveType::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_moveType = v;
}

// 0x42a2f0
float ScriptActor::ExposeAirSpeed::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_airSpeed;
}

// 0x42a300
void ScriptActor::ExposeAirSpeed::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_airSpeed = v;
}

// 0x42a350
float ScriptActor::ExposeAirAccel::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_airAccel;
}

// 0x42a360
void ScriptActor::ExposeAirAccel::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_airAccel = v;
}

// 0x42a3b0
int ScriptActor::ExposeJumpDelay::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_jumpDelay;
}

// 0x42a3c0
void ScriptActor::ExposeJumpDelay::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_jumpDelay = v;
}

// 0x42a540
float ScriptActor::ExposeWeight::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_weight;
}

// 0x42a550
void ScriptActor::ExposeWeight::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_weight = v;
}

// 0x42a5a0
float ScriptActor::ExposeXOffset::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_xOffset;
}

// 0x42a5b0
float ScriptActor::ExposeYOffset::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_yOffset;
}

// 0x42a5c0
float ScriptActor::ExposeZOffset::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_zOffset;
}

// 0x42a5d0
float ScriptActor::ExposeXExtent::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_xExtent;
}

// 0x42a5e0
float ScriptActor::ExposeYExtent::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_yExtent;
}

// 0x42a5f0
float ScriptActor::ExposeZExtent::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_zExtent;
}

// 0x42a600
float ScriptActor::ExposeConeAngle::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_coneAngle;
}

// 0x42a610
void ScriptActor::ExposeConeAngle::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_coneAngle = v;
}

// 0x42ab70
float ScriptActor::ExposeGroundSpeed::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_groundSpeed;
}

// 0x42ab80
void ScriptActor::ExposeGroundSpeed::SetValue(float v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_groundSpeed = v;
}

// 0x42ac10
int ScriptActor::ExposeID::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_id;
}

// 0x42ac20
void ScriptActor::ExposeID::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_id = v;
}

// 0x42a630
int ScriptActor::ExposeConeColor::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_coneColor;
}

// 0x42a640
void ScriptActor::ExposeConeColor::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_params.m_coneColor = v;
}

// 0x42a680
void ScriptActor::ExposeConeColor::ExposeB::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_coneColor = (a->m_params.m_coneColor & 0xFF00FFFF) | ((unsigned char)v << 16);
}

// 0x42a6b0
int ScriptActor::ExposeConeColor::ExposeG::GetInt(ScriptReference *owner)
{
	return ((unsigned char *)&Actor(owner)->m_params.m_coneColor)[1];
}

// 0x42a6c0
void ScriptActor::ExposeConeColor::ExposeG::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_coneColor = (a->m_params.m_coneColor & 0xFFFF00FF) | ((unsigned char)v << 8);
}

// 0x42a6f0
int ScriptActor::ExposeConeColor::ExposeR::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_coneColor & 0xff;
}

// 0x42a710
void ScriptActor::ExposeConeColor::ExposeR::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_coneColor = (a->m_params.m_coneColor & 0xFFFFFF00) | (unsigned char)v;
}

// 0x42a730
int ScriptActor::ExposeConeColor::ExposeA::GetInt(ScriptReference *owner)
{
	return ((unsigned char *)&Actor(owner)->m_params.m_coneColor)[3];
}

// 0x42a740
void ScriptActor::ExposeConeColor::ExposeA::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	a->m_params.m_coneColor = (a->m_params.m_coneColor & 0xFFFFFF) | ((unsigned char)v << 24);
}

// 0x42b040
int ScriptActor::ExposeTintColor::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_tintColor;
}

// 0x42b050
void ScriptActor::ExposeTintColor::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_tintColor = v;
}

// 0x42b070
int ScriptActor::ExposeTintColor::ExposeR::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_tint[0];
}

// 0x42b080
void ScriptActor::ExposeTintColor::ExposeR::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_tint[0] = v;
}

// 0x42b0a0
int ScriptActor::ExposeTintColor::ExposeG::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_tint[1];
}

// 0x42b0b0
void ScriptActor::ExposeTintColor::ExposeG::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_tint[1] = v;
}

// 0x42b0d0
int ScriptActor::ExposeTintColor::ExposeB::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_tint[2];
}

// 0x42b0e0
void ScriptActor::ExposeTintColor::ExposeB::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_tint[2] = v;
}

// 0x42b100
int ScriptActor::ExposeTintColor::ExposeA::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_tint[3];
}

// 0x42b110
void ScriptActor::ExposeTintColor::ExposeA::SetValue(int v, ScriptReference *owner)
{
	Actor(owner)->m_tint[3] = v;
}

// 0x42aa00
int ScriptActor::ExposeWallClimber::GetInt(ScriptReference *owner)
{
	return (Actor(owner)->m_params.m_flags >> 1) & 1;
}

// 0x42aa20
void ScriptActor::ExposeWallClimber::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	if (v)
		a->m_params.m_flags |= 0x2;
	else
		a->m_params.m_flags &= ~0x2;
}

// 0x42aa50
int ScriptActor::ExposeIgnoresCutScenes::GetInt(ScriptReference *owner)
{
	return (Actor(owner)->m_params.m_flags >> 5) & 1;
}

// 0x42aa70
void ScriptActor::ExposeIgnoresCutScenes::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	if (v)
		a->m_params.m_flags |= 0x20;
	else
		a->m_params.m_flags &= ~0x20;
}

// 0x42a9a0
int ScriptActor::ExposeWallCollision::GetInt(ScriptReference *owner)
{
	return (Actor(owner)->m_params.m_flags >> 2) & 1;
}

// 0x42a9c0
void ScriptActor::ExposeWallCollision::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	if (v) {
		a->m_params.m_flags |= 4;
		if (a->m_corpus)
			(a->m_corpus)->m_flags &= ~0x4000;
	} else {
		a->m_params.m_flags &= ~4;
	}
}

// 0x42a950
void ScriptActor::ExposeMeshCollider::SetValue(int v, ScriptReference *owner)
{
	RWS::TFB::GameActor *a = Actor(owner);
	if (v)
		a->m_params.m_flags |= 0x100;
	else
		a->m_params.m_flags &= ~0x100;
}

// 0x42a910
int ScriptActor::ExposeSolidCollider::GetInt(ScriptReference *owner)
{
	return (unsigned char)((Placement *)owner)->IsSolidCollider();
}

// 0x42a930
int ScriptActor::ExposeMeshCollider::GetInt(ScriptReference *owner)
{
	return (unsigned char)((Placement *)owner)->IsMeshCollider();
}

// 0x435d70
const char *ScriptActor::ExposeHitPoints::GetName()
{
	return "health";
}

// 0x435d80
const char *ScriptActor::ExposeSpeed::GetName()
{
	return "current speed";
}

// 0x435d90
const char *ScriptActor::ExposeAltitude::GetName()
{
	return "altitude";
}

// 0x435da0
const char *ScriptActor::ExposeFadePercent::GetName()
{
	return "activation range fade percent";
}

// 0x435dd0
const char *ScriptActor::ExposeGroundSpeed::GetName()
{
	return "ground top speed";
}

// 0x435e10
const char *ScriptActor::ExposeMoveType::GetName()
{
	return "move type";
}

// 0x435ee0
const char *ScriptActor::ExposeWeight::GetName()
{
	return "weight";
}

// 0x435f00
const char *ScriptActor::ExposeXOffset::GetName()
{
	return "X offset";
}

// 0x435f60
const char *ScriptActor::ExposeConeLength::GetName()
{
	return "cone length";
}

// 0x436010
const char *ScriptActor::ExposeID::GetName()
{
	return "ID";
}

// 0x436040
const char *ScriptActor::ExposeTintColor::GetName()
{
	return "tint color";
}

// 0x436ab0
const char *ScriptActor::ExposeOrigin::GetName()
{
	return "origin";
}

// 0x436af0
const char *ScriptActor::ExposeConeAngle::GetName()
{
	return "cone angle";
}

// 0x436b10
const char *ScriptActor::ExposeSweepAngle::GetName()
{
	return "cone sweep offset";
}

// 0x42a320
float ScriptActor::ExposeAirTurnRate::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_airTurnRate;
}

// 0x42a330
void ScriptActor::ExposeAirTurnRate::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_airTurnRate = value;
}

// 0x42a380
float ScriptActor::ExposeAirDecel::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_airDecel;
}

// 0x42a390
void ScriptActor::ExposeAirDecel::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_airDecel = value;
}

// 0x42a3e0
float ScriptActor::ExposeInitialJumpVelocity::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_initialJumpVelocity;
}

// 0x42a3f0
void ScriptActor::ExposeInitialJumpVelocity::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_initialJumpVelocity = value;
}

// 0x42a410
int ScriptActor::ExposeCoastFrames::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_coastFrames;
}

// 0x42a420
void ScriptActor::ExposeCoastFrames::SetValue(int value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_coastFrames = value;
}

// 0x42a440
float ScriptActor::ExposeUpSpeed::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_upSpeed;
}

// 0x42a450
void ScriptActor::ExposeUpSpeed::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_upSpeed = value;
}

// 0x42a470
float ScriptActor::ExposeUpAccel::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_upAccel;
}

// 0x42a480
void ScriptActor::ExposeUpAccel::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_upAccel = value;
}

// 0x42a4a0
float ScriptActor::ExposeDownSpeed::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_downSpeed;
}

// 0x42a4b0
void ScriptActor::ExposeDownSpeed::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_downSpeed = value;
}

// 0x42a4d0
float ScriptActor::ExposeDownAccel::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_downAccel;
}

// 0x42a4e0
void ScriptActor::ExposeDownAccel::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_downAccel = value;
}

// 0x42a570
float ScriptActor::ExposeBounceRestitution::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_bounceRestitution;
}

// 0x42a580
void ScriptActor::ExposeBounceRestitution::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_bounceRestitution = value;
}

// 0x42a8b0
float ScriptActor::ExposeConeLength::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_coneLength;
}

// 0x42a8c0
void ScriptActor::ExposeConeLength::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_coneLength = value;
}

// 0x42a8e0
float ScriptActor::ExposeSweepAngle::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_sweepAngle;
}

// 0x42a8f0
void ScriptActor::ExposeSweepAngle::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_sweepAngle = value;
}

// 0x42aba0
float ScriptActor::ExposeGroundTurnRate::GetFloat(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_groundTurnRate;
}

// 0x42abb0
void ScriptActor::ExposeGroundTurnRate::SetValue(float value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_groundTurnRate = value;
}

// 0x42a770
int ScriptActor::ExposeBlipColor::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_blipColor;
}

// 0x42a780
void ScriptActor::ExposeBlipColor::SetValue(int value, ScriptReference *owner)
{
	Actor(owner)->m_params.m_blipColor = value;
}

// 0x42a7c0
void ScriptActor::ExposeBlipColor::ExposeB::SetValue(int value, ScriptReference *owner)
{
	unsigned &color = Actor(owner)->m_params.m_blipColor;
	color = (color & 0xFF00FFFF) | ((unsigned char)value << 16);
}

// 0x42a7f0
int ScriptActor::ExposeBlipColor::ExposeG::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_blip[1];
}

// 0x42a800
void ScriptActor::ExposeBlipColor::ExposeG::SetValue(int value, ScriptReference *owner)
{
	unsigned &color = Actor(owner)->m_params.m_blipColor;
	color = (color & 0xFFFF00FF) | ((unsigned char)value << 8);
}

// 0x42a830
int ScriptActor::ExposeBlipColor::ExposeR::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_blipColor & 0xff;
}

// 0x42a850
void ScriptActor::ExposeBlipColor::ExposeR::SetValue(int value, ScriptReference *owner)
{
	unsigned &color = Actor(owner)->m_params.m_blipColor;
	color = (color & 0xFFFFFF00) | (unsigned char)value;
}

// 0x42a870
int ScriptActor::ExposeBlipColor::ExposeA::GetInt(ScriptReference *owner)
{
	return Actor(owner)->m_params.m_blip[3];
}

// 0x42a880
void ScriptActor::ExposeBlipColor::ExposeA::SetValue(int value, ScriptReference *owner)
{
	unsigned &color = Actor(owner)->m_params.m_blipColor;
	color = (color & 0xFFFFFF) | ((unsigned char)value << 24);
}

// 0x42a980
int ScriptActor::ExposeMeshCollidee::GetInt(ScriptReference *owner)
{
	return (unsigned char)((Placement *)owner)->IsMeshCollidee();
}

// 0x42aaa0
int ScriptActor::ExposeNoShadow::GetInt(ScriptReference *owner)
{
	return (Actor(owner)->m_params.m_flags >> 3) & 1;
}

// 0x42aac0
int ScriptActor::ExposeGroundTilter::GetInt(ScriptReference *owner)
{
	return (Actor(owner)->m_params.m_flags >> 7) & 1;
}

// 0x42aae0
void ScriptActor::ExposeGroundTilter::SetValue(int value, ScriptReference *owner)
{
	if (value)
		Actor(owner)->m_params.m_flags |= 0x80;
	else
		Actor(owner)->m_params.m_flags &= ~0x80;
}

// 0x42ab10
int ScriptActor::ExposeVisibleOnRadar::GetInt(ScriptReference *owner)
{
	return (Actor(owner)->m_params.m_flags >> 4) & 1;
}

// 0x42ab30
void ScriptActor::ExposeVisibleOnRadar::SetValue(int value, ScriptReference *owner)
{
	if (value)
		Actor(owner)->m_params.m_flags |= 0x10;
	else
		Actor(owner)->m_params.m_flags &= ~0x10;
}

// 0x42aff0
int ScriptActor::ExposeIgnoresGroundColor::GetInt(ScriptReference *owner)
{
	return (Actor(owner)->m_params.m_flags >> 6) & 1;
}

// 0x42b010
void ScriptActor::ExposeIgnoresGroundColor::SetValue(int value, ScriptReference *owner)
{
	if (value)
		Actor(owner)->m_params.m_flags |= 0x40;
	else
		Actor(owner)->m_params.m_flags &= ~0x40;
}

// 0x42d190
void ScriptActor::ExposeSolidCollider::SetValue(int value, ScriptReference *owner)
{
	((ActorLinkView *)owner)->SetSolidCollider(value != 0);
}

// 0x42d1b0
void ScriptActor::ExposeMeshCollidee::SetValue(int value, ScriptReference *owner)
{
	((ActorLinkView *)owner)->SetMeshCollidee(value != 0);
}

// 0x42d1d0
void ScriptActor::ExposeNoShadow::SetValue(int value, ScriptReference *owner)
{
	((ActorLinkView *)owner)->SetNoShadow(value != 0);
}

}  // namespace Script
