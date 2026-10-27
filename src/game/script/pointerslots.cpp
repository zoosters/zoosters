#include "script/script.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "script/types/actorturret.h"
#include "script/types/scriptcamera.h"

Script::ScriptReference *TFBUser_Camera();   // 0x438a80

// GLOBAL: 0x61ff6c g_cam61ff6c
// GLOBAL: 0x61ff68 g_cam61ff68
extern Script::ScriptReference *g_cam61ff6c;   // the default "look from" object (slot 9 of ExposePosPointer)
extern Script::ScriptReference *g_cam61ff68;   // the default "look at" object
namespace Script {


// slot 9 of ExposePosPointer ("look from", the slot at camera+0x24) and ExposeTgtPointer ("look at", camera+0xc):
// the resolved actor, else the default slot object

// NOVTABLE: Script::CameraLookFromPointer
class CameraLookFromPointer {   // body never reads this
public:
	static ScriptReference *Deref(ScriptReference *owner, int flags);
};
// NOVTABLE: Script::CameraLookAtPointer
class CameraLookAtPointer {   // body never reads this
public:
	static ScriptReference *Deref(ScriptReference *owner, int flags);
};

// 0x4391c0
ScriptReference *CameraLookFromPointer::Deref(ScriptReference *owner, int flags)
{
	ScriptReference *r = ((ScriptCamera *)owner)->m_lookFrom.GetActor();
	if (r == 0)
		r = g_cam61ff6c;
	return r;
}

// 0x4391e0
ScriptReference *CameraLookAtPointer::Deref(ScriptReference *owner, int flags)
{
	ScriptReference *r = ((ScriptCamera *)owner)->m_lookAt.GetActor();
	if (r == 0)
		r = g_cam61ff68;
	return r;
}


static __forceinline void ReplaceDefault(ScriptReference *&slot, ScriptReference *v)
{
	if (v != slot) {
		if (slot)
			slot->AdjustRefCount(-1);
		slot = v;
		if (v)
			v->AdjustRefCount(1);
	}
}

static __forceinline void ReplaceResolved(ScriptReference *&slot, const char *&name, ScriptReference *v)
{
	if (v) {
		ScriptReference *old = slot;
		name = 0;
		if (v != old) {
			if (old)
				old->AdjustRefCount(-1);
			slot = v;
			v->AdjustRefCount(1);
		}
	}
}

// HACK: the game reads the resolved-actor field twice (test, then again as `old`); cl merges the two reads unless the first one is
// volatile. The two bodies are the same inline helpers: the else branch and the then branch use separate copies of `value`.
// NOVTABLE: Script::CamPosPointerSetW
class CamPosPointerSetW {   // body never reads this
public:
	static void SetValue(ScriptReference *value, ScriptCamera *owner);
};
// NOVTABLE: Script::CamTgtPointerSetW
class CamTgtPointerSetW {   // body never reads this
public:
	static void SetValue(ScriptReference *value, ScriptCamera *owner);
};

// 0x438db0
void CamPosPointerSetW::SetValue(ScriptReference *value, ScriptCamera *owner)
{
	if (*(ScriptReference * volatile *)&owner->m_lookFrom.m_actor) {
		ReplaceResolved(owner->m_lookFrom.m_actor, owner->m_lookFrom.m_pendingName, value);
	} else if (owner == (ScriptCamera *)TFBUser_Camera()->Deref(0, 0)) {
		ReplaceDefault(g_cam61ff6c, value);
	}
}

// 0x438e40
void CamTgtPointerSetW::SetValue(ScriptReference *value, ScriptCamera *owner)
{
	if (*(ScriptReference * volatile *)&owner->m_lookAt.m_actor) {
		ReplaceResolved(owner->m_lookAt.m_actor, owner->m_lookAt.m_pendingName, value);
	} else if (owner == (ScriptCamera *)TFBUser_Camera()->Deref(0, 0)) {
		ReplaceDefault(g_cam61ff68, value);
	}
}

// FOLDED: 0x438db0 Script::ScriptCamera::ExposePosPointer::SetReferent
void ScriptCamera::ExposePosPointer::SetReferent(ScriptReference *value, ScriptReference *owner) { CamPosPointerSetW::SetValue(value, (ScriptCamera *)owner); }
// FOLDED: 0x438e40 Script::ScriptCamera::ExposeTgtPointer::SetReferent
void ScriptCamera::ExposeTgtPointer::SetReferent(ScriptReference *value, ScriptReference *owner) { CamTgtPointerSetW::SetValue(value, (ScriptCamera *)owner); }
// FOLDED: 0x4391c0 Script::ScriptCamera::ExposePosPointer::Deref
ScriptReference *ScriptCamera::ExposePosPointer::Deref(ScriptReference *owner, int flags) { return CameraLookFromPointer::Deref(owner, flags); }
// FOLDED: 0x4391e0 Script::ScriptCamera::ExposeTgtPointer::Deref
ScriptReference *ScriptCamera::ExposeTgtPointer::Deref(ScriptReference *owner, int flags) { return CameraLookAtPointer::Deref(owner, flags); }
// FOLDED: 0x438960 Script::ScriptCamera::ExposeTgtPointer::GetName
const char *ScriptCamera::ExposeTgtPointer::GetName() { return "look at"; }

}  // namespace Script

// 0x42df80 stdcall(obj, arg): slot 9 of ActorTurret::ExposeLookAtPointer (vtable 0x5cd924) resolves the turret's PointerTarget
// 0x42df80
Script::ScriptReference *__stdcall ActorTurret_ExposeLookAtPointer_vf09(char *owner, int unused)
{
	return (&((Script::ActorTurret *)owner)->m_lookAt)->GetActor();
}

// Defined here, not aliased: as an x64 virtual call `this` is the first argument, so the owner arrived as the member object and
// GetActor read a PointerTarget from the member table (fault at frame 7 of every level but 0).
// FOLDED: 0x42df80 Script::ActorTurret::ExposeLookAtPointer::Deref
Script::ScriptReference *Script::ActorTurret::ExposeLookAtPointer::Deref(Script::ScriptReference *owner, int flags)
{
	return (&((Script::ActorTurret *)owner)->m_lookAt)->GetActor();
}
