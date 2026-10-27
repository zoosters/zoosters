// Camera type prototype and its reset.
#include "script/ops_camera.h"
#include "script/types/scriptmessage.h"
#include "script/types/scriptcamera.h"
using namespace Script;

// GLOBAL: 0x5d0a40 Script::CameraUse::vftable
// GLOBAL: 0x61ff70 ?s@?1??OpProto_CameraUse@@YAPAVScriptReference@Script@@XZ@4VCameraUse@3@A
// GLOBAL: 0x61ff88 ?$S1@?1??OpProto_CameraUse@@YAPAVScriptReference@Script@@XZ@4IA
// 0x438970 OpProto_CameraUse: same store set (s+4=1, vptr, bytes +0xC/+0xD=0, dwords +0x10/+0x14=0) in the same order, same guard/atexit; only diff is zeroing via al/eax register stores (A3, 5 B) vs the game's immediate stores (C7 05, 10 B), same values; no behaviour difference
ScriptReference *OpProto_CameraUse()
{
	static CameraUse s;
	return &s;
}

// vtable slot 10 of ScriptCamera: TypeRegistry_ResetAll calls it on every type
Script::ScriptReference *TFBType_Camera();   // 0x4389d0
extern Script::ScriptReference *g_cam61ff68;   // GLOBAL: 0x61ff68 g_cam61ff68 (slot 2 AdjustRefCount)
extern Script::ScriptReference *g_cam61ff6c;   // GLOBAL: 0x61ff6c g_cam61ff6c
#define CB3_CAM(p) ((ScriptCamera *)(p))

// 0x438b10
void ScriptCamera_Reset()
{
	ScriptReference *p;
	p = TFBType_Camera();
	const Script::ScriptReference *q4 = p;
	Script::ScriptReference *q2 = p;
	Script::ScriptReference *q3;
	q3 = q2;
	TFBUser_Camera()->SetReferent(p, 0);
	CB3_CAM(q3)->m_flags = 1;
	ScriptReference *r = CB3_CAM(q4)->m_lookAt.m_actor;
	CB3_CAM(q4)->m_lookAt.m_pendingName = 0;
	if (r != 0) {
		r->AdjustRefCount(-1);
		CB3_CAM(q3)->m_lookAt.m_actor = 0;
	}
	ScriptReference *q1 = CB3_CAM(q2)->m_lookFrom.m_actor;
	r = q1;
	CB3_CAM(q3)->m_lookFrom.m_pendingName = 0;
	if (r) {
		r->AdjustRefCount(-1);
		CB3_CAM(q3)->m_lookFrom.m_actor = 0;
	}
	CB3_CAM(q4)->m_horzFollow = 30;
	CB3_CAM(q3)->m_vertFollowAt = 0;
	CB3_CAM(q3)->m_vertFollowFrom = 0;
	CB3_CAM(q2)->m_roll = 0;
	if (g_cam61ff68 != 0) {
		g_cam61ff68->AdjustRefCount(-1);
		g_cam61ff68 = 0;
	}
	if (g_cam61ff6c) {
		g_cam61ff6c->AdjustRefCount(-1);
		g_cam61ff6c = 0;
	}
}
