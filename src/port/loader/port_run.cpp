// Harness: runs the game's WinMain (0x4646f0) for N frames.
//   port_run.exe <game dir> [frames] [options...]
// Working directory = the game install. Options: k = log missing externals and continue, n = skip the game's static initialisers, wNNNxNNN video mode (default 800x600).
// Log: port_run.log next to the exe.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <filesystem>
#include <string>
#include <thread>
#include "zoosters/os.h"

extern "C" {
#ifdef _WIN32
int __stdcall WinMain(void *hInstance, void *hPrev, char *cmdLine, int showCmd);   // src/game (_WinMain@16)
#else
int WinMain(void *hInstance, void *hPrev, char *cmdLine, int showCmd);
#endif
#ifndef ZOOSTERS_OS_SDL
__declspec(dllimport) int __stdcall SetCurrentDirectoryA(const char *path);
__declspec(dllimport) void *__stdcall GetModuleHandleA(const char *name);
__declspec(dllimport) int __stdcall CreateDirectoryA(const char *path, void *sa);
__declspec(dllimport) int __stdcall SetEnvironmentVariableA(const char *name, const char *value);
#endif
void port_install_fault_handler(void);
void port_win_set_frames(int n);
int port_win_pumps(void);
void port_set_video_mode(int w, int h);
void port_skip_plugin(unsigned id);
void port_skip_plugin_ctor(unsigned id);
extern FILE *port_log;
extern int port_trap_continue;
int port_game_static_init(void);
void port_dump_symbols(const char *path);   // provisional/game_access.cpp (PORT_SYMDUMP; normal build)
void port_symdump_install(void);           // provisional/game_access.cpp (PORT_SYMDUMP_AT)
int port_apply_vtable_fixups(void);
void port_trace_set_path(const char *path);
void port_trace_dump(const char *path);
}

// PORT_HEAPFILL=<hex byte>: every HeapAlloc / HeapReAlloc block (the CRT heap: malloc, operator new, the game's MemAlloc) is
// filled with that byte (diagnostic for reads of uninitialised heap memory, whose old contents are pointers into the exe)
extern "C" void port_semtrace_init(void);
#ifdef PORT_CALLTRACE_BUILD
extern "C" void port_calltrace_init(void);   // provisional/calltrace.cpp (build option --calltrace)
#endif
#ifndef ZOOSTERS_OS_SDL
extern "C" __declspec(dllimport) int __stdcall VirtualProtect(void *, unsigned long, unsigned long, unsigned long *);
// The exe's own import address table: port_iat_slot(dll, function) = the slot every caller in the exe reaches that import
// through, found by walking the PE import descriptors (dll compared without case); 0 when the exe does not import it by name. The
// test-harness options below replace slots. Windows only.
static int port_iat_ieq(const char *a, const char *b)
{
	for (; *a && *b; a++, b++)
		if ((*a | 0x20) != (*b | 0x20)) return 0;
	return *a == *b;
}
extern "C" void **port_iat_slot(const char *dll, const char *function)
{
	unsigned char *base = (unsigned char *)GetModuleHandleA(0);
	unsigned pe = *(unsigned *)(base + 0x3c);
	unsigned short magic = *(unsigned short *)(base + pe + 24);
	unsigned char *dirs = base + pe + 24 + (magic == 0x20b ? 112 : 96);   // optional header: data directories (PE32+ / PE32)
	unsigned rva = *(unsigned *)(dirs + 8);                                 // IMAGE_DIRECTORY_ENTRY_IMPORT
	if (!rva) return 0;
	for (unsigned *d = (unsigned *)(base + rva); d[3]; d += 5) {          // IMAGE_IMPORT_DESCRIPTOR: names, -, -, dll name, IAT
		if (!port_iat_ieq((const char *)base + d[3], dll)) continue;
		size_t *names = (size_t *)(base + (d[0] ? d[0] : d[4])), *iat = (size_t *)(base + d[4]);
		for (; *names; names++, iat++) {
			if (*names >> (sizeof(size_t) * 8 - 1)) continue;             // by ordinal
			if (!strcmp((const char *)base + (unsigned)*names + 2, function)) return (void **)iat;   // IMAGE_IMPORT_BY_NAME: hint, name
		}
	}
	return 0;
}
// Replaces the import's slot with fn; returns the previous target (0 when the exe does not import it).
extern "C" void *port_iat_hook(const char *dll, const char *function, void *fn)
{
	void **slot = port_iat_slot(dll, function);
	if (!slot) return 0;
	unsigned long old;
	void *prev = *slot;
	VirtualProtect(slot, sizeof(void *), 4 /* PAGE_READWRITE */, &old);
	*slot = fn;
	VirtualProtect(slot, sizeof(void *), old, &old);
	return prev;
}
extern "C" __declspec(dllimport) unsigned long __stdcall HeapSize(void *, unsigned long, const void *);
typedef void *(__stdcall *HeapAllocFn)(void *, unsigned long, unsigned long);
typedef void *(__stdcall *HeapReAllocFn)(void *, unsigned long, void *, unsigned long);
static HeapAllocFn g_realHeapAlloc;
static HeapReAllocFn g_realHeapReAlloc;
static int g_heapFill = -1;
static void *__stdcall fillHeapAlloc(void *h, unsigned long flags, unsigned long n)
{
	void *p = g_realHeapAlloc(h, flags, n);
	if (p && !(flags & 8 /* HEAP_ZERO_MEMORY */)) memset(p, g_heapFill, n);
	return p;
}
static void *__stdcall fillHeapReAlloc(void *h, unsigned long flags, void *old, unsigned long n)
{
	unsigned long was = old ? HeapSize(h, 0, old) : 0;
	void *p = g_realHeapReAlloc(h, flags, old, n);
	if (p && n > was && !(flags & 8)) memset((char *)p + was, g_heapFill, n - was);
	return p;
}
static void port_install_heapfill(void)
{
	const char *e = getenv("PORT_HEAPFILL");
	if (!e) return;
	g_heapFill = (int)strtoul(e, 0, 16);
	g_realHeapAlloc = (HeapAllocFn)port_iat_hook("kernel32.dll", "HeapAlloc", (void *)&fillHeapAlloc);
	g_realHeapReAlloc = (HeapReAllocFn)port_iat_hook("kernel32.dll", "HeapReAlloc", (void *)&fillHeapReAlloc);
}
#else
// The portable build has no import table to hook and no Win32 heap, so the IAT-based options (PORT_HEAPFILL, the GetCursorPos
// replacement, PORT_MULTI_INSTANCE) do not exist there. The fixed clock and the mouse reach the game through zoosters::os
// (SetVirtualClock; os_sdl.cpp's CursorPos calls port_mouse_get).
static void port_install_heapfill(void) {}
#endif

// PORT_FIXEDTIME=<whole ms per frame, 0 = 16.667>: a virtual clock for the windowed build (the headless one has its own counter):
// every time source the game reads (QueryPerformanceCounter/Frequency, GetTickCount, timeGetTime) returns start + presented
// frames x the frame time + 1 us per call (so that wait loops still advance). Runs become repeatable frame by frame (the title
// fly-over otherwise depends on how long loading took).
extern "C" int port_frames_presented(void);
static unsigned g_vtFrameUs;
static unsigned __int64 g_vtCalls;
static std::thread::id g_vtMainTid;                    // the presenting (game) thread
static bool g_vtHaveMain;
static int g_vtCarry;                                  // PORT_FIXEDTIME_CARRY=1: the literal rule (+1 us per read from any thread, never reset)
// gtrace clock rule (as src/fixture/gtrace.cpp): the per-read counter restarts at 0 after every Present (port_vt_present, called
// by RwCameraShowRaster), so spin loops on the clock do not shift later frames; reads by threads other than the presenting one
// return the current time without advancing the counter.
extern "C" void (*port_present_hook)(void);            // rwcore.cpp: called after every Present
static void port_vt_present(void) { if (!g_vtCarry) g_vtCalls = 0; }
static uint64_t vtNowUs(void)   // not unsigned __int64: on Linux uint64_t is unsigned long
{
	unsigned __int64 base = 1000000ull + (unsigned __int64)(unsigned)port_frames_presented() * g_vtFrameUs;   // integer math only (no CRT float helpers)
	if (!g_vtCarry && g_vtHaveMain && std::this_thread::get_id() != g_vtMainTid) return base + g_vtCalls;
	return base + (++g_vtCalls);
}
#ifndef ZOOSTERS_OS_SDL
static int __stdcall vtQPC(__int64 *c) { *c = (__int64)vtNowUs(); return 1; }
static int __stdcall vtQPF(__int64 *f) { *f = 1000000; return 1; }
static unsigned long __stdcall vtTick(void) { return (unsigned long)(vtNowUs() / 1000); }
#endif
static void port_install_fixedtime(void)
{
	const char *e = getenv("PORT_FIXEDTIME");
	if (!e) return;
	g_vtFrameUs = (unsigned)(atoi(e) * 1000);              // whole milliseconds
	{ const char *c = getenv("PORT_FIXEDTIME_CARRY"); g_vtCarry = c && c[0] == '1'; }
	port_present_hook = &port_vt_present;
	g_vtMainTid = std::this_thread::get_id();              // main installs it and runs the game on the same thread
	g_vtHaveMain = true;
	if (!g_vtFrameUs) g_vtFrameUs = 16667;
#ifdef ZOOSTERS_OS_SDL
	zoosters::os::SetVirtualClock(&vtNowUs);                  // TickMs / PerfCounter / PerfFrequency answer from the virtual clock
#else
	port_iat_hook("kernel32.dll", "QueryPerformanceCounter", (void *)&vtQPC);
	port_iat_hook("kernel32.dll", "QueryPerformanceFrequency", (void *)&vtQPF);
	port_iat_hook("kernel32.dll", "GetTickCount", (void *)&vtTick);
	port_iat_hook("winmm.dll", "timeGetTime", (void *)&vtTick);
#endif
}

// PORT_SRAND=<n>: every srand(seed) of the game uses n. The game's srand is libcmt's `_srand` (13 bytes, the same code as
// Game.exe 0x558CED); its first bytes are overwritten by a jump to a replacement that writes the forced seed into the thread data
// (_holdrand). Normal build: srand / rand are the port's own (shim/gtrace_port.cpp, PORT_SRAND read there).
static void port_install_srand(void) {}

// The game's GetCursorPos (Cursor_Show 0x462eb0) assumes a full-screen window (client == screen coordinates): replaced by
// port_mouse_get (port/provisional/mouse.cpp: client coordinates, real mouse while focused or PORT_MOUSE).
#ifndef ZOOSTERS_OS_SDL
extern "C" int __stdcall port_mouse_get(void *pt);
static void port_install_mouse(void) { port_iat_hook("user32.dll", "GetCursorPos", (void *)&port_mouse_get); }
#else
static void port_install_mouse(void) {}   // zoosters::os::CursorPos (os_sdl.cpp) asks port_mouse_get itself
#endif

// PORT_MULTI_INSTANCE=1: WinMain takes the named mutex "Madagascar Running" and quits at once when another copy holds it, so
// parallel test runs would all end at frame 0. With the option the mutex name gets the process id appended. Off = the game's
// rule.
#ifndef ZOOSTERS_OS_SDL
extern "C" __declspec(dllimport) unsigned long __stdcall GetCurrentProcessId(void);
typedef void *(__stdcall *PortCreateMutexFn)(void *, int, const char *);
static PortCreateMutexFn g_realCreateMutexA;
static void *__stdcall port_CreateMutexA(void *attr, int owner, const char *name)
{
	char buf[160];
	if (name) { _snprintf(buf, sizeof buf - 1, "%s #%lu", name, GetCurrentProcessId()); buf[sizeof buf - 1] = 0; name = buf; }
	return g_realCreateMutexA(attr, owner, name);
}
static void port_install_multi_instance(void)
{
	const char *e = getenv("PORT_MULTI_INSTANCE");
	if (!e || e[0] != '1') return;
	g_realCreateMutexA = (PortCreateMutexFn)port_iat_hook("kernel32.dll", "CreateMutexA", (void *)&port_CreateMutexA);
}
#else
static void port_install_multi_instance(void) {}   // zoosters::os::InstanceLock of the portable backend never reports a second instance
#endif

extern "C" void port_gtrace_init(void);
extern "C" void port_rnglog_init(void);
extern "C" void port_dinput_install(void);   // port/shim/dinput_port.cpp: the game's InputManager gets the port's DirectInput
// Portable helpers of main: the last path separator of either kind, the process environment (CRT copy), the current directory, a directory
static char *port_last_sep(char *path)
{
	char *a = strrchr(path, '/'), *b = strrchr(path, '\\');
	return a > b ? a : b;
}
static void port_putenv(const char *kv)
{
#ifdef _WIN32
	_putenv(kv);
#else
	putenv(strdup(kv));
#endif
}
#ifdef ZOOSTERS_OS_SDL
static bool port_chdir(const char *dir) { std::error_code ec; std::filesystem::current_path(dir, ec); return !ec; }
static void port_mkdir(const char *dir) { std::error_code ec; std::filesystem::create_directory(dir, ec); }
static void *port_module_base(void) { return 0; }
static void port_absolute(char *path, size_t size)   // relative to the current directory, which main leaves for the game folder
{
	std::error_code ec;
	std::string abs = std::filesystem::absolute(path, ec).lexically_normal().string();
	if (!ec && abs.size() < size)
		strcpy(path, abs.c_str());
}
#else
static bool port_chdir(const char *dir) { return SetCurrentDirectoryA(dir) != 0; }
static void port_mkdir(const char *dir) { CreateDirectoryA(dir, 0); }
static void *port_module_base(void) { return GetModuleHandleA(0); }
static void port_absolute(char *, size_t) {}   // Windows: argv[0] is the exe's full path
#endif

#ifdef ZOOSTERS_OS_SDL
extern "C" int port_player_game_dir(char *out, int size);   // port/loader/player_launch.cpp
extern "C" int port_player_mod_dir(char *out, int size);
extern "C" int port_user_file(const char *name, char *buf, int size);   // port/provisional/userdir.cpp
extern "C" int port_make_dirs(const char *path);
#endif

int main(int argc, char **argv)
{
	bool player = false;
#ifdef ZOOSTERS_OS_SDL
	// Player launch (no arguments: Finder, the .app bundle): the game folder from player_launch.cpp, no frame limit, the OS user folder
	// (`u`), the log in that folder (not next to the exe: inside a bundle), the bundle's mod files.
	static char playerDir[1024];
	static char *playerArgv[5];
	if (argc < 2) {
		if (!port_player_game_dir(playerDir, sizeof playerDir)) return 2;
		playerArgv[0] = argv[0]; playerArgv[1] = playerDir; playerArgv[2] = (char *)"0"; playerArgv[3] = (char *)"u"; playerArgv[4] = 0;
		argv = playerArgv; argc = 4;
		player = true;
		char mod[1024];
		if (!getenv("PORT_MOD_DIR") && port_player_mod_dir(mod, sizeof mod)) { std::string kv = std::string("PORT_MOD_DIR=") + mod; putenv(strdup(kv.c_str())); }
	}
	// a scripted run (fixture, tests: started with arguments) runs in the background: no focus, no Dock icon (window_sdl.cpp);
	// PORT_BACKGROUND=0 keeps it in front
	if (!player && !getenv("PORT_BACKGROUND")) putenv((char *)"PORT_BACKGROUND=1");
#endif
	port_install_mouse();
	port_dinput_install();
	port_install_multi_instance();
	port_install_heapfill();
	port_install_fixedtime();
	port_install_srand();
	port_rnglog_init();
	FILE *log = 0;
	char tracePath[512] = "";
	{
		char path[512];
		strcpy(path, argv[0]);
		char *sl = port_last_sep(path);
		{ char *dot = strrchr(sl ? sl + 1 : path, '.'); if (dot) *dot = 0; }      // <exe name>.log next to the exe
		strcat(path, ".log");
#ifdef ZOOSTERS_OS_SDL
		if (player && port_user_file("port.log", path, sizeof path)) {             // player launch: <user folder>/Activision/Madagascar/port.log
			char dir[512]; strcpy(dir, path);
			char *ds = port_last_sep(dir); if (ds) { *ds = 0; port_make_dirs(dir); }
			sl = port_last_sep(path);
		}
#endif
		log = fopen(path, "w");
		if (log) setvbuf(log, NULL, _IONBF, 0);
		port_log = log;
		if (log) { freopen(path, "a", stderr); setvbuf(stderr, NULL, _IONBF, 0); }
		if (log) fprintf(log, "port: image base %p\n", port_module_base());   // the exe may load anywhere (ASLR): a crash-log reader rebases addresses with it
		port_install_fault_handler();
		{ char *dot = strrchr(sl ? sl + 1 : path, '.'); if (dot) *dot = 0; }
		strcat(path, "_trace.bin");
		port_trace_set_path(path);
		strcpy(tracePath, path);
	}
	setvbuf(stdout, NULL, _IONBF, 0);
	port_gtrace_init();             // PORT_GTRACE / PORT_GTRACE_OUT (shim/gtrace_port.cpp); chains behind the fixed clock's present hook
	port_semtrace_init();   // PORT_SEMTRACE=<file> (provisional/semtrace.cpp)
#ifdef PORT_CALLTRACE_BUILD
	port_calltrace_init();  // PORT_CALLTRACE=<file>[,<from>-<to>] (provisional/calltrace.cpp)
#endif
	if (argc < 2) { printf("usage: port_run <game dir> [frames] [k] [n] [s<frame>] [f] [u] [wWxH] [800000xx|c800000xx plugin skips]\n"); return 2; }
	int frames = argc > 2 ? atoi(argv[2]) : 3;
	int gameInit = 1, osUserDir = 0;
	for (int i = 3; i < argc; i++) {
		const char *a = argv[i];
		if (!strcmp(a, "k")) port_trap_continue = 1;                       // keep going past missing externals (diagnostic)
		else if (!strcmp(a, "h")) {                                         // self-test: does _heapchk see an overflow under this Wine?
#ifdef _WIN32
			char *p = (char *)malloc(16); memset(p, 0x41, 16 + 64);
			int hc = _heapchk();
			printf("port_run: heap self-test after a 64-byte overflow: _heapchk = %d (%s)\n", hc, hc == _HEAPOK ? "NOT detected" : "detected");
			if (log) fprintf(log, "port_run: heap self-test: _heapchk = %d\n", hc);
#else
			printf("port_run: heap self-test needs the Windows CRT's _heapchk\n");
#endif
			return 0;
		}
		else if (a[0] == 's') { char kv[64]; strcpy(kv, "PORT_CAPTURE="); strcat(kv, a + 1); port_putenv(kv); }   // s<N>: save presented frame N as frame_N.png (windowed build)
		else if (!strcmp(a, "f")) port_putenv("PORT_FULLSCREEN=1");
		else if (!strcmp(a, "n")) gameInit = 0;                            // skip the game's static initialisers
		else if (!strcmp(a, "u")) osUserDir = 1;                           // the OS user folder (userdir.cpp), not <exe dir>\user
		else if (a[0] == 'w') { int w = 0, h = 0; if (sscanf(a + 1, "%dx%d", &w, &h) == 2) port_set_video_mode(w, h); }
		else if (a[0] == 'c') { unsigned id = (unsigned)strtoul(a + 1, 0, 16); if (id) port_skip_plugin_ctor(id); }
		else { unsigned id = (unsigned)strtoul(a, 0, 16); if (id) port_skip_plugin(id); }
	}
	if (!osUserDir && !getenv("PORT_USER_DIR")) {
		char user[512];                                   // user folder (saves, screenshots, settings.ini) next to the exe, never in the game dir
		strcpy(user, argv[0]);
		char *sl = port_last_sep(user);
		strcpy(sl ? sl + 1 : user, "user");
		port_absolute(user, sizeof user);
		port_mkdir(user);
#ifndef ZOOSTERS_OS_SDL
		SetEnvironmentVariableA("PORT_USER_DIR", user);
#endif
		{ char kv[560]; strcpy(kv, "PORT_USER_DIR="); strcat(kv, user); port_putenv(kv); }
	}
	if (!getenv("PORT_GADDR_FILE")) {                     // the game-address map next to the exe (port/shim/gaddr.cpp), as an absolute path: the chdir below moves away
		char kv[600] = "PORT_GADDR_FILE=";
		char *path = kv + strlen(kv);
		size_t room = sizeof(kv) - strlen(kv);
		const char *exe = argv[0];
		bool absolute = exe[0] == '/' || exe[0] == '\\' || (exe[0] && exe[1] == ':');
		if (!absolute && zoosters::os::CurrentDir(path, room) && strlen(path) + 1 < room)
			strcat(path, "/");
		else
			path[0] = 0;
		const char *dot = strrchr(exe, '.');                     // <exe name>.gaddr.tsv
		size_t stem = dot && dot > port_last_sep((char *)exe) ? (size_t)(dot - exe) : strlen(exe);
		if (strlen(path) + stem + sizeof(".gaddr.tsv") <= room) {
			strncat(path, exe, stem);
			strcat(path, ".gaddr.tsv");
			port_putenv(kv);
		}
	}
	if (!port_chdir(argv[1])) { printf("cannot chdir to %s\n", argv[1]); return 2; }
	port_win_set_frames(frames);
	if (log) fprintf(log, "port_run: dir=%s frames=%d\n", argv[1], frames);
	port_apply_vtable_fixups();
	if (gameInit) {
		int ni = port_game_static_init();
		printf("port_run: %d game static initialisers run\n", ni);
		if (log) fprintf(log, "port_run: %d game static initialisers run\n", ni);
		{ const char *sd = getenv("PORT_SYMDUMP"); if (sd && *sd) port_dump_symbols(sd); port_symdump_install(); }   // game_access.cpp
	}
	printf("port_run: WinMain, %d frames\n", frames);
	int rc = WinMain(port_module_base(), 0, (char *)"", 1);
	printf("port_run: WinMain returned %d after %d pumps\n", rc, port_win_pumps());
	if (log) fprintf(log, "port_run: WinMain returned %d after %d pumps\n", rc, port_win_pumps());
	port_trace_dump(tracePath);
	return 0;
}
