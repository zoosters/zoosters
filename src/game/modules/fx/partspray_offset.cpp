// Out-of-line copy of the inline FrameTrans::SetOffset (0x40d6b0), called by CFXPartSpray::HandleAttributes.
#include "framework/toolkits/math/maths.h"

namespace RWS {
// 0x40d6b0
void (FrameTrans::*g_keepSetOffset)(const RwV3d &) = &FrameTrans::SetOffset;
}
