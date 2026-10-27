// RenderWare Audio object UUIDs.

#include "rw/rwaudio.h"   // RwaUuid (src/game)

// UUID of the DXAudio output object definition, looked up by AudioInit: 260DBB3A-5005-482E-A0D7-9948B340E33C.
// RWA's other UUIDs and constants follow it in the game and are not defined here.
// GLOBAL: 0x5e1900 g_audioOutputUuid
extern const RwaUuid g_audioOutputUuid = { 0x260DBB3A, 0x5005, 0x482E, { 0xA0, 0xD7, 0x99, 0x48, 0xB3, 0x40, 0xE3, 0x3C } };
