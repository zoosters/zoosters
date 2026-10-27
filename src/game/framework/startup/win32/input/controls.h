// Default and registry key bindings of the nine input contexts, built once at start-up (Controls_LoadSettings).
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CONTROLS_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_CONTROLS_H

#include "framework/startup/win32/input/ctlbuf.h"

// Row of the control name table g_ctlBindings (0x602448, src/data/g/data_run.cpp): the registry key name of an action, two small
// ints, then for binding slots 1 and 2 a default (type << 16 | code) and a "fixed" flag (never read from the registry).
struct CtlBinding {
	char m_name[0x28];
	int m_28;
	int m_2c;
	int m_def1;
	char m_fixed1;
	char m_pad35[3];
	int m_def2;
	char m_fixed2;
	char m_pad3d[3];
};
ZOOSTERS_ASSERT_SIZE(CtlBinding, 0x40);

void CreateInputManager();
int FindPressedInput(CtlMap *map);   // first pad kind (3..12) bound anywhere in the map, else 0
int Controls_LoadSettings();
void CtlRefresh();   // JUMP claims a free pad
bool IsWaitingForPad(int which);
void SetPadRumble(float a, float b);

extern CtlBuf *g_ctlBuffers[9];             // GLOBAL: 0x617ba0 g_ctlBuffers

#endif
