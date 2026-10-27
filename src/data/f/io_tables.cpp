// File I/O slots and the IMA ADPCM tables behind them.
#include "framework/startup/win32/fileio.h"   // FileRwFn

void WriteFile_thunk();
void FileIo_CreateFile_thunk();
void FileIo_ReadFile_thunk();
void FileIo_ReadFileEx_thunk();
void FileIo_WriteFile_thunk();
void FileIo_GetOverlappedResult_thunk();
void FileIo_CancelIo_thunk();
void FileIo_CloseHandle_thunk();
typedef void (*IoThunk)();

// GLOBAL: 0x60d804 g_wndBorderY
int g_wndBorderY = 1;
// unnamed int (1) between g_wndBorderY and g_ioActive, keeps the game's distance
// GLOBAL: 0x60d808 DAT_0060d808
int DAT_0060d808 = 1;
// GLOBAL: 0x60d80c g_ioActive
unsigned char g_ioActive = 1;
// GLOBAL: 0x60d810 g_ioPrio
int g_ioPrio = 8192;

// ALONE: g_ioParam int, only assigned (platform_b: g_ioParam = cfg->m_param); the I/O thunks jump through g_ioFn by its own name
// GLOBAL: 0x60d814 g_ioParam
int g_ioParam = 1;

// One 424 B block from 0x60d824, as one object because cl 13.10 pads between separate objects of this size. Slot 0 of the eight
// function pointers is the pointer the save file layer calls (g_pfnFileWrite, fileio.h; the tree calls it g_pfnReadFile); the rest
// is IoThunkTail: the thunks, a zero dword, the IMA ADPCM index table (0x60d848) and step table (0x60d868).
struct IoThunkTail { IoThunk m_fn[7]; unsigned m_zero; int m_imaIndex[8]; int m_imaStep[89]; };
typedef char io_size_check_IoThunkTail[sizeof(void *) != 4 || sizeof(IoThunkTail) == 420 ? 1 : -1];   // no static_assert in cl 13.10
// GLOBAL: 0x60d824 g_pfnFileWrite
__declspec(align(4)) FileRwFn g_pfnFileWrite = (FileRwFn)WriteFile_thunk;
// GLOBAL: 0x60d828 g_ioThunkTail
__declspec(align(4)) IoThunkTail g_ioThunkTail = {
	{ FileIo_CreateFile_thunk, FileIo_ReadFile_thunk, FileIo_ReadFileEx_thunk, FileIo_WriteFile_thunk, FileIo_GetOverlappedResult_thunk, FileIo_CancelIo_thunk, FileIo_CloseHandle_thunk },
	0,
	{
	-1, -1, -1, -1, 2, 4, 6, 8,
	},
	{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767,
	}
};

// GLOBAL: 0x60d9cc g_audioParamA
int g_audioParamA = 20;

// GLOBAL: 0x60d9d0 g_audioParamB
int g_audioParamB = 1;

#include "framework/core/csystemcommands.h"   // RWS::CSystemCommands (src/game)
// ALONE: RWS::CSystemCommands::pCSystemCommandsClassID single const char * (CAttributeCommandIterator argument); followed by CRT data
// GLOBAL: 0x611634 RWS::CSystemCommands::pCSystemCommandsClassID
const char * RWS::CSystemCommands::pCSystemCommandsClassID = "CSystemCommands";
