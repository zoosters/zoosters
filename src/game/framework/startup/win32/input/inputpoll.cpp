// InputManager part 2: the per-frame poll, and the values the key bindings read.
// CFLAGS: /GS
#include "framework/startup/win32/input/inputmanager.h"
#include <string.h>

extern int g_mouseSmoothY;        // GLOBAL: 0x62f168 g_mouseSmoothY (last mouse delta Y after smoothing; guess name)
extern int g_mouseSmoothX;        // GLOBAL: 0x62f16c g_mouseSmoothX (last mouse delta X after smoothing; guess name)

// A device that lost its acquisition is acquired again once. False when it cannot be polled.
static bool PollDevice(IDirectInputDevice8A *device)
{
	if (device->Poll() < 0) {
		if (device->Acquire() < 0)
			return false;
		if (device->Poll() < 0)
			return false;
	}
	return true;
}

// A value equal to its non-zero snapshot is still held from before the context switch and is cleared (true); any other value ends the hiding.
template <class T>
static bool HideHeld(T &current, T &snapshot)
{
	if (current == snapshot && snapshot != 0) {
		current = 0;
		return true;
	}
	snapshot = 0;
	return false;
}

// While m_flushHeld is set, input still exactly as when SetPlayerContext took the snapshots is hidden; the flag clears once nothing
// is held. m_padPolledMask gets a bit per pad that delivered a state.
// 0x51b450
void InputManager::Poll()
{
	unsigned char *keys = m_keys;
	memcpy(m_keysPrev, keys, sizeof(m_keys));
	long result = m_keyboard->GetDeviceState(sizeof(m_keys), keys);
	if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
		if (m_keyboard->Acquire() >= 0)
			m_keyboard->GetDeviceState(sizeof(m_keys), keys);
		else
			memset(keys, 0, sizeof(m_keys));
	}
	if (m_mouse) {
		bool polled = PollDevice(m_mouse);
		m_mousePrev = m_mouseState;
		if (polled)
			m_mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &m_mouseState);
		else
			memset(&m_mouseState, 0, sizeof(DIMOUSESTATE2));
		if (m_smoothMouse) {
			// a frame without movement repeats half of the last
			if (m_mouseState.lX == 0 && g_mouseSmoothX != 0)
				m_mouseState.lX = g_mouseSmoothX / 2;
			g_mouseSmoothX = m_mouseState.lX;
			if (m_mouseState.lY == 0 && g_mouseSmoothY != 0)
				m_mouseState.lY = g_mouseSmoothY / 2;
			g_mouseSmoothY = m_mouseState.lY;
		}
	}
	m_padPolledMask = 0;
	for (int i = 0; i < INPUT_MAX_PADS; i++) {
		IDirectInputDevice8A *pad = m_padDevices[i];
		if (!pad)
			continue;
		m_padPrev[i] = m_padState[i];
		if (PollDevice(pad) && pad->GetDeviceState(sizeof(DIJOYSTATE2), &m_padState[i]) >= 0)
			m_padPolledMask |= 1 << i;
		else
			memset(&m_padState[i], 0, sizeof(DIJOYSTATE2));
	}
	if (m_flushHeld) {
		bool held = false;
		for (int k = 0; k < 0x100; k++) {
			if (m_keysSnap[k]) {
				if (HideHeld(keys[k], m_keysSnap[k]))
					held = true;
			}
		}
		if (HideHeld(m_mouseState.lX, m_mouseSnap.lX))
			held = true;
		if (HideHeld(m_mouseState.lY, m_mouseSnap.lY))
			held = true;
		if (HideHeld(m_mouseState.lZ, m_mouseSnap.lZ))
			held = true;
		for (int k = 0; k < 8; k++) {
			if (HideHeld(m_mouseState.rgbButtons[k], m_mouseSnap.rgbButtons[k]))
				held = true;
		}
		for (int i = 0; i < INPUT_MAX_PADS; i++) {
			if (!m_padDevices[i])
				continue;
			int *state = &m_padState[i].lX;
			int *snapshot = &m_padSnap[i].lX;
			for (int k = 0; k < 12; k++) {
				if (HideHeld(state[k], snapshot[k]))
					held = true;
			}
			for (int k = 0; k < 30; k++) {
				if (HideHeld(m_padState[i].rgbButtons[k], m_padSnap[i].rgbButtons[k]))
					held = true;
			}
		}
		if (!held)
			m_flushHeld = 0;
	}
	for (unsigned player = 0; player < m_numContexts; player++) {
		CtlBuf *context = m_contexts[player];
		if (context)
			UpdateBindings(&context->m_map, player);
	}
}

// A context that accumulates the mouse (mouse look) adds the new deltas to its position (decayed, clamped to +-m_accumLimit) and
// the axes read that; otherwise they read the raw delta scaled by m_mouseSens.
// 0x51abe0
float InputManager::GetMouseControl(int code, int player)
{
	CtlBuf *context = m_contexts[player];
	float y = (float)m_mouseState.lY * m_mouseSens;
	float x = (float)m_mouseState.lX * m_mouseSens;
	if (context && context->m_accumulateMouse) {
		x = (x * context->m_mouseGain + context->m_accumX) * context->m_accumDecay;
		y = (y * context->m_mouseGain + context->m_accumY) * context->m_accumDecay;
		if (x >= context->m_accumLimit)
			x = context->m_accumLimit;
		if (x <= -context->m_accumLimit)
			x = -context->m_accumLimit;
		if (y >= context->m_accumLimit)
			y = context->m_accumLimit;
		if (y <= -context->m_accumLimit)
			y = -context->m_accumLimit;
		context->m_accumX = x;
		context->m_accumY = y;
	}
	switch (code) {
	case MOUSE_BUTTON_0:
		return (float)(m_mouseState.rgbButtons[0] != 0);
	case MOUSE_BUTTON_1:
		return (float)(m_mouseState.rgbButtons[1] != 0);
	case MOUSE_BUTTON_2:
		return (float)(m_mouseState.rgbButtons[2] != 0);
	case MOUSE_WHEEL_FAST:
		if (m_mouseState.lZ > 1)
			return 1.0f;
		if (m_mouseState.lZ < -1)
			return -1.0f;
		break;
	case MOUSE_WHEEL_UP:
		if (m_mouseState.lZ > 0)
			return 1.0f;
		break;
	case MOUSE_WHEEL_DOWN:
		if (m_mouseState.lZ < 0)
			return 1.0f;
		break;
	case MOUSE_AXIS_Y:
		return y;
	case MOUSE_AXIS_X:
		return x;
	case MOUSE_Y_NEGATIVE:
		if (y < 0.0f)
			return -y;
		break;
	case MOUSE_Y_POSITIVE:
		if (y > 0.0f)
			return y;
		break;
	case MOUSE_X_NEGATIVE:
		if (x < 0.0f)
			return -x;
		break;
	case MOUSE_X_POSITIVE:
		if (x > 0.0f)
			return x;
		break;
	}
	return 0.0f;
}

// Controls 1..16: the positive (odd) and negative (even) half of the eight axes (0..1000). 17..20: the hat directions, 1000 within 67.5 degrees.
// 0x51ae00
int InputManager::GetPadAxisControl(const DIJOYSTATE2 *pad, int control)
{
	if (control >= PADCTL_AXIS_FIRST && control < PADCTL_HAT_RIGHT) {
		const int *axes = &pad->lX;     // the axes and sliders follow each other
		int value = axes[(control - PADCTL_AXIS_FIRST) / 2];
		if ((control - PADCTL_AXIS_FIRST) % 2 == 0)
			return value > 0 ? value : 0;
		return value < 0 ? -value : 0;
	}
	if (control >= PADCTL_HAT_RIGHT && control <= PADCTL_HAT_UP) {
		// hundredths of a degree; each direction's range (first, last)
		static const unsigned kHatRange[4][2] = {
			{ 2250, 15750 },     // right
			{ 20250, 33750 },    // left
			{ 11250, 24750 },    // down
			{ 29250, 6750 }      // up: wraps around 0
		};
		unsigned hat = pad->rgdwPOV[0];
		if (hat == 0xffffffff)
			return 0;
		unsigned first = kHatRange[control - PADCTL_HAT_RIGHT][0];
		unsigned last = kHatRange[control - PADCTL_HAT_RIGHT][1];
		bool inside = first <= last ? (hat >= first && hat <= last) : (hat >= first || hat <= last);
		return inside ? 1000 : 0;
	}
	return 0;
}

// 0x51b020
float InputManager::GetPadControl(int pad, int code)
{
	if (code >= PADCTL_BUTTON_FIRST && code <= PADCTL_BUTTON_LAST) {
		if (m_padState[pad].rgbButtons[code - PADCTL_BUTTON_FIRST])
			return 1.0f;
		return 0.0f;
	}
	if (code >= PADCTL_AXIS_FIRST && code <= PADCTL_AXIS_LAST)
		return (float)GetPadAxisControl(&m_padState[pad], code) * 0.001f;
	return 0.0f;
}

// A binding of an unknown kind keeps the value read by the previous binding.
// 0x51b0a0
void InputManager::UpdateBindings(CtlMap *map, int player)
{
	float value = 0.0f;
	for (unsigned action = 0; action < map->m_count; action++) {
		for (int slot = 0; slot < CTL_SLOTS; slot++) {
			CtlMapEntry *e = &map->m_entries[action][slot];
			if (!e->m_type)
				continue;
			if (e->m_type == CTL_BIND_KEYBOARD) {
				if (m_keys[e->m_code])
					value = 1.0f;
				else
					value = 0.0f;
			} else if (e->m_type == CTL_BIND_MOUSE) {
				value = GetMouseControl(e->m_code, player);
			} else if (e->m_type >= CTL_BIND_PAD_FIRST && e->m_type <= CTL_BIND_PAD_LAST) {
				value = GetPadControl(e->m_type - CTL_BIND_PAD_FIRST, e->m_code);
			}
			e->m_value = value;
		}
	}
}
