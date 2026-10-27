// Bearings helpers of the script positions (script/bearings.cpp, 0x4404c0-0x440990) and the globals they and the Position script
// members read: the script object that runs the current command (self) and its owner.
#ifndef ZOOSTERS_SCRIPT_BEARINGS_H
#define ZOOSTERS_SCRIPT_BEARINGS_H

#include "script/script.h"

extern Script::ScriptReference *g_scriptSelf;   // 0x621f14
extern Script::ScriptReference *g_scriptOwner;  // 0x621f18
extern const float g_floatZero;                 // 0x5ca5a8
bool HeadingFromXZ(float x, float z, float *heading);          // 0x4404c0: heading (0..360) of the (x, z) direction; false if both ~0
bool PitchFromXYZ(float x, float y, float z, float *pitch);   // 0x440520: elevation angle of (x, y, z); false never returned

#endif
