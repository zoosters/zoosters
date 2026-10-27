// zoosters::os: what the normal build's game and port code ask of the operating system. Backends (--os): port/os/os_win32.cpp
// (the Win32 calls the game made) and port/os/os_sdl.cpp (SDL3 + the C++ standard library; macOS / Linux). The cl 13.10 view
// (ZOOSTERS_GAME_ABI) never uses this header. No system header here; handles are opaque pointers, sizes fixed-width.
#ifndef ZOOSTERS_OS_H
#define ZOOSTERS_OS_H

#include <stddef.h>
#include <stdint.h>

namespace zoosters {
namespace os {

// ---- time
uint32_t TickMs();                         // milliseconds since an arbitrary start, wrapping (timeGetTime / GetTickCount)
uint64_t PerfCounter();                    // high-resolution counter (QueryPerformanceCounter)
uint64_t PerfFrequency();                  // its ticks per second
void SleepMs(unsigned ms);
unsigned TimerMinPeriod();                 // smallest timer period the system offers, ms (timeGetDevCaps; 1 when unknown)
void TimerResolutionBegin(unsigned ms);    // ask for a finer scheduler tick (timeBeginPeriod); no-op where it does not exist
void TimerResolutionEnd(unsigned ms);
uint64_t TickMs64();                       // milliseconds since the system started, not wrapping (GetTickCount64); always the real clock, also
                                           // under the test clock: the game's FrameRate static initialiser reads the real uptime

// ---- process
// Single-instance lock: returns a handle (null on failure) and sets *alreadyRunning when another instance holds `name`.
void *InstanceLock(const char *name, bool *alreadyRunning);
void InstanceUnlock(void *lock);
void ShowError(const char *caption, const char *text);   // modal message to the user (MessageBox with an OK button)
void DebugPrint(const char *text);                        // debugger output / stderr
void RequestQuit();                                       // ask the main loop to end (PostQuitMessage(0))

// ---- display (the desktop mode; the renderer owns the window's fullscreen mode)
bool DesktopMode(uint32_t *w, uint32_t *h);
bool SetDisplayMode(uint32_t w, uint32_t h);   // change the desktop mode; false when the change failed
bool RestoreDisplayMode();                     // undo SetDisplayMode; false when the system refused

// ---- window (`win` = the game window handle the window backend created; null = none)
bool ClientSize(void *win, int *w, int *h);
bool ClientToScreen(void *win, int *x, int *y);
void ShowWindow(void *win, int cmd);           // Win32 SW_* numbers (the game's own values)
void UpdateWindow(void *win);
void InvalidateWindow(void *win);
void ValidateWindow(void *win);
void DestroyWindow(void *win);
intptr_t DefaultWindowProc(void *win, unsigned msg, uintptr_t wparam, intptr_t lparam);   // DefWindowProc; 0 where there is none
void CaptureMouse(void *win, bool on);         // SetCapture(win) / release
void ScreenSaver(bool enable);                 // SystemParametersInfo(SPI_SETSCREENSAVEACTIVE)

// ---- cursor
int ShowCursor(bool show);                     // ShowCursor's display counter semantics: returns the new count
bool CursorPos(int *x, int *y);                // screen coordinates
void SetCursorPos(int x, int y);

// ---- port side: the harness clock, memory probing, run-time libraries, the per-user folder
// Test harness clock (PORT_FIXEDTIME): while set, TickMs / PerfCounter / PerfFrequency answer from fn (microseconds, ticks at
// 1 MHz). The Win32 backend ignores it (the harness replaces the kernel32 / winmm imports there); null = the real clock.
typedef uint64_t (*VirtualClockFn)();
void SetVirtualClock(VirtualClockFn fn);
bool MemoryReadable(const void *p, size_t n);   // every byte of [p, p+n) is mapped and readable (diagnostics only); true where the system cannot tell
uint32_t ThreadId();                            // id of the calling thread (diagnostics: logs, "is this the game thread")
void *LibraryOpen(const char *name);            // load a shared library by file name (null on failure)
void *LibrarySymbol(void *lib, const char *name);
// The per-user data root the game's section paths ("Activision\Madagascar\Save") hang below: My Documents on Windows (the original
// game's place), the platform's per-user data folder (SDL_GetPrefPath's parent of Activision/Madagascar) in the portable backend.
bool UserDataRoot(char *buf, size_t size);

// ---- keyboard
unsigned VirtualKeyToScanCode(unsigned vk);    // MapVirtualKey(vk, 0)

// ---- files (the save layer; synchronous)
enum FileAccess { FILE_READ = 1, FILE_WRITE = 2 };
enum FileCreate { FILE_OPEN_EXISTING, FILE_CREATE_ALWAYS, FILE_OPEN_ALWAYS };
intptr_t FileOpen(const char *path, int access, FileCreate create);   // -1 on failure
bool FileRead(intptr_t f, void *buf, uint32_t n, uint32_t *done);
bool FileWrite(intptr_t f, const void *buf, uint32_t n, uint32_t *done);
bool FileClose(intptr_t f);
bool MakeDir(const char *path);                // one level; true when it exists afterwards
bool RemoveFile(const char *path);
bool FileInfo(const char *path, bool *isDir, uint64_t *size);   // false when the path does not exist
bool DiskFreeBytes(const char *path, uint64_t *freeForUser);
// Why the last failed FileOpen / MakeDir / RemoveFile (this thread) failed, for the few places where the save layer branches on it.
enum FileError { FILE_ERROR_NONE = 0, FILE_ERROR_NOT_FOUND, FILE_ERROR_NOT_READY, FILE_ERROR_DISK_FULL, FILE_ERROR_OTHER };
FileError LastFileError();
bool CurrentDir(char *buf, size_t size);
// A game path ("Levels\\title.stream", "Activision\\Madagascar\\Save") as the file system spells it: Win32 = the path itself (returned
// unchanged, buf unused); POSIX = `\` -> `/` and, for a path that does not exist as spelled, each component's case matched against the
// directory (case-sensitive file systems). Returns buf, or `path` when the result does not fit.
const char *NativePath(const char *path, char *buf, size_t size);
// Optical drives (the game's CD check): writes the drive roots ("D:\\") into `roots` (NUL-separated, double-NUL-terminated) and
// returns how many; 0 where the system has no drive letters (the game then treats the install folder as the disc).
int CdDriveRoots(char *roots, size_t size);

// ---- framework / start-up
// Keyboard: the character a DirectInput scan code types (MapVirtualKey(MapVirtualKey(sc, MAPVK_VSC_TO_VK), MAPVK_VK_TO_CHAR)); 0 = none.
unsigned ScanCodeToChar(unsigned scanCode);
// Accessibility: the game switches the Shift-hotkey of the sticky keys off while it runs.
void StickyKeysInit();                         // remember the user's setting and switch the hotkey off
void StickyKeysSet(bool gameActive);           // true: hotkey off (game has the focus); false: the user's setting back
unsigned ScreenSaverTimeout();                 // seconds (SPI_GETSCREENSAVETIMEOUT); 0 when unknown
// File-system roots: writes the drive roots the game registers file systems for ("B:\" ..., drive A skipped, unusable drives left out;
// NUL-separated, double-NUL-terminated) and returns how many; 0 where the system has no drive letters.
int DriveRoots(char *roots, size_t size);

}  // namespace os
}  // namespace zoosters

#endif
