// This unit emits the vtables, the scalar deleting destructors and the destructor thunk of CXboxStndController and ControlsManager.
#include "framework/startup/win32/input/xboxcontroller.h"
#include "framework/core/ceventhandler.h"
#include "framework/startup/win32/input/controls.h"
#include "framework/startup/win32/input/inputmanager.h"
#include "framework/startup/win32/input/xinput.h"
using namespace RWS;

void *MemAllocHint(size_t size, int hint);
// GLOBAL: 0x617c04 RWS::ERROR_NOT_A_VALID_MESSAGE_ID
// GLOBAL: 0x617bc4 g_xboxController (extern in xboxcontroller.h)
extern ControlsManager *g_controlsManager;        // GLOBAL: 0x617b98 g_controlsManager
extern int g_ctlFlag617bd0;                       // GLOBAL: 0x617bd0 g_ctlFlag617bd0
extern const char *g_xboxControllerClassId;       // GLOBAL: 0x602868 g_xboxControllerClassId
extern unsigned g_btnMask[8];                     // GLOBAL: 0x60286c g_btnMask (1 << k)
extern const float g_floatZero;                   // GLOBAL: 0x5ca5a8 g_floatZero
extern float g_rumbleA;                           // GLOBAL: 0x617bf0 g_rumbleA
extern float g_rumbleB;                           // GLOBAL: 0x617bec g_rumbleB
bool IsLanguage7Selectable();

// Out-of-line CtlEvent constructor; the arrays' vector constructor iterators call it.
// 0x405dc0
// GLOBAL: 0x51b440 ??1InputManager@@QAE@XZ
// GLOBAL: 0x5caec0 CXboxStndController::vftable_for_CEventHandler
// GLOBAL: 0x5caeb0 CXboxStndController::vftable_for_CAttributeHandler
// GLOBAL: 0x5caea8 ControlsManager::vftable
// 0x406440
// 0x405fa0
// 0x406410
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z
// GLOBAL: 0x550ac0 ?LinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@PBDG@Z
// GLOBAL: 0x550b20 ?ReplaceLinkedMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@PBD1@Z
// GLOBAL: 0x558c33 ??_L@YGXPAXIHP6EX0@Z1@Z
// GLOBAL: 0x558bd3 ??_M@YGXPAXIHP6EX0@Z@Z

// 0x405a30
ControlsManager::ControlsManager()
{
	Controls_LoadSettings();
}

// 0x405a80
ControlsManager::~ControlsManager()
{
	if (g_inputManager) {
		delete g_inputManager;
		g_inputManager = 0;
	}
	for (int i = 0; i < 9; i++) {
		if (g_ctlBuffers[i])
			::operator delete(g_ctlBuffers[i]);
	}
}

void *ControlsManager::operator new(size_t size, RwsMemHint type)
{
	return MemAllocHint(size, (int)type);
}

// 0x405fb0
ControlsManager *GetControlsManager()
{
	if (!g_controlsManager)
		g_controlsManager = new (2) ControlsManager;
	g_ctlFlag617bd0 = 1;
	return g_controlsManager;
}

// 0x406020
CXboxStndController::CXboxStndController(const CAttributePacket &)
	: m_player1Pad(-1), m_player2Pad(-1), m_player2PadHeld(-1), m_lostPad(-1), m_connectedPads(0), m_padClaimed(0)
{
	m_controls = GetControlsManager();
	g_xboxController = this;
	LinkMsg(RWS::iMsgRunningPreTick);
	ReplaceLinkedMsg(m_turn.m_id, "INQ_ACTN_TURN", "return RwReal");
	ReplaceLinkedMsg(m_fwd.m_id, "INQ_ACTN_FWD", "return RwReal");
	ReplaceLinkedMsg(m_strafe.m_id, "INQ_ACTN_STRAFE", "return RwReal");
	ReplaceLinkedMsg(m_pitch.m_id, "INQ_CAMERA_PITCH", "return RwReal");
	ReplaceLinkedMsg(m_analogButton[0].m_id, "INQ_ACTN_JUMP", "return RwReal");
	ReplaceLinkedMsg(m_analogButton[1].m_id, "INQ_ACTN_FIRE", "return RwReal");
}

// 0x405de0
CXboxStndController::~CXboxStndController()
{
	UnLinkMsg(m_turn.m_id);
	UnRegisterMsg(m_turn.m_id);
	UnLinkMsg(m_fwd.m_id);
	UnRegisterMsg(m_fwd.m_id);
	UnLinkMsg(m_strafe.m_id);
	UnRegisterMsg(m_strafe.m_id);
	UnLinkMsg(m_pitch.m_id);
	UnRegisterMsg(m_pitch.m_id);
	for (int i = 0; i < 8; i++) {
		UnLinkMsg(m_analogButton[i].m_id);
		UnRegisterMsg(m_analogButton[i].m_id);
		UnLinkMsg(m_digitalButton[i].m_id);
		UnRegisterMsg(m_digitalButton[i].m_id);
	}
	UnLinkMsg(m_dpadX.m_id);
	UnRegisterMsg(m_dpadX.m_id);
	UnLinkMsg(m_dpadY.m_id);
	UnRegisterMsg(m_dpadY.m_id);
	UnLinkMsg(RWS::iMsgRunningPreTick);
}

// The message data holds the player on the way in and becomes the event's value for that player.
static bool Answer(CMsg &msg, CtlEvent &event)
{
	if (msg.Id != event.m_id.pEventId)
		return false;
	msg.pData = (void *)(intptr_t)event.m_val[(int)(intptr_t)msg.pData];
	return true;
}

// Any message but the running tick is an inquiry: msg.pData holds the player (0 or 1), first translated to that player's pad; the answer replaces it.
// 0x406250
void CXboxStndController::HandleEvents(CMsg &msg)
{
	if (msg.Id == RWS::iMsgRunningPreTick) {
		m_controls->Poll();
		UpdatePadAssignment(false);
		return;
	}
	int player = (int)(intptr_t)msg.pData;
	int pad = -1;
	if (player == 0)
		pad = m_player1Pad;
	else if (player > 0)
		pad = m_player2Pad;
	if (pad >= 0)
		msg.pData = (void *)(intptr_t)pad;
	if (Answer(msg, m_turn) || Answer(msg, m_fwd) || Answer(msg, m_strafe) || Answer(msg, m_pitch) || Answer(msg, m_dpadX)
		|| Answer(msg, m_dpadY))
		return;
	for (unsigned i = 0; i < 8; i++) {
		if (Answer(msg, m_analogButton[i]))
			return;
		if (Answer(msg, m_digitalButton[i]))
			return;
	}
}

// Commands 1-4: the sticks, 5-12: analog buttons, 13-20: digital buttons, 21 and 22: the d-pad axes.
CtlEvent *CXboxStndController::GetEventForCommand(int command)
{
	switch (command) {
	case 1: return &m_turn;
	case 2: return &m_fwd;
	case 3: return &m_strafe;
	case 4: return &m_pitch;
	case 21: return &m_dpadX;
	case 22: return &m_dpadY;
	}
	if (command >= 5 && command <= 12)
		return &m_analogButton[command - 5];
	if (command >= 13 && command <= 20)
		return &m_digitalButton[command - 13];
	return 0;
}

// 0x404e70
void CXboxStndController::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	CAttributeCommandIterator attrIt(attr, g_xboxControllerClassId);
	while (!attrIt.IsFinished()) {
		CtlEvent *event = GetEventForCommand(attrIt->GetCommandId());
		if (event)
			ReplaceLinkedMsg(event->m_id, attrIt->GetAs_RwChar_ptr(), "return RwReal");
		++attrIt;
	}
}

// JUMP / A also claims a free pad here.
// 0x406370
void CtlRefresh()
{
	if (g_xboxController) {
		g_xboxController->m_controls->Poll();
		g_xboxController->UpdatePadAssignment(true);
	}
}

static inline PadState *GetPad(int index)
{
	return (g_controlsManager && (unsigned)index < 4) ? &g_pads[index] : 0;
}

// True while player `which` still needs a pad (1: only with two active controllers). Only meaningful once a pad was claimed.
// 0x4057f0
bool IsWaitingForPad(int which)
{
	if (!g_xboxController || !g_xboxController->m_padClaimed)
		return false;
	g_xboxController->m_connectedPads = 0;
	for (int i = 0; i < 4; i++) {
		if (g_pads[i].m_device)
			g_xboxController->m_connectedPads++;
	}
	if (g_activeControllers < 2) {
		g_xboxController->m_player2Pad = -1;
		g_xboxController->m_player2PadHeld = -1;
		if (g_xboxController->m_player1Pad < 0)
			return true;
	} else if (which == 0) {
		if (g_xboxController->m_player1Pad < 0)
			return true;
	} else if (which == 1) {
		if (g_xboxController->m_player2Pad < 0 && g_xboxController->m_player2PadHeld >= 0)
			return true;
	}
	return false;
}

// Strengths are 0..100, remapped to 25..100 unless zero. Nothing is sent when both are unchanged.
// The motor speeds are scaled by 655.35 and then by another 100, as the game does.
// 0x4058e0
void SetPadRumble(float a, float b)
{
	if (!g_xboxController)
		return;
	int pad = g_xboxController->m_player1Pad;
	if (pad < 0)
		return;
	if (a == g_rumbleA && b == g_rumbleB)
		return;
	g_rumbleA = a;
	g_rumbleB = b;
	if (a > 0.0f)
		a = a * 0.75f + 25.0f;
	if (a > 100.0f)
		a = 100.0f;
	if (b > 0.0f)
		b = b * 0.75f + 25.0f;
	if (b > 100.0f)
		b = 100.0f;
	PadState *p = &g_pads[pad];
	if (p->m_feedbackStatus != 0x3e5) {
		if (p->m_rumbleRightCaps)
			p->m_rumbleRight = (int)(a * 655.35f);
		if (p->m_rumbleLeftCaps)
			p->m_rumbleLeft = (int)(b * 655.35f);
	}
	p->m_rumbleRight *= 100;
	p->m_rumbleLeft *= 100;
	if (p->m_device)
		XInputSetState(p->m_device, &p->m_feedbackStatus);
}

// 0..1: the 32 bit stick value offset to unsigned, scaled by 2^-32.
static inline float StickUnit(int value)
{
	return ((float)value + 2147483648.0f) * 4.656612873077393e-10f;
}

// True when the pad is not idle (a stick away from rest, a button or d-pad pressed); it is then marked (m_hasInput).
// 0x405180
bool CXboxStndController::ReadPad(int player)
{
	// the game doesn't check the pad pointer: null when no controls manager exists
	PadState *pad = GetPad(player);
	bool changed = false;
	if (pad->m_device) {
		m_turn.f(player) = (1.0f - StickUnit(pad->m_thumbLX)) * 65535.0f;
		m_fwd.f(player) = (StickUnit(pad->m_thumbLY) - 1.0f) * 65535.0f;
		m_strafe.f(player) = (1.0f - StickUnit(pad->m_thumbRX)) * 65535.0f;
		m_pitch.f(player) = (StickUnit(pad->m_thumbRY) - 1.0f) * 65535.0f;
		unsigned short buttons = pad->m_buttons;
		if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT)
			m_dpadX.f(player) = 1.0f;
		else
			m_dpadX.m_val[player] = 0;
		if (buttons & XINPUT_GAMEPAD_DPAD_LEFT)
			m_dpadX.f(player) = -1.0f;
		if (buttons & XINPUT_GAMEPAD_DPAD_UP)
			m_dpadY.f(player) = 1.0f;
		else
			m_dpadY.m_val[player] = 0;
		if (buttons & XINPUT_GAMEPAD_DPAD_DOWN)
			m_dpadY.f(player) = -1.0f;
		// the game tests the left stick x (m_turn) four times here
		if (!(m_turn.f(player) < 0.8f && m_turn.f(player) > -0.8f && m_dpadX.f(player) == g_floatZero
			&& m_dpadY.f(player) == g_floatZero))
			changed = true;
		for (int k = 0; k < 8; k++) {
			m_analogButton[k].f(player) = pad->m_analogButtons[k] * (1.0f / 255.0f);
			if (pad->m_buttons & (unsigned short)g_btnMask[k])
				m_digitalButton[k].f(player) = 1.0f;
			else
				m_digitalButton[k].m_val[player] = 0;
			if (m_analogButton[k].f(player) != g_floatZero || m_digitalButton[k].f(player) != g_floatZero)
				changed = true;
		}
		if (changed)
			pad->m_hasInput = 1;
	}
	return changed;
}

// A pad with input while player 1 has none becomes player 1's (START, A when jumpClaims, anything while the language screen is
// selectable). With two active controllers START on another pad gives player 2 its pad. Vanished pads are released.
// 0x405680
void CXboxStndController::UpdatePadAssignment(bool jumpClaims)
{
	for (int i = 0; i < 4; i++) {
		if (ReadPad(i)) {
			if (m_player1Pad < 0 && i != m_player2Pad && i != m_player2PadHeld && i != m_lostPad
				&& (IsLanguage7Selectable() || m_digitalButton[4].f(i) != 0.0f || (jumpClaims && m_analogButton[0].f(i) > 0.8f))) {
				m_player1Pad = i;
				m_padClaimed = 1;
			} else if (g_activeControllers == 2 && m_player2Pad < 0 && i != m_player1Pad && m_digitalButton[4].f(i) != 0.0f) {
				m_player2Pad = i;
				m_player2PadHeld = i;
			}
		} else if (i == m_lostPad) {
			m_lostPad = -1;
		}
	}
	if (m_player1Pad >= 0) {
		if (!GetPad(m_player1Pad)->m_device) {
			m_player1Pad = -1;
			m_lostPad = -1;
		}
	}
	if (m_player2Pad >= 0) {
		if (!GetPad(m_player2Pad)->m_device)
			m_player2Pad = -1;
		if (g_activeControllers == 1) {
			m_player2Pad = -1;
			m_player2PadHeld = -1;
		}
	}
}

// CEventId::~CEventId, jumped to from the exception unwind funclets: a folded no-op.
ZOOSTERS_NOINLINE void Fold_Nop();
// 0x405db0
void CEventId_Dtor_thunk()
{
	Fold_Nop();
}
