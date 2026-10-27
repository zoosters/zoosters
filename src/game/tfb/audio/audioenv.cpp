#include "tfb/audio/audioenv.h"
#include "framework/toolkits/audio/audio.h"

// GLOBAL: 0x602a18 g_envPresets
// GLOBAL: 0x602a0c g_envRoom

// 0x4177a0
void SetEnvironment(int type, float level)
{
	RwaEnvironment *cur = RWS::RwsAudio::GetEnvironment();
	*cur = g_envPresets[type];
	int room = (int)((1.0f - level) * -1000.0f);
	g_envRoom = room;
	cur->lRoom = room;
	RWS::RwsAudio::GetOutputObject()->environment = *cur;
	RWS::RwsAudio::GetOutputObject()->dirtyFlags |= RWA_OUTPUT_ENVIRONMENT;
}
