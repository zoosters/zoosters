#ifndef ZOOSTERS_MODULES_TFB_SAVEFILE_H
#define ZOOSTERS_MODULES_TFB_SAVEFILE_H

#include "zoosters/base.h"

// The last error of the save file layer; MemCard::m_error overrides it for the script.
enum MemCardStatus {
	MEMCARD_STATUS_OK = 0,
	MEMCARD_STATUS_BUSY = 1,          // script only: a request is running (never stored in g_memCardStatus)
	MEMCARD_STATUS_NO_SPACE = 3,      // ERROR_DISK_FULL, or fewer than 15 free blocks (SaveFileOpen)
	MEMCARD_STATUS_OPEN_FAILED = 4,   // CreateFile failed / save file missing
	MEMCARD_STATUS_NOT_READY = 5,     // ERROR_NOT_READY / ERROR_WRITE_PROTECT-like (0x14, 0x15) on open for write
	MEMCARD_STATUS_IO_ERROR = 6,      // the save directory or the file attributes could not be read
	MEMCARD_STATUS_BAD_DATA = 7,      // wrong file size, short transfer or a damaged slot (also MemCard::m_error)
	MEMCARD_STATUS_NOT_INITIALISED = 8 // MemCardInit has not run (MemCardGetStatus)
};

// SaveFileOpen modes (g_saveMode). Modes 0 and 3 open the existing file for reading, mode 1 opens (creating) it for writing.
// 3 differs from 0 only by the caller: both take the same path.
enum SaveFileMode {
	SAVEFILE_READ = 0,
	SAVEFILE_WRITE = 1,
	SAVEFILE_READ_CHECKED = 3 // used by the init and save sequences, which pass the free-space check size
};

// The Win32 OVERLAPPED of the pending transfer. The game's is 20 bytes; ReadFile / WriteFile get the real one (ULONG_PTR Internal /
// InternalHigh, 32 bytes on x64): the 20-byte form
// overlapped the kernel's fields, so the file offset was read as InternalHigh and the save never loaded on x64.
struct SaveFileOverlapped {
	uintptr_t internal;      // 0x103 (STATUS_PENDING) while the transfer runs
	uintptr_t internalHigh;
	unsigned offset;
	unsigned offsetHigh;
	void *event;
};
static_assert(sizeof(void *) != 4 || sizeof(SaveFileOverlapped) == 20, "OVERLAPPED (32-bit)");
static_assert(sizeof(void *) != 8 || sizeof(SaveFileOverlapped) == 32, "OVERLAPPED (64-bit)");

extern char g_savePath[];   // save directory with a trailing backslash
extern const char *g_saveName;   // file name inside it
extern int g_saveHandle;   // file handle, -1 = none
extern unsigned char g_writeBusy;   // a write transfer is pending (set by SaveFileWriteAsync)
extern unsigned char g_readBusy;   // a read transfer is pending (set by SaveFileReadAsync)
extern int g_saveMode;   // mode of the open file (SaveFileMode)
extern unsigned g_saveOffset;   // file position of the next transfer
extern void *g_saveBuf;   // buffer of the running transfer
extern int g_saveSize;   // size of the running transfer
extern SaveFileOverlapped g_saveOverlapped;
extern unsigned char g_memCardReady;   // set once the directories exist (MemCardInit)
extern int g_memCardStatus;   // MemCardStatus

int Script_AlwaysTrue();   // (returns 1) the card-inserted test (MemCard::m_changed tracks its changes)
void MemCardSendRunningPostTick();
bool MemCardInit(int);   // creates the screenshot and save directories
int MemCardGetStatus();
void MemCardClearStatus();
void MemCardClearReady();
bool SaveFileDelete();
int SaveDiskFreeSpace(bool clamp);   // free blocks (16 KiB units) of the save drive, or bytes clamped to 2 GiB
unsigned char SaveFileIoDone();   // true once the pending transfer finished
int SaveFileOpen(int mode, int size);   // opens the save file in a SaveFileMode, returns 1 on success
bool SaveFileClose();
bool SaveFileReset();   // clears both busy flags; closes the file only for a mode that is not read/write (never in practice)
int SaveFileWriteAsync(void *buf, int size);   // starts an overlapped WriteFile of buf at the file position
int SaveFileReadAsync(void *buf, int size);   // starts an overlapped ReadFile
unsigned __stdcall SaveChecksum(const char *data, int size);

#endif
