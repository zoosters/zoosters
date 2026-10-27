// Image layout (saveformat.h): kSlotCount records of [slot header (0x2c bytes)][slot data (SaveSize bytes)] one after the other.
// The headers go between image and MemCardSlot through LoadSlotHeader / StoreSlotHeader only.
#include <string.h>
#include "modules/tfb/memcard.h"
#include "modules/tfb/savefile.h"
#include "tfb/save/language.h"

// GLOBAL: 0x445ee0 ?Hash@MemCard@@QAEIPBDH@Z

static const int kSaveBufferSize = 0x4000;   // g_saveBuffer is cleared whole, whatever the slots need

// 0x446630
int MemCard::SlotStride()
{
	return SaveSize() + SaveFormat::kSlotHeaderSize;
}

// 0x446670
void MemCard::ResetSlots()
{
	memset(m_image, 0, kSaveBufferSize);
	for (int i = 0; i < kSlotCount; i++) {
		memset(&m_slots[i], 0, sizeof(MemCardSlot));
		m_slots[i].m_state = MEMCARD_SLOT_VALID;
		m_slots[i].m_checksum = SaveFormat::SaveChecksum((const uint8_t *)m_image + SaveFormat::kSlotHeaderSize, SaveSize());
		m_slots[i].m_language = -1;
	}
}

// 0x446860
bool MemCard::LoadSlot(int slot)
{
	MemCardSlot *s = &m_slots[slot];
	m_curSlot = slot;
	s->m_buf = m_image + SlotStride() * slot + SaveFormat::kSlotHeaderSize;
	if (SaveFileOpen(SAVEFILE_READ, 0))
		m_state = MEMCARD_LOAD_READ_IMAGE;
	return false;
}

// 0x446d00
void MemCard::ClearSlot(int slot)
{
	char *header = m_image + SlotStride() * slot;
	memset(header, 0, SlotStride());
	MemCardSlot *s = &m_slots[slot];
	memset(s, 0, sizeof(MemCardSlot));
	s->m_state = MEMCARD_SLOT_VALID;
	int lang = GetLanguage();
	s->m_language = lang;
	unsigned checksum = SaveFormat::SaveChecksum((const uint8_t *)header + SaveFormat::kSlotHeaderSize, SaveSize());
	s->m_checksum = checksum;
	// the slot in the file stays empty (state 0, size 0): only the language and the checksum of the zero data are set
	MemCardSlot empty;
	memset(&empty, 0, sizeof(MemCardSlot));
	empty.m_language = lang;
	empty.m_checksum = checksum;
	StoreSlotHeader(header, empty);
}


// 0x446720
void MemCard::ValidateSlots()
{
	for (int i = 0; i < kSlotCount; i++) {
		MemCardSlot *dst = &m_slots[i];
		char *header = m_image + SlotStride() * i;
		const uint8_t *data = (const uint8_t *)header + SaveFormat::kSlotHeaderSize;
		MemCardSlot probe;
		LoadSlotHeader(header, &probe);
		if ((probe.m_size == 0 || probe.m_size == SaveSize()) && SaveFormat::SaveChecksum(data, SaveSize()) == probe.m_checksum) {
			LoadSlotHeader(header, dst);
			SaveFormat::WriteSlotLanguage((uint8_t *)header, GetLanguage());
			dst->m_buf = header + SaveFormat::kSlotHeaderSize;
		} else {
			m_error = MEMCARD_STATUS_BAD_DATA;
			dst->m_buf = 0;
			return;
		}
	}
}


// 0x447020
bool MemCard::SaveSlot(int slot)
{
	if (SaveFileOpen(SAVEFILE_READ_CHECKED, slot >= 0 ? ImageSize() : -1) == 0) {
		if (MemCardGetStatus() != MEMCARD_STATUS_OK && MemCardGetStatus() != MEMCARD_STATUS_IO_ERROR && MemCardGetStatus() != MEMCARD_STATUS_OPEN_FAILED)
			return false;
		if (slot >= 0) {
			MemCardSlot *s = &m_slots[slot];
			char *header = m_image + SlotStride() * slot;
			s->m_buf = header + SaveFormat::kSlotHeaderSize;
			SaveVariables(s, 0);
			StoreSlotHeader(header, *s);
		} else {
			// (one call per slot as the game: a loop compiles differently)
			ClearSlot(0);
			ClearSlot(1);
			ClearSlot(2);
			ClearSlot(3);
		}
		m_state = MEMCARD_SAVE_WRITE_IMAGE;
		bool ok = SaveFileOpen(SAVEFILE_WRITE, ImageSize()) != 0;
		return ok;
	}
	m_state = MEMCARD_SAVE_READ_IMAGE;
	return true;
}
