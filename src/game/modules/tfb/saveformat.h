#ifndef ZOOSTERS_MODULES_TFB_SAVEFORMAT_H
#define ZOOSTERS_MODULES_TFB_SAVEFORMAT_H
// On-disk format of SaveGames.mem: 4 slots of [44-byte header][2056 bytes of script variables], no file header. Pure (no game globals, no
// Win32): every field is read and
// written explicitly little-endian at its offset, so 32-bit and 64-bit hosts read and write the retail bytes (the in-memory MemCardSlot has
// a pointer and changes size).
#include "zoosters/base.h"

namespace SaveFormat {

static const int kSlotCount = 4;              // slots in the file
static const int kSlotHeaderSize = 0x2c;      // one slot header
static const int kSlotDataSize = 2056;        // the saved script variables of one slot in this build (MemCard::SaveSize)
static const int kSlotStride = kSlotHeaderSize + kSlotDataSize; // 2100: the distance from one slot to the next
static const int kFileSize = kSlotStride * kSlotCount;          // 8400
static const int kEntryCount = 70;            // saved script variables per slot
static const int kLevelNameSize = 12;         // SaveSlotHeader::levelName
static const int kUsedState = 8;              // SaveSlotHeader::state of a slot that holds a save
static const uint32_t kChecksumSeed = 0x55555555;   // the checksum of an empty block (and the start of every checksum)

// offsets of the fields inside a slot header
static const int kOffSize = 0x00;
static const int kOffState = 0x04;
static const int kOffEntryCount = 0x08;
static const int kOffLanguage = 0x0c;
static const int kOffPercent = 0x10;
static const int kOffGameTime = 0x14;
static const int kOffChecksum = 0x18;
static const int kOffLevelName = 0x1c;
static const int kOffBufPtr = 0x28;

struct SaveSlotHeader {
	int32_t size;         // +0x00 bytes of slot data in use: kSlotDataSize for a saved slot, 0 for an empty one
	int32_t state;        // +0x04 kUsedState in a saved slot
	int32_t entryCount;   // +0x08 number of saved variables (kEntryCount)
	int32_t language;     // +0x0c language id at save time (informational)
	int32_t percent;      // +0x10 completion percent (the script writes it)
	int32_t gameTime;     // +0x14 seconds played at save time
	uint32_t checksum;    // +0x18 SaveChecksum of the slot data (the header itself is not covered)
	char levelName[kLevelNameSize]; // +0x1c stream name of the level, cut to 11 characters, zero-terminated
	uint32_t bufPtr;      // +0x28 the 32-bit address of the game's own buffer; written, never read back (see PointerWord)
};

static inline uint32_t ReadWord(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static inline void WriteWord(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

// What the game stores at +0x28: the buffer address, as a 32-bit value. The low 32 bits of the pointer, so a 32-bit build writes
// exactly the bytes the retail game wrote (e.g. 0x622564) and a 64-bit build writes something harmless.
static inline uint32_t PointerWord(const void *p)
{
	return (uint32_t)(uintptr_t)p;
}

// Reads the 0x2c bytes at p. bufPtr is read for completeness; the game overwrites it after every copy.
static inline void ReadSlotHeader(const uint8_t *p, SaveSlotHeader *h)
{
	h->size = (int32_t)ReadWord(p + kOffSize);
	h->state = (int32_t)ReadWord(p + kOffState);
	h->entryCount = (int32_t)ReadWord(p + kOffEntryCount);
	h->language = (int32_t)ReadWord(p + kOffLanguage);
	h->percent = (int32_t)ReadWord(p + kOffPercent);
	h->gameTime = (int32_t)ReadWord(p + kOffGameTime);
	h->checksum = ReadWord(p + kOffChecksum);
	for (int i = 0; i < kLevelNameSize; i++)
		h->levelName[i] = (char)p[kOffLevelName + i];
	h->bufPtr = ReadWord(p + kOffBufPtr);
}

static inline void WriteSlotHeader(uint8_t *p, const SaveSlotHeader &h)
{
	WriteWord(p + kOffSize, (uint32_t)h.size);
	WriteWord(p + kOffState, (uint32_t)h.state);
	WriteWord(p + kOffEntryCount, (uint32_t)h.entryCount);
	WriteWord(p + kOffLanguage, (uint32_t)h.language);
	WriteWord(p + kOffPercent, (uint32_t)h.percent);
	WriteWord(p + kOffGameTime, (uint32_t)h.gameTime);
	WriteWord(p + kOffChecksum, h.checksum);
	for (int i = 0; i < kLevelNameSize; i++)
		p[kOffLevelName + i] = (uint8_t)h.levelName[i];
	WriteWord(p + kOffBufPtr, h.bufPtr);
}

// Overwrites the language word of the header at p (the game stamps the current language into every slot it validates).
static inline void WriteSlotLanguage(uint8_t *p, int32_t language)
{
	WriteWord(p + kOffLanguage, (uint32_t)language);
}

// The checksum of a slot's data: start at kChecksumSeed, then for each byte xor in the byte sign-extended to 32 bits and rotate left
// by one. (A byte >= 0x80 flips the upper 24 bits: that is the game's movsx, kept.)
static inline uint32_t SaveChecksum(const uint8_t *data, size_t n)
{
	uint32_t hash = kChecksumSeed;
	for (size_t i = 0; i < n; i++) {
		hash ^= (uint32_t)(int32_t)(int8_t)data[i];
		hash = (hash << 1) | (hash >> 31);
	}
	return hash;
}

static inline int SlotOffset(int slot)
{
	return slot * kSlotStride;
}
static inline int SlotDataOffset(int slot)
{
	return slot * kSlotStride + kSlotHeaderSize;
}

// An entry of the slot data is a key word and, for a scalar variable, one value word (8 bytes). The key is the CRC-32 of the
// lower-case typed name. The value word of an int variable has bit 31 set on top of the value; a float is stored raw. (The
// array variable has a key and then 1500 raw bytes: the caller copies those.)
static const int kScalarEntrySize = 8;
static const uint32_t kIntTag = 0x80000000;

static inline uint32_t ReadEntryKey(const uint8_t *entry)
{
	return ReadWord(entry);
}
static inline void WriteEntryKey(uint8_t *entry, uint32_t key)
{
	WriteWord(entry, key);
}

// NOTE: the game's format keeps the retail limits: an int is stored as value | kIntTag, so it survives only as 0..0x7fffffff (a
// negative int loses its sign), and a float with its sign bit set (negative) is read back as an int. Not fixed.
static inline void WriteEntryInt(uint8_t *entry, int32_t value)
{
	WriteWord(entry + 4, (uint32_t)value | kIntTag);
}
static inline void WriteEntryFloat(uint8_t *entry, float value)
{
	union {
		float f;
		uint32_t u;
	} bits;
	bits.f = value;
	WriteWord(entry + 4, bits.u);
}
static inline bool EntryIsInt(const uint8_t *entry)
{
	return (ReadWord(entry + 4) & kIntTag) != 0;
}
static inline int32_t ReadEntryInt(const uint8_t *entry)
{
	return (int32_t)(ReadWord(entry + 4) & ~kIntTag);
}
static inline float ReadEntryFloat(const uint8_t *entry)
{
	union {
		float f;
		uint32_t u;
	} bits;
	bits.u = ReadWord(entry + 4);
	return bits.f;
}

} // namespace SaveFormat

#endif
