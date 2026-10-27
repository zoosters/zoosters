#include "modules/fx/attachpoint.h"
#include "framework/toolkits/math/maths.h"
#include "rw/rwcore.h"

extern const float g_floatZero;     // GLOBAL: 0x5ca5a8 g_floatZero

// 0x40a4c0
void AttachPoint::RebuildLocalMatrix()
{
	RwV3d axis;
	m_local.right.x = m_local.up.y = m_local.at.z = 1.0f;
	m_local.right.y = m_local.right.z = m_local.up.x = 0.0f;
	m_local.up.z = m_local.at.x = m_local.at.y = 0.0f;
	m_local.pos.x = m_local.pos.y = m_local.pos.z = 0.0f;
	m_local.flags |= 0x20003;
	axis.x = 0.0f;
	axis.y = 0.0f;
	axis.z = 1.0f;
	RwMatrixRotate(&m_local, &axis, m_rotZ, rwCOMBINEPOSTCONCAT);
	axis.x = 1.0f;
	axis.z = 0.0f;
	RwMatrixRotate(&m_local, &axis, m_rotX, rwCOMBINEPOSTCONCAT);
	if (m_rotY != g_floatZero) {
		axis.x = 0.0f;
		axis.y = 1.0f;
		RwMatrixRotate(&m_local, &axis, m_rotY, rwCOMBINEPOSTCONCAT);
	}
	RwMatrixTranslate(&m_local, &m_position, rwCOMBINEPOSTCONCAT);
	RwMatrixInvert(&m_localInv, &m_local);
	m_dirty = 0;
}

// 0x40a5b0
__declspec(noinline) RwMatrix *AttachPoint::GetLocalMatrix()
{
	if (m_dirty)
		RebuildLocalMatrix();
	return &m_local;
}
