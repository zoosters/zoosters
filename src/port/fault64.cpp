// x64 fault / hang logging. runtime.cpp's handlers read an x86 CONTEXT by offset; on x64 they call these instead. The stack is
// walked with the x64 unwind tables (RtlLookupFunctionEntry / RtlVirtualUnwind). Output lines ("port: FAULT ...", "port:
// frames64: <rip> ...") are resolved against the link map.
#ifdef _WIN64
#include <windows.h>
#include <stdio.h>

static void port_walk64(CONTEXT c, FILE *f)
{
	fprintf(f, "port: frames64:");
	for (int i = 0; i < 64 && c.Rip; i++) {
		fprintf(f, " %016llx", (unsigned long long)c.Rip);
		DWORD64 base = 0;
		PRUNTIME_FUNCTION fn = RtlLookupFunctionEntry(c.Rip, &base, NULL);
		if (!fn) {                                  // leaf function: the return address is at [rsp]
			if (IsBadReadPtr((void *)c.Rsp, 8)) break;
			c.Rip = *(DWORD64 *)c.Rsp;
			c.Rsp += 8;
			continue;
		}
		void *handlerData = 0;
		DWORD64 establisher = 0;
		RtlVirtualUnwind(UNW_FLAG_NHANDLER, base, c.Rip, fn, &c, &handlerData, &establisher, NULL);
	}
	fprintf(f, "\n");
}

static void port_regs64(const CONTEXT *c, FILE *f)
{
	fprintf(f, " rax=%llx rbx=%llx rcx=%llx rdx=%llx rsi=%llx rdi=%llx rbp=%llx rsp=%llx r8=%llx r9=%llx r10=%llx r11=%llx",
	        c->Rax, c->Rbx, c->Rcx, c->Rdx, c->Rsi, c->Rdi, c->Rbp, c->Rsp, c->R8, c->R9, c->R10, c->R11);
}

extern "C" void port_fault_log64(void *exceptionPointers, FILE *f)
{
	EXCEPTION_POINTERS *p = (EXCEPTION_POINTERS *)exceptionPointers;
	EXCEPTION_RECORD *r = p->ExceptionRecord;
	fprintf(f, "port: FAULT %08lx at %p (param %llx %llx)", r->ExceptionCode, r->ExceptionAddress,
	        r->NumberParameters > 0 ? (unsigned long long)r->ExceptionInformation[0] : 0ull,
	        r->NumberParameters > 1 ? (unsigned long long)r->ExceptionInformation[1] : 0ull);
	port_regs64(p->ContextRecord, f);
	fprintf(f, "\n");
	port_walk64(*p->ContextRecord, f);
	fflush(f);
}

extern "C" FILE *port_log;

// Fast-fail (0xC0000409) paths bypass the exception handlers: abort (also reached by std::terminate), a CRT invalid-parameter
// call and a /GS cookie failure end the process with no report. The first two are hooked here to log the stack first (terminate
// runs during dispatch of the uncaught exception, so the throw's frames are still there).
#include <stdlib.h>
#include <signal.h>
#include <exception>
static void port_log_stack_and_exit(const char *what, unsigned code)
{
	FILE *f = port_log ? port_log : stderr;
	CONTEXT c;
	RtlCaptureContext(&c);
	fprintf(f, "port: %s\n", what);
	port_walk64(c, f);
	fflush(f);
	ExitProcess(code);
}
// the calling thread's stack, unwound (diagnostic logs: a `port: frames64:` line in f)
extern "C" void port_log_stack(FILE *f)
{
	CONTEXT c;
	RtlCaptureContext(&c);
	port_walk64(c, f);
}
static void port_terminate_handler() { port_log_stack_and_exit("std::terminate (an uncaught C++ exception or noexcept violation)", 8); }
static void port_abort_handler(int) { port_log_stack_and_exit("abort()", 9); }
// /GS: __security_check_cookie jumps to __report_gsfailure, which fast-fails with nothing logged. Defined here (the linker takes
// this object's symbol before the CRT's) it logs the stack; the frames above are the function whose epilogue found its cookie
// overwritten.
#ifdef PORT_GS_HOOK   // diagnostic build (PORT_GS_HOOK=1: links with /FORCE:MULTIPLE, the CRT object defines the symbol too)
extern "C" __declspec(noreturn) void __cdecl __report_gsfailure(uintptr_t)
{
	port_log_stack_and_exit("/GS stack cookie check failed (a stack buffer overrun in the first frame below)", 10);
	__assume(0);
}
#endif
static void __cdecl port_invalid_parameter(const wchar_t *expr, const wchar_t *func, const wchar_t *file, unsigned line, uintptr_t)
{
	FILE *f = port_log ? port_log : stderr;
	CONTEXT c;
	RtlCaptureContext(&c);
	fprintf(f, "port: INVALID PARAMETER in %ls (%ls:%u): %ls\n", func ? func : L"?", file ? file : L"?", line, expr ? expr : L"?");
	port_walk64(c, f);
	fflush(f);
	ExitProcess(7);
}
static struct PortInvalidParam {
	PortInvalidParam()
	{
		_set_invalid_parameter_handler(port_invalid_parameter);
		std::set_terminate(port_terminate_handler);
		signal(SIGABRT, port_abort_handler);
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	}
} g_portInvalidParam;

// Debug-CRT build (--debug-heap, _DEBUG): PORT_CRTCHECK=1 checks the whole heap at every allocation and free, so an overrun is
// reported at the next allocator call after it ("HEAP CORRUPTION DETECTED: after Normal block (#n)") with the stack.
// PORT_CRTBREAK=<n> logs the stack of allocation #n (who allocated the damaged block) and continues. Runs before the game's
// static initialisers (init_seg(lib)).
#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>
static void port_walk64(CONTEXT c, FILE *f);
static int __cdecl port_crt_report(int type, char *msg, int *ret)
{
	FILE *f = port_log ? port_log : stderr;
	fprintf(f, "port: CRT report %d: %s", type, msg);
	CONTEXT c;
	RtlCaptureContext(&c);
	port_walk64(c, f);
	fflush(f);
	if (ret) *ret = 0;                              // no debugger break
	return 1;                                       // handled: no message box
}
#pragma warning(disable : 4073)
#pragma init_seg(lib)
static struct PortCrtCheck {
	PortCrtCheck()
	{
		_CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, port_crt_report);
		for (int t = 0; t < _CRT_ERRCNT; t++) _CrtSetReportMode(t, _CRTDBG_MODE_DEBUG);
		const char *e = getenv("PORT_CRTCHECK");
		if (e && e[0] == '1') _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
		const char *b = getenv("PORT_CRTBREAK");
		if (b) _CrtSetBreakAlloc(atol(b));
	}
} g_portCrtCheck;
#endif

// the VEH handler's x64 path (port/runtime.cpp) calls this for a breakpoint (PORT_CRTBREAK, __debugbreak): log it and step over the int3
extern "C" int port_break_log64(void *exceptionPointers, FILE *f)
{
	EXCEPTION_POINTERS *p = (EXCEPTION_POINTERS *)exceptionPointers;
	fprintf(f, "port: breakpoint at %p\n", p->ExceptionRecord->ExceptionAddress);
	port_walk64(*p->ContextRecord, f);
	fflush(f);
	p->ContextRecord->Rip += 1;
	return -1;                                      // EXCEPTION_CONTINUE_EXECUTION
}

// a stack overflow leaves the handler no stack for the walk: reserve 64 KB for it on the main thread (static init runs there)
static struct PortStackGuarantee {
	PortStackGuarantee() { ULONG n = 64 * 1024; SetThreadStackGuarantee(&n); }
} g_portStackGuarantee;

// the watchdog: the game thread is suspended by the caller
extern "C" void port_thread_log64(void *thread, FILE *f, const char *what)
{
	CONTEXT c;
	memset(&c, 0, sizeof c);
	c.ContextFlags = CONTEXT_FULL;
	if (!GetThreadContext((HANDLE)thread, &c)) { fprintf(f, "port: %s: GetThreadContext failed\n", what); return; }
	fprintf(f, "port: FAULT %s at %016llx", what, (unsigned long long)c.Rip);
	port_regs64(&c, f);
	fprintf(f, "\n");
	port_walk64(c, f);
	fflush(f);
}

extern "C" FILE *port_log;
// PORT_WATCH=<hex address> (x64: a port address, e.g. from the map file): a hardware write watchpoint (DR0, 8 bytes) on the calling thread.
// Each write logs "watch: write to ..." with the value and the unwound stack, then the run continues (first 40 hits).
static void *g_watchAddr;
static HANDLE g_watchTarget;
static int g_watchHits;
static DWORD WINAPI port_watch_setter(void *)
{
	SuspendThread(g_watchTarget);
	CONTEXT c;
	memset(&c, 0, sizeof c);
	c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(g_watchTarget, &c);
	c.Dr0 = (DWORD64)g_watchAddr;
	c.Dr7 = 1ull | (1ull << 16) | (2ull << 18);      // L0, write, 8 bytes (len code 2 = 8 on x64)
	SetThreadContext(g_watchTarget, &c);
	ResumeThread(g_watchTarget);
	return 0;
}
extern "C" void port_watch_arm(void *addr)
{
	g_watchAddr = addr;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &g_watchTarget, 0, 0, DUPLICATE_SAME_ACCESS);
	HANDLE t = CreateThread(0, 0, port_watch_setter, 0, 0, 0);
	WaitForSingleObject(t, INFINITE);
	if (port_log) { fprintf(port_log, "watch: armed on %p (now %016llx)\n", addr, *(unsigned long long *)addr); fflush(port_log); }
}
extern "C" int port_watch_hit64(void *exceptionPointers, FILE *f)
{
	EXCEPTION_POINTERS *p = (EXCEPTION_POINTERS *)exceptionPointers;
	if (!g_watchAddr)
		return 0;
	p->ContextRecord->Dr6 = 0;
	if (g_watchHits++ < 40) {
		fprintf(f, "watch: write to %p now %016llx by rip %p\n", g_watchAddr, *(unsigned long long *)g_watchAddr, p->ExceptionRecord->ExceptionAddress);
		port_walk64(*p->ContextRecord, f);
		fflush(f);
	}
	return -1;
}
#endif
