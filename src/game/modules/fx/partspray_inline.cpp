// Out-of-line copies of the CPB_BasePart / CPB_TexPart setters (0x40d6d0-0x40d870) that the game's uninlined
// CFXPartSpray::HandleAttributes calls; the member-function-pointer globals force the compiler to emit them.
#include "modules/fx/particles.h"

namespace RWS { namespace Particle {
typedef CPB_BasePart::Stage Stage;

// 0x40d6d0
void (CPB_BasePart::*g_keepAddTraits)(Stage, RwUInt32) = &CPB_BasePart::AddTraits;
// 0x40d6f0
void (CPB_BasePart::*g_keepRemoveTraits)(Stage, RwUInt32) = &CPB_BasePart::RemoveTraits;
// 0x40d710
void (CPB_BasePart::*g_keepEnableSplit)(bool) = &CPB_BasePart::EnableSplit;
// 0x40d720
void (CPB_BasePart::*g_keepSetSplitAge)(RwUInt32) = &CPB_BasePart::SetSplitAge;
// 0x40d730
void (CPB_BasePart::*g_keepSetChildAgeJitter)(RwUInt32) = &CPB_BasePart::SetChildAgeJitter;
// 0x40d740
void (CPB_BasePart::*g_keepSetChildCount)(RwUInt32) = &CPB_BasePart::SetChildCount;
// 0x40d780
void (CPB_BasePart::*g_keepSetChildCountJitter)(RwUInt32) = &CPB_BasePart::SetChildCountJitter;
// 0x40d7c0
void (CPB_BasePart::*g_keepSetConeMin)(RwReal) = &CPB_BasePart::SetConeMin;
// 0x40d7e0
void (CPB_BasePart::*g_keepSetConeMax)(RwReal) = &CPB_BasePart::SetConeMax;
// 0x40d800
void (CPB_TexPart::*g_keepSetFirstCell)(Stage, RwUInt32) = &CPB_TexPart::SetFirstCell;
// 0x40d820
void (CPB_TexPart::*g_keepSetLastCell)(Stage, RwUInt32) = &CPB_TexPart::SetLastCell;
// 0x40d840
void (CPB_TexPart::*g_keepSetCellLoops)(Stage, RwReal) = &CPB_TexPart::SetCellLoops;
// 0x40d860
void (CPB_TexPart::*g_keepSetCellLoopJitter)(Stage, RwReal) = &CPB_TexPart::SetCellLoopJitter;
}}
