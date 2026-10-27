// Stand-in for the Xbox XDK pad API: the PC build keeps the interface (xinput_emul.cpp) and answers it from the keyboard, mouse and
// DirectInput bindings. One record per port: the XDK's XINPUT_STATE (with 32 bit thumb axes instead of shorts), XINPUT_CAPABILITIES,
// XINPUT_FEEDBACK and the controller code's own bookkeeping.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_PADS_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_PADS_H

#include "zoosters/base.h"

// wButtons bits.
enum {
	XINPUT_GAMEPAD_DPAD_UP = 0x01,
	XINPUT_GAMEPAD_DPAD_DOWN = 0x02,
	XINPUT_GAMEPAD_DPAD_LEFT = 0x04,
	XINPUT_GAMEPAD_DPAD_RIGHT = 0x08,
	XINPUT_GAMEPAD_START = 0x10,
	XINPUT_GAMEPAD_BACK = 0x20,
	XINPUT_GAMEPAD_LEFT_THUMB = 0x40,
	XINPUT_GAMEPAD_RIGHT_THUMB = 0x80
};

// bAnalogButtons indices.
enum {
	XINPUT_GAMEPAD_A = 0,
	XINPUT_GAMEPAD_B = 1,
	XINPUT_GAMEPAD_X = 2,
	XINPUT_GAMEPAD_Y = 3,
	XINPUT_GAMEPAD_BLACK = 4,
	XINPUT_GAMEPAD_WHITE = 5,
	XINPUT_GAMEPAD_LEFT_TRIGGER = 6,
	XINPUT_GAMEPAD_RIGHT_TRIGGER = 7
};

// The emulation only sets the sub type.
struct XInputCapabilities {
	short m_subType;        // +0x00 1 = gamepad
	char m_data[0x1e];      // +0x02 ControlsManager::Poll tests the byte at +0x06
};
ZOOSTERS_ASSERT_SIZE(XInputCapabilities, 0x20);

struct PadState {
	unsigned m_packetNumber;               // +0x00 never written by the emulation
	unsigned short m_buttons;           
	unsigned char m_analogButtons[8];      // +0x06 0..255
	char m_pad0e[2];
	int m_thumbLX;                         // +0x10 -32767..32767
	int m_thumbLY;
	int m_thumbRX;
	int m_thumbRY;
	XInputCapabilities m_caps;          
	int m_rumbleLeftCaps;                  // +0x40 left motor: 1000 when the DirectInput pad has force feedback
	int m_rumbleRightCaps;                 // +0x44 right motor
	int m_feedbackStatus;                  // +0x48 0x3e5 (ERROR_IO_PENDING): no force feedback update wanted
	char m_feedbackHeader[8];              // +0x4c unused
	int m_rumbleLeft;                      // +0x54 SetPadRumble: strength * 655.35 * 100
	int m_rumbleRight;                     // +0x58
	int m_device;                          // +0x5c XInput handle = port + 1; 0 = no pad on this port
	int m_justInserted;                    // +0x60 ControlsManager::Poll sets it for the frame the port was opened
	int m_hasInput;                        // +0x64 CXboxStndController::ReadPad sets it when the pad changed
};
ZOOSTERS_ASSERT_SIZE(PadState, 0x68);
ZOOSTERS_ASSERT_OFFSET(PadState, m_thumbLX, 0x10);
ZOOSTERS_ASSERT_OFFSET(PadState, m_rumbleLeftCaps, 0x40);
ZOOSTERS_ASSERT_OFFSET(PadState, m_feedbackStatus, 0x48);
ZOOSTERS_ASSERT_OFFSET(PadState, m_device, 0x5c);

extern PadState g_pads[4];          // GLOBAL: 0x6179f8 g_pads
extern int g_activeControllers;     // GLOBAL: 0x603074 g_activeControllers

#endif
