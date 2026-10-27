// Only the jump thunks and the settings of the original layer exist here: the pre-Windows-2000 overlapped I/O emulation and the
// kernel32 slots are not built. The save layer (modules/tfb/savefile.cpp) calls zoosters::os directly, synchronously, and the
// single-instance lock is zoosters::os::InstanceLock.
// CFLAGS: /GS
#include "framework/startup/win32/fileio_maps.h"
#include <string.h>
#include "zoosters/base.h"
#include "zoosters/platform.h"
#include "framework/core/memoryhandler.h"
#include "framework/startup/win32/fileio.h"

extern int g_ioPrio;                       // GLOBAL: 0x60d810 g_ioPrio
extern int g_ioParam;                      // GLOBAL: 0x60d814 g_ioParam
extern unsigned char g_ioActive;           // GLOBAL: 0x60d80c g_ioActive

// `jmp dword ptr [slot]`: the first four through the kernel32 IAT slots, the rest through the function-pointer slots (g_ioFn / g_pfn*).
// 0x51c890
ZOOSTERS_JUMP_THUNK(CreateFileA_thunk, _imp__CreateFileA)
// 0x51c8a0
ZOOSTERS_JUMP_THUNK(ReadFile_thunk, _imp__ReadFile)
// 0x51c8b0
ZOOSTERS_JUMP_THUNK(ReadFileEx_thunk, _imp__ReadFileEx)
// 0x51c8c0
ZOOSTERS_JUMP_THUNK(WriteFile_thunk, _imp__WriteFile)
// 0x51c8d0
ZOOSTERS_JUMP_THUNK(FileIo_CreateFile_thunk, g_ioFn)   // slot 0
// 0x51c8e0
ZOOSTERS_JUMP_THUNK(FileIo_ReadFileEx_thunk, g_ioFn + 8)   // slot 2
// 0x51c8f0
ZOOSTERS_JUMP_THUNK(FileIo_ReadFile_thunk, g_ioFn + 4)   // slot 1
// 0x51c900
ZOOSTERS_JUMP_THUNK(FileIo_WriteFile_thunk, g_ioFn + 12)   // slot 3
// 0x51c910
ZOOSTERS_JUMP_THUNK(FileIo_GetOverlappedResult_thunk, g_pfnGetOverlappedResult)
// 0x51c920
ZOOSTERS_JUMP_THUNK(FileIo_CancelIo_thunk, g_pfnCancelIo)
// 0x51c930
ZOOSTERS_JUMP_THUNK(FileIo_CloseHandle_thunk, g_pfnCloseHandle)

// 0x51c940
void FileIo_Reset()
{
	g_ioActive = 0;
	// nothing calls the slots: the jump thunks above are empty functions here. On x64 ReadFile through the empty thunk returned the
	// file handle and the save file never loaded.
}

// 0x51dc80
void FileIo_Init(FileIoConfig *cfg)
{
	// no emulation thread; only the settings it read are kept
	if (cfg) {
		g_ioPrio = (int)cfg->m_limit;
		g_ioParam = cfg->m_param;
	}
}

// Instantiation driver, never called.
void InstantiateFileIoMaps()
{
	FileMap fileMap;
	ReqMap reqMap;
}

// std::hash_map<unsigned,bool> members; the game folds the g_reqMap copies into the same code.
// 0x51c9c0
// 0x51c9e0
// 0x51ca20
// 0x51ca50
// 0x51ca80
// 0x51caa0
// 0x51cad0
// 0x51cb00
// 0x51ccd0
// 0x51cd50
// 0x51cdf0
// 0x51ce20
// 0x51ce60
// 0x51d0c0
// 0x51d130
// 0x51d1d0
// 0x51d410
// 0x51d4c0
// 0x51d500
// 0x51d570
// 0x51d5f0
// 0x51d610
