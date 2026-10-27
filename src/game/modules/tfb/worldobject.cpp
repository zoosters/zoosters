#include "script/script.h"
#include "modules/tfb/worldobject.h"
#include "framework/core/csystemcommands.h"
#include "rw/rpplugins.h"
#include "tfb/level/level.h"
#include "script/symbolregistry.h"
#include "script/runtime.h"
#include "rw/rwcore.h"
#include "tfb/render/atomicflags.h"
#include "modules/tfb/gamecamera.h"

// GLOBAL: 0x5ccb94 WorldObject::vftable_for_CAttributeHandler
// GLOBAL: 0x5ccb8c WorldObject::vftable_for_CEventHandler
// 0x41d3e0
// 0x41c5f0
// GLOBAL: 0x618318 g_frameDt
// GLOBAL: 0x62a334 RWS::iMsgRunningPreTick
// GLOBAL: 0x4500b0 ?GetGuid@LevelResourceRef@@QBEXPAVRWSGUID@RWS@@@Z
extern const RwV3d g_propRotateAxis;
extern float g_frameDt;
// GLOBAL: 0x602ef8 g_worldObjectClassId
// GLOBAL: 0x61f65c g_curScript
// GLOBAL: 0x5ccb80 g_propRotateAxis
extern const char *g_worldObjectClassId;
void ClumpSetColourARGB(RpClump *clump, unsigned argb);          // 0x44ed40

// 0x41ced0
WorldObject::WorldObject(const RWS::CAttributePacket &attr)
	: m_clump(), m_dmorphAnim(0), m_morphAtomic(0), m_cameraNear(0), m_nearRadius(0), m_nearRadiusSq(0), m_margin(0), m_farRadiusSq(0), m_lastVisible(1), m_lastAnimForward(1), m_unk68(0),
	  m_lastAnimRate(100), m_unk70(0), m_unk74(0), m_unk78(0), m_unk80(0), m_unk84(0)
{
	m_clump = RWS::CreateClumpInWorldFromResource(attr, this);
	RpClump *clump = m_clump;
	RpWorld *world = RpClumpGetWorld(clump);
	if (world)
		RpWorldRemoveClump(world, clump);
	m_prop = new Script::ScriptProp(this);
	m_prop->AdjustRefCount(1);
	m_prop->m_animSet.SetTarget(m_prop->m_clump);
	m_prop->m_animSet.HandleSystemCommands(attr);
	unsigned n = m_prop->m_animSet.animData_.size();
	if (m_prop->m_animSet.HasTarget() && n != 0) {
		m_prop->m_animIndex = 0;
		m_prop->m_animSet.SetDstAnimation(m_prop->m_animIndex);
	}
	LoadDMorphAnimation(attr);
	LinkMsg(RWS::iMsgRunningPreTick, 0, 0x8000);
}

// 0x41c4f0
WorldObject::~WorldObject()
{
	RpClump *&pc = m_prop->m_clump;
	if (pc) {
		RWS::ClumpHelper::Destroy(pc);
		pc = 0;
	}
	m_prop->AdjustRefCount(-1);
	UnLinkMsg(RWS::iMsgRunningPreTick);
}

// 0x41c820
void WorldObject::HandleEvents(RWS::CMsg &msg)
{
	if ((RWS::CRegisteredMsgs *)RWS::iMsgRunningPreTick != msg.Id)
		return;
	if (m_nearRadius != 0.0f || m_margin != 0.0f) {
		if (m_cameraNear == 0) {
			volatile float delta[3];   // every component must be stored and reloaded (game: fstp [esp+0Ch] for z); volatile is the only spelling found
			CameraGetPosition((RwV3d *)delta);
			const RwV3d *pos = &RpClumpGetFrame(m_prop->m_clump)->modelling.pos;
			delta[0] -= pos->x;
			delta[1] -= pos->y;
			delta[2] -= pos->z;
			float distSq = (delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2];
			if (!(distSq < m_nearRadiusSq))
				return;
			m_cameraNear = 1;
		} else {
			volatile float delta[3];   // every component must be stored and reloaded (game: fstp [esp+0Ch] for z); volatile is the only spelling found
			CameraGetPosition((RwV3d *)delta);
			const RwV3d *pos = &RpClumpGetFrame(m_prop->m_clump)->modelling.pos;
			delta[0] -= pos->x;
			delta[1] -= pos->y;
			delta[2] -= pos->z;
			float distSq = (delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2];
			if (distSq > m_farRadiusSq) {
				m_cameraNear = 0;
				return;
			}
		}
	}
	int speed = m_prop->m_animRate;
	bool changed = false;
	if (speed != m_lastAnimRate) {
		m_lastAnimRate = speed;
		changed = true;
	}
	unsigned char dir = m_prop->m_animForward;
	if (m_lastAnimForward != dir) {
		m_lastAnimForward = dir;
		changed = true;
	}
	if (changed) {
		if (m_lastAnimForward == 0)
			speed = -speed;
		m_prop->SetAnimRate((float)speed * 0.01f);
	}
	m_animDone = !m_prop->m_animSet.Update(g_frameDt);
	if (m_lastVisible != m_prop->m_visible)
		ClumpSetIsVisible(m_prop->m_clump, m_lastVisible = m_prop->m_visible, -1.0f, -1.0f);
	if (m_dmorphAnim) {
		if (m_dmorphDirty) {
			SetupDMorph();
			m_dmorphDirty = 0;
		}
		RpDMorphAtomicAddTime(m_morphAtomic, g_frameDt);
	}
}

// Near-match of 0x41d0a0: layout only (case 0 slot address lea, case 4 redundant re-store of m_134 that the compiler drops, GetGuid lea vs
// mov+add). The rotate-axis tail copies a float as int (m_194f), a ternary like the game's fld/fstp.
void WorldObject::HandleAttributes(const RWS::CAttributePacket &attr)
{
	RWS::CAttributeHandler::HandleAttributes(attr);
	for (RWS::CAttributeCommandIterator it(attr, g_worldObjectClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0: {
			const char *nm = it->GetAs_RwChar_ptr();
			Script::ScriptProp *prop = m_prop;
			int idx = ((::Script::ScriptReference *)prop)->ResolveSymbol(prop->GetType(), nm, 0);
			if (idx >= 0) {
				Script::ScriptProp *obj;
				if (idx >= 0x3fc0d) {
					obj = (Script::ScriptProp *)g_curScript->m_locals[idx - 0x3fc0d];
				} else {
					SymbolSlot *slot = &(*g_symRegistryY)[idx];
					obj = (Script::ScriptProp *)(slot->obj & ~(uintptr_t)1);
				}
				if (obj != m_prop) {
					if (m_prop)
						delete m_prop;
					m_prop = obj;
					obj->AdjustRefCount(1);
					obj->m_owner = this;
				}
			}
			break;
		}
		case 1: {
			const char *data = it->GetAs_RwChar_ptr();
			RWS::RWSGUID guid;
			((const LevelResourceRef *)data)->GetGuid(&guid);
			m_guid = guid;
			break;
		}
		case 2:
			m_prop->m_animRate = (int)(it->GetAs_RwReal() * 100.0f);
			break;
		case 3:
			m_prop->SetAnimEndBehavior(it->GetAs_RwUInt32() != 0);
			break;
		case 4: {
			unsigned *tintSlot = &m_prop->m_tint;
			*tintSlot = it->GetAs_RwUInt32();
			unsigned tint = m_prop->m_tint;
			m_prop->m_tint = tint;
			if (m_prop->m_clump)
				ClumpSetColourARGB(m_prop->m_clump, tint);
			break;
		}
		case 5:
			m_cameraNear = it->GetAs_RwInt32();
			break;
		case 6:
			m_nearRadius = it->GetAs_RwReal();
			break;
		case 7:
			m_margin = it->GetAs_RwReal();
			break;
		}
	}
	RpClump *&pc = m_prop->m_clump;
	if (pc)
		RWS::ClumpHelper::HandleSystemCommands(*pc, attr);
	m_prop->m_animSet.HandleSystemCommands(attr);
	m_nearRadiusSq = m_nearRadius * m_nearRadius;
	m_farRadiusSq = (m_margin + m_nearRadius) * (m_margin + m_nearRadius);
	RwV3d axis = g_propRotateAxis;
	float angle;
	RwV3d centre;
	RwMatrixQueryRotate(&RpClumpGetFrame(m_prop->m_clump)->modelling, &axis, &angle, &centre);
	m_prop->m_facing = axis.y < 0.0f ? 360.0f - angle : angle;
}

// GLOBAL: 0x611634 RWS::CSystemCommands::pCSystemCommandsClassID

// RpClumpForAllAtomics callback: stops at the first atomic with a morph target and stores it in *found.
// 0x41c4b0
RpAtomic *FindDMorphAtomicCB(RpAtomic *atomic, void *found)
{
	RpGeometry *geom = RpAtomicGetGeometry(atomic);
	if (geom && RpDMorphGeometryGetNumDMorphTargets(geom) > 0 && RpDMorphGeometryGetDMorphTarget(geom, 0) != 0) {
		*(RpAtomic **)found = atomic;
		return 0;
	}
	return atomic;
}

// 0x41c7c0
void WorldObject::SetupDMorph()
{
	RpClumpGetNumAtomics(m_prop->m_clump);
	RpClumpForAllAtomics(m_prop->m_clump, FindDMorphAtomicCB, &m_morphAtomic);
	if (m_dmorphAnim) {
		m_numMorphTargets = RpDMorphAnimationGetNumDMorphTargets(m_dmorphAnim);
		if (m_morphAtomic)
			RpDMorphAtomicSetAnimation(m_morphAtomic, m_dmorphAnim);
	}
}


// 0x41c710
void WorldObject::LoadDMorphAnimation(const RWS::CAttributePacket &attr)
{
	for (RWS::CAttributeCommandIterator it(attr, RWS::CSystemCommands::pCSystemCommandsClassID); !it.IsFinished(); ++it) {
		if (it->GetCommandId() == 0) {
			const char *type;
			const char *aux;
			const void *guid = it->GetAs_RwChar_ptr();
			const void *res = RWS::CResourceManager::FindById((const RWS::RWSGUID *)guid, &type, 0, &aux);
			if (res && !rwstrcmp("rwID_DMORPHANIMATION", type)) {
				m_dmorphAnim = (RpDMorphAnimation *)res;
				m_dmorphDirty = 1;
			}
		}
	}
}
