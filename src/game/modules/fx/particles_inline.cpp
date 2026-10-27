// CPB_BasePart / CPB_TexPart inline members that the game emits out of line; the member-function-pointer globals force the compiler to emit them.
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "modules/fx/particles.h"

namespace RWS { namespace Particle {
typedef CPB_BasePart::Stage Stage;

// 0x409820
void (CPB_BasePart::SlotCursor::*g_keepSlotCursorOpen)(TankView, CPB_BasePart::Motion **) = &CPB_BasePart::SlotCursor::Open;
// 0x4098a0
bool (CPB_BasePart::SplitFrame::*g_keepSplitFrameBuild)(RwV3d *, RwReal, RwReal) = &CPB_BasePart::SplitFrame::Build;

// 0x4099b0
void (CPB_BasePart::*g_keepSetRate)(RwUInt32) = &CPB_BasePart::SetRate;
// 0x4099f0
void (CPB_BasePart::*g_keepSetRateScale)(RwReal) = &CPB_BasePart::SetRateScale;
// 0x409aa0
void (CPB_BasePart::*g_keepSetRateJitter)(RwUInt32) = &CPB_BasePart::SetRateJitter;
// 0x409ae0
void (CPB_BasePart::*g_keepSetLife)(RwUInt32) = &CPB_BasePart::SetLife;
// 0x409b00
void (CPB_BasePart::*g_keepSetLifeJitter)(RwUInt32) = &CPB_BasePart::SetLifeJitter;
// 0x409b20
void (CPB_BasePart::*g_keepSetTiltX)(RwReal) = &CPB_BasePart::SetTiltX;
// 0x409b40
void (CPB_BasePart::*g_keepSetTiltZ)(RwReal) = &CPB_BasePart::SetTiltZ;
// 0x409b60
void (CPB_BasePart::*g_keepSetBoxX)(RwReal) = &CPB_BasePart::SetBoxX;
// 0x409b70
void (CPB_BasePart::*g_keepSetBoxY)(RwReal) = &CPB_BasePart::SetBoxY;
// 0x409b80
void (CPB_BasePart::*g_keepSetBoxZ)(RwReal) = &CPB_BasePart::SetBoxZ;
// 0x409b90
void (CPB_BasePart::*g_keepSetSpeed)(Stage, RwReal) = &CPB_BasePart::SetSpeed;
// 0x409bb0
void (CPB_BasePart::*g_keepSetSpeedJitter)(Stage, RwReal) = &CPB_BasePart::SetSpeedJitter;
// 0x409bd0
void (CPB_BasePart::*g_keepSetColor)(Stage, RwRGBA) = &CPB_BasePart::SetColor;
// 0x409c10
void (CPB_BasePart::*g_keepSetRedJitter)(Stage, RwUInt32) = &CPB_BasePart::SetRedJitter;
// 0x409c30
void (CPB_BasePart::*g_keepSetGreenJitter)(Stage, RwUInt32) = &CPB_BasePart::SetGreenJitter;
// 0x409c50
void (CPB_BasePart::*g_keepSetBlueJitter)(Stage, RwUInt32) = &CPB_BasePart::SetBlueJitter;
// 0x409c70
void (CPB_BasePart::*g_keepSetAlphaJitter)(Stage, RwUInt32) = &CPB_BasePart::SetAlphaJitter;
// 0x409c90
void (CPB_BasePart::*g_keepSetFall)(Stage, RwReal) = &CPB_BasePart::SetFall;
// 0x409cb0
void (CPB_BasePart::*g_keepSetResistance)(Stage, RwReal) = &CPB_BasePart::SetResistance;
// 0x409cd0
void (CPB_BasePart::*g_keepSetExtent)(Stage, RwReal) = &CPB_BasePart::SetExtent;
// 0x409cf0
void (CPB_BasePart::*g_keepSetExtentJitter)(Stage, RwReal) = &CPB_BasePart::SetExtentJitter;
// 0x409d10
void (CPB_BasePart::*g_keepGainSpeed)(RwV3d *, CPB_BasePart::Motion *) = &CPB_BasePart::GainSpeed;
// 0x409d90
void (CPB_BasePart::*g_keepEnterSplit)(RwReal) = &CPB_BasePart::EnterSplit;
// 0x409e50
void (CPB_BasePart::*g_keepSpawnChild)(TankView &, RwReal) = &CPB_BasePart::SpawnChild;
// 0x409f00
void (CPB_BasePart::*g_keepAnimateCurrent)(void) = &CPB_BasePart::AnimateCurrent;
// 0x40a010
RwReal (CPB_BasePart::*g_keepSpawnParticle)(TankView &) = &CPB_BasePart::SpawnParticle;

// 0x40a060
void (CPB_TexPart::*g_keepShowCell)(RwV2d *, RwUInt32) = &CPB_TexPart::ShowCell;
// 0x40a100
void (CPB_TexPart::*g_keepAnimateCell)(RwV2d *, CPB_BasePart::Motion *, CPB_TexPart::CellTrack *) = &CPB_TexPart::AnimateCell;
// 0x40a220
void (CPB_TexPart::*g_keepTexBeginFill)(TankView &) = &CPB_TexPart::BeginFill;
// 0x40a2a0
void (CPB_TexPart::*g_keepTexSpawnParticle)(TankView &) = &CPB_TexPart::SpawnParticle;
// 0x40d880
void (CPB_TexPart::*g_keepSetAtlasGrid)(RwUInt32, RwUInt32) = &CPB_TexPart::SetAtlasGrid;

// the constructor (and with it the vtable and the scalar deleting destructor) is emitted by the first use
// 0x40a1a0
// 0x40a320
CPB_TexPart *g_makeTexPart(RwUInt32 blockCount, RwUInt32 recordsPerBlock) { return new CPB_TexPart(blockCount, recordsPerBlock); }
}}

// 0x40a210
int CPB_TexPart_TankDataFlags()
{
	return 131;
}
