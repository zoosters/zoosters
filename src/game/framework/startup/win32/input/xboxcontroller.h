// The controller behaviour: answers the scripts' controller inquiries (INQ_ACTN_TURN, INQ_ACTN_JUMP ...) with the values of the
// asking player's pad, and assigns pads to players. PC: pads, keyboard and mouse through the XInput emulation.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_XBOXCONTROLLER_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_INPUT_XBOXCONTROLLER_H

#include "framework/startup/win32/input/controlsmanager.h"

// A controller message with its four per-player results (HandleEvents answers with m_val[player index]). The values are floats
// stored as their bit pattern: the message data is a void *.
// Out-of-line ctor 0x405dc0, dtor 0x405db0 (jmp ~CEventId).
struct CtlEvent {
	CtlEvent() { m_val[3] = 0; m_val[2] = 0; m_val[1] = 0; m_val[0] = 0; }
	RWS::CEventId m_id;
	int m_val[4];
	float &f(int i) { return ((float *)m_val)[i]; }
};
ZOOSTERS_ASSERT_SIZE(CtlEvent, 0x18);

// PS2 base order; no CSystemCommands, so the members start at +0x24.
class CXboxStndController : public RWS::CEventHandler, public RWS::CAttributeHandler, public RWS::LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(CXboxStndController);
	CXboxStndController(const RWS::CAttributePacket &attr);
	~CXboxStndController();
	virtual void HandleEvents(RWS::CMsg &msg);
	virtual void HandleAttributes(const RWS::CAttributePacket &attr);
	bool ReadPad(int player);   // true when the pad changed
	void UpdatePadAssignment(bool jumpClaims);
	CtlEvent *GetEventForCommand(int command);

	CtlEvent m_turn;            // +0x24 INQ_ACTN_TURN: left stick x
	CtlEvent m_fwd;             // +0x3c INQ_ACTN_FWD: left stick y
	CtlEvent m_strafe;          // +0x54 INQ_ACTN_STRAFE: right stick x
	CtlEvent m_pitch;           // +0x6c INQ_CAMERA_PITCH: right stick y
	CtlEvent m_analogButton[8];   // +0x84 0..255 -> 0..1; A first (INQ_ACTN_JUMP), B: INQ_ACTN_FIRE
	CtlEvent m_digitalButton[8];   // +0x144 wButtons bit k: 0 or 1
	CtlEvent m_dpadX;           // +0x204 +1 right, -1 left
	CtlEvent m_dpadY;           // +0x21c +1 up, -1 down
	ControlsManager *m_controls;     // +0x234
	int m_player1Pad;   // +0x238 port of player 1; -1 = none yet
	int m_player2Pad;   // +0x23c port of player 2 (only with two active controllers); -1 = none
	int m_player2PadHeld;   // +0x240 follows m_player2Pad once claimed; excludes that pad from player 1 until the count drops
	int m_lostPad;   // +0x244 only ever reset to -1 by UpdatePadAssignment; excludes a pad from player 1 while set
	int m_connectedPads;   // +0x248 ports with a pad (IsWaitingForPad)
	int m_padClaimed;   // +0x24c player 1 got a pad
};
ZOOSTERS_ASSERT_SIZE(CXboxStndController, 0x250);
ZOOSTERS_ASSERT_OFFSET(CXboxStndController, m_turn, 0x24);
ZOOSTERS_ASSERT_OFFSET(CXboxStndController, m_analogButton, 0x84);
ZOOSTERS_ASSERT_OFFSET(CXboxStndController, m_digitalButton, 0x144);
ZOOSTERS_ASSERT_OFFSET(CXboxStndController, m_controls, 0x234);
ZOOSTERS_ASSERT_OFFSET(CXboxStndController, m_player1Pad, 0x238);

extern CXboxStndController *g_xboxController;

#endif
