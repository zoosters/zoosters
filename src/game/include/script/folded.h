// Shared functions the game's linker folded several virtual slots into (ICF), normal build only.
#pragma once
#include <stdint.h>
namespace Script { class ScriptReference; void DetachParticleSetClones(); void DetachSpriteSetClones(); }
struct Vec3;

Script::ScriptReference * ActorAttachment_GetType();
Script::ScriptReference * ActorTurret_GetType();
Script::ScriptReference * ActorWayPoint_GetType();
Script::ScriptReference * AngleMeasurement_GetType();
int AnimMapMemberCount();
Script::ScriptReference * ColorMeasurement_GetType();
int ColorMeasurement_GetValueType();
char const * ExposeActiveSet_GetName();
char const * ExposeBeenoxQuitGame_GetName();
int ExposeFOV_GetMax();
char const * ExposeFOV_GetName();
char const * ExposeHeading_GetName();
char const * ExposeHorzComponent_GetName();
char const * ExposeInactiveSet_GetName();
char const * ExposeLookAtHeadingOffs_GetName();
char const * ExposeLookAtHeadingRel_GetName();
char const * ExposeLookAtHorzOffs_GetName();
char const * ExposeLookAtPointer_GetName();
char const * ExposeLookAtVertOffs_GetName();
int ExposeParticleSpread_GetMax();
char const * ExposePitch_GetName();
char const * ExposeRemovedSet_GetName();
char const * ExposeScales_GetName();
char const * ExposeTimeRemaining_GetName();
char const * ExposeVertComponent_GetName();
int FloatMeasurement_GetValueType();
int __stdcall Fold_Return0_Arg1(void *);
bool Fold_ReturnFalse();
Script::ScriptReference * IntMeasurement_GetType();
char const * OpBehavior_GetAltName();
bool __stdcall Placement_Control(int);
Script::ScriptReference * Placement_GetType();
bool __stdcall Placement_PlayAnimation(int);
intptr_t __stdcall Placement_Spawn(intptr_t, int);
char const * PositionMeasurement_GetName();
Script::ScriptReference * PositionMeasurement_GetType();
bool __stdcall PositionMeasurement_SetFacing(int, int);
Script::ScriptReference * ReferenceSet_GetType();
Script::ScriptReference * ScriptController_GetType();
char const * ScriptLight_GetName();
Script::ScriptReference * ScriptLight_GetType();
char const * ScriptMessage_GetName();
char const * ScriptParticle_GetName();
Script::ScriptReference * ScriptParticle_GetType();
char const * ScriptRumble_GetName();
Script::ScriptReference * ScriptRumble_GetType();
char const * ScriptSurfaceAnim_GetName();
Script::ScriptReference * ScriptSurfaceAnim_GetType();
int Script_AlwaysTrue();
Script::ScriptReference * TFBType_Behavior();
int TwoDMeasurement_GetValueType();
int ValueInterface_GetMax();
char const * ValueInterface_GetName();
Script::ScriptReference * ValueInterface_GetType();
char const * VectorMeasurement_GetName();
Script::ScriptReference * VectorMeasurement_GetType();
Script::ScriptReference * thunk_FUN_0043da40();
Script::ScriptReference * thunk_FUN_0043e330();

// cross-class folds called from files whose local stand-in classes clash with the real header (normal_folded_members.cpp)
void *Folded_SurfaceAnimW_GetMembers(void *self, int *count);       // SurfaceAnimW::GetMembers (ScriptSurfaceAnim; member_tables.cpp)
Vec3 *GetZeroVec3();                                             // 0x4039c0 (script/measurement.cpp)
const char *ReferenceSet_GetName();                             // 0x402ba0 (script/reference.cpp)
const char *ScriptActor_GetName();                              // 0x42d5f0
void ScriptActor_Reset();                                       // 0x4386c0
void ScriptCamera_Reset();                                      // 0x438b10
void *__stdcall ExposeCameraLocation_GetVector(Script::ScriptReference *owner);   // 0x4391b0 (owner unused)

const char * ExposeA_GetName();
int ExposeColorComponent_GetMax();
const char * ExposeB_GetName();
const char * ExposeG_GetName();
const char * ExposeR_GetName();
const char * ExposeHeadingFromMe_GetName();
const char * ExposePitchFromMe_GetName();
const char * ExposePercentComplete_GetName();
const char * ExposeRoll_GetName();
int ExposeVertFollow_GetMax();
const char * ExposeActivationRange_GetName();
int ExposeConeAngle_GetMax();
const char * ExposeDeactivationRange_GetName();
const char * ExposeIgnoreCutScenes_GetName();
int ExposeOutputRate_GetMax();
int ExposePrimaryVelocity_GetMax();
int ExposePrimarySizeSpread_GetMax();
const char * ExposeFacing_GetName();
const char * ExposeTilt_GetName();
const char * ExposeViewPyramidDist_GetName();
