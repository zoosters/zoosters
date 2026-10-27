// Reverb presets the scripts select (ScriptSound.EnvironmentType / EnvironmentLevel), installed on the audio output.
#ifndef ZOOSTERS_TFB_AUDIO_AUDIOENV_H
#define ZOOSTERS_TFB_AUDIO_AUDIOENV_H

#include "rw/rwaudio.h"

// GLOBAL: 0x602a18 g_envPresets
// GLOBAL: 0x602a0c g_envRoom
extern RwaEnvironment g_envPresets[];
extern int g_envRoom;  // room level installed by the last SetEnvironment: (1 - level) x -1000
extern int g_envType;
extern float g_envLevel;                // 0..1 (percent for scripts)

void SetEnvironment(int type, float level);

#endif
