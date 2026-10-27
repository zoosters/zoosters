#include <string.h>
#include "script/script.h"
#include "framework/core/csystemcommands.h"
#include "modules/tfb/cameradata.h"
#include "framework/core/cresourcemanager.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
#include "script/runtime.h"
#include "script/symbolregistry.h"
#include "framework/stringpool.h"
#include "rw/rwcore.h"
#include "script/actor.h"
// GLOBAL: 0x602f1c g_cameraDataClassId
extern const char *g_cameraDataClassId;
// GLOBAL: 0x611634 RWS::CSystemCommands::pCSystemCommandsClassID

void CameraSetHeading(float v);

namespace RWS { namespace TFB {
// 0x41d710
void CameraData::HandleSystemCommands(const CAttributePacket &attr)
{
	for (CAttributeCommandIterator it(attr, RWS::CSystemCommands::pCSystemCommandsClassID); !it.IsFinished(); ++it) {
		if (it->GetCommandId() == 0) {
			const char *type;
			const char *aux;
			const void *guid = it->GetAs_RwChar_ptr();
			const void *res = CResourceManager::FindById((const RWSGUID *)guid, &type, 0, &aux);
			if (res && !rwstrcmp("rwID_HANIMANIMATION", type))
				m_animation = (intptr_t)res;
		}
	}
}


// Disabled (below the bar): 668 B vs 672 B. The game keeps a stack copy of the vector in the two Vec3 cases (2 and 5) and stores the fields
// from registers; ours reloads from the copy.
struct V3i { float x, y, z; };
union FI { int i; float f; };

// Near-match of 0x41d9c0 (668 B vs 672 B): case 5 register order (the game loads x, y, z into ecx, edx, eax and overwrites the pointer
// last), setne al vs dl in case 3, edx / eax in case 4.
// 0x41d9c0
void CameraData::HandleAttributes(const CAttributePacket &attr)
{
	CameraData::HandleSystemCommands(attr);
	CAttributeCommandIterator it(attr, g_cameraDataClassId);
	::Script::ScriptCamera *cam = m_cam;
	for (; !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0: {
			const CAttributeCommand *cmd = it.operator->();
			int idx = ((::Script::ScriptReference *)cam)->ResolveSymbol(cam->GetType(), cmd->GetAs_RwChar_ptr(), 0);
			if (idx < 0)
				break;
			::Script::ScriptReference *obj;
			if (idx >= 0x3fc0d)
				obj = g_curScript->m_locals[idx - 0x3fc0d];
			else {
				SymbolSlot *slot = &(*g_symRegistryY)[idx];
				int f = 0;
				if (slot->obj & f)
					obj = 0;
				else
					obj = (::Script::ScriptReference *)(slot->obj & ~(uintptr_t)1);
			}
			cam = (::Script::ScriptCamera *)obj;
			if (cam == m_cam)
				break;
			if (m_cam)
				delete m_cam;
			m_cam = cam;
			cam->AdjustRefCount(1);
			cam->m_ownerSlot = &m_cam;
			break;
		}
		case 1:
			m_lookFromName = InternString(it->GetAs_RwChar_ptr());
			if (_stricmp(m_lookFromName, "(Current)") == 0) {
				cam->m_lookFrom.m_pendingName = 0;
				if (cam->m_lookFrom.m_actor) {
					cam->m_lookFrom.m_actor->AdjustRefCount(-1);
					cam->m_lookFrom.m_actor = 0;
				}
			} else {
				cam->m_lookFrom.m_pendingName = m_lookFromName;
			}
			break;
		case 2: {
			V3i v = *static_cast<const V3i *>(it->GetAs_void_ptr());
			float *dst = &cam->m_lookFrom.m_horzOffs;
			dst[0] = v.x;
			dst[1] = v.y;
			dst[2] = v.z;
			if (cam == GetCameraObject())
				CameraSetHeading(v.z);
			break;
		}
		case 3:
			cam->m_lookFrom.m_headingRel = it->GetAs_RwInt32() != 0;
			break;
		case 4:
			m_lookAtName = InternString(it->GetAs_RwChar_ptr());
			if (_stricmp(m_lookAtName, "(Current)") == 0) {
				cam->m_lookAt.m_pendingName = 0;
				if (cam->m_lookAt.m_actor) {
					cam->m_lookAt.m_actor->AdjustRefCount(-1);
					cam->m_lookAt.m_actor = 0;
				}
			} else {
				cam->m_lookAt.m_pendingName = m_lookAtName;
			}
			break;
		case 5: {
			V3i v = *static_cast<const V3i *>(it->GetAs_void_ptr());
			float *dst = &cam->m_lookAt.m_horzOffs;
			dst[0] = v.x;
			dst[1] = v.y;
			dst[2] = v.z;
			break;
		}
		case 6:
			cam->m_lookAt.m_headingRel = it->GetAs_RwInt32() != 0;
			break;
		case 7:
			*(int *)&cam->m_fov = it->GetAs_RwInt32();
			break;
		case 8:
			if (it->GetAs_RwInt32())
				cam->m_flags |= 1;
			else
				cam->m_flags &= ~1;
			break;
		}
	}
}


}}
