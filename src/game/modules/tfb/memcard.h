#ifndef ZOOSTERS_MODULES_TFB_MEMCARD_H
#define ZOOSTERS_MODULES_TFB_MEMCARD_H
#include <string.h>
#include "script/script.h"
#include "modules/tfb/saveformat.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
// PS2 base order: CEventHandler@0, CAttributeHandler@8, LinearAllocationPolicy@36 (no CSystemCommands).
// Vtables 0x5d5078 (CEventHandler part: ~ 0x4475f0, HandleEvents 0x447170) and 0x5d5068 (CAttributeHandler part at +8:
// thunk 0x4463b0, HandleAttributes 0x445eb0 (the same address is in GameController's vtable), base, base). Registered size 0x100.
// A save slot description as the game keeps it in memory: 0x2c bytes at 32 bits, four of them at MemCard+0x38. Only the fields of
// SaveFormat::SaveSlotHeader go to the save file, and only through LoadSlotHeader / StoreSlotHeader below (m_buf is a pointer).
struct MemCardSlot {
	int m_size;       // +0x00
	int m_state;      // +0x04 8 = valid
	int m_entryCount; // +0x08 number of saved script variables
	int m_language;   // +0x0c language id (-1 after ResetSlots)
	int m_percent;    // +0x10
	int m_gameTime;   // +0x14
	unsigned m_checksum;   // +0x18 checksum of the slot's data
	char m_levelName[0xc]; // +0x1c
	char *m_buf;      // +0x28 points into the save buffer behind this slot's data; PTR32: not in the file
};
ZOOSTERS_ASSERT_SIZE(MemCardSlot, 0x2c);

// The header of a slot in the save image (SaveFormat::kHeaderSize bytes at `header`) into the in-memory slot. m_buf is not read from
// the file: the game stored its own buffer address there and every reader overwrote it, so it is left as it is.
static inline void LoadSlotHeader(const char *header, MemCardSlot *slot)
{
	SaveFormat::SaveSlotHeader h;
	SaveFormat::ReadSlotHeader((const uint8_t *)header, &h);
	slot->m_size = h.size;
	slot->m_state = h.state;
	slot->m_entryCount = h.entryCount;
	slot->m_language = h.language;
	slot->m_percent = h.percent;
	slot->m_gameTime = h.gameTime;
	slot->m_checksum = h.checksum;
	memcpy(slot->m_levelName, h.levelName, sizeof(slot->m_levelName));
}

// The in-memory slot into the save image. The 0x28 word receives the low 32 bits of m_buf, as the 32-bit game wrote the pointer.
static inline void StoreSlotHeader(char *header, const MemCardSlot &slot)
{
	SaveFormat::SaveSlotHeader h;
	h.size = slot.m_size;
	h.state = slot.m_state;
	h.entryCount = slot.m_entryCount;
	h.language = slot.m_language;
	h.percent = slot.m_percent;
	h.gameTime = slot.m_gameTime;
	h.checksum = slot.m_checksum;
	memcpy(h.levelName, slot.m_levelName, sizeof(h.levelName));
	h.bufPtr = SaveFormat::PointerWord(slot.m_buf);
	SaveFormat::WriteSlotHeader((uint8_t *)header, h);
}

// m_state is the step of the HandleEvents state machine; m_mode is the request a script member leaves (ExposeMemCardAutoSave
// etc.) and becomes the state of the same value. Names are ours, numbers the game's. Three file sequences:
//  init   (REQUEST_INIT): OPEN_INIT -> READ_IMAGE -> VALIDATE_IMAGE   open the file for reading, read all four slots, check them
//  load   (LOAD_SLOT):    LOAD_READ_IMAGE -> LOAD_FINISH             read the image, then LoadVariables is run by the script
//  save   (SAVE_SLOT):    SAVE_READ_IMAGE -> SAVE_MERGE -> SAVE_WRITE_IMAGE -> SAVE_FINISH   read-modify-write of one slot
enum MemCardState {
	MEMCARD_IDLE = 0,
	MEMCARD_CLOSE = 1,           // close the file, then idle
	MEMCARD_SAVE_SLOT = 2,
	MEMCARD_LOAD_SLOT = 3,
	MEMCARD_DELETE_FILE = 4,
	MEMCARD_FORMAT = 5,          // does nothing (Fold_ReturnFalse), flagged as failed
	MEMCARD_CREATE_FILE = 6,     // writes a fresh image with four empty slots
	MEMCARD_REQUEST_INIT = 7,
	MEMCARD_POLL = 8,            // periodic check of the card, only moves to idle
	MEMCARD_READ_IMAGE = 9,
	MEMCARD_VALIDATE_IMAGE = 10,
	MEMCARD_OPEN_INIT = 11,
	MEMCARD_LOAD_READ_IMAGE = 12,
	MEMCARD_LOAD_FINISH = 13,
	MEMCARD_SAVE_READ_IMAGE = 14,
	MEMCARD_SAVE_MERGE = 15,
	MEMCARD_SAVE_WRITE_IMAGE = 16,
	MEMCARD_SAVE_FINISH = 17
};

enum { MEMCARD_SLOT_VALID = 8 };

class MemCard : public RWS::CEventHandler, public RWS::CAttributeHandler, public RWS::LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(MemCard);
	static const int kSlotCount = SaveFormat::kSlotCount;
	MemCard(const RWS::CAttributePacket &attr);
	~MemCard();
	virtual void HandleEvents(RWS::CMsg &msg);
	virtual void HandleAttributes(const RWS::CAttributePacket &attr);
	char m_pad24[4];              // +0x24 unused
	int m_mode;                   // +0x28
	int m_activeMode;             // +0x2c the request taken over from m_mode
	int m_state;                  // +0x30
	int m_error;                  // +0x34
	MemCardSlot m_slots[kSlotCount]; // +0x38
	int m_curSlot;                // +0xe8
	unsigned char m_loadDone;     // +0xec (cleared by AutoLoad::GetInt after it triggered the load)
	unsigned char m_saveDone;     // +0xed
	unsigned char m_deleteDone;   // +0xee
	unsigned char m_formatDone;   // +0xef
	unsigned char m_createDone;   // +0xf0
	unsigned char m_f1;           // +0xf1 only ever cleared by the constructor
	char m_padf2[2];
	char *m_image;                // +0xf4 save image: 4 x SlotStride bytes (g_saveBuffer)
	int m_slotDataSize;           // +0xf8 size of one slot's data, computed lazily by SaveSize
	unsigned char m_changed;      // +0xfc
	unsigned char m_changedBlock; // +0xfd
	char m_padfe[2];
	// the bookkeeping the "auto" members reset (inlined by the game)
	void ClearAuto()
	{
		m_error = 0;
		m_state = 1;
		m_mode = 0;
		m_loadDone = 0;
		m_formatDone = 0;
		m_saveDone = 0;
		m_deleteDone = 0;
	}
	bool LoadVariables();   // true = the save slot was valid and applied
	void ClearSlot(int slot);
	int ImageSize() { return SaveSize() * kSlotCount + kSlotCount * SaveFormat::kSlotHeaderSize; }
	int SlotStride();   // SaveSize + SaveFormat::kSlotHeaderSize: the size of one slot in m_image; computes m_slotDataSize lazily
	void ValidateSlots();   // checks size and checksum of the four slot headers in m_image, copies them to m_slots
	int SaveVariables(MemCardSlot *slot, int flag);   // serialises the script variables into slot->m_buf, flag 1 = size only
	bool SaveSlot(int slot);   // slot < 0 clears all four
	bool LoadSlot(int slot);
	void ResetSlots();
	// lazily computed size of one save slot's data (inlined everywhere; out-of-line copy SlotStride returns it + 0x2c)
	int SaveSize()
	{
		if (!m_slotDataSize) {
			MemCardSlot tmp;
			tmp.m_buf = 0;
			m_slotDataSize = SaveVariables(&tmp, 1);
		}
		return m_slotDataSize;
	}
	unsigned Hash(const char *data, int size);   // thiscall form of the checksum, this unused
};
ZOOSTERS_ASSERT_SIZE(MemCard, 0x100);
ZOOSTERS_ASSERT_OFFSET(MemCard, m_mode, 0x28);
ZOOSTERS_ASSERT_OFFSET(MemCard, m_state, 0x30);
ZOOSTERS_ASSERT_OFFSET(MemCard, m_slots, 0x38);
ZOOSTERS_ASSERT_OFFSET(MemCard, m_curSlot, 0xe8);
ZOOSTERS_ASSERT_OFFSET(MemCard, m_image, 0xf4);
ZOOSTERS_ASSERT_OFFSET(MemCard, m_changed, 0xfc);

bool QuitGame();   // PostQuitMessage(0), returns false
extern MemCard *g_memCard;   // the one MemCard behaviour (set by its constructor while playing, null otherwise)
#endif
