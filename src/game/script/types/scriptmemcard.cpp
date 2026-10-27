// The script members of the MemCard object (Script::ScriptMemCard::Expose*): they read and write the one MemCard behaviour
// (g_memCard) and the save file layer. Script requests (AutoSave = 1 ...) leave the request in MemCard::m_mode; HandleEvents runs it.
#include "modules/tfb/memcard.h"
#include "modules/tfb/savefile.h"
#include "tfb/save/language.h"
#include "script/types/scriptmemcard.h"
#include "script/types/scriptmemcard.h"

// GLOBAL: 0x626538 g_memCard
// GLOBAL: 0x622524 g_memCardReady
// GLOBAL: 0x603558 g_memCardStatus

// What the Auto-* members return to the script: a request is running (the card is busy) or failed, else 0 (idle / done).
enum { AUTO_IDLE = 0, AUTO_RUNNING = 1, AUTO_FAILED = 2 };
#define MemCardGetStatusAgain MemCardGetStatus   // the same function: a normal build links one symbol

// NOTE: defined here, not in savefile.cpp: the four Auto-* GetInt below only compile to the game's code when cl sees this body
// (it proves the call leaves m_state / m_mode alone and drops their re-test; the game re-tests).
// 0x445700
ZOOSTERS_NOINLINE int MemCardGetStatus()
{
	int status;
	if (!g_memCardReady)
		status = MEMCARD_STATUS_NOT_INITIALISED;
	else
		status = g_memCardStatus;
	return status;
}
// the four Expose*::GetInt match with the early-return test split from the body: `if (m && (state || mode)) return AUTO_RUNNING; if (m) {...}`.
// AutoLoad calls MemCardGetStatusAgain = the same function through a second declaration: with the visible same-TU body cl proves the call
// leaves state/mode alone and drops the game's re-test.

// 0x445f70
int Script::ScriptMemCard::ExposeMemCardAutoSave::GetInt(ScriptReference *owner)
{
	MemCard *m = g_memCard;
	if (m && (m->m_state != 0 || m->m_mode != 0))
		return AUTO_RUNNING;
	if (m) {
		m->m_saveDone = 0;
		if (MemCardGetStatus() > MEMCARD_STATUS_BUSY)
			return AUTO_FAILED;
	}
	return 0;
}

// 0x446010
int Script::ScriptMemCard::ExposeMemCardAutoDelete::GetInt(ScriptReference *owner)
{
	MemCard *m = g_memCard;
	if (m && (m->m_state != 0 || m->m_mode != 0))
		return AUTO_RUNNING;
	if (m) {
		m->m_deleteDone = 0;
		if (MemCardGetStatus() > MEMCARD_STATUS_BUSY)
			return AUTO_FAILED;
	}
	return 0;
}

// 0x4460b0
int Script::ScriptMemCard::ExposeMemCardAutoFormat::GetInt(ScriptReference *owner)
{
	MemCard *m = g_memCard;
	if (m && (m->m_state != 0 || m->m_mode != 0))
		return AUTO_RUNNING;
	if (m) {
		m->m_formatDone = 0;
		if (MemCardGetStatus() > MEMCARD_STATUS_BUSY)
			return AUTO_FAILED;
	}
	return 0;
}

// 0x446150
int Script::ScriptMemCard::ExposeMemCardAutoCreate::GetInt(ScriptReference *owner)
{
	MemCard *m = g_memCard;
	if (m && (m->m_state != 0 || m->m_mode != 0))
		return AUTO_RUNNING;
	if (m) {
		m->m_createDone = 0;
		if (MemCardGetStatus() > MEMCARD_STATUS_BUSY)
			return AUTO_FAILED;
	}
	return 0;
}

// 0x446e50
int Script::ScriptMemCard::ExposeMemCardAutoLoad::GetInt(ScriptReference *owner)
{
	MemCard *m = g_memCard;
	if (m && (m->m_state != 0 || m->m_mode != 0))
		return AUTO_RUNNING;
	if (m) {
		int st = MemCardGetStatusAgain();
		if (m->m_error)
			st = m->m_error;
		if (st == 0) {
			if ((m->m_state != 0 || m->m_mode != 0) && m->m_state != 8 && m->m_mode != 8)
				st = 1;
		}
		if (!m->m_loadDone) {
			if (st > MEMCARD_STATUS_BUSY || MemCardGetStatusAgain() > MEMCARD_STATUS_BUSY)
				return AUTO_FAILED;
		}
		if (st == 0) {
			m->LoadVariables();
			m->m_loadDone = 0;
		}
	}
	return 0;
}


namespace Script {

// 0x445f10
void ScriptMemCard::ExposeMemCardAutoSave::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc) {
		if (value) {
			mc->m_mode = MEMCARD_SAVE_SLOT;
		} else {
			SaveFileClose();
			MemCardClearStatus();
			mc->ClearAuto();
		}
	}
}

// 0x445fb0
void ScriptMemCard::ExposeMemCardAutoDelete::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc) {
		if (value) {
			mc->m_mode = MEMCARD_DELETE_FILE;
		} else {
			SaveFileClose();
			MemCardClearStatus();
			mc->ClearAuto();
		}
	}
}

// 0x446050
void ScriptMemCard::ExposeMemCardAutoFormat::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc) {
		if (value) {
			mc->m_mode = MEMCARD_FORMAT;
		} else {
			SaveFileClose();
			MemCardClearStatus();
			mc->ClearAuto();
		}
	}
}

// 0x4460f0
void ScriptMemCard::ExposeMemCardAutoCreate::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc) {
		if (value) {
			mc->m_mode = MEMCARD_CREATE_FILE;
		} else {
			SaveFileClose();
			MemCardClearStatus();
			mc->ClearAuto();
		}
	}
}

// 0x446190
int ScriptMemCard::ExposeMemCardUnplugged::GetInt(ScriptReference *owner)
{
	return Script_AlwaysTrue() == 0;
}

// 0x4461a0
int ScriptMemCard::ExposeMemCardStatus::GetInt(ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	int status = MemCardGetStatusAgain();
	if (mc) {
		if (mc->m_error)
			status = mc->m_error;
		if (status == MEMCARD_STATUS_OK) {
			if (mc->m_state || mc->m_mode) {
				if (mc->m_state != MEMCARD_POLL && mc->m_mode != MEMCARD_POLL)
					status = 1;
			}
		}
	}
	return status;
}

// 0x4461e0
void ScriptMemCard::ExposeMemCardStatus::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc) {
		if (value == 0) {
			SaveFileClose();
			MemCardClearStatus();
			mc->m_error = 0;
			mc->m_mode = 0;
			mc->m_loadDone = 0;
			mc->m_formatDone = 0;
			mc->m_saveDone = 0;
			mc->m_deleteDone = 0;
			mc->m_state = MEMCARD_CLOSE;
		} else if (value == MEMCARD_STATUS_BUSY) {
			mc->m_mode = MEMCARD_REQUEST_INIT;
		} else if (value == MEMCARD_STATUS_NOT_INITIALISED) {
			QuitGame();
		}
	}
}

// 0x446260
void ScriptMemCard::ExposeMemCardCurrentSlot::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc && value < MemCard::kSlotCount)
		mc->m_curSlot = value;
}

// 0x446280
int ScriptMemCard::ExposeMemCardCurrentSlot::GetInt(ScriptReference *owner)
{
	if (g_memCard)
		return g_memCard->m_curSlot;
	return 0;
}

// 0x4462a0
int ScriptMemCard::ExposeMemCardSlotSize::GetInt(ScriptReference *owner)
{
	if (g_memCard) {
		MemCardSlot *slot = &g_memCard->m_slots[g_memCard->m_curSlot];
		if (slot->m_state == MEMCARD_SLOT_VALID)
			return slot->m_size;
	}
	return 0;
}

// 0x4462d0
int ScriptMemCard::ExposeMemCardSlotPercent::GetInt(ScriptReference *owner)
{
	if (g_memCard) {
		MemCardSlot *slot = &g_memCard->m_slots[g_memCard->m_curSlot];
		if (slot->m_state == MEMCARD_SLOT_VALID)
			return slot->m_percent;
	}
	return 0;
}

// 0x446300
void ScriptMemCard::ExposeMemCardSlotPercent::SetValue(int value, ScriptReference *owner)
{
	if (g_memCard)
		g_memCard->m_slots[g_memCard->m_curSlot].m_percent = value;
}

// 0x446320
int ScriptMemCard::ExposeMemCardLanguage::GetInt(ScriptReference *owner)
{
	return GetLanguage();
}

// 0x446330
int ScriptMemCard::ExposeMemCardSlotGameTime::GetInt(ScriptReference *owner)
{
	if (g_memCard) {
		MemCardSlot *slot = &g_memCard->m_slots[g_memCard->m_curSlot];
		if (slot->m_state == MEMCARD_SLOT_VALID)
			return slot->m_gameTime;
	}
	return 0;
}

// 0x446360
int ScriptMemCard::ExposeMemCardChangedState::GetInt(ScriptReference *owner)
{
	if (g_memCard)
		return g_memCard->m_changed;
	return 0;
}

// 0x446380
void ScriptMemCard::ExposeMemCardChangedState::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc) {
		mc->m_changed = value != 0;
		mc->m_changedBlock = Script_AlwaysTrue() != 0;
	}
}

// 0x446df0
void ScriptMemCard::ExposeMemCardAutoLoad::SetValue(int value, ScriptReference *owner)
{
	MemCard *mc = g_memCard;
	if (mc) {
		if (value) {
			mc->LoadVariables();
		} else {
			SaveFileClose();
			MemCardClearStatus();
			mc->ClearAuto();
		}
	}
}

// 0x446ef0
void ScriptMemCard::ExposeMemCardSlotSize::SetValue(int value, ScriptReference *owner)
{
	if (g_memCard) {
		if (value == 0)
			g_memCard->ClearSlot(g_memCard->m_curSlot);
		else
			g_memCard->m_mode = MEMCARD_REQUEST_INIT;
	}
}


}  // namespace Script

Script::ScriptReference *TFBType_MemCard();

namespace Script {

// 0x43ea40
const char *ScriptMemCard::ExposeMemCardUnplugged::GetName()
{
	return "Unplugged";
}

// 0x43f880
ScriptReference *ScriptMemCard::GetType()
{
	return TFBType_MemCard();
}

// 0x43e690
const char *ScriptMemCard::GetName()
{
	return "MemCard";
}

}  // namespace Script


namespace Script {

// 0x43ea50
const char *ScriptMemCard::ExposeMemCardFreeBlocks::GetName()
{
	return "Free Space";
}

// 0x43ea60
const char *ScriptMemCard::ExposeMemCardStatus::GetName()
{
	return "Status";
}

// 0x43ea70
const char *ScriptMemCard::ExposeMemCardAutoSave::GetName()
{
	return "Auto-Save";
}

// 0x43ea80
const char *ScriptMemCard::ExposeMemCardAutoLoad::GetName()
{
	return "Auto-Load";
}

// 0x43ea90
const char *ScriptMemCard::ExposeMemCardCurrentSlot::GetName()
{
	return "Current Slot";
}

// 0x43eab0
const char *ScriptMemCard::ExposeMemCardSlotSize::GetName()
{
	return "Current Slot Size";
}

// 0x43ead0
const char *ScriptMemCard::ExposeMemCardAutoFormat::GetName()
{
	return "Auto-Format";
}

// 0x43eaf0
const char *ScriptMemCard::ExposeMemCardAutoDelete::GetName()
{
	return "Auto-Delete";
}

// 0x43eb10
const char *ScriptMemCard::ExposeMemCardLanguage::GetName()
{
	return "Saved Language";
}

// 0x43eb30
const char *ScriptMemCard::ExposeMemCardSlotPercent::GetName()
{
	return "Current Slot Percent Complete";
}

// 0x43eb50
const char *ScriptMemCard::ExposeMemCardAutoCreate::GetName()
{
	return "Auto-Create";
}

// 0x43eb70
const char *ScriptMemCard::ExposeMemCardSlotGameTime::GetName()
{
	return "Current Slot Game Time";
}

// 0x43eb90
const char *ScriptMemCard::ExposeMemCardChangedState::GetName()
{
	return "Changed State";
}

}  // namespace Script

// fallback free function; Ghidra name: Script::ScriptMemCard::ExposeMemCardFreeBlocks::GetInt
// 0x446250
int __stdcall ExposeMemCardFreeBlocks_GetInt(int a1)
{
	return 0xf4240;
}
