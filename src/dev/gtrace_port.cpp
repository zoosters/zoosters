// gtrace, port side: the shared recorder (src/common/gtrace_core.h) opened from the environment.
//   PORT_GTRACE=<watch file>   PORT_GTRACE_OUT=<dir> (default: the current directory)  -> <dir>\port.gtrace.tsv
// Translator is port_gaddr, reads run under SEH, rand is this CRT's thread data _holdrand (_getptd + 0x14, what srand writes and
// PORT_SRAND forces). Frame runs in port_present_hook, after each frame's logic and before the next input poll, chained behind the hook
// port_run.cpp installed. While a recorder is open, PORT_INPUT key / shot frames count from trace frame 0 (game_stubs.cpp asks port_gtrace_base).
#ifdef _WIN32
#define PORT_DIR_SEP '\\'
#else
#define PORT_DIR_SEP '/'
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <mutex>
#include "zoosters/os.h"
#ifndef ZOOSTERS_OS_SDL
#include <windows.h>     // the Windows lanes: SEH codes, the exe's PE header (return-address scan), the legacy lane's rand patch
#endif

#include "common/gtrace_core.h"

extern "C" void *port_gaddr(unsigned game_addr);
extern "C" int port_frames_presented(void);
extern "C" FILE *port_log;
extern "C" void (*port_present_hook)(void);
// The normal build links the modern CRT, which has no MSVC 7.1 thread data (_getptd), so rand / srand are defined below (the game's LCG)
// over a thread-local _holdrand; the recorder and PORT_SRAND need no binary patching.
static __declspec(thread) unsigned t_holdrand = 1;
static unsigned *port_holdrand(void) { return &t_holdrand; }
// Per-thread rand statistics for the semtrace `rng` line (PORT_SEMTRACE): rand calls, a running FNV-1a hash of every returned value and
// srand seed, and srand calls. Per thread so other threads never mix into the game thread's numbers.
static __declspec(thread) unsigned t_rngCalls, t_rngSeeds;
static __declspec(thread) unsigned long long t_rngHash = 0xcbf29ce484222325ull;
static inline void rng_mix(unsigned v)
{
	for (int i = 0; i < 4; i++, v >>= 8) t_rngHash = (t_rngHash ^ (v & 0xff)) * 0x100000001b3ull;
}
extern "C" void port_rng_stats(unsigned *calls, unsigned *seeds, unsigned long long *hash)
{
	*calls = t_rngCalls;
	*seeds = t_rngSeeds;
	*hash = t_rngHash;
}

extern "C" volatile int port_guarded_read;
static gtrace::Recorder *g_rec;
static void (*g_prevHook)(void);
static int g_base = -1;          // presented-frame counter during trace frame 0's logic (-1 = not started)

static void *xlate(unsigned a) { return port_gaddr(a); }
#if !defined(_MSC_VER)
// Without structured exception handling (MinGW / clang GNU mode): every page of [p, p + n) must be committed and readable.
static int readable(const void *p, size_t n) { return zoosters::os::MemoryReadable(p, n) ? 1 : 0; }
#endif
static int readmem(void *dst, const void *src, int n)
{
#if defined(_MSC_VER)
	port_guarded_read = 1;                                // port/runtime.cpp: the fault handler leaves faults of this read to the SEH below
	__try {
		memcpy(dst, src, n);
		port_guarded_read = 0;
		return 1;
	}
	__except (1 /* EXCEPTION_EXECUTE_HANDLER */) {
		port_guarded_read = 0;
		return 0;
	}
#else
	if (!readable(src, (size_t)n)) return 0;
	memcpy(dst, src, n);
	return 1;
#endif
}
static int randfn(unsigned *out)
{
	*out = *port_holdrand();
	return 1;
}

static void port_gtrace_hook(void)
{
	if (g_prevHook) g_prevHook();
	if (!g_rec) return;
	g_rec->Frame();                                       // the counter was already incremented: the logic ran at counter - 1
	if (g_rec->started && g_base < 0) {
		g_base = port_frames_presented() - 1;
		if (port_log) { fprintf(port_log, "gtrace: trace frame 0 = presented frame %d (x87 control word %08x)\n", g_base, _control87(0, 0)); fflush(port_log); }
	}
}

// Called by port_run.cpp after the clock hook is installed.
extern "C" void port_gtrace_init(void)
{
	const char *watch = getenv("PORT_GTRACE");
	if (!watch || !*watch) return;
	const char *dir = getenv("PORT_GTRACE_OUT");
	char out[600];
	_snprintf(out, sizeof(out), "%s%cport.gtrace.tsv", dir && *dir ? dir : ".", PORT_DIR_SEP);
	out[sizeof(out) - 1] = 0;
	g_rec = (gtrace::Recorder *)calloc(1, sizeof(gtrace::Recorder));
	if (!g_rec->Open(watch, out, "port", &xlate, &readmem, &randfn)) {
		printf("gtrace: PORT_GTRACE: Open failed: %s\n", g_rec->err);
		if (port_log) fprintf(port_log, "gtrace: PORT_GTRACE: Open failed: %s\n", g_rec->err);
		free(g_rec);
		g_rec = 0;
		return;
	}
	g_prevHook = port_present_hook;
	port_present_hook = &port_gtrace_hook;
	printf("gtrace: recording %s -> %s\n", watch, out);
	if (port_log) fprintf(port_log, "gtrace: recording %s -> %s\n", watch, out);
}

// game_stubs.cpp (PORT_INPUT): is a recorder requested / open, and the counter value of trace frame 0 (-1 until the start condition has held)
extern "C" int port_gtrace_wanted(void) { const char *w = getenv("PORT_GTRACE"); return w && *w; }
extern "C" int port_gtrace_base(void) { return g_base; }
// trace frame whose logic is running (-1 before the start condition holds), like the stock GTraceTraceFrame: mathlog (rwcore.cpp)
extern "C" int port_gtrace_trace_frame(void) { return g_rec && g_rec->started ? (int)g_rec->frame : -1; }

// ---- rnglog: PORT_RNGLOG=<file> logs every CRT rand and RpRandom draw in the format of the stock side's [options] RngLog
// (src/fixture/rnglog.cpp): <trace frame> <presented frame> <rand|rp> <value> <caller return address, hex> <m|o>
// (trace frame -1 = recorder not started; m = game thread, the one that called port_rnglog_init; o = another). The CRT rand
// (libcmt rand.obj, same 34 bytes as Game.exe 0x558CFA) gets a jump to port_rand written over its first 5 bytes. PORT_SRAND also
// forces RpRandomSeed's seed (stock: GTraceSeed).
static FILE *g_rngf;
static std::mutex g_rngMutex;
static unsigned g_rngMainTid;
static unsigned rng_thread_id(void) { return zoosters::os::ThreadId(); }
#define RNG_LOCK() g_rngMutex.lock()
#define RNG_UNLOCK() g_rngMutex.unlock()
static int g_rngForce;
static unsigned g_rngSeed;

// PORT_RNGLOG_STACK=1: append "caller2,caller3,..." as a 7th field: up to 4 return addresses found by scanning the stack above the
// draw for words pointing right behind a call instruction in the exe, i.e. who called the function that called rand.
static int g_rngStack;
static int rng_is_ret(unsigned a)
{
#ifdef ZOOSTERS_OS_SDL
	(void)a;
	return 0;                                             // reads the exe's PE header: Windows exes only
#else
	static size_t lo, hi;                                 // the exe image
	if (!lo) { unsigned char *b = (unsigned char *)GetModuleHandleA(0); lo = (size_t)b + 0x1000; hi = (size_t)b + ((IMAGE_NT_HEADERS *)(b + ((IMAGE_DOS_HEADER *)b)->e_lfanew))->OptionalHeader.SizeOfImage; }
	if (a < lo || a >= hi) return 0;
	unsigned char c[6];                                   // 6 bytes before the return address
	if (!readmem(c, (const void *)(size_t)(a - 6), 6)) return 0;
	return c[1] == 0xE8 || c[4] == 0xFF || c[3] == 0xFF || c[0] == 0xFF;   // c[k] = byte at a - 6 + k
#endif
}
static void rng_draw(const char *which, unsigned value, void *caller)
{
	if (!g_rngf) return;
	char thr = rng_thread_id() == g_rngMainTid ? 'm' : 'o';
	int tf = g_rec && g_rec->started ? (int)g_rec->frame : -1;
	char more[96]; more[0] = 0;
	if (g_rngStack) {
		unsigned *sp = (unsigned *)&which; int n = 0, len = 0, seen = 0;
		for (int i = 0; i < 300 && n < 4; i++) {
			unsigned w = sp[i];
			if (w == (unsigned)(size_t)caller) { seen = 1; continue; }
			if (seen && rng_is_ret(w)) { len += _snprintf(more + len, sizeof more - len, "%s%08X", n ? "," : " ", w); n++; }
		}
	}
	RNG_LOCK();
	fprintf(g_rngf, "%d %d %s %u %08X %c%s\n", tf, port_frames_presented(), which, value, (unsigned)(size_t)caller, thr, more);
	RNG_UNLOCK();
}
extern "C" void port_rnglog_rp(unsigned value, void *caller) { rng_draw("rp", value, caller); }
extern "C" unsigned port_rnglog_seed(unsigned seed)
{
	if (!g_rngForce) return seed;
	if (port_log) { fprintf(port_log, "rnglog: RpRandomSeed(%08X) -> PORT_SRAND seed %u\n", seed, g_rngSeed); fflush(port_log); }
	return g_rngSeed;
}

extern "C" void RpRandomSeed(unsigned seed);                       // rwcore.cpp
extern "C" int __cdecl rand(void)             // libcmt's LCG (state * 214013 + 2531011, bits 16..30), logged when PORT_RNGLOG is on
{
	void *ret = PORT_RETURN_ADDRESS();
	unsigned *h = port_holdrand();
	*h = *h * 0x343FDu + 0x269EC3u;
	int v = (int)((*h >> 16) & 0x7FFF);
	t_rngCalls++;
	rng_mix((unsigned)v);
	rng_draw("rand", (unsigned)v, ret);
	return v;
}
extern "C" void __cdecl srand(unsigned seed)   // PORT_SRAND=<n>: every srand(seed) of the game uses n (stock: GTraceSeed)
{
	if (g_rngForce) {
		if (port_log) { fprintf(port_log, "port_run: srand(%u) -> PORT_SRAND seed %u (frame %d)\n", seed, g_rngSeed, port_frames_presented()); fflush(port_log); }
		seed = g_rngSeed;
	}
	t_rngSeeds++;
	rng_mix(seed ^ 0x5eed0000u);
	*port_holdrand() = seed;
}

// Called by port_run.cpp (main thread, after port_install_srand).
extern "C" void port_rnglog_init(void)
{
	const char *sr = getenv("PORT_SRAND");
	if (sr && *sr) { g_rngForce = 1; g_rngSeed = (unsigned)strtoul(sr, 0, 0); }
	const char *path = getenv("PORT_RNGLOG");
	if (!path || !*path) return;
	{ const char *st = getenv("PORT_RNGLOG_STACK"); g_rngStack = st && st[0] == '1'; }
	g_rngMainTid = rng_thread_id();
	g_rngf = fopen(path, "w");
	if (!g_rngf) { printf("rnglog: cannot open %s\n", path); return; }
	fprintf(g_rngf, "# rnglog 1 side=port\n");
	RpRandomSeed(0x9a319039u);                                // rand above logs by itself, nothing to patch
	printf("rnglog: PORT_RNGLOG=%s: rand logged\n", path);
	if (port_log) { fprintf(port_log, "rnglog: PORT_RNGLOG=%s: rand logged\n", path); fflush(port_log); }
	return;
}
