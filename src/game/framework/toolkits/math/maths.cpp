#include "framework/toolkits/math/maths.h"
#include "zoosters/base.h"

namespace RWS {

ZOOSTERS_ASSERT_OFFSET(FrameTrans, dirty, 0x40);
ZOOSTERS_ASSERT_OFFSET(FrameTrans, m_offset, 0x44);
ZOOSTERS_ASSERT_OFFSET(FrameTrans, m_forwardVect, 0x68);
ZOOSTERS_ASSERT_SIZE(FrameTrans, 0x74);

// Degrees: heading about Z, then deflection about the turned X axis.
// 0x44aa70
void FrameTrans::SetFromAngles(RwReal deflect, RwReal heading)
{
	static const RwV3d zAxis = { 0.0f, 0.0f, 1.0f };
	static const RwV3d xUnit = { 1.0f, 0.0f, 0.0f };
	static const RwV3d yUnit = { 0.0f, 1.0f, 0.0f };
	static const RwV3d zUnit = { 0.0f, 0.0f, 1.0f };

	RwMatrix rotation;
	RwMatrixRotate(&rotation, &zAxis, heading, rwCOMBINEREPLACE);
	RwV3d deflectAxis = rotation.right;
	RwMatrixRotate(&rotation, &deflectAxis, deflect, rwCOMBINEPOSTCONCAT);

	// Points, not vectors: adding the zero translation turns -0.0 into +0.0, as the original does.
	RwV3dTransformPoints(&m_rightVect, &xUnit, 1, &rotation);
	RwV3dTransformPoints(&m_upVect, &yUnit, 1, &rotation);
	RwV3dTransformPoints(&m_forwardVect, &zUnit, 1, &rotation);
	dirty = true;
}

// 0x44ab70
void FrameTrans::Recalc()
{
	if (!dirty)
		return;
	m_transMtx.right = m_rightVect;
	m_transMtx.up = m_upVect;
	m_transMtx.at = m_forwardVect;
	m_transMtx.pos = m_offset;
	RwMatrixOptimize(&m_transMtx, nullptr);
	dirty = false;
}

} // namespace RWS
