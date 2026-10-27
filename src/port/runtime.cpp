// Port runtime: the trap every generated stand-in jumps to, fault logging, and the diagnostics (watchdog, sampler, write watch, gap trace).
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#ifndef ZOOSTERS_OS_SDL
__declspec(dllimport) void __stdcall ExitProcess(unsigned int code);
#endif
}
#ifdef ZOOSTERS_OS_SDL
#define PORT_EXIT(code) _Exit(code)       // the portable build: no ExitProcess (the process ends without the atexit chain, as ExitProcess does)
#else
#define PORT_EXIT(code) ExitProcess(code)
#endif

// Entered by `push idx; call _port_trap` from a stand-in: idx names the symbol that has no implementation yet.
extern "C" FILE *port_log = 0;                       // a harness may point this at its log file (port_load.cpp)

// Faults: log the exception code and address (map it with port/build/port_load.map) and exit instead of hanging in Wine.
extern "C" void port_trace_dump(const char *path);
extern "C" void port_dump_registry(void);
static char port_trace_path[512];
#ifdef ZOOSTERS_OS_SDL
// The portable build has none of the Windows crash / diagnostics harness below (vectored exception handler, write watch, watchdog
// and sampler threads, gap tracing); fault64.cpp is left out of it too.
extern "C" volatile int port_guarded_read = 0;
extern "C" void port_watch_arm(void *) {}
extern "C" void port_install_fault_handler(void) {}
#else
struct PortExceptionRecord { unsigned long code, flags; void *rec, *addr; unsigned long nparams; unsigned long info[15]; };
struct PortExceptionPointers { PortExceptionRecord *rec; void *ctx; };
struct PortMemInfo { void *base, *allocBase; unsigned long allocProtect, regionSize, state, protect, type; };
extern "C" __declspec(dllimport) unsigned long __stdcall VirtualQuery(const void *, PortMemInfo *, unsigned long);
static int port_readable(const void *p, unsigned n)   // committed, readable pages (no probing: Wine's IsBadReadPtr raises the fault our vectored handler would log)
{
	PortMemInfo mi;
	if (!VirtualQuery(p, &mi, sizeof(mi)) || mi.state != 0x1000 || (mi.protect & 0x101) || !(mi.protect & 0xEE)) return 0;
	return (unsigned)((char *)mi.base + mi.regionSize - (const char *)p) >= n;
}
extern "C" __declspec(dllimport) void *__stdcall AddVectoredExceptionHandler(unsigned long first, void *handler);
static int port_in_fault;
extern "C" void port_trace_ring_dump(FILE *f);
static void *port_ww_addr;                            // port_watch_arm: hardware write watchpoint (DR0, 4 bytes)
static int port_ww_hits;
// PORT_GAPTRACE=<tsv "port_addr size game_addr" per $gap$ block> PORT_GAPTRACE_OUT=<file>: which bytes of the image gaps (unnamed
// Game.exe data between placed blocks) the port reads or writes. Gap pages become PAGE_NOACCESS; a violation records the byte
// (first access per byte: "<game addr> r|w"), opens the page and single-steps (trap flag), which closes it again. Slow (every
// access to those pages traps): short runs only.
extern "C" __declspec(dllimport) int __stdcall VirtualProtect(void *addr, unsigned size, unsigned prot, unsigned *old);
struct PortGap { uintptr_t lo, hi; unsigned game; };
static PortGap *port_gaps; static int port_ngaps;
static uintptr_t *port_gpages; static int port_ngpages;
static unsigned char *port_gseen;                    // per gap byte: 1 read, 2 written
static unsigned *port_gseen_off;                     // per gap: offset into port_gseen
static FILE *port_gapout;
static volatile uintptr_t port_gpending;              // page to close after the single step
static int port_gap_page(uintptr_t a)
{
	uintptr_t pg = a & ~(uintptr_t)0xfff; int lo = 0, hi = port_ngpages - 1;
	while (lo <= hi) { int m = (lo + hi) / 2; if (port_gpages[m] == pg) return 1; if (port_gpages[m] < pg) lo = m + 1; else hi = m - 1; }
	return 0;
}
static void port_gap_hit(uintptr_t a, int write)
{
	int lo = 0, hi = port_ngaps - 1;
	while (lo <= hi) {
		int m = (lo + hi) / 2;
		if (a < port_gaps[m].lo) hi = m - 1; else if (a >= port_gaps[m].hi) lo = m + 1;
		else {
			unsigned char *s = &port_gseen[port_gseen_off[m] + (a - port_gaps[m].lo)];
			unsigned char bit = write ? 2 : 1;
			if (!(*s & bit)) { *s |= bit; if (port_gapout) { fprintf(port_gapout, "%08x %c\n", port_gaps[m].game + (a - port_gaps[m].lo), write ? 'w' : 'r'); fflush(port_gapout); } }
			return;
		}
	}
}
static void port_gaptrace_init(void)
{
	const char *in = getenv("PORT_GAPTRACE"), *out = getenv("PORT_GAPTRACE_OUT");
	if (!in || !*in || !out || !*out) return;
	FILE *f = fopen(in, "r"); if (!f) return;
	int cap = 1024; port_gaps = (PortGap *)malloc(cap * sizeof(PortGap)); port_gseen_off = (unsigned *)malloc(cap * sizeof(unsigned));
	unsigned long long pa; unsigned sz, ga, total = 0;
	while (port_ngaps < cap && fscanf(f, "%llx %u %x", &pa, &sz, &ga) == 3) {
		port_gaps[port_ngaps].lo = (uintptr_t)pa; port_gaps[port_ngaps].hi = (uintptr_t)pa + sz; port_gaps[port_ngaps].game = ga;
		port_gseen_off[port_ngaps] = total; total += sz; port_ngaps++;
	}
	fclose(f);
	port_gseen = (unsigned char *)calloc(total + 1, 1);
	port_gpages = (uintptr_t *)malloc(sizeof(uintptr_t) * (total / 4096 + 2 * port_ngaps + 8));
	for (int i = 0; i < port_ngaps; i++)                 // input sorted by port address: pages come out sorted, skip repeats
		for (uintptr_t pg = port_gaps[i].lo & ~(uintptr_t)0xfff; pg < port_gaps[i].hi; pg += 0x1000)
			if (!port_ngpages || port_gpages[port_ngpages - 1] != pg) port_gpages[port_ngpages++] = pg;
	port_gapout = fopen(out, "w");
	unsigned old;
	for (int i = 0; i < port_ngpages; i++) VirtualProtect((void *)port_gpages[i], 0x1000, 0x01, &old);   // PAGE_NOACCESS
	if (port_log) { fprintf(port_log, "gaptrace: %d gaps, %u bytes, %d pages guarded\n", port_ngaps, total, port_ngpages); fflush(port_log); }
}
#ifdef _WIN64
extern "C" void port_fault_log64(void *exceptionPointers, FILE *f);     // port/fault64.cpp
extern "C" void port_thread_log64(void *thread, FILE *f, const char *what);
extern "C" int port_break_log64(void *exceptionPointers, FILE *f);
extern "C" int port_watch_hit64(void *exceptionPointers, FILE *f);
#endif
// Set by gtrace_port.cpp while the recorder reads watched memory (stale pointers of a torn-down level fault there, under SEH): a
// fault inside it is the recorder's, not a crash.
extern "C" volatile int port_guarded_read = 0;
static long __stdcall port_fault(PortExceptionPointers *p)
{
	unsigned long code = p->rec->code;
	if (port_guarded_read) return 0;
#ifdef _WIN64
	// x64: the x86 CONTEXT offsets below do not apply; fatal faults are logged with an unwind-table stack walk (port/fault64.cpp)
	if (code == 0x80000003 && port_log && getenv("PORT_CRTBREAK"))     // the debug CRT's break at allocation #n: log who allocates it
		return port_break_log64(p, port_log);
	if (code == 0x80000004 && port_log && port_watch_hit64(p, port_log))     // PORT_WATCH: a write to the watched address
		return -1;
	if (code == 0xE06D7363 || code == 0x406D1388 || (code & 0xF0000000) != 0xC0000000 || port_in_fault)
		return 0;
	port_in_fault = 1;
	printf("port: FAULT %08lx at %p\n", code, p->rec->addr);
	fflush(stdout);
	if (port_log) { port_fault_log64(p, port_log); port_trace_ring_dump(port_log); }
	ExitProcess(4);
	return 0;
#endif
#ifndef _WIN64   // the rest reads x86 CONTEXT words (EFlags, Dr6, Ebp/Eip/Esp ...) and 32-bit addresses
	if (port_ngpages) {
		unsigned *c = (unsigned *)p->ctx;                 // x86 CONTEXT: EFlags 0xc0
		if (code == 0xC0000005 && p->rec->nparams >= 2 && port_gap_page((unsigned)p->rec->info[1])) {
			unsigned a = (unsigned)p->rec->info[1]; unsigned old;
			port_gap_hit(a, p->rec->info[0] == 1);
			VirtualProtect((void *)(a & ~0xfffu), 0x1000, 0x04, &old);   // PAGE_READWRITE for one instruction
			port_gpending = a & ~0xfffu;
			c[0xc0 / 4] |= 0x100;                         // TF
			return -1;
		}
		if (code == 0x80000004 && port_gpending) {
			unsigned old;
			VirtualProtect((void *)port_gpending, 0x1000, 0x01, &old);
			port_gpending = 0;
			return -1;
		}
	}
	if (code == 0x80000004 && port_ww_addr) {             // EXCEPTION_SINGLE_STEP from DR0: log the writer, continue
		unsigned *c = (unsigned *)p->ctx;                 // x86 CONTEXT: Dr6 0x14, Ebp 0xb4, Eip 0xb8, Esp 0xc4
		if (!(c[0x14 / 4] & 1)) return 0;
		c[0x14 / 4] = 0;
		if (port_log && port_ww_hits++ < 40) {
			unsigned *bp = (unsigned *)c[0xb4 / 4], *sp = (unsigned *)c[0xc4 / 4];
			fprintf(port_log, "watch: write to %p now %08x by eip %08x; frames:", port_ww_addr, *(unsigned *)port_ww_addr, c[0xb8 / 4]);
			for (int i = 0; i < 8 && (unsigned)bp >= (unsigned)sp && (unsigned)bp < (unsigned)sp + 0x100000 && ((unsigned)bp & 3) == 0 && port_readable(bp, 8); i++) { fprintf(port_log, " %08x", bp[1]); bp = (unsigned *)bp[0]; }
			fprintf(port_log, "; stack:");                // raw words too: the game code omits frame pointers
			for (int i = 0; i < 32 && port_readable(sp + i, 4); i++) fprintf(port_log, " %08x", sp[i]);
			fprintf(port_log, "\n"); fflush(port_log);
		}
		return -1;                                        // EXCEPTION_CONTINUE_EXECUTION
	}
	if (code == 0xE06D7363 && port_log && !port_in_fault) {               // a C++ throw (MSVC): the game catches some; log where it came from
		unsigned *c = (unsigned *)p->ctx;
		unsigned *bp = (unsigned *)c[0xb4 / 4], *sp = (unsigned *)c[0xc4 / 4];
		fprintf(port_log, "port: C++ exception thrown (object %08lx type %08lx); frames:", p->rec->nparams > 1 ? p->rec->info[1] : 0, p->rec->nparams > 2 ? p->rec->info[2] : 0);
		for (int i = 0; i < 16 && (unsigned)bp >= (unsigned)sp && (unsigned)bp < (unsigned)sp + 0x100000 && ((unsigned)bp & 3) == 0; i++) {
			unsigned *nbp = (unsigned *)bp[0];
			fprintf(port_log, " %08x", bp[1]);
			if (nbp <= bp) break;
			bp = nbp;
		}
		fprintf(port_log, "\n"); fflush(port_log);
		return 0;
	}
	if (code == 0x406D1388 || (code & 0xF0000000) != 0xC0000000)        // debugger names and non-fatal codes pass through
		return 0;
	if (port_in_fault)                                                   // a fault while logging one: let the frame handlers have it
		return 0;
	port_in_fault = 1;
	printf("port: FAULT %08lx at %p\n", code, p->rec->addr);
	fflush(stdout);
	if (port_log) {
		unsigned *c = (unsigned *)p->ctx;                 // x86 CONTEXT: Edi 0x9c Esi 0xa0 Ebx 0xa4 Edx 0xa8 Ecx 0xac Eax 0xb0 Ebp 0xb4 Eip 0xb8 Esp 0xc4
		unsigned *sp = (unsigned *)c[0xc4 / 4];
		fprintf(port_log, "port: FAULT %08lx at %p (param %08lx %08lx) eax=%08x ecx=%08x edx=%08x ebx=%08x esi=%08x edi=%08x ebp=%08x esp=%08x; stack:",
		        code, p->rec->addr, p->rec->info[0], p->rec->info[1], c[0xb0 / 4], c[0xac / 4], c[0xa8 / 4], c[0xa4 / 4], c[0xa0 / 4], c[0x9c / 4], c[0xb4 / 4], (unsigned)sp);
		for (int i = 0; i < 40; i++) fprintf(port_log, " %08x", sp[i]);
		fprintf(port_log, "\n");
		fprintf(port_log, "port: stack objects (word -> first dword, for readable non-image pointers):");   // vtable of `this` candidates: map them with mapaddr.py
		for (int i = 0; i < 48; i++) { unsigned v = sp[i]; if (v >= 0x01000000 && v < 0x7f000000 && (v & 3) == 0 && port_readable((void *)v, 4)) fprintf(port_log, " %08x->%08x", v, *(unsigned *)v); }
		fprintf(port_log, "\n");
		unsigned *bp = (unsigned *)c[0xb4 / 4];            // EBP chain where frames keep it (the CRT and the shims do): only within the stack
		fprintf(port_log, "port: frames:");
		for (int i = 0; i < 24 && (unsigned)bp >= (unsigned)sp && (unsigned)bp < (unsigned)sp + 0x100000 && ((unsigned)bp & 3) == 0; i++) {
			unsigned *nbp = (unsigned *)bp[0];
			fprintf(port_log, " %08x", bp[1]);
			if (nbp <= bp) break;
			bp = nbp;
		}
		fprintf(port_log, "\n");
		static const char *regNames[] = { "edi", "esi", "ebx", "edx", "ecx", "eax" };   // memory each register points at (object dumps)
		for (int r = 0; r < 6; r++) {
			unsigned *q = (unsigned *)(c[(0x9c + 4 * r) / 4] & ~3u);
			if ((unsigned)q < 0x10000 || !port_readable(q - 4, 64)) continue;
			fprintf(port_log, "port: [%s-10]:", regNames[r]);
			for (int i = -4; i < 12; i++) fprintf(port_log, " %08x", q[i]);
			fprintf(port_log, "\n");
		}
		fflush(port_log);
	}
	if (port_log) port_trace_ring_dump(port_log);
	if (port_trace_path[0]) port_trace_dump(port_trace_path);
	port_dump_registry();
	ExitProcess(4);
	return 0;
#endif
}
// PORT_WATCHDOG=<seconds>: a hang detector. A thread watches the presented-frame counter; when it does not move for that long it
// suspends the game thread, logs its registers and stack in the FAULT format (so `mapaddr.py --log` resolves it) and exits with 5.
extern "C" {
__declspec(dllimport) void *__stdcall CreateThread(void *, unsigned long, unsigned long (__stdcall *)(void *), void *, unsigned long, unsigned long *);
__declspec(dllimport) int __stdcall DuplicateHandle(void *, void *, void *, void **, unsigned long, int, unsigned long);
__declspec(dllimport) void *__stdcall GetCurrentProcess(void);
__declspec(dllimport) void *__stdcall GetCurrentThread(void);
__declspec(dllimport) unsigned long __stdcall SuspendThread(void *);
__declspec(dllimport) int __stdcall GetThreadContext(void *, void *);
__declspec(dllimport) void __stdcall Sleep(unsigned long);
int port_frames_presented(void);
}
static void *port_wd_thread;
static unsigned long port_wd_secs;
static unsigned long __stdcall port_watchdog(void *)
{
	int last = -1; unsigned long still = 0;
	for (;;) {
		Sleep(1000);
		int f = port_frames_presented();
		if (f != last) { last = f; still = 0; continue; }
		if (++still < port_wd_secs) continue;
		SuspendThread(port_wd_thread);
#ifdef _WIN64
		if (port_log) { char what[64]; sprintf(what, "watchdog (no frame for %lu s, frame %d)", port_wd_secs, f); port_thread_log64(port_wd_thread, port_log, what); }
		ExitProcess(5);
#else   // x86 CONTEXT offsets
		unsigned c[0x2d0 / 4]; memset(c, 0, sizeof(c)); c[0] = 0x10007;       // CONTEXT_CONTROL | CONTEXT_INTEGER
		GetThreadContext(port_wd_thread, c);
		unsigned *sp = (unsigned *)c[0xc4 / 4];
		if (port_log) {
			fprintf(port_log, "port: FAULT watchdog (no frame for %lu s, frame %d) at %08x eax=%08x ecx=%08x edx=%08x ebx=%08x esi=%08x edi=%08x ebp=%08x esp=%08x; stack: %08x",
			        port_wd_secs, f, c[0xb8 / 4], c[0xb0 / 4], c[0xac / 4], c[0xa8 / 4], c[0xa4 / 4], c[0xa0 / 4], c[0x9c / 4], c[0xb4 / 4], (unsigned)sp, c[0xb8 / 4]);
			for (int i = 0; i < 400 && port_readable(sp + i, 4); i++) fprintf(port_log, " %08x", sp[i]);
			fprintf(port_log, "\n"); fflush(port_log);
		}
		ExitProcess(5);
#endif
	}
	return 0;
}
// PORT_SAMPLE=<ms>: a sampling profiler. A thread suspends the game thread every <ms>, records EIP (and the module it is in) and every 15 s
// logs the hottest addresses ("sample:" lines; resolve with mapaddr.py --map port/build/port_run_d3d9.map) and per-module totals.
extern "C" {
__declspec(dllimport) unsigned long __stdcall ResumeThread(void *);
__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void *, char *, unsigned long);
__declspec(dllimport) void *__stdcall GetModuleHandleA(const char *);
__declspec(dllimport) int __stdcall SetThreadContext(void *, const void *);
__declspec(dllimport) unsigned long __stdcall WaitForSingleObject(void *, unsigned long);
}
#ifndef _WIN64   // sampler and write watch read/write x86 CONTEXT words (Eip/Esp, DR0/DR7); no x64 equivalent yet
static unsigned long port_smp_ms;
static unsigned long __stdcall port_sampler(void *)
{
	static unsigned adr[65536], cnt[65536]; static unsigned long long total; unsigned long tick = 0;
	struct Mod { void *base; unsigned n; } mods[32]; int nmods = 0;
	for (;;) {
		Sleep(port_smp_ms);
		if (SuspendThread(port_wd_thread) == (unsigned long)-1) continue;
		unsigned c[0x2d0 / 4]; memset(c, 0, sizeof(c)); c[0] = 0x10001;          // CONTEXT_CONTROL (eip, esp)
		unsigned eip = 0;
		if (GetThreadContext(port_wd_thread, c)) eip = c[0xb8 / 4];
		PortMemInfo mbi; void *base = 0;
		if (eip && VirtualQuery((void *)eip, &mbi, sizeof mbi)) base = (void *)mbi.allocBase;       // AllocationBase
		unsigned caller = 0;                                                  // outside the exe (a wait in ntdll, a driver call): the nearest return address into the exe on the stack
		{ static unsigned exeLo, exeHi; if (!exeLo) { exeLo = (unsigned)GetModuleHandleA(0); exeHi = exeLo + *(unsigned *)(exeLo + *(unsigned *)(exeLo + 0x3c) + 0x50); }
		  if (eip && (eip < exeLo || eip >= exeHi)) { unsigned *sp = (unsigned *)c[0xc4 / 4]; for (int i = 0; i < 1024 && port_readable(sp + i, 4); i++) if (sp[i] >= exeLo && sp[i] < exeHi) { caller = sp[i]; break; } } }
		ResumeThread(port_wd_thread);
		if (!eip) continue;
		if (caller) eip = caller | 0x80000000u;                              // bit 31 = "blocked/outside, called from"
		total++;
		unsigned h = (eip * 2654435761u) >> 16;
		while (adr[h] && adr[h] != eip) h = (h + 1) & 0xffff;
		adr[h] = eip; cnt[h]++;
		int m = 0; while (m < nmods && mods[m].base != base) m++;
		if (m == nmods && nmods < 32) { mods[nmods].base = base; mods[nmods].n = 0; nmods++; }
		if (m < nmods) mods[m].n++;
		tick += port_smp_ms;
		if (tick >= 15000 && port_log) {
			tick = 0;
			fprintf(port_log, "sample: %llu samples; modules:", total);
			for (int i = 0; i < nmods; i++) { char name[260] = ""; GetModuleFileNameA(mods[i].base, name, sizeof name); const char *b = strrchr(name, '\\'); fprintf(port_log, " %s=%u", b ? b + 1 : (name[0] ? name : "?"), mods[i].n); }
			fprintf(port_log, "\n");
			for (int k = 0; k < 40; k++) {                          // top 40 addresses
				unsigned best = 0, bi = 0; for (unsigned i = 0; i < 65536; i++) if (cnt[i] > best) { best = cnt[i]; bi = i; }
				if (!best) break; fprintf(port_log, "sample: %08x %u\n", adr[bi], best); cnt[bi] = 0; adr[bi] = 0xffffffffu;      // consumed (the probe chain keeps the slot)
			}
			memset(cnt, 0, sizeof cnt); memset(adr, 0, sizeof adr); total = 0; nmods = 0;
			fflush(port_log);
		}
	}
	return 0;
}
// port_watch_arm(addr): every later write to the 4 bytes at addr by the calling (game) thread logs "watch:" lines (eip + return
// addresses; map them with port/build/port_run_d3d9.map). A helper thread sets DR0/DR7 because a thread cannot set its own.
static void *port_ww_target;
static unsigned long __stdcall port_ww_setter(void *)
{
	SuspendThread(port_ww_target);
	unsigned c[0x2d0 / 4]; memset(c, 0, sizeof(c)); c[0] = 0x10010;      // CONTEXT_DEBUG_REGISTERS
	GetThreadContext(port_ww_target, c);
	c[0] = 0x10010; c[4 / 4] = (unsigned)port_ww_addr; c[0x18 / 4] = 1u | (1u << 16) | (3u << 18);   // DR0, DR7: L0, write, 4 bytes
	SetThreadContext(port_ww_target, c);
	ResumeThread(port_ww_target);
	return 0;
}
#endif
extern "C" void *port_gaddr(unsigned game_addr);      // port/shim/gaddr.cpp
extern "C" int port_gaddr_selftest(void);
#ifdef _WIN64
extern "C" void port_watch_arm(void *addr);      // port/fault64.cpp (x64: DR0 write watchpoint, hits logged with the unwound stack)
#else
extern "C" void port_watch_arm(void *addr)
{
	port_ww_addr = addr;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &port_ww_target, 0, 0, 2);
	void *t = CreateThread(0, 0, port_ww_setter, 0, 0, 0);
	WaitForSingleObject(t, 0xFFFFFFFF);
	if (port_log) { fprintf(port_log, "watch: armed on %p (now %08x)\n", addr, *(unsigned *)addr); fflush(port_log); }
}
#endif
// PORT_WATCH_FRAME: the watchpoint armed from the present hook once that frame is presented (the presenting thread is the game thread)
extern "C" void (*port_present_hook)(void);   // port/shim/rwcore.cpp
static void *port_watch_later_addr;
static int port_watch_later_frame;
static void (*port_watch_prev_hook)(void);
static void port_watch_later(void)
{
	if (port_watch_prev_hook) port_watch_prev_hook();
	if (port_watch_later_addr && port_frames_presented() >= port_watch_later_frame) {
		port_watch_arm(port_watch_later_addr);
		port_watch_later_addr = 0;
	}
}
extern "C" void port_install_fault_handler(void)
{
	AddVectoredExceptionHandler(1, (void *)port_fault);
	port_gaptrace_init();
#ifndef _WIN64
	{ const char *sm = getenv("PORT_SAMPLE");
	  if (sm && atoi(sm) > 0) {
		port_smp_ms = (unsigned long)atoi(sm);
		if (!port_wd_thread) DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &port_wd_thread, 0, 0, 2);
		CreateThread(0, 0, port_sampler, 0, 0, 0);
	} }
#endif
	if (getenv("PORT_GADDR_SELFTEST")) port_gaddr_selftest();   // port/shim/gaddr.cpp: game address -> port address map vs src/game symbols
	{ const char *ww = getenv("PORT_WATCH");                // PORT_WATCH=<hex address>: write watchpoint from the start (diag);
	  uintptr_t wa = ww ? (uintptr_t)_strtoui64(ww, 0, 16) : 0;       // a game address (0x5c0000..0x700000, e.g. 62a2f8) is translated by port_gaddr
	  if (wa >= 0x5c0000 && wa < 0x700000) {
		void *pa = port_gaddr((unsigned)wa);
		if (port_log) { fprintf(port_log, "watch: game address 0x%lx -> port %p\n", (unsigned long)wa, pa); fflush(port_log); }
		wa = (uintptr_t)pa;
	  }
	  const char *wf = getenv("PORT_WATCH_FRAME");          // PORT_WATCH_FRAME=<n>: arm only once frame n is presented (the 40 logged hits then cover
	  if (wa && wf && *wf) {                                // the frames after it, not the start-up)
		port_watch_later_addr = (void *)wa;
		port_watch_later_frame = atoi(wf);
		port_watch_prev_hook = port_present_hook;
		port_present_hook = port_watch_later;
	  } else if (wa) port_watch_arm((void *)wa); }
	const char *w = getenv("PORT_WATCHDOG");
	if (w && atoi(w) > 0 && !port_wd_thread) {
		port_wd_secs = (unsigned long)atoi(w);
		DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &port_wd_thread, 0, 0, 2);   // DUPLICATE_SAME_ACCESS
		CreateThread(0, 0, port_watchdog, 0, 0, 0);
	}
}

#endif   // ZOOSTERS_OS_SDL

// The normal build has no generated stand-ins: unwritten game functions are traps by name (game_traps.cpp) that call
// port_trap_named; same behaviour as _port_trap.
extern "C" int port_trap_continue = 0;
extern "C" void port_trap_named(const char *name)
{
	static const char *seen[256]; static int nseen;
	if (port_trap_continue) {
		for (int i = 0; i < nseen; i++) if (seen[i] == name) return;
		if (nseen < 256) seen[nseen++] = name;
		printf("port: MISSING %s\n", name);
		if (port_log) { fprintf(port_log, "port: MISSING %s\n", name); fflush(port_log); }
		return;
	}
	printf("port: unimplemented external called: %s\n", name);
	fflush(stdout);
	if (port_log) { fprintf(port_log, "port: unimplemented external called: %s\n", name); fflush(port_log); }
	PORT_EXIT(3);
}

// Normal build: port_game_static_init is the generated ordered list src/data/normal/p4b_static_init.cpp.

// Call tracing: the entered functions with call counts, dumped as (address, count) pairs to port_trace.bin next to the exe
// by port_trace_dump (the harness at the end, the fault handler before exiting).
enum { TRACE_SLOTS = 1 << 16 };
static unsigned trace_addr[TRACE_SLOTS], trace_count[TRACE_SLOTS];
static unsigned trace_used;
static unsigned trace_ring[256], trace_ringpos;      // the last 256 entered functions (in order), dumped by the fault handler
extern "C" void __cdecl port_trace_record(unsigned ret)
{
	unsigned entry = ret - 5;                        // the `call _penter` is the function's first instruction
	trace_ring[trace_ringpos++ & 255] = entry;
	unsigned h = (entry * 2654435761u) >> 16;
	for (unsigned i = 0; i < TRACE_SLOTS; i++) {
		unsigned k = (h + i) & (TRACE_SLOTS - 1);
		if (trace_addr[k] == entry) { trace_count[k]++; return; }
		if (trace_addr[k] == 0) { trace_addr[k] = entry; trace_count[k] = 1; trace_used++; return; }
	}
}
extern "C" void port_trace_ring_dump(FILE *f)
{
	if (!trace_ringpos) return;
	fprintf(f, "port: last entered functions (oldest first):");
	for (unsigned i = trace_ringpos > 256 ? trace_ringpos - 256 : 0; i < trace_ringpos; i++) fprintf(f, " %08x", trace_ring[i & 255]);
	fprintf(f, "\n"); fflush(f);
}
extern "C" void port_trace_dump(const char *path)
{
	if (!trace_used) return;
	FILE *f = fopen(path, "wb");
	if (!f) return;
	for (unsigned k = 0; k < TRACE_SLOTS; k++)
		if (trace_addr[k]) { fwrite(&trace_addr[k], 4, 1, f); fwrite(&trace_count[k], 4, 1, f); }
	fclose(f);
	printf("port: trace %u functions -> %s\n", trace_used, path);
	if (port_log) fprintf(port_log, "port: trace %u functions -> %s\n", trace_used, path);
}
extern "C" void port_trace_set_path(const char *path) { strncpy(port_trace_path, path, sizeof(port_trace_path) - 1); }

// Normal build: classes use the compiler's own vtables and no src/game function is patched, so there is nothing to fix up.
extern "C" int port_apply_vtable_fixups(void) { return 0; }

#ifndef PORT_NO_MAIN
float sqrt(float);                                  // src/game's own float overloads (leaf object)
float fabs(float);
int FUN_00461c70();                                 // src/game: matched; its callee chain reaches a stand-in

int main(int argc, char **argv)
{
	printf("port skeleton alive: %u stand-ins linked\n", _port_stub_names[0]);
	printf("smoke: sqrt(2.0f) = %f, fabs(-3.5f) = %f (decomp objects g/obb)\n", sqrt(2.0f), fabs(-3.5f));
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '-' && argv[1][2] == 't') {
		printf("trap demo: calling FUN_00461c70() ...\n");
		fflush(stdout);
		FUN_00461c70();                               // never returns: GetScriptTimePtr -> FUN_0043fa20 is a stand-in
	}
	return 0;
}
#endif
