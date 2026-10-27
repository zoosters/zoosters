#include <stdlib.h>
#include "tfb/math/random.h"

extern const float g_randScale; // GLOBAL: 0x5cc9ac g_randScale

// 0x45ffb0
float RandRange(float lo, float hi)
{
	float r = (float)rand() * g_randScale;
	return r * (hi - lo) + lo;
}
