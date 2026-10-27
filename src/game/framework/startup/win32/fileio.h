// The file I/O layer: function pointers for the kernel32 file calls, and the overlapped I/O emulation (FileHook_*) for systems before Windows 2000.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_FILEIO_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_FILEIO_H

// Settings_Create's parameters; they go to the emulation's worker thread (0x51cb70).
struct FileIoConfig {
	int m_threadPriority;   // SetThreadPriority value
	float m_limit;          // stored as an int in g_ioPrio; role unknown
	int m_param;            // stored in g_ioParam; role unknown
};

// Windows 2000 or later gets the real API, older systems the FileHook_* emulation.
void FileIo_Init(FileIoConfig *cfg);
// Back to the plain kernel32 entry points.
void FileIo_Reset();

// CloseHandle is also used for WinMain's single-instance mutex.
extern int(__stdcall *g_pfnCloseHandle)(void *h);
// g_ioFn: CreateFile, ReadFile, ReadFileEx, WriteFile. The save file layer calls three through names of their own: the same slots
// in the game, here separate variables the static-init list copies from g_ioFn.
typedef int (__stdcall *FileCreateFn)(const char *name, unsigned access, unsigned share, void *sec, unsigned disp, unsigned flags, void *tmpl);
typedef int (__stdcall *FileRwFn)(int h, void *buf, int n, int *done, void *ov);
extern void *g_ioFn[4];
extern FileCreateFn g_pfnCreateFile;   // = g_ioFn[0]
extern FileRwFn g_pfnFileRead;   // = g_ioFn[1]
extern FileRwFn g_pfnFileWrite;   // = g_ioFn[3]; the game's block of eight thunks starts here (src/data/f/io_tables.cpp)

#endif
