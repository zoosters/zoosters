// Game clock: script time minus the time spent stopped; frozen while TFBClockStart is set. Plus the frame-rate counter.
#ifndef ZOOSTERS_FRAMEWORK_MAINLOOP_GAMECLOCK_H
#define ZOOSTERS_FRAMEWORK_MAINLOOP_GAMECLOCK_H

extern int TFBClockStart;     // GLOBAL: 0x62a380 TFBClockStart: script time at which the clock was stopped (0 = running)
extern int TFBClockSum;       // GLOBAL: 0x62a384 TFBClockSum: time carried over from earlier stops of the system
extern int TFBClockBase;      // GLOBAL: 0x62a388 TFBClockBase: script time that counts as 0 (grows by the stopped intervals)

int TFBClock_GetRawTime();
float TFBClock_GetRawSeconds();
void TFBClock_SetTime(int ms);   // adjusts TFBClockSum
int TFBClock_GetTime();   // includes TFBClockSum
float TFBClock_GetSeconds();
int TFBClock_IsStopped();
bool TFBClock_Resume();   // true if it was stopped
int TFBClock_Stop();
void TFBHook_Pause();
void TFBHook_Resume();

struct FrameRate {
	unsigned m_last;
	unsigned m_accum;
	int m_count;
	int m_prev;
	bool Update();   // true when a new second began
};
extern FrameRate g_fpsCounter;   // GLOBAL: 0x62a4a8 g_fpsCounter

#endif
