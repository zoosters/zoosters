// Own translation unit: the compiler must not inline its constructor and destructor into CFXMotionBlur's.
#include "modules/fx/cfxmotionblur.h"

namespace RWS { namespace FX {

// 0x4110e0
MotionBlurState::MotionBlurState()
{
	m_matrix = RwMatrixCreate();
	m_updateBlendPoly = 1;
}

// 0x411100
MotionBlurState::~MotionBlurState()
{
	if (m_matrix)
		RwMatrixDestroy(m_matrix);
}

}}
