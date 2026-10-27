// Vptr-only constructors of Expose* members of ScriptMemCard / ScriptShadow / ScriptFog / ScriptMotionBlur.
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptmemcard.h"
#include "script/types/scriptshadow.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptmotionblur.h"

namespace Script {

// 0x4314a0
// (compiler-made scalar deleting destructor; the game shares one copy among all script op/Expose classes)

// GLOBAL: 0x5d3f30 Script::ScriptMemCard::ExposeMemCardSlotSize::vftable
// 0x43eaa0
ScriptMemCard::ExposeMemCardSlotSize::ExposeMemCardSlotSize() {}

// GLOBAL: 0x5d3f88 Script::ScriptMemCard::ExposeMemCardAutoFormat::vftable
// 0x43eac0
ScriptMemCard::ExposeMemCardAutoFormat::ExposeMemCardAutoFormat() {}

// GLOBAL: 0x5d3fd8 Script::ScriptMemCard::ExposeMemCardAutoDelete::vftable
// 0x43eae0
ScriptMemCard::ExposeMemCardAutoDelete::ExposeMemCardAutoDelete() {}

// GLOBAL: 0x5d4028 Script::ScriptMemCard::ExposeMemCardLanguage::vftable
// 0x43eb00
ScriptMemCard::ExposeMemCardLanguage::ExposeMemCardLanguage() {}

// GLOBAL: 0x5d4080 Script::ScriptMemCard::ExposeMemCardSlotPercent::vftable
// 0x43eb20
ScriptMemCard::ExposeMemCardSlotPercent::ExposeMemCardSlotPercent() {}

// GLOBAL: 0x5d40e8 Script::ScriptMemCard::ExposeMemCardAutoCreate::vftable
// 0x43eb40
ScriptMemCard::ExposeMemCardAutoCreate::ExposeMemCardAutoCreate() {}

// GLOBAL: 0x5d4138 Script::ScriptMemCard::ExposeMemCardSlotGameTime::vftable
// 0x43eb60
ScriptMemCard::ExposeMemCardSlotGameTime::ExposeMemCardSlotGameTime() {}

// GLOBAL: 0x5d4198 Script::ScriptMemCard::ExposeMemCardChangedState::vftable
// 0x43eb80
ScriptMemCard::ExposeMemCardChangedState::ExposeMemCardChangedState() {}

// GLOBAL: 0x5d4380 Script::ScriptShadow::ExposeEnableFading::vftable
// 0x43eff0
ScriptShadow::ExposeEnableFading::ExposeEnableFading() {}

// GLOBAL: 0x5d43d8 Script::ScriptShadow::ExposeFadeRadius::vftable
// 0x43f010
ScriptShadow::ExposeFadeRadius::ExposeFadeRadius() {}

// GLOBAL: 0x5d4600 Script::ScriptFog::ExposePixelFogColor::vftable
// 0x43f1f0
ScriptFog::ExposePixelFogColor::ExposePixelFogColor() {}

// GLOBAL: 0x5d4658 Script::ScriptFog::ExposeBloomEnable::vftable
// 0x43f210
ScriptFog::ExposeBloomEnable::ExposeBloomEnable() {}

// GLOBAL: 0x5d46b0 Script::ScriptFog::ExposeBloomAddFactor::vftable
// 0x43f230
ScriptFog::ExposeBloomAddFactor::ExposeBloomAddFactor() {}

// GLOBAL: 0x5d4708 Script::ScriptFog::ExposeBloomHaloSize::vftable
// 0x43f250
ScriptFog::ExposeBloomHaloSize::ExposeBloomHaloSize() {}

// GLOBAL: 0x5d49b0 Script::ScriptFog::ExposeDOFBluriness::vftable
// 0x43f380
ScriptFog::ExposeDOFBluriness::ExposeDOFBluriness() {}

// GLOBAL: 0x5d4bb8 Script::ScriptMotionBlur::ExposeScaleY::vftable
// 0x43f5c0
ScriptMotionBlur::ExposeScaleY::ExposeScaleY() {}

// GLOBAL: 0x5d4c10 Script::ScriptMotionBlur::ExposeIsoScale::vftable
// 0x43f5e0
ScriptMotionBlur::ExposeIsoScale::ExposeIsoScale() {}

// GLOBAL: 0x5d4c58 Script::ScriptMotionBlur::ExposeField40::vftable
// 0x43f600
ScriptMotionBlur::ExposeField40::ExposeField40() {}

}  // namespace Script
