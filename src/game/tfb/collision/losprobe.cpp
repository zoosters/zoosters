// Callbacks of the intersection queries that probe a line of sight (the probe record keeps its "clear" flag at +0x2c).
#include "script/losprobe.h"

// FUNCTION: 0x42b130 AtomicIsectHitCB
int AtomicIsectHitCB(int, int, int, unsigned char *p)
{
	((LosProbe *)p)->m_clear = 0;   // +0x30 on x64: the raw +0x2c hit m_dir.z and the line of sight was never blocked
	return 0;
}

// FUNCTION: 0x42b140 LosProbeTriangleCB
int LosProbeTriangleCB(int, int, int, int, unsigned char *p)
{
	((LosProbe *)p)->m_clear = 0;
	return 0;
}
