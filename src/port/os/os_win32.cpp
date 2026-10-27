// zoosters::os, Win32 backend (build option --os win32, the Windows default): the calls the game itself makes.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
#include "zoosters/os.h"

namespace zoosters {
namespace os {

uint32_t TickMs() { return timeGetTime(); }
uint64_t PerfCounter() { LARGE_INTEGER v; QueryPerformanceCounter(&v); return (uint64_t)v.QuadPart; }
uint64_t PerfFrequency() { LARGE_INTEGER v; QueryPerformanceFrequency(&v); return (uint64_t)v.QuadPart; }
void SleepMs(unsigned ms) { ::Sleep(ms); }
unsigned TimerMinPeriod() { TIMECAPS tc; return timeGetDevCaps(&tc, sizeof(tc)) == TIMERR_NOERROR ? tc.wPeriodMin : 1; }
void TimerResolutionBegin(unsigned ms) { timeBeginPeriod(ms); }
void TimerResolutionEnd(unsigned ms) { timeEndPeriod(ms); }
uint64_t TickMs64() { return GetTickCount64(); }

void *InstanceLock(const char *name, bool *alreadyRunning)
{
	HANDLE h = CreateMutexA(0, TRUE, name);
	*alreadyRunning = h != 0 && GetLastError() == ERROR_ALREADY_EXISTS;
	return h;
}
void InstanceUnlock(void *lock) { if (lock) CloseHandle((HANDLE)lock); }
void ShowError(const char *caption, const char *text) { MessageBoxA(0, text, caption, MB_OK); }
void DebugPrint(const char *text) { OutputDebugStringA(text); }
void RequestQuit() { PostQuitMessage(0); }

bool DesktopMode(uint32_t *w, uint32_t *h)
{
	DEVMODEA dm = {};
	dm.dmSize = sizeof(dm);
	if (!EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &dm))
		return false;
	*w = dm.dmPelsWidth;
	*h = dm.dmPelsHeight;
	return true;
}
bool SetDisplayMode(uint32_t w, uint32_t h)
{
	DEVMODEA dm = {};
	dm.dmSize = sizeof(dm);
	dm.dmPelsWidth = w;
	dm.dmPelsHeight = h;
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	return ChangeDisplaySettingsA(&dm, 0) == DISP_CHANGE_SUCCESSFUL;
}
bool RestoreDisplayMode() { return ChangeDisplaySettingsA(0, 0) == DISP_CHANGE_SUCCESSFUL; }

bool ClientSize(void *win, int *w, int *h)
{
	RECT rc;
	if (!GetClientRect((HWND)win, &rc))
		return false;
	*w = rc.right - rc.left;
	*h = rc.bottom - rc.top;
	return true;
}
bool ClientToScreen(void *win, int *x, int *y)
{
	POINT p = { *x, *y };
	if (!::ClientToScreen((HWND)win, &p))
		return false;
	*x = p.x;
	*y = p.y;
	return true;
}
void ShowWindow(void *win, int cmd) { ::ShowWindow((HWND)win, cmd); }
void UpdateWindow(void *win) { ::UpdateWindow((HWND)win); }
void InvalidateWindow(void *win) { InvalidateRect((HWND)win, 0, TRUE); }
void ValidateWindow(void *win) { ValidateRect((HWND)win, 0); }
void DestroyWindow(void *win) { ::DestroyWindow((HWND)win); }
intptr_t DefaultWindowProc(void *win, unsigned msg, uintptr_t wparam, intptr_t lparam) { return DefWindowProcA((HWND)win, msg, wparam, lparam); }
void CaptureMouse(void *win, bool on)
{
	if (on)
		SetCapture((HWND)win);
	else
		ReleaseCapture();
}
void ScreenSaver(bool enable) { SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, enable, 0, 0); }

int ShowCursor(bool show) { return ::ShowCursor(show); }
bool CursorPos(int *x, int *y)
{
	POINT p;
	if (!GetCursorPos(&p))
		return false;
	*x = p.x;
	*y = p.y;
	return true;
}
void SetCursorPos(int x, int y) { ::SetCursorPos(x, y); }

unsigned VirtualKeyToScanCode(unsigned vk) { return MapVirtualKeyA(vk, 0); }

// ---- port side
void SetVirtualClock(VirtualClockFn) {}   // the harness hooks the kernel32 / winmm imports instead (port_run.cpp)
bool MemoryReadable(const void *p, size_t n)
{
	const char *a = (const char *)p, *end = a + n;
	while (a < end) {
		MEMORY_BASIC_INFORMATION mi;
		if (!VirtualQuery(a, &mi, sizeof mi) || mi.State != MEM_COMMIT || (mi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) ||
		    !(mi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
			return false;
		a = (const char *)mi.BaseAddress + mi.RegionSize;
	}
	return true;
}
uint32_t ThreadId() { return GetCurrentThreadId(); }
void *LibraryOpen(const char *name) { return (void *)LoadLibraryA(name); }
void *LibrarySymbol(void *lib, const char *name) { return (void *)GetProcAddress((HMODULE)lib, name); }
bool UserDataRoot(char *buf, size_t size)
{
	char docs[MAX_PATH];
	if (SHGetFolderPathA(0, CSIDL_PERSONAL, 0, 0, docs) < 0 || strlen(docs) + 1 > size)
		return false;
	strcpy(buf, docs);
	return true;
}

// ---- file errors
static thread_local FileError g_lastFileError = FILE_ERROR_NONE;
static void NoteFileError()
{
	switch (GetLastError()) {
	case ERROR_FILE_NOT_FOUND: case ERROR_PATH_NOT_FOUND: g_lastFileError = FILE_ERROR_NOT_FOUND; break;
	case ERROR_BAD_UNIT: case ERROR_NOT_READY: g_lastFileError = FILE_ERROR_NOT_READY; break;
	case ERROR_DISK_FULL: g_lastFileError = FILE_ERROR_DISK_FULL; break;
	default: g_lastFileError = FILE_ERROR_OTHER; break;
	}
}
FileError LastFileError() { return g_lastFileError; }

intptr_t FileOpen(const char *path, int access, FileCreate create)
{
	DWORD acc = ((access & FILE_READ) ? GENERIC_READ : 0) | ((access & FILE_WRITE) ? GENERIC_WRITE : 0);
	DWORD disp = create == FILE_CREATE_ALWAYS ? CREATE_ALWAYS : create == FILE_OPEN_ALWAYS ? OPEN_ALWAYS : OPEN_EXISTING;
	HANDLE h = CreateFileA(path, acc, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, disp, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE) {
		NoteFileError();
		return -1;
	}
	return (intptr_t)h;
}
bool FileRead(intptr_t f, void *buf, uint32_t n, uint32_t *done)
{
	DWORD d = 0;
	BOOL ok = ReadFile((HANDLE)f, buf, n, &d, 0);
	*done = d;
	return ok != 0;
}
bool FileWrite(intptr_t f, const void *buf, uint32_t n, uint32_t *done)
{
	DWORD d = 0;
	BOOL ok = WriteFile((HANDLE)f, buf, n, &d, 0);
	*done = d;
	return ok != 0;
}
bool FileClose(intptr_t f) { return CloseHandle((HANDLE)f) != 0; }
bool MakeDir(const char *path)
{
	if (CreateDirectoryA(path, 0) || GetLastError() == ERROR_ALREADY_EXISTS)
		return true;
	NoteFileError();
	return false;
}
bool RemoveFile(const char *path)
{
	if (DeleteFileA(path))
		return true;
	NoteFileError();
	return false;
}
bool FileInfo(const char *path, bool *isDir, uint64_t *size)
{
	WIN32_FILE_ATTRIBUTE_DATA d;
	if (!GetFileAttributesExA(path, GetFileExInfoStandard, &d))
		return false;
	*isDir = (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	*size = ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
	return true;
}
bool DiskFreeBytes(const char *path, uint64_t *freeForUser)
{
	ULARGE_INTEGER avail;
	if (!GetDiskFreeSpaceExA(path, &avail, 0, 0))
		return false;
	*freeForUser = avail.QuadPart;
	return true;
}
const char *NativePath(const char *path, char *, size_t) { return path; }   // Win32 takes the game's paths as they are
bool CurrentDir(char *buf, size_t size)
{
	DWORD n = GetCurrentDirectoryA((DWORD)size, buf);
	return n != 0 && n < size;
}
int CdDriveRoots(char *roots, size_t size)
{
	DWORD mask = GetLogicalDrives();
	size_t at = 0;
	int count = 0;
	for (int i = 0; i < 26; i++) {
		if (!(mask & (1u << i)))
			continue;
		char root[4] = { (char)('A' + i), ':', '\\', 0 };
		if (GetDriveTypeA(root) != DRIVE_CDROM || at + 5 > size)
			continue;
		memcpy(roots + at, root, 4);
		at += 4;
		count++;
	}
	if (at < size)
		roots[at] = 0;
	return count;
}

// ---- framework / start-up additions
unsigned ScanCodeToChar(unsigned scanCode) { return MapVirtualKeyA(MapVirtualKeyA(scanCode, MAPVK_VSC_TO_VK), MAPVK_VK_TO_CHAR); }

static STICKYKEYS s_stickySaved, s_stickyOff;
void StickyKeysInit()
{
	s_stickySaved.cbSize = sizeof(STICKYKEYS);
	SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &s_stickySaved, 0);
	s_stickyOff.cbSize = s_stickySaved.cbSize;
	s_stickyOff.dwFlags = s_stickySaved.dwFlags & ~(SKF_AVAILABLE | SKF_HOTKEYACTIVE);
	SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &s_stickyOff, 0);
}
void StickyKeysSet(bool gameActive)
{
	SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), gameActive ? &s_stickyOff : &s_stickySaved, 0);
}
unsigned ScreenSaverTimeout()
{
	UINT t = 0;
	return SystemParametersInfoA(SPI_GETSCREENSAVETIMEOUT, 0, &t, 0) ? t : 0;
}
int DriveRoots(char *roots, size_t size)
{
	DWORD mask = GetLogicalDrives() >> 1;   // drive A is skipped, as the game's scan did
	size_t at = 0;
	int count = 0;
	for (int i = 1; mask != 0; i++, mask >>= 1) {
		if (!(mask & 1))
			continue;
		char root[4] = { (char)('A' + i), ':', '\\', 0 };
		UINT type = GetDriveTypeA(root);
		if (type == DRIVE_UNKNOWN || type == DRIVE_NO_ROOT_DIR || at + 5 > size)
			continue;
		memcpy(roots + at, root, 4);
		at += 4;
		count++;
	}
	if (at < size)
		roots[at] = 0;
	return count;
}

}  // namespace os
}  // namespace zoosters
