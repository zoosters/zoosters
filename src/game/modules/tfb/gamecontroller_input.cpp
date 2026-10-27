#define ZOOSTERS_RWS_SENDMSG
// The owner is one of the five player-reference objects of TFB_GetPlayerRefs; input -1 polls every input (0..5).
// (the disabled version below needs: #define ZOOSTERS_RWS_SENDMSG 1, and // GLOBAL: 0x617990 RWS::ERROR_NOT_A_VALID_MESSAGE_ID)
#include "modules/tfb/gamecontroller.h"
#include <math.h>
#include "script/types/scriptcontroller.h"

// GLOBAL: 0x6179a4 g_ctrlTimeBase
int g_ctrlTimeBase;                            // time base of ControllerElapsedMs (ms)
// m_blockMask: inputs 0..5 ignored until released, checked from m_blockUntil (ms) on (set by 0x4028a0)
extern RWS::TFB::GameController *g_gameController;   // GLOBAL: 0x61798c g_gameController
int *CutScene_StackSlot(int delta);            // 0x436c00
unsigned GetTimeMs();                       // 0x44d2c0

// 0x402880
float GetPlayerInput(int input, float threshold, int player)
{
	return GetInputValue((int *)(TFB_GetPlayerRefs() + player), input, threshold);   // the ref objects are pointer-sized (vptr): 8 bytes on x64
}

// GLOBAL: 0x617990 RWS::ERROR_NOT_A_VALID_MESSAGE_ID
// Disabled: semantically complete, but the game's block layout differs.
// 0x402c80
float GetInputValue(int *owner, int input, float threshold)
{
	// the pad index in ControllerPlayerRef units (x64: 8-byte objects; the int * difference made ref #1, the keyboard "player 1" that is
	// read as pad 0, into player 2: a different pad)
	int player = (int)((Script::ControllerPlayerRef *)owner - TFB_GetPlayerRefs());
	int ctl = input;
	if (player == 1) {
		player = 0;
		if (input == 2)
			ctl = 1;
		else if (input == 3)
			ctl = 6;
		else if (input == 4)
			ctl = 7;
		else if (input == 5)
			ctl = 8;
		else if (input == 0)
			ctl = 9;
	} else if (player < 0 || player >= 4)
		return 0.0f;
	if (!g_gameController || *CutScene_StackSlot(0) == -1)
		return 0.0f;
	int lo = ctl;
	int hi = ctl;
	if (ctl == -1) {
		unsigned mask = g_gameController->m_blockMask;
		lo = 0;
		hi = 5;
		if (mask && GetTimeMs() >= g_gameController->m_blockUntil) {
			g_gameController->m_blockMask = 0;
			for (int i = 0; i < 6; i++) {
				unsigned bit = 1 << i;
				if ((mask & bit) && GetPlayerInput(i, threshold, 0) == 0.0f)
					mask &= ~bit;
			}
			g_gameController->m_blockMask = mask;
		}
	}
	for (int i = lo; i <= hi; i++) {
		if (!(g_gameController->m_blockMask & (1 << i))) {
			RWS::CMsg msg(g_gameController->m_events[i], (void *)(intptr_t)player);
			RWS::SendMsg(msg);
			float a = fabsf(*(float *)&msg.pData);
			if (a > threshold) {
				if (a > 0.25f) {
					g_ctrlTimeBase = GetTimeMs();
					GetTimeMs();
				}
				return *(float *)&msg.pData;
			}
		}
	}
	return 0.0f;
}

// 0x402860
int ControllerElapsedMs(char reset)
{
	if (reset)
		g_ctrlTimeBase = GetTimeMs();
	return GetTimeMs() - g_ctrlTimeBase;
}

// Ignores every input (0..5) until it is released and the delay (ms) has passed.
// 0x4028a0
void BlockInputsUntilReleased(int delay)
{
	if (g_gameController) {
		g_gameController->m_blockMask = 0;
		g_gameController->m_blockMask |= 1;
		g_gameController->m_blockMask |= 2;
		g_gameController->m_blockMask |= 4;
		g_gameController->m_blockMask |= 8;
		g_gameController->m_blockMask |= 0x10;
		g_gameController->m_blockMask |= 0x20;
		if (g_gameController->m_blockMask)
			g_gameController->m_blockUntil = GetTimeMs() + delay;
	}
}
