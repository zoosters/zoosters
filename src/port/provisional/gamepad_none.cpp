// No game controllers for the builds without SDL: port_gamepad_init returns 0, so dinput_port.cpp offers no pad.
#include <string.h>
#include "framework/startup/win32/input/dinput.h"

extern "C" int port_gamepad_state(DIJOYSTATE2 *s)
{
	memset(s, 0, sizeof *s);
	for (int i = 0; i < 4; i++)
		s->rgdwPOV[i] = 0xffffffffu;
	return 0;
}
extern "C" void port_gamepad_rumble(float, float) {}
extern "C" void port_gamepad_pulse(float, int) {}
extern "C" int port_pad_invert_x = 0, port_pad_invert_y = 0;
extern "C" void port_gamepad_invert_camera(int axis, int value) { if (value >= 0) (axis ? port_pad_invert_y : port_pad_invert_x) = value != 0; }
extern "C" int port_gamepad_init(void) { return 0; }
