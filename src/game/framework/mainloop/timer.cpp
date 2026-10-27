// winmm timeGetTime with the timer resolution raised for the call.
#include "zoosters/os.h"

// 0x44d2c0
unsigned GetTimeMs()
{
	unsigned period = zoosters::os::TimerMinPeriod();
	zoosters::os::TimerResolutionBegin(period);
	unsigned now = zoosters::os::TickMs();
	zoosters::os::TimerResolutionEnd(period);
	return now;
}

