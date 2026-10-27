#include "script/membertable.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "modules/tfb/gamecamera.h"
#include "script/runtime.h"
#include "script/script.h"
#include "script/symbolregistry.h"
#include "framework/toolkits/animation/canimset.h"
#include "script/types/anglemeasurement.h"
#include "script/types/scriptmedia.h"
#include "script/types/colormeasurement.h"
#include "script/types/twodmeasurement.h"
#include "zoosters/memberrow.h"

// GLOBAL: 0x5ca5a8 g_floatZero

using namespace Script;

// 0x41d490
int ScriptCamera::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		if (this != TFBType_Camera())
			delete this;
		return 0;
	}
	return m_refCount;
}

// 0x41d570
int ScriptCamera::ExposePercentComplete::GetInt(ScriptReference *owner)
{
	ScriptReference *cam = GetCameraObject();
	if (cam && cam == owner)
		return (int)(CameraPercent() * 100.0f);
	return 0;
}

// 0x41d5a0
float ScriptCamera::ExposeTimeRemaining::GetFloat(ScriptReference *owner)
{
	ScriptReference *cam = GetCameraObject();
	if (cam && cam == owner)
		return CameraTimeRemaining();
	return g_floatZero;
}

// 0x41d5c0
int ScriptCamera::ExposeVertFollowAt::GetInt(ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)GetCameraObject();
	if (cam)
		return cam->m_vertFollowAt;
	return 0;
}

// 0x41d5e0
void ScriptCamera::ExposeVertFollowAt::SetValue(int value, ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)GetCameraObject();
	if (cam)
		cam->m_vertFollowAt = value;
}

// 0x41d600
int ScriptCamera::ExposeVertFollowFrom::GetInt(ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)GetCameraObject();
	if (cam)
		return cam->m_vertFollowFrom;
	return 0;
}

// 0x41d620
void ScriptCamera::ExposeVertFollowFrom::SetValue(int value, ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)GetCameraObject();
	if (cam)
		cam->m_vertFollowFrom = value;
}

// 0x41d640
int ScriptCamera::ExposeHorzFollow::GetInt(ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)GetCameraObject();
	if (cam)
		return cam->m_horzFollow;
	return 0;
}

// 0x41d660
void ScriptCamera::ExposeHorzFollow::SetValue(int value, ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)GetCameraObject();
	if (cam)
		cam->m_horzFollow = value;
}

// 0x41d680
float ScriptCamera::ExposeFacing::GetFloat(ScriptReference *owner)
{
	ScriptReference *cam = GetCameraObject();
	if (cam && cam == owner)
		return CameraFacing();
	return g_floatZero;
}

// 0x41d6a0
void ScriptCamera::ExposeFacing::SetValue(float value, ScriptReference *owner)
{
	ScriptReference *cam = GetCameraObject();
	if (cam && cam == owner)
		SetCameraFacing(cam, value);
}

// 0x41d6f0
const char *ScriptCamera::GetName()
{
	return "camera";
}

// 0x41d700
ScriptReference *ScriptCamera::GetType()
{
	return TFBType_Camera();
}

namespace Script {
// GLOBAL: 0x5d0f10 Script::g_vt_ScriptCamera_ExposeFOV
extern char g_vt_ScriptCamera_ExposeFOV[];
// GLOBAL: 0x5d1008 Script::g_vt_ScriptCamera_ExposeFacing
extern char g_vt_ScriptCamera_ExposeFacing[];
// GLOBAL: 0x5d0fb0 Script::g_vt_ScriptCamera_ExposeHorzFollow
extern char g_vt_ScriptCamera_ExposeHorzFollow[];
// GLOBAL: 0x5d0ce8 Script::g_vt_ScriptCamera_ExposeLazy
extern char g_vt_ScriptCamera_ExposeLazy[];
// GLOBAL: 0x5d0d38 Script::g_vt_ScriptCamera_ExposePercentComplete
extern char g_vt_ScriptCamera_ExposePercentComplete[];
// GLOBAL: 0x5d0dd8 Script::g_vt_ScriptCamera_ExposePitchControl
extern char g_vt_ScriptCamera_ExposePitchControl[];
// GLOBAL: 0x5d0e68 Script::g_vt_ScriptCamera_ExposePosHeadingOffs
extern char g_vt_ScriptCamera_ExposePosHeadingOffs[];
// GLOBAL: 0x5d0bb0 Script::g_vt_ScriptCamera_ExposePosHeadingRel
extern char g_vt_ScriptCamera_ExposePosHeadingRel[];
// GLOBAL: 0x5d0af0 Script::g_vt_ScriptCamera_ExposePosHorzOffs
extern char g_vt_ScriptCamera_ExposePosHorzOffs[];
// GLOBAL: 0x5d0a88 Script::g_vt_ScriptCamera_ExposePosPointer
extern char g_vt_ScriptCamera_ExposePosPointer[];
// GLOBAL: 0x5d0b50 Script::g_vt_ScriptCamera_ExposePosVertOffs
extern char g_vt_ScriptCamera_ExposePosVertOffs[];
// GLOBAL: 0x5d10c0 Script::g_vt_ScriptCamera_ExposeRoll
extern char g_vt_ScriptCamera_ExposeRoll[];
// GLOBAL: 0x5d0ec8 Script::g_vt_ScriptCamera_ExposeTgtHeadingOffs
extern char g_vt_ScriptCamera_ExposeTgtHeadingOffs[];
// GLOBAL: 0x5d0ca0 Script::g_vt_ScriptCamera_ExposeTgtHeadingRel
extern char g_vt_ScriptCamera_ExposeTgtHeadingRel[];
// GLOBAL: 0x5d0c10 Script::g_vt_ScriptCamera_ExposeTgtHorzOffs
extern char g_vt_ScriptCamera_ExposeTgtHorzOffs[];
// GLOBAL: 0x5d0ac0 Script::g_vt_ScriptCamera_ExposeTgtPointer
extern char g_vt_ScriptCamera_ExposeTgtPointer[];
// GLOBAL: 0x5d0c58 Script::g_vt_ScriptCamera_ExposeTgtVertOffs
extern char g_vt_ScriptCamera_ExposeTgtVertOffs[];
// GLOBAL: 0x5d0d90 Script::g_vt_ScriptCamera_ExposeTimeRemaining
extern char g_vt_ScriptCamera_ExposeTimeRemaining[];
// GLOBAL: 0x5d0f58 Script::g_vt_ScriptCamera_ExposeVertFollowAt
extern char g_vt_ScriptCamera_ExposeVertFollowAt[];
// GLOBAL: 0x5d1060 Script::g_vt_ScriptCamera_ExposeVertFollowFrom
extern char g_vt_ScriptCamera_ExposeVertFollowFrom[];

// 0x439200
void *ScriptCamera::GetMembers(int *count)
{
	static zoosters::MemberRow<ScriptCamera::ExposePosPointer, ScriptCamera::ExposePosHorzOffs, ScriptCamera::ExposePosVertOffs, ScriptCamera::ExposePosHeadingOffs, ScriptCamera::ExposePosHeadingRel, ScriptCamera::ExposeTgtPointer, ScriptCamera::ExposeTgtHorzOffs, ScriptCamera::ExposeTgtVertOffs, ScriptCamera::ExposeTgtHeadingOffs, ScriptCamera::ExposeTgtHeadingRel, ScriptCamera::ExposeFOV, ScriptCamera::ExposeLazy, ScriptCamera::ExposePercentComplete, ScriptCamera::ExposeTimeRemaining, ScriptCamera::ExposeVertFollowAt, ScriptCamera::ExposeHorzFollow, ScriptCamera::ExposeFacing, ScriptCamera::ExposeVertFollowFrom, ScriptCamera::ExposePitchControl, ScriptCamera::ExposeRoll> row;
	*count = row.count;
	return row.data();
}

}  // namespace Script

// ---- PointerTarget: the pending actor name of a look-at / look-from target (0x41d6c0-0x41d920)
// GLOBAL: 0x61f65c g_curScript
// GLOBAL: 0x61f878 g_symRegistryY

Script::ScriptReference *TFBType_Actor();     // 0x436e30

namespace Script {

// 0x41d6c0
void PointerTarget::ClearActor()
{
	m_pendingName = 0;
	if (m_actor) {
		m_actor->AdjustRefCount(-1);
		m_actor = 0;
	}
}

// 0x41d920
ScriptReference *PointerTarget::GetActor()
{
	const char *name = m_pendingName;
	if (name) {
		ScriptReference *actorType = (ScriptReference *)TFBType_Actor();   // the actor prototype object resolves symbol names to registry indices
		int symIndex = actorType->ResolveSymbol(actorType->GetType(), name, 0);
		if (symIndex >= 0) {
			ScriptReference *obj;
			if (symIndex >= 0x3fc0d)
				obj = g_curScript->m_locals[symIndex - 0x3fc0d];
			else {
				SymbolSlot *slot = &(*g_symRegistryY)[symIndex];
				if (slot->obj & 1)
					obj = 0;
				else
					obj = (ScriptReference *)(slot->obj & ~(uintptr_t)1);
			}
			m_pendingName = 0;
			if (obj != m_actor) {
				if (m_actor)
					m_actor->AdjustRefCount(-1);
				m_actor = obj;
				if (obj)
					obj->AdjustRefCount(1);
			}
		}
		m_pendingName = 0;
	}
	return m_actor;
}

}  // namespace Script

namespace Script {

// 0x438ba0
const char *ScriptCamera::ExposePosHorzOffs::GetName()
{
	return "look from horizontal offset";
}

// 0x438ce0
const char *ScriptCamera::ExposeLazy::GetName()
{
	return "laziness";
}

// 0x438d50
const char *ScriptCamera::ExposePitchControl::GetName()
{
	return "pitch control";
}

// 0x438bb0
float ScriptCamera::ExposePosHorzOffs::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookFrom.m_horzOffs;
}

// 0x438bc0
void ScriptCamera::ExposePosHorzOffs::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookFrom.m_horzOffs = value;
}

// 0x438be0
float ScriptCamera::ExposePosVertOffs::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookFrom.m_vertOffs;
}

// 0x438bf0
void ScriptCamera::ExposePosVertOffs::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookFrom.m_vertOffs = value;
}

// 0x438c50
float ScriptCamera::ExposeTgtHorzOffs::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookAt.m_horzOffs;
}

// 0x438c60
void ScriptCamera::ExposeTgtHorzOffs::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookAt.m_horzOffs = value;
}

// 0x438c80
float ScriptCamera::ExposeTgtVertOffs::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookAt.m_vertOffs;
}

// 0x438c90
void ScriptCamera::ExposeTgtVertOffs::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookAt.m_vertOffs = value;
}

// 0x439090
float ScriptCamera::ExposePosHeadingOffs::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookFrom.m_headingOffs;
}

// 0x4390a0
void ScriptCamera::ExposePosHeadingOffs::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookFrom.m_headingOffs = value;
}

// 0x4390c0
float ScriptCamera::ExposeTgtHeadingOffs::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookAt.m_headingOffs;
}

// 0x4390d0
void ScriptCamera::ExposeTgtHeadingOffs::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookAt.m_headingOffs = value;
}

// 0x439100
float ScriptCamera::ExposeFOV::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_fov;
}

// 0x439110
void ScriptCamera::ExposeFOV::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_fov = value;
}

// 0x439180
float ScriptCamera::ExposeRoll::GetFloat(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_roll;
}

// 0x439190
void ScriptCamera::ExposeRoll::SetValue(float value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_roll = value;
}

// 0x438c10
int ScriptCamera::ExposePosHeadingRel::GetInt(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookFrom.m_headingRel;
}

// 0x438c20
void ScriptCamera::ExposePosHeadingRel::SetValue(int value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookFrom.m_headingRel = value != 0;
}

// 0x438cb0
int ScriptCamera::ExposeTgtHeadingRel::GetInt(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_lookAt.m_headingRel;
}

// 0x438cc0
void ScriptCamera::ExposeTgtHeadingRel::SetValue(int value, ScriptReference *owner)
{
	((ScriptCamera *)owner)->m_lookAt.m_headingRel = value != 0;
}

// 0x438cf0
int ScriptCamera::ExposeLazy::GetInt(ScriptReference *owner)
{
	return ((ScriptCamera *)owner)->m_flags & 1;
}

// 0x438d00
void ScriptCamera::ExposeLazy::SetValue(int value, ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)owner;
	if (value)
		cam->m_flags |= 1;
	else
		cam->m_flags &= ~1;
}

// 0x438d60
int ScriptCamera::ExposePitchControl::GetInt(ScriptReference *owner)
{
	return ~(((ScriptCamera *)owner)->m_flags >> 1) & 1;
}

// 0x438d80
void ScriptCamera::ExposePitchControl::SetValue(int value, ScriptReference *owner)
{
	ScriptCamera *cam = (ScriptCamera *)owner;
	if (value)
		cam->m_flags &= ~2;
	else
		cam->m_flags |= 2;
}

}  // namespace Script


extern Script::ScriptReference *g_cam61ff68;   // GLOBAL: 0x61ff68 g_cam61ff68
extern Script::ScriptReference *g_cam61ff6c;   // GLOBAL: 0x61ff6c g_cam61ff6c
extern RwV3d g_camLookAt;

namespace Script {

// 0x438940
const char *ScriptCamera::ExposePosPointer::GetName()
{
	return "look from";
}

// 0x438bd0
const char *ScriptCamera::ExposePosVertOffs::GetName()
{
	return "look from vertical offset";
}

// 0x438c00
const char *ScriptCamera::ExposePosHeadingRel::GetName()
{
	return "look from heading relative";
}

// 0x439080
const char *ScriptCamera::ExposePosHeadingOffs::GetName()
{
	return "look from heading offset";
}

// 0x439120
const char *ScriptCamera::ExposeVertFollowAt::GetName()
{
	return "Vertical Follow At";
}

// 0x439130
const char *ScriptCamera::ExposeHorzFollow::GetName()
{
	return "Horizontal Follow";
}

// 0x439140
const char *ScriptCamera::ExposeFacing::GetName()
{
	return "Current Facing";
}

// 0x439150
const char *ScriptCamera::ExposeVertFollowFrom::GetName()
{
	return "Vertical Follow From";
}

// 0x4391a0
const char *ScriptCamera::ExposeCameraLocation::GetName()
{
	return "camera position";
}

}  // namespace Script

// 0x438910
void *ScriptCamera_DefaultLookAtSlot()
{
	return &g_cam61ff68;
}

// 0x438920
void *ScriptCamera_DefaultLookFromSlot()
{
	return &g_cam61ff6c;
}

// 0x438960
const char *ExposeLookAtPointer_GetName()
{
	return "look at";
}

// 0x438c40
const char *ExposeLookAtHorzOffs_GetName()
{
	return "look at horizontal offset";
}

// 0x438c70
const char *ExposeLookAtVertOffs_GetName()
{
	return "look at vertical offset";
}

// 0x438ca0
const char *ExposeLookAtHeadingRel_GetName()
{
	return "look at heading relative";
}

// 0x438d30
const char *ExposePercentComplete_GetName()
{
	return "percent complete";
}

// 0x438d40
const char *ExposeTimeRemaining_GetName()
{
	return "time remaining";
}

// 0x4390b0
const char *ExposeLookAtHeadingOffs_GetName()
{
	return "look at heading offset";
}

// 0x4390e0
const char *ExposeFOV_GetName()
{
	return "field of view";
}

// 0x4390f0
int ExposeFOV_GetMax()
{
	return 180;
}

// 0x439160
int ExposeVertFollow_GetMax()
{
	return 300;
}

// 0x439170
const char *ExposeRoll_GetName()
{
	return "roll";
}

// 0x4391b0
void *__stdcall ExposeCameraLocation_GetVector(Script::ScriptReference *owner)
{
	return &g_camLookAt;
}

// slot 17 of the member is the free function above


// 0x41d8b0
// The look-from slot (+0x24) is released first, then the look-at slot (+0x0c): each drops its pending name and its actor reference.
ScriptCamera::~ScriptCamera()
{
	m_lookFrom.ClearActor();
	m_lookAt.ClearActor();
}

// The game's slot 10 is the prototype's reset (ScriptCamera_Reset 0x438b10, camera_proto.cpp); vf10 IS that function (a linker alias).
#if defined(_MSC_VER)
#endif
