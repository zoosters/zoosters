#include <string.h>
#include "script/ops_camera.h"

// GLOBAL: 0x603064 g_cameraTypeNames
// GLOBAL: 0x603054 g_cameraTransitionNames

namespace Script {

// 0x438930
const char *CameraUse::GetName()
{
	return "use camera";
}

// 0x438ed0
OpCode *CameraUse::Clone()
{
	return CloneToPool(this);
}

// 0x438ee0
CameraUse *CameraUse::CloneToPool(const CameraUse *src)
{
	CameraUse *r = new (g_scriptPool) CameraUse(*src);
	g_scriptPool += sizeof(CameraUse);
	return r;
}

// 0x438f80
int CameraUse::Execute()
{
	ScriptReference *owner;
	ScriptReference *set;
	ScriptReference *cam = m_camera.Resolve(&owner, &set, true);
	if (cam != 0) {
		TFBUser_Camera()->SetReferent(cam, 0);
		float param, transitionParam;
		memcpy(&param, &m_param, sizeof param);
		memcpy(&transitionParam, &m_transitionParam, sizeof transitionParam);
		CameraUse_Apply(m_type, param, m_transition, transitionParam);
	}
	return 1;
}

// 0x438fe0
int CameraUse::Parse()
{
	GetName();
	ScriptRefReader refs(TFBType_Camera());
	refs.Read(&m_camera);
	g_scriptLastType = 0;
	ScriptEnumReader enums1(&m_type, g_cameraTypeNames, 0);
	TFBRead_u32_thunk(&m_param);
	ScriptEnumReader enums2(&m_transition, g_cameraTransitionNames, 0);
	if (m_transition != 0)
		TFBRead_u32_thunk(&m_transitionParam);
	else
		m_transitionParam = 0;
	return sizeof(CameraUse);
}

// CameraUse's copy constructor (0x438f40) lives in ops_media.cpp: visible here, cl 13.10 sees that it cannot throw and drops
// the EH frame the game's CloneToPool has.

}  // namespace Script
