// zoosters::os on SDL3 and the C++ standard library (macOS, Linux). Window calls act on the port's one SDL window (port_sdl_window).
#include <SDL3/SDL.h>
#include <chrono>
#include <cerrno>
#include <cstdio>
#include <cstring>
#ifndef _WIN32
#include <strings.h>    // strcasecmp (NativePath)
#endif
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <thread>
#include "zoosters/os.h"
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_vm.h>
#elif !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

extern "C" SDL_Window *port_sdl_window(void);   // port/provisional/window_sdl.cpp

namespace fs = std::filesystem;

namespace zoosters {
namespace os {

// ---- time
static VirtualClockFn g_virtualClock = nullptr;   // PORT_FIXEDTIME (SetVirtualClock)
uint32_t TickMs() { return g_virtualClock ? (uint32_t)(g_virtualClock() / 1000) : (uint32_t)SDL_GetTicks(); }
uint64_t PerfCounter() { return g_virtualClock ? g_virtualClock() : SDL_GetPerformanceCounter(); }
uint64_t PerfFrequency() { return g_virtualClock ? 1000000 : SDL_GetPerformanceFrequency(); }
// Sleep(0) yields and returns at once, as Win32 does. SDL_Delay(0) waits on a kernel timer, and the game's frame timer spins on
// Sleep(0) (MainLoop::Poll).
void SleepMs(unsigned ms)
{
	if (ms)
		SDL_Delay(ms);
	else
		std::this_thread::yield();
}
unsigned TimerMinPeriod() { return 1; }
void TimerResolutionBegin(unsigned) {}
void TimerResolutionEnd(unsigned) {}
// Real uptime (GetTickCount64), never the test clock: SDL_GetTicks starts near 0, which would shift the FrameRate phase the game
// seeds from it.
uint64_t TickMs64() { return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); }

// Real uptime (GetTickCount64), never the test clock: SDL_GetTicks starts near 0, which would shift the FrameRate phase the game
// seeds from it.
void *InstanceLock(const char *, bool *alreadyRunning)
{
	static int token;
	*alreadyRunning = false;
	return &token;
}
void InstanceUnlock(void *) {}
void ShowError(const char *caption, const char *text) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, caption, text, port_sdl_window()); }
void DebugPrint(const char *text) { std::fputs(text, stderr); }
void RequestQuit()
{
	SDL_Event e;
	SDL_zero(e);
	e.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&e);
}

// ---- display: the gl3 device owns fullscreen modes; the desktop mode is only read
bool DesktopMode(uint32_t *w, uint32_t *h)
{
	const SDL_DisplayMode *m = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
	if (!m)
		return false;
	*w = (uint32_t)m->w;
	*h = (uint32_t)m->h;
	return true;
}
bool SetDisplayMode(uint32_t, uint32_t) { return true; }
bool RestoreDisplayMode() { return true; }

// ---- window
bool ClientSize(void *, int *w, int *h)
{
	SDL_Window *win = port_sdl_window();
	return win && SDL_GetWindowSize(win, w, h);
}
bool ClientToScreen(void *, int *x, int *y)
{
	SDL_Window *win = port_sdl_window();
	int wx = 0, wy = 0;
	if (!win || !SDL_GetWindowPosition(win, &wx, &wy))
		return false;
	*x += wx;
	*y += wy;
	return true;
}
// PORT_HIDDEN=1: the window stays hidden, also when the game asks to show it
static bool windowHidden()
{
	const char *v = SDL_getenv("PORT_HIDDEN");
	return v && *v == '1';
}

void ShowWindow(void *, int cmd)
{
	SDL_Window *win = port_sdl_window();
	if (!win)
		return;
	if (cmd == 0)          // SW_HIDE
		SDL_HideWindow(win);
	else if (cmd == 6) {   // SW_MINIMIZE: the game asks for it when it loses the focus (made for exclusive full screen). A window stays where it is,
		               // as desktop games do (on macOS a minimised window does not come back with Cmd-Tab: the game looked dead); the game still pauses
		// exclusive full screen only: minimising a borderless one (macOS: a full-screen Space) pulls it out of its Space
		if ((SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN) && SDL_GetWindowFullscreenMode(win))
			SDL_MinimizeWindow(win);
	}
	else if (!windowHidden())
		SDL_ShowWindow(win);
}
void UpdateWindow(void *) {}
void InvalidateWindow(void *) {}
void ValidateWindow(void *) {}
void DestroyWindow(void *) {}   // the window backend owns the SDL window
intptr_t DefaultWindowProc(void *, unsigned, uintptr_t, intptr_t) { return 0; }
void CaptureMouse(void *, bool on) { SDL_CaptureMouse(on); }
void ScreenSaver(bool enable)
{
	if (enable)
		SDL_EnableScreenSaver();
	else
		SDL_DisableScreenSaver();
}

// ---- cursor: ShowCursor's display counter (shown while >= 0)
static int g_cursorCount = 0;
int ShowCursor(bool show)
{
	g_cursorCount += show ? 1 : -1;
	if (g_cursorCount >= 0)
		SDL_ShowCursor();
	else
		SDL_HideCursor();
	return g_cursorCount;
}
// The game assumes client == screen coordinates: the pointer comes from port_mouse_get (mouse.cpp).
extern "C" int port_mouse_get(void *pt);
bool CursorPos(int *x, int *y)
{
	int pt[2] = { 0, 0 };
	if (!port_mouse_get(pt))
		return false;
	*x = pt[0];
	*y = pt[1];
	return true;
}
void SetCursorPos(int x, int y) { SDL_WarpMouseGlobal((float)x, (float)y); }

// ---- keyboard: MapVirtualKey(vk, 0) for the keys the game asks about (US layout set-1 scan codes)
unsigned VirtualKeyToScanCode(unsigned vk)
{
	static const unsigned char letters[26] = { 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32,
	                                           0x31, 0x18, 0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c };
	static const unsigned char digits[10] = { 0x0b, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };
	if (vk >= 'A' && vk <= 'Z')
		return letters[vk - 'A'];
	if (vk >= '0' && vk <= '9')
		return digits[vk - '0'];
	if (vk >= 0x70 && vk <= 0x79)        // F1..F10
		return 0x3b + (vk - 0x70);
	switch (vk) {
	case 0x08: return 0x0e;   // backspace
	case 0x09: return 0x0f;   // tab
	case 0x0d: return 0x1c;   // enter
	case 0x10: return 0x2a;   // shift
	case 0x11: return 0x1d;   // control
	case 0x12: return 0x38;   // alt
	case 0x1b: return 0x01;   // escape
	case 0x20: return 0x39;   // space
	case 0x25: return 0x4b;   // left
	case 0x26: return 0x48;   // up
	case 0x27: return 0x4d;   // right
	case 0x28: return 0x50;   // down
	case 0x7a: return 0x57;   // F11
	case 0x7b: return 0x58;   // F12
	}
	return 0;
}

// ---- port side
void SetVirtualClock(VirtualClockFn fn) { g_virtualClock = fn; }
// The kernel reads the bytes and fails instead of faulting: mach_vm_read_overwrite on macOS (ASan does not intercept it), a pipe
// write elsewhere (EFAULT). Diagnostics and the debug overlay only.
bool MemoryReadable(const void *p, size_t n)
{
	if (!p)
		return false;
	const char *at = (const char *)p;
	char sink[256];
#ifdef __APPLE__
	while (n) {
		mach_vm_size_t k = n < sizeof sink ? n : sizeof sink, got = 0;
		if (mach_vm_read_overwrite(mach_task_self(), (mach_vm_address_t)at, k, (mach_vm_address_t)sink, &got) != KERN_SUCCESS || got != k)
			return false;
		at += k;
		n -= (size_t)k;
	}
	return true;
#elif defined(_WIN32)
	(void)at; (void)sink; (void)n;
	return true;   // (the Windows build uses os_win32.cpp's VirtualQuery probe; an SDL build on Windows has none)
#else
	static int fds[2] = { -1, -1 };
	if (fds[0] < 0) {
		if (pipe(fds) != 0)
			return false;
		fcntl(fds[0], F_SETFL, O_NONBLOCK);
		fcntl(fds[1], F_SETFL, O_NONBLOCK);
	}
	while (n) {
		size_t k = n < sizeof sink ? n : sizeof sink;
		ssize_t w = write(fds[1], at, k);
		if (w <= 0)
			return false;
		if (read(fds[0], sink, (size_t)w) != w)
			return false;
		at += w;
		n -= (size_t)w;
	}
	return true;
#endif
}
uint32_t ThreadId() { return (uint32_t)SDL_GetCurrentThreadID(); }
// A bare name is looked up next to the exe first, as LoadLibrary does (the build puts libopenal.dylib there on macOS), then on
// the system path.
void *LibraryOpen(const char *name)
{
	if (!strchr(name, '/') && !strchr(name, '\\')) {
		if (const char *base = SDL_GetBasePath()) {
			std::string local = std::string(base) + name;
			if (SDL_SharedObject *lib = SDL_LoadObject(local.c_str()))
				return (void *)lib;
		}
#ifdef __APPLE__
		std::string bundled = std::string("@executable_path/../Frameworks/") + name;   // the .app bundle (base path = Contents/Resources)
		if (SDL_SharedObject *lib = SDL_LoadObject(bundled.c_str()))
			return (void *)lib;
#endif
	}
	return (void *)SDL_LoadObject(name);
}
void *LibrarySymbol(void *lib, const char *name) { return (void *)SDL_LoadFunction((SDL_SharedObject *)lib, name); }
// SDL_GetPrefPath("Activision", "Madagascar") = <per-user data folder>/Activision/Madagascar/ (it creates it): the root is two levels up
bool UserDataRoot(char *buf, size_t size)
{
	char *pref = SDL_GetPrefPath("Activision", "Madagascar");
	if (!pref)
		return false;
	std::string root(pref);
	SDL_free(pref);
	for (int level = 0; level < 2; level++) {
		while (!root.empty() && (root.back() == '/' || root.back() == '\\'))
			root.pop_back();
		size_t cut = root.find_last_of("/\\");
		if (cut == std::string::npos)
			return false;
		root.resize(cut);
	}
	if (root.empty() || root.size() + 1 > size)
		return false;
	std::memcpy(buf, root.c_str(), root.size() + 1);
	return true;
}

// ---- files
// Handles are small table indexes (1..N), not FILE pointers: the save layer keeps its handle in an int.
static const int kMaxFiles = 16;
static FILE *g_files[kMaxFiles];
static thread_local FileError g_lastFileError = FILE_ERROR_NONE;
static void NoteErrno(int e)
{
	switch (e) {
	case ENOENT: case ENOTDIR: g_lastFileError = FILE_ERROR_NOT_FOUND; break;
	case ENOSPC: g_lastFileError = FILE_ERROR_DISK_FULL; break;
	case ENXIO: case ENODEV: g_lastFileError = FILE_ERROR_NOT_READY; break;
	default: g_lastFileError = FILE_ERROR_OTHER; break;
	}
}
FileError LastFileError() { return g_lastFileError; }

// ---- paths
#ifdef _WIN32
const char *NativePath(const char *path, char *, size_t) { return path; }
#else
// A component that does not exist as spelled: the directory entry matching it ignoring ASCII case (the game asks for
// "LEVELS\Title.stream"; Linux file systems are case sensitive).
static bool MatchCase(const std::string &dir, std::string &name)
{
	std::error_code ec;
	for (fs::directory_iterator it(dir.empty() ? "." : dir, ec), end; !ec && it != end; it.increment(ec)) {
		std::string entry = it->path().filename().string();
		if (entry.size() == name.size() && !strcasecmp(entry.c_str(), name.c_str())) {
			name = entry;
			return true;
		}
	}
	return false;
}
const char *NativePath(const char *path, char *buf, size_t size)
{
	if (!path)
		return path;
	std::string p(path);
	for (char &c : p)
		if (c == '\\')
			c = '/';
	std::error_code ec;
	if (!fs::exists(p, ec)) {          // fold the case of each component that does not exist as spelled (the last one may be a file to create)
		std::string out = !p.empty() && p[0] == '/' ? "/" : "";
		size_t at = out.size();
		while (at <= p.size()) {
			size_t next = p.find('/', at);
			std::string part = p.substr(at, next == std::string::npos ? std::string::npos : next - at);
			if (!part.empty()) {
				std::string here = out + part;
				if (part != "." && part != ".." && !fs::exists(here, ec))
					MatchCase(out, part);
				out += part;
			}
			if (next == std::string::npos)
				break;
			if (!out.empty() && out.back() != '/')
				out += '/';
			at = next + 1;
		}
		p = out;
	}
	if (p.size() + 1 > size)
		return path;
	std::memcpy(buf, p.c_str(), p.size() + 1);
	return buf;
}
#endif
static FILE *FileOf(intptr_t f) { return f >= 1 && f <= kMaxFiles ? g_files[f - 1] : nullptr; }
static intptr_t OpenRaw(const char *path, int access, FileCreate create);
intptr_t FileOpen(const char *path, int access, FileCreate create)
{
	for (int i = 0; i < kMaxFiles; i++) {
		if (g_files[i])
			continue;
		char native[1024];
		intptr_t raw = OpenRaw(NativePath(path, native, sizeof native), access, create);
		if (raw == -1) {
			NoteErrno(errno);
			return -1;
		}
		g_files[i] = (FILE *)raw;
		return i + 1;
	}
	g_lastFileError = FILE_ERROR_OTHER;
	return -1;
}
static intptr_t OpenRaw(const char *path, int access, FileCreate create)
{
	const char *mode;
	if (create == FILE_CREATE_ALWAYS)
		mode = (access & FILE_READ) ? "w+b" : "wb";
	else if (create == FILE_OPEN_ALWAYS) {
		std::error_code ec;
		if (!fs::exists(path, ec)) {
			FILE *f = std::fopen(path, "wb");
			if (!f)
				return -1;
			std::fclose(f);
		}
		mode = (access & FILE_WRITE) ? "r+b" : "rb";
	} else
		mode = (access & FILE_WRITE) ? "r+b" : "rb";
	FILE *f = std::fopen(path, mode);
	return f ? (intptr_t)f : -1;
}
bool FileRead(intptr_t f, void *buf, uint32_t n, uint32_t *done)
{
	FILE *fp = FileOf(f);
	*done = 0;
	if (!fp)
		return false;
	*done = (uint32_t)std::fread(buf, 1, n, fp);
	return !std::ferror(fp);
}
bool FileWrite(intptr_t f, const void *buf, uint32_t n, uint32_t *done)
{
	FILE *fp = FileOf(f);
	*done = 0;
	if (!fp)
		return false;
	*done = (uint32_t)std::fwrite(buf, 1, n, fp);
	return *done == n;
}
bool FileClose(intptr_t f)
{
	FILE *fp = FileOf(f);
	if (!fp)
		return false;
	g_files[f - 1] = nullptr;
	return std::fclose(fp) == 0;
}
bool MakeDir(const char *path)
{
	char native[1024];
	path = NativePath(path, native, sizeof native);
	std::error_code ec;
	fs::create_directory(path, ec);
	if (fs::is_directory(path, ec))
		return true;
	NoteErrno(ec ? ec.value() : ENOENT);
	return false;
}
bool RemoveFile(const char *path)
{
	char native[1024];
	path = NativePath(path, native, sizeof native);
	std::error_code ec;
	if (fs::remove(path, ec))
		return true;
	if (ec)
		NoteErrno(ec.value());
	else
		g_lastFileError = FILE_ERROR_NOT_FOUND;
	return false;
}
bool FileInfo(const char *path, bool *isDir, uint64_t *size)
{
	char native[1024];
	path = NativePath(path, native, sizeof native);
	std::error_code ec;
	fs::file_status st = fs::status(path, ec);
	if (ec || !fs::exists(st))
		return false;
	*isDir = fs::is_directory(st);
	*size = *isDir ? 0 : (uint64_t)fs::file_size(path, ec);
	return true;
}
bool DiskFreeBytes(const char *path, uint64_t *freeForUser)
{
	char native[1024];
	path = NativePath(path, native, sizeof native);
	std::error_code ec;
	fs::space_info si = fs::space(path, ec);
	if (ec)
		return false;
	*freeForUser = (uint64_t)si.available;
	return true;
}
bool CurrentDir(char *buf, size_t size)
{
	std::error_code ec;
	std::string s = fs::current_path(ec).string();
	if (ec || s.size() + 1 > size)
		return false;
	std::memcpy(buf, s.c_str(), s.size() + 1);
	return true;
}
int CdDriveRoots(char *roots, size_t size)
{
	if (size)
		roots[0] = 0;
	return 0;
}

// ---- framework / start-up additions
// DirectInput scan code (PC set 1) -> the character MapVirtualKey gives on a US layout (upper-case letters, no shift states).
unsigned ScanCodeToChar(unsigned sc)
{
	static const char row0[] = "1234567890-=";   // 0x02..0x0D
	static const char row1[] = "QWERTYUIOP[]";   // 0x10..0x1B
	static const char row2[] = "ASDFGHJKL;'`";   // 0x1E..0x29
	static const char row3[] = "\\ZXCVBNM,./";      // 0x2B..0x35
	if (sc >= 0x02 && sc <= 0x0d) return (unsigned char)row0[sc - 0x02];
	if (sc >= 0x10 && sc <= 0x1b) return (unsigned char)row1[sc - 0x10];
	if (sc >= 0x1e && sc <= 0x29) return (unsigned char)row2[sc - 0x1e];
	if (sc >= 0x2b && sc <= 0x35) return (unsigned char)row3[sc - 0x2b];
	if (sc == 0x39) return ' ';
	return 0;
}
void StickyKeysInit() {}
void StickyKeysSet(bool) {}
unsigned ScreenSaverTimeout() { return 0; }
int DriveRoots(char *roots, size_t size)
{
	if (size)
		roots[0] = 0;
	return 0;
}

}  // namespace os
}  // namespace zoosters
