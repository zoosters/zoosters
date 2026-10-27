// A slot is a list of entries, one per saveable script variable (every Value in the global symbol range):
// [key = hash of the typed name, or the user variable's name][value: 4 bytes, or the data block of a user array].
// Ints are stored with bit 31 set, floats as is. LoadVariables checks size, checksum and state, then walks the variables
// in step with the entries; any mismatch sets m_error = MEMCARD_STATUS_BAD_DATA
#include <string.h>
#include "modules/tfb/memcard.h"
#include "modules/tfb/savefile.h"
#include "script/ops_message.h"
#include "script/symbolregistry.h"
#include "tfb/save/language.h"
using namespace Script;

// GLOBAL: 0x606a38 g_currentLevel
// GLOBAL: 0x62a2f8 g_nextLevel
// GLOBAL: 0x602ea0 g_globalSymbolCount
// GLOBAL: 0x621f38 g_typeSymCount
// GLOBAL: 0x61f878 g_symRegistryY
// GLOBAL: 0x4476c0 GetLanguage
// GLOBAL: 0x460c30 LevelRemapTitle
// GLOBAL: 0x461d60 TFBClock_GetSeconds
// GLOBAL: 0x460c40 LevelTable_GetFirst
// GLOBAL: 0x43bb20 NameHashNoCase
extern int g_currentLevel;
extern int g_nextLevel;
extern int g_globalSymbolCount;          // end of the global symbol range
extern int g_typeSymCount;               // start of the variables: the symbols below it are type names
int LevelRemapTitle();
float TFBClock_GetSeconds();
const char *LevelTable_GetFirst(int level);
unsigned NameHashNoCase(const char *s);


// Symbol numbers from this one on belong to the current script's locals, not to the global registry.
static const int kFirstLocalSymbol = 0x3fc0d;
// Entries with symbol numbers 95..97 are loaded into the entry list but never written back to the variables.
static const int kFirstReadOnlySymbol = 95;
static const int kLastReadOnlySymbol = 97;
// User array blocks of this size or more carry 100 bytes of trailer that are not saved.
static const int kLargeUserBlock = 0x640;
static const int kUserBlockTrailer = 100;
static const int kLevelNameLength = 0xc;

// 0x4463c0
// Near-match: stack frame 0x139c vs 0x1394 and register allocation of the loop.
int MemCard::SaveVariables(MemCardSlot *slot, int measureOnly)
{
	uint8_t buf[0x1380];
	uint8_t *out = buf;
	int count = 0;
	slot->m_state = MEMCARD_SLOT_VALID;
	if (!(char)measureOnly) {
		g_nextLevel = LevelRemapTitle();
		int level;
		if (g_currentLevel == g_nextLevel) {
			slot->m_gameTime = (int)TFBClock_GetSeconds();
			level = g_currentLevel;
		} else {
			level = 0;
			slot->m_percent = 0;
			slot->m_gameTime = 0;
		}
		strncpy(slot->m_levelName, LevelTable_GetFirst(level), kLevelNameLength);
		slot->m_levelName[0xb] = 0;
		slot->m_language = GetLanguage();
	}
	int limit = g_globalSymbolCount;
	for (int i = g_typeSymCount; i < limit; i++) {
		ScriptReference *obj;
		if (i >= kFirstLocalSymbol)
			obj = g_curScript->m_locals[i - kFirstLocalSymbol];
		else {
			uintptr_t slotBits = (*g_symRegistryY)[i].obj;
			if (slotBits & 1)
				obj = 0;
			else
				obj = (ScriptReference *)(slotBits & ~(uintptr_t)1);
		}
		if (obj == 0)
			continue;
		if (obj->Deref(0, 0) != obj)
			continue;
		if (!obj->IsKindOf(TFBType_Value()))
			continue;
		uint8_t *entry = out;
		out += SaveFormat::kScalarEntrySize;
		if (obj->GetType() != TFBType_User()) {
			SaveFormat::WriteEntryKey(entry, NameHashNoCase(obj->FormatTypedName(0, 0)));
		} else {
			UserReference *user = (UserReference *)obj;
			if (user->m_count == 0) {
				SaveFormat::WriteEntryKey(entry, SaveFormat::PointerWord(user->GetName()));   // the name hash, passed as a pointer
			} else {
				SaveFormat::WriteEntryKey(entry, NameHashNoCase(obj->FormatTypedName(0, 0)));
				int dataSize = user->m_dataSize;
				if (dataSize >= kLargeUserBlock)
					dataSize -= kUserBlockTrailer;
				memcpy(entry + 4, user->m_data, dataSize);
				out += dataSize - 4;
				if (dataSize != 0) {
					count++;
					continue;
				}
			}
		}
		Measurement *meas = (Measurement *)obj;
		if (meas->GetValueType() == 0x10)
			SaveFormat::WriteEntryFloat(entry, meas->GetFloat(meas));
		else
			SaveFormat::WriteEntryInt(entry, meas->GetInt(meas));
		count++;
	}
	slot->m_entryCount = count;
	int size = out - buf;
	slot->m_size = size;
	if (!(char)measureOnly) {
		slot->m_checksum = SaveFormat::SaveChecksum(buf, size);
		if (slot->m_buf != 0)
			memcpy(slot->m_buf, buf, size);
		g_nextLevel = g_currentLevel;
	}
	return slot->m_size;
}

// 0x4468e0
// Near-match: slot-copy loop counter, shared fail block placement, checksum loop shape. The game's non-user key-mismatch path stores 0 at
// [cursor+0x28]; replicated.
bool MemCard::LoadVariables()
{
	bool ok = true;
	MemCardSlot *dst = m_slots;
	for (int i = 0; i < kSlotCount; i++, dst++) {
		char *header = m_image + SlotStride() * i;
		LoadSlotHeader(header, dst);
		dst->m_buf = header + SaveFormat::kSlotHeaderSize;
	}
	MemCardSlot *slot = &m_slots[m_curSlot];
	uint8_t *data = (uint8_t *)m_image + SlotStride() * m_curSlot + SaveFormat::kSlotHeaderSize;
	bool checksumOk = false;
	if (slot->m_size == 0 || slot->m_size == SaveSize())
		checksumOk = SaveFormat::SaveChecksum(data, SaveSize()) == slot->m_checksum;
	if (!checksumOk) {
		m_error = MEMCARD_STATUS_BAD_DATA;
		slot->m_buf = 0;
		return false;
	}
	if (slot->m_state != MEMCARD_SLOT_VALID || slot->m_size != SaveSize()) {
		m_error = MEMCARD_STATUS_BAD_DATA;
		slot->m_buf = 0;
		return false;
	}
	slot->m_buf = (char *)data;
	int count = 0;
	uint8_t *cur = data;
	int limit = g_globalSymbolCount;
	for (int i = g_typeSymCount; i < limit; i++) {
		bool writable = true;
		if (i >= kFirstReadOnlySymbol && i <= kLastReadOnlySymbol)
			writable = false;
		ScriptReference *obj;
		if (i >= kFirstLocalSymbol)
			obj = g_curScript->m_locals[i - kFirstLocalSymbol];
		else {
			uintptr_t slotBits = (*g_symRegistryY)[i].obj;
			if (slotBits & 1)
				obj = 0;
			else
				obj = (ScriptReference *)(slotBits & ~(uintptr_t)1);
		}
		if (obj == 0)
			continue;
		if (obj->Deref(0, 0) != obj)
			continue;
		if (!obj->IsKindOf(TFBType_Value()))
			continue;
		const uint8_t *entry = cur;
		cur += SaveFormat::kScalarEntrySize;
		if (obj->GetType() != TFBType_User()) {
			if (SaveFormat::ReadEntryKey(entry) != NameHashNoCase(obj->FormatTypedName(0, 0))) {
				m_error = MEMCARD_STATUS_BAD_DATA;
				SaveFormat::WriteWord(cur + 0x28, 0);   // NOTE: the game clears a word of the save image 0x28 bytes behind the cursor here (instead of slot->m_buf); kept
				return false;
			}
		} else {
			UserReference *user = (UserReference *)obj;
			if (user->m_count == 0) {
				if (SaveFormat::ReadEntryKey(entry) != SaveFormat::PointerWord(user->GetName())) {
					m_error = MEMCARD_STATUS_BAD_DATA;
					slot->m_buf = 0;
					return false;
				}
			} else {
				if (SaveFormat::ReadEntryKey(entry) != NameHashNoCase(obj->FormatTypedName(0, 0))) {
					m_error = MEMCARD_STATUS_BAD_DATA;
					slot->m_buf = 0;
					return false;
				}
				int dataSize = user->m_dataSize;
				if (dataSize >= kLargeUserBlock)
					dataSize -= kUserBlockTrailer;
				memcpy(user->m_data, entry + 4, dataSize);
				cur += dataSize - 4;
				if (dataSize != 0) {
					count++;
					continue;
				}
			}
		}
		if (writable) {
			Measurement *meas = (Measurement *)obj;
			// NOTE: bit 31 of the value word marks an int (SaveVariables stores GetInt | 0x80000000), so ints survive only as
			// 0..0x7fffffff and a negative float would come back as an int; the game's format, kept.
			if (SaveFormat::EntryIsInt(entry))
				meas->SetValue(SaveFormat::ReadEntryInt(entry), meas);
			else
				meas->SetValue(SaveFormat::ReadEntryFloat(entry), meas);
		}
		count++;
	}
	if (slot->m_entryCount != count || slot->m_size != (int)(cur - data)) {
		ok = false;
		m_error = MEMCARD_STATUS_BAD_DATA;
		return ok;
	}
	g_currentLevel = g_nextLevel - 1;
	if (g_currentLevel < 0)
		g_currentLevel = 1;
	return ok;
}
