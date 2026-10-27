#include "modules/tfb/memcard.h"
#include "modules/tfb/savefile.h"

// GLOBAL: 0x62a344 RWS::iMsgRunningPostTick
// GLOBAL: 0x626550 g_memCardLastTick
namespace RWS { extern CEventId iMsgRunningPostTick; }
extern unsigned g_memCardLastTick;     // 0x626550 time of the last request taken over
unsigned GetTimeMs();                  // 0x44d2c0
bool Fold_ReturnFalse();               // 0x445e10 returns 0

static const unsigned kPollIntervalMs = 0xA6;       // idle: ask for a POLL step this often ...
static const unsigned kNotReadyPollIntervalMs = 0x14; // ... or this often while the card reports NOT_READY

// Once per RunningPostTick, advances the state by one step once the pending file transfer is done. m_mode is the request written by the
// script, m_state the current state.
// 0x447170
void MemCard::HandleEvents(RWS::CMsg &msg)
{
	if (!(msg.Id == RWS::iMsgRunningPostTick.pEventId))
		return;
	unsigned char ready = SaveFileIoDone();
	if (!m_changed) {
		if (m_changedBlock) {
			unsigned char b = Script_AlwaysTrue() != 0;
			m_changedBlock = b;
			if (!b)
				m_changed = 1;
		} else {
			unsigned char b = Script_AlwaysTrue() != 0;
			m_changedBlock = b;
			if (b)
				m_changed = 1;
		}
	}
	if (MemCardGetStatus() > 1) {
		if (m_state == MEMCARD_OPEN_INIT || m_state == MEMCARD_READ_IMAGE) {
			if (MemCardGetStatus() == MEMCARD_STATUS_OPEN_FAILED || MemCardGetStatus() == MEMCARD_STATUS_IO_ERROR) {
				SaveFileClose();
				MemCardClearStatus();
			}
		}
		if (MemCardGetStatus() == MEMCARD_STATUS_NOT_READY)
			m_error = 0;
		if (m_state != MEMCARD_IDLE && m_state != MEMCARD_POLL && m_state != MEMCARD_FORMAT && m_state != MEMCARD_DELETE_FILE)
			m_state = MEMCARD_CLOSE;
		m_saveDone = 0;
		m_loadDone = 0;
		m_deleteDone = 0;
		m_formatDone = 0;
		m_createDone = 0;
	}
	if (!ready)
		return;
	switch (m_state) {
	case MEMCARD_CLOSE:
		SaveFileClose();
		m_state = MEMCARD_IDLE;
		// fall through
	case MEMCARD_IDLE:
		if (m_mode == 0) {
			unsigned elapsed = GetTimeMs() - g_memCardLastTick;
			if (elapsed > kPollIntervalMs || (MemCardGetStatus() == MEMCARD_STATUS_NOT_READY && elapsed > kNotReadyPollIntervalMs))
				m_mode = MEMCARD_POLL;
		}
		if (m_mode != 0) {
			g_memCardLastTick = GetTimeMs();
			int mode = m_mode;
			m_mode = 0;
			m_activeMode = mode;
			m_state = mode;
		}
		return;
	case MEMCARD_SAVE_SLOT:
		m_error = 0;
		SaveSlot(m_curSlot);
		m_saveDone = 1;
		return;
	case MEMCARD_LOAD_SLOT:
		m_error = 0;
		LoadSlot(m_curSlot);
		m_loadDone = 1;
		return;
	case MEMCARD_DELETE_FILE:
		m_error = 0;
		m_deleteDone = 1;
		if (!SaveFileDelete())
			m_deleteDone = 0;
		m_state = MEMCARD_IDLE;
		return;
	case MEMCARD_FORMAT:
		m_error = 0;
		m_formatDone = 1;
		if (!Fold_ReturnFalse())
			m_formatDone = 0;
		// fall through
	case MEMCARD_POLL:
		m_state = MEMCARD_IDLE;
		return;
	case MEMCARD_CREATE_FILE:
		m_error = 0;
		m_createDone = 1;
		SaveSlot(-1);
		return;
	case MEMCARD_REQUEST_INIT:
		m_error = 0;
		m_state = MEMCARD_OPEN_INIT;
		return;
	case MEMCARD_OPEN_INIT:
		SaveFileOpen(SAVEFILE_READ_CHECKED, SlotStride() * kSlotCount);
		m_state = MEMCARD_READ_IMAGE;
		return;
	case MEMCARD_READ_IMAGE:
		SaveFileReadAsync(m_image, SlotStride() * kSlotCount);
		m_state = MEMCARD_VALIDATE_IMAGE;
		return;
	case MEMCARD_VALIDATE_IMAGE:
		SaveFileReset();
		m_state = MEMCARD_CLOSE;
		ValidateSlots();
		return;
	case MEMCARD_LOAD_READ_IMAGE:
		m_state = MEMCARD_LOAD_FINISH;
		SaveFileReadAsync(m_image, SlotStride() * kSlotCount);
		return;
	case MEMCARD_SAVE_READ_IMAGE:
		m_state = MEMCARD_SAVE_MERGE;
		SaveFileReadAsync(m_image, SlotStride() * kSlotCount);
		return;
	case MEMCARD_SAVE_MERGE: {
		m_state = MEMCARD_SAVE_WRITE_IMAGE;
		SaveFileClose();
		MemCardSlot *slot = &m_slots[m_curSlot];
		MemCardSlot saved = *slot;
		ValidateSlots();
		*slot = saved;
		char *header = m_image + SlotStride() * m_curSlot;
		slot->m_buf = header + SaveFormat::kSlotHeaderSize;
		SaveVariables(slot, 0);
		StoreSlotHeader(header, *slot);
		if (SaveFileOpen(SAVEFILE_WRITE, SlotStride() * kSlotCount) == 0) {
			m_state = MEMCARD_CLOSE;
			m_error = MEMCARD_STATUS_NOT_READY;
		}
		return;
	}
	case MEMCARD_SAVE_WRITE_IMAGE:
		m_state = MEMCARD_SAVE_FINISH;
		SaveFileWriteAsync(m_image, SlotStride() * kSlotCount);
		return;
	case MEMCARD_LOAD_FINISH:
	case MEMCARD_SAVE_FINISH:
		SaveFileReset();
		m_state = MEMCARD_CLOSE;
		return;
	}
}
