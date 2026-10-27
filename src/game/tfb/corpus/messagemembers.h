// Type getters of the actor message members the collision and corpus code posts.
#ifndef ZOOSTERS_TFB_CORPUS_MESSAGEMEMBERS_H
#define ZOOSTERS_TFB_CORPUS_MESSAGEMEMBERS_H

namespace Script { class ScriptReference; }

Script::ScriptReference *TFBMember_HitWallMessage();        // 0x439da0
Script::ScriptReference *TFBMember_WallNormalMessage();     // 0x439f20
Script::ScriptReference *TFBMember_LedgeNearbyMessage();    // 0x439fe0
Script::ScriptReference *TFBMember_BounceNormalMessage();   // 0x43a160
Script::ScriptReference *TFBMember_FootPrintMessage();   // 0x43a0a0
Script::ScriptReference *TFBMember_HitActorMessage();   // 0x43b000
Script::ScriptReference *TFBMember_HitGroundMessage();   // 0x43a220
Script::ScriptReference *TFBMember_HitTriggerMessage();   // 0x439e60
Script::ScriptReference *TFBMember_JumpStartedMessage();   // 0x43a3a0
Script::ScriptReference *TFBMember_LeftGroundMessage();   // 0x43a2e0

#endif
