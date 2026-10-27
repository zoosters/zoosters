// g_pfnFileRead is the ReadFile slot (0x60d81c), g_pfnFileWrite the WriteFile slot (0x60d824). Not g_pfnReadFile /
// g_pfnWriteFile: other trees use those names for the opposite slot, and the link resolves a GLOBAL name to one address
// (a ReadFile call once landed in WriteFile_thunk, so Load Game read nothing and reported BAD_DATA)
#include <string.h>
#include "framework/startup/win32/fileio.h"
#include "modules/tfb/savefile.h"
#include "modules/tfb/memcard.h"
#include "framework/startup/win32/platform.h"
#include "zoosters/os.h"

// GLOBAL: 0x622498 g_savePath
// GLOBAL: 0x603550 g_saveName
// GLOBAL: 0x603554 g_saveHandle
// GLOBAL: 0x622525 g_writeBusy
// GLOBAL: 0x622526 g_readBusy
// GLOBAL: 0x622520 g_saveMode
// GLOBAL: 0x622528 g_saveOffset
// GLOBAL: 0x622518 g_saveBuf
// GLOBAL: 0x62251c g_saveSize
// GLOBAL: 0x622480 g_saveOverlapped
// GLOBAL: 0x622524 g_memCardReady
// GLOBAL: 0x603558 g_memCardStatus
// GLOBAL: 0x626538 g_memCard
// GLOBAL: 0x62f1dc g_pfnCloseHandle
// GLOBAL: 0x60d818 g_pfnCreateFile
// GLOBAL: 0x60d81c g_pfnFileRead
// GLOBAL: 0x60d824 g_pfnFileWrite
// GLOBAL: 0x62a390 g_shotDir
// GLOBAL: 0x5ca180 _imp__DeleteFileA
// GLOBAL: 0x5ca184 _imp__CreateDirectoryA
// GLOBAL: 0x5ca188 _imp__GetLastError
// GLOBAL: 0x5ca0e4 _imp__GetDiskFreeSpaceExA
// GLOBAL: 0x5ca024 _imp__GetFileAttributesExA
// GLOBAL: 0x5ca250 _imp__wsprintfW


// the game keeps its own function pointers to ReadFile/WriteFile/CreateFile/CloseHandle (the hooked file layer): g_pfn* in fileio.h
extern char g_shotDir[];

static const unsigned kGenericRead = 0x80000000;
static const unsigned kGenericWrite = 0x40000000;
static const unsigned kOpenExisting = 3;
static const unsigned kOpenAlways = 4;
static const unsigned kShareReadWrite = 3;
static const unsigned kSaveFileFlags = 0x50000080;  // FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_RANDOM_ACCESS
static const unsigned kErrorAlreadyExists = 0xB7;
static const unsigned kErrorIoPending = 0x3E5;
static const unsigned kErrorFileNotFound = 2;
static const unsigned kErrorBadUnit = 0x14;
static const unsigned kErrorNotReady = 0x15;
static const unsigned kErrorHandleEof = 0x26;
static const unsigned kErrorDiskFull = 0x70;
static const unsigned kStatusPending = 0x103;
static const unsigned kFreeBlocksNeeded = 15;       // 16 KiB blocks the save file needs when it is created

// ---------------------------------------------------------------- status flags
// MemCardGetStatus (0x445700) is defined in script/types/scriptmemcard.cpp (see the NOTE there).

// 0x445720
void MemCardClearStatus()
{
	g_memCardStatus = MEMCARD_STATUS_OK;
}

// 0x445880
void MemCardClearReady()
{
	g_memCardReady = 0;
}

// ---------------------------------------------------------------- directories
// 0x445730
bool MemCardInit(int)
{
	if (g_memCardReady)
		return true;
	g_memCardStatus = MEMCARD_STATUS_OK;
	GetUserDataPath("Activision", "Screen shots", g_shotDir, 0x100);
	zoosters::os::MakeDir(g_shotDir);
	GetUserDataPath("Activision\\Madagascar", "Screen shots", g_shotDir, 0x100);
	zoosters::os::MakeDir(g_shotDir);
	GetUserDataPath("Activision\\Madagascar\\Screen shots", "Screen shots", g_shotDir, 0x100);
	zoosters::os::MakeDir(g_shotDir);
	strcat(g_shotDir, "\\");
	g_memCardStatus = MEMCARD_STATUS_OK;
	if (!GetUserDataPath("Activision\\Madagascar\\Save", "Save", g_savePath, 0x7f)) {
		g_memCardStatus = MEMCARD_STATUS_IO_ERROR;
		return false;
	}
	if (!zoosters::os::MakeDir(g_savePath)) {   // MakeDir succeeds for a directory that exists already
		if (zoosters::os::LastFileError() == zoosters::os::FILE_ERROR_DISK_FULL)
			g_memCardStatus = MEMCARD_STATUS_NO_SPACE;
		else {
			g_memCardStatus = MEMCARD_STATUS_IO_ERROR;
			return false;
		}
	}
	strcat(g_savePath, "\\");
	g_memCardReady = 1;
	return true;
}

// ---------------------------------------------------------------- the file
// 0x445890
bool SaveFileDelete()
{
	char path[260];
	strcpy(path, g_savePath);
	strcat(path, g_saveName);
	if (!zoosters::os::RemoveFile(path)) {
		if (zoosters::os::LastFileError() != zoosters::os::FILE_ERROR_NOT_FOUND)
			return false;
	}
	return true;
}

// 0x445910
int SaveDiskFreeSpace(bool clamp)
{
	uint64_t freeBytes;
	if (!zoosters::os::DiskFreeBytes(g_savePath, &freeBytes))
		return 0;
	if (clamp) {
		if (freeBytes >> 32)
			return 0x7FFFFFFF;
		return (int)(unsigned)freeBytes;
	}
	return (int)(unsigned)(freeBytes >> 14);   // 16 KiB blocks (the game's high word << 18 wraps the same way)
}

// 0x445970
unsigned char SaveFileIoDone()
{
	if (g_saveOverlapped.internal == kStatusPending)
		return false;
	int mode = g_saveMode;
	unsigned char done = true;
	if (mode == SAVEFILE_READ || mode == SAVEFILE_READ_CHECKED) {
		g_readBusy = 0;
	} else if (mode == SAVEFILE_WRITE) {
		g_writeBusy = 0;
		done = mode;
	}
	return done;
}

// The open tail the game duplicates per mode (write copy / read copy): one forced-inline template.
// kWrite: create or open for writing; otherwise open the existing file and check its size.
template <int kWrite> static __forceinline int OpenSaveFile(int size, char *path, unsigned access, unsigned disposition)
{
	if (size < 0) {
		uint64_t freeBytes;
		if (!zoosters::os::DiskFreeBytes(g_savePath, &freeBytes) || (unsigned)(freeBytes >> 14) < kFreeBlocksNeeded) {
			g_memCardStatus = MEMCARD_STATUS_NO_SPACE;
			return 0;
		}
	}
	strcpy(path, g_savePath);
	strcat(path, g_saveName);
	// synchronous zoosters::os file (the save file is accessed sequentially: SaveFileReadAsync / WriteAsync advance g_saveOffset by the size)
	int file = (int)zoosters::os::FileOpen(path, ((access & kGenericRead) ? zoosters::os::FILE_READ : 0) | ((access & kGenericWrite) ? zoosters::os::FILE_WRITE : 0),
	                                    disposition == kOpenAlways ? zoosters::os::FILE_OPEN_ALWAYS : zoosters::os::FILE_OPEN_EXISTING);
	if (kWrite) {
		if (file == -1) {
			g_memCardStatus = zoosters::os::LastFileError() == zoosters::os::FILE_ERROR_NOT_READY ? MEMCARD_STATUS_NOT_READY : MEMCARD_STATUS_OPEN_FAILED;
			return 0;
		}
	} else {
		if (file == -1) {
			g_memCardStatus = MEMCARD_STATUS_OPEN_FAILED;
			return 0;
		}
		bool isDir;
		uint64_t fileSize;
		if (!zoosters::os::FileInfo(path, &isDir, &fileSize)) {
			zoosters::os::FileClose(file);
			g_memCardStatus = MEMCARD_STATUS_IO_ERROR;
			return 0;
		}
		if ((unsigned)fileSize != (unsigned)(g_memCard->SlotStride() * MemCard::kSlotCount)) {
			g_memCardStatus = MEMCARD_STATUS_BAD_DATA;
			zoosters::os::FileClose(file);
			return 0;
		}
	}
	g_saveHandle = file;
	return 1;
}

// Opens the save file in a SaveFileMode. A negative size asks for the free-space check (a new file is going to be created).
// Near-match of 0x4459b0: the game duplicates the open tail per mode (a forceinline template<WRITE> here); residual: argument push
// scheduling, the shared GetDiskFreeSpaceEx failure block at the end, frame 4 B.
int SaveFileOpen(int mode, int size)
{
	char path[0x104];   // port: the game's 0x84 overflows once g_savePath (up to 0x80) is longer than 0x84 - sizeof "<save name>" (ASan)
	g_saveMode = mode;
	g_memCardStatus = MEMCARD_STATUS_OK;
	g_saveOffset = 0;
	g_saveOverlapped.offsetHigh = 0;   // OffsetHigh, hEvent of g_saveOverlapped
	g_saveOverlapped.event = 0;
	switch (mode) {
	case SAVEFILE_WRITE:
		return OpenSaveFile<1>(size, path, kGenericRead | kGenericWrite, kOpenAlways);
	case SAVEFILE_READ:
	case SAVEFILE_READ_CHECKED:
		return OpenSaveFile<0>(size, path, kGenericRead, kOpenExisting);
	}
	g_memCardStatus = MEMCARD_STATUS_NOT_INITIALISED;
	return 0;
}

// 0x445c80
bool SaveFileClose()
{
	bool ok = false;
	if (g_saveHandle != -1) {
		int closed = zoosters::os::FileClose(g_saveHandle);
		g_saveHandle = -1;
		if (closed)
			ok = true;
	} else
		g_saveHandle = -1;
	return ok;
}

// 0x445cc0
bool SaveFileReset()
{
	int mode = g_saveMode;
	g_readBusy = 0;
	g_writeBusy = 0;
	if (mode != SAVEFILE_WRITE && mode != SAVEFILE_READ && mode != SAVEFILE_READ_CHECKED)
		SaveFileClose();
	return true;
}

// 0x445cf0
int SaveFileWriteAsync(void *buf, int size)
{
	int done;
	if (g_writeBusy)
		return 0;
	g_writeBusy = 1;
	g_saveBuf = buf;
	g_saveSize = size;
	g_saveOverlapped.offset = g_saveOffset;
	g_saveOffset = g_saveOffset + size;
	// synchronous: the transfer is finished when the call returns (the overlapped state reads "done", SaveFileIoDone)
	uint32_t written = 0;
	zoosters::os::FileWrite(g_saveHandle, buf, (uint32_t)size, &written);
	g_saveOverlapped.internal = 0;
	done = (int)written;
	if (done != size)
		g_memCardStatus = MEMCARD_STATUS_BAD_DATA;
	return 0;
}

// 0x445d70
int SaveFileReadAsync(void *buf, int size)
{
	int done;
	if (g_readBusy)
		return 0;
	g_readBusy = 1;
	g_saveBuf = buf;
	g_saveSize = size;
	g_saveOverlapped.offset = g_saveOffset;
	g_saveOffset += size;
	uint32_t got = 0;
	zoosters::os::FileRead(g_saveHandle, buf, (uint32_t)size, &got);   // a short read (end of file) leaves done != size: BAD_DATA below
	g_saveOverlapped.internal = 0;
	done = (int)got;
	if (done != size)
		g_memCardStatus = MEMCARD_STATUS_BAD_DATA;
	return 0;
}

// 0x445ee0
unsigned __stdcall SaveChecksum(const char *data, int size)
{
	int hash = 0x55555555;
	if (size) {
		int n = size;
		do {
			hash ^= *data++;
			int carry = 0;
			if (hash & 0x80000000)
				carry = 1;
			hash = carry + hash * 2;
		} while (--n);
	}
	return hash;
}
