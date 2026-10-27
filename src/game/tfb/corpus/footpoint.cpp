#include "tfb/corpus/footpoint.h"

// 0x429860
float FootPoint::CrossGround(RwV3d *at) const
{
	const RwV3d &m_from = reinterpret_cast<const RwV3d &>(m_prevPos);
	const RwV3d &m_to = reinterpret_cast<const RwV3d &>(m_curPos);
	if ((m_from.y > 0.0f && m_to.y < 0.0f) || (m_from.y < 0.0f && m_to.y > 0.0f)) {
		float t = m_from.y / (m_from.y - m_to.y);
		at->x = (m_to.x - m_from.x) * t + m_from.x;
		at->y = 0.0f;
		at->z = (m_to.z - m_from.z) * t + m_from.z;
		return m_to.y;
	}
	return 0.0f;
}
