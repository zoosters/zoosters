// Call-sequence trace for build-vs-build comparison. Normal builds only, `build option --calltrace`: src/game is compiled with /Gh /GH,
// so every game function calls _penter on entry and _pexit before it returns.
// PORT_CALLTRACE=<file>[,<from>-<to>]  per presented-frame block one `F` line (frame label, calls, FNV hash over (id, depth) of every
//     call in order, max depth, order-insensitive sum, with arguments also the two argument hashes); for frames <from>..<to> every call
//     as `<depth> <id>[ <args>][ =<ret>]`
// PORT_CALLTRACE_ARGS=1  also the first parameters (up to 4 register slots, `this` not recorded) and the return value of every recorded
//     call, normalised by the plan in <exe>.ctsig (calltrace_map.py, comparable across x86 and x64 by `--reconcile`): ints by value,
//     pointers null / non-null, `char const *` by content, floats by bits (hashed rounded to 11 mantissa bits, |x| < 1e-6 as 0).
//     Tags: c I8, h I16, i I32, l I64 (folded), b bool, p pointer (0 / 1), s string hash, f float bits (F64 converted), v RwV3d hash, - skipped
// Ids come from <exe>.ctmap (link map function ranges; id = crc32 of the architecture-neutral demangled name), so x86 and x64 builds
// agree. Only the main thread records. The hook calls no inline CRT function (the linker could pick a hooked COMDAT copy) and has a
// re-entrancy flag.
// Block label = port_frames_presented when the block ends (block 1 = start-up up to the first present). Calls made by the present
// hook chain itself (semtrace etc.) are not recorded.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>
#include <windows.h>

extern "C" int port_frames_presented(void);
extern "C" void (*port_present_hook)(void);

namespace {
struct Range { unsigned start, end, id; };
Range *g_ranges;
unsigned g_nranges;
size_t g_base;
unsigned g_mainTid;
volatile int g_in;
int g_on;
int g_depth, g_maxDepth;
unsigned g_n, g_hash, g_sum, g_unknown;   // g_sum: order-insensitive hash of the ids
int g_label;                     // label of the block being recorded
int g_from = 1 << 30, g_to = -1; // full-sequence window
int g_rec;
FILE *g_f;
void (*g_prev)(void);

enum { NARG = 4 };
struct Ev { int depth; unsigned id; unsigned a[NARG]; unsigned r; };
Ev *g_ev;
unsigned g_nev, g_capev;
enum { EV_MAX = 1 << 24 };

// argument plans (<exe>.ctsig, calltrace_map.py): kind 0 SKIP 1 I8 2 I16 3 I32 4 I64 5 BOOL 6 PTR 7 STR 8 F32 9 F64; src 1 int word,
// 2 xmm (x64), 3 x86 stack word pair, 4 x86 st0, 5 x86 edx:eax
struct Op { unsigned char kind, src; unsigned short word; };
struct Sig { unsigned id; unsigned char flags, nops; unsigned short pad; Op ret; Op op[NARG]; };
static_assert(sizeof(Sig) == 28, "ctsig record");
Sig *g_sigs;
unsigned *g_sigTab, g_sigMask;     // open addressing: id -> index + 1
int g_args;
unsigned g_ahash, g_asum;
enum { OPEN_MAX = 4096 };
int g_open[OPEN_MAX];              // event index of the open call at each depth (-1 = not recorded)

struct Cache { unsigned rva, id; };
Cache g_cache[1 << 16];

inline unsigned curTid()
{
#ifdef _WIN64
	return __readgsdword(0x48);          // TEB.ClientId.UniqueThread (low 32 bits)
#else
	return __readfsdword(0x24);
#endif
}

unsigned lookup(size_t rva)
{
	unsigned r = (unsigned)rva;
	Cache &c = g_cache[(r * 2654435761u) >> 16];
	if (c.rva == r) return c.id;
	unsigned lo = 0, hi = g_nranges, id = 0;
	bool found = false;
	while (lo < hi) {
		unsigned mid = (lo + hi) >> 1;
		if (r < g_ranges[mid].start) hi = mid;
		else if (r >= g_ranges[mid].end) lo = mid + 1;
		else { id = g_ranges[mid].id; found = true; break; }
	}
	if (!found) g_unknown++;
	c.rva = r; c.id = id;
	return id;
}

inline unsigned mix(unsigned h, unsigned v) { return (h ^ v) * 16777619u; }

const Sig *findSig(unsigned id)
{
	if (!g_sigTab) return 0;
	for (unsigned h = (id * 2654435761u) & g_sigMask;; h = (h + 1) & g_sigMask) {
		unsigned v = g_sigTab[h];
		if (!v) return 0;
		if (g_sigs[v - 1].id == id) return &g_sigs[v - 1];
	}
}

inline unsigned fbits(float f) { union { float f; unsigned u; } x; x.f = f; return x.u; }
inline float dtof(unsigned lo, unsigned hi) { union { unsigned u[2]; double d; } x; x.u[0] = lo; x.u[1] = hi; return (float)x.d; }

inline unsigned quant(unsigned u)                 // float bits for the hashes: tiny -> 0, 11 mantissa bits, rounded
{
	unsigned a = u & 0x7fffffffu;
	if (a < 0x358637bdu) return 0;                // |x| < 1e-6 (also -0)
	if (a >= 0x7f800000u) return u;
	return (u + 0x800u) & 0xfffff000u;
}

// Readable-memory check without faulting (a __try does not help: the port's vectored fault logger sees the fault first).
// VirtualQuery results are cached per region.
struct Region { size_t lo, hi; int ok; };
Region g_regions[8];
unsigned g_regNext;

bool readable(size_t p)
{
	for (unsigned i = 0; i < 8; i++)
		if (p >= g_regions[i].lo && p < g_regions[i].hi) return g_regions[i].ok != 0;
	MEMORY_BASIC_INFORMATION mi;
	if (!VirtualQuery((const void *)p, &mi, sizeof mi)) return false;
	bool ok = mi.State == MEM_COMMIT && !(mi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) && (mi.Protect & 0xEE) != 0;
	if (ok) {                                      // only readable regions are cached: a reserved heap range is committed later
		Region &r = g_regions[g_regNext++ & 7];
		r.lo = (size_t)mi.BaseAddress; r.hi = r.lo + mi.RegionSize; r.ok = 1;
	}
	return ok;
}

// content hash of a `char const *` argument; 1 (= only "not null") when it is not a readable printable string: the game passes symbol keys
// (name hashes / addresses) as `char const *` names (ScriptReference::ResolveSymbol, ScriptSubset), whose bytes differ between builds
// strings seen in the recording window: hash -> text, written as `# S <hash> <text>` lines after the frame (calldiff shows the text)
struct StrEnt { unsigned h; char t[48]; };
StrEnt *g_strs;
enum { STR_MAX = 1 << 14 };
unsigned *g_strNew, g_nStrNew;

void noteString(unsigned h, const char *p)
{
	if (!g_strs) {
		g_strs = (StrEnt *)calloc(STR_MAX, sizeof(StrEnt));
		g_strNew = (unsigned *)calloc(STR_MAX, sizeof(unsigned));
		if (!g_strs || !g_strNew) return;
	}
	for (unsigned k = (h * 2654435761u) & (STR_MAX - 1), n = 0; n < 64; k = (k + 1) & (STR_MAX - 1), n++) {
		if (g_strs[k].h == h) return;
		if (!g_strs[k].h) {
			if (g_nStrNew >= STR_MAX / 2) return;     // table half full: stop collecting
			g_strs[k].h = h;
			int i = 0;
			for (; i < 47 && p[i]; i++) g_strs[k].t[i] = (p[i] == '\n' || p[i] == '\r' || p[i] == '\t') ? ' ' : p[i];
			g_strs[k].t[i] = 0;
			g_strNew[g_nStrNew++] = k;
			return;
		}
	}
}

unsigned strHash(const char *p)
{
	unsigned h = 2166136261u;
	size_t a = (size_t)p;
	if (a < 0x10000 || !readable(a)) return 1;
	for (int i = 0; i < 64; i++) {
		if (((a + i) & 0xfff) == 0 && !readable(a + i)) return 1;   // next page
		unsigned char c = (unsigned char)p[i];
		if (!c) { if (!i) return 1; break; }               // "" counts as a non-string too (a key pointing at a zero byte)
		if ((c < 0x20 && c != '\t' && c != '\n' && c != '\r') || c > 0x7e) return 1;
		h = (h ^ c) * 16777619u;
	}
	h |= 2;
	if (g_rec) noteString(h, p);
	return h;
}

// normalised value (floats: float bits) of one op; ints = the stub's register / stack block, fp = the xmm block (x64) or {double st0, int
// valid} (x86 _pexit)
unsigned fetch(const Op &o, const size_t *ints, const unsigned char *fp)
{
	if (!o.kind || !o.src) return 0;
	unsigned long long v = 0;
	switch (o.src) {
	case 1: v = ints[o.word]; break;
#ifdef _WIN64
	case 2: if (!fp) return 0; v = *(const unsigned long long *)(fp + 16 * o.word); break;
#else
	case 3: v = (unsigned)ints[o.word] | ((unsigned long long)(unsigned)ints[o.word + 1] << 32); break;
	case 4: { if (!fp) return 0; const unsigned *x = (const unsigned *)fp; if (!x[2]) return 0xdeadf0; return fbits(dtof(x[0], x[1])); }
	case 5: v = (unsigned)ints[8] | ((unsigned long long)(unsigned)ints[6] << 32); break;
#endif
	default: return 0;
	}
	switch (o.kind) {
	case 1: return (unsigned char)v;
	case 2: return (unsigned short)v;
	case 3: return (unsigned)v;
	case 4: return (unsigned)v ^ (unsigned)(v >> 32) * 2654435761u;
	case 5: return (unsigned char)v != 0;
	case 6: return v != 0;
	case 7: return v ? strHash((const char *)(size_t)v) : 0;
	case 10: {                                     // RwV3d const * / &: the three floats (rounded) hashed
		if (!v) return 0;
		size_t a = (size_t)v;
		if (a < 0x10000 || !readable(a) || !readable(a + 11)) return 1;
		const unsigned *f = (const unsigned *)a;
		return mix(mix(mix(2166136261u, quant(f[0])), quant(f[1])), quant(f[2])) | 2;
	}
	case 8: return (unsigned)v;
	case 9: return fbits(dtof((unsigned)v, (unsigned)(v >> 32)));
	}
	return 0;
}

inline unsigned hval(unsigned char kind, unsigned v) { return kind == 8 || kind == 9 ? quant(v) : v; }
}

extern "C" void ct_enter(void *ret, const size_t *ints, const unsigned char *fp)
{
	if (!g_on || g_in || curTid() != g_mainTid) return;
	g_in = 1;
	unsigned id = lookup((size_t)ret - g_base);
	const Sig *sg = id && g_args ? findSig(id) : 0;
	if (sg && (sg->flags & 2)) {                   // dropped by the noise table (`call` rows): only the depth is kept
		if (g_depth >= 0 && g_depth < OPEN_MAX) g_open[g_depth] = -1;
		if (++g_depth > g_maxDepth) g_maxDepth = g_depth;
		g_in = 0;
		return;
	}
	if (id) {
		g_n++;
		g_hash = mix(mix(g_hash, id), (unsigned)g_depth);
		g_sum += mix(2166136261u, id);
		unsigned a[NARG] = {0, 0, 0, 0};
		if (g_args) {
			unsigned h = mix(2166136261u, id);
			if (sg && !(sg->flags & 1))
				for (int i = 0; i < sg->nops && i < NARG; i++) {
					a[i] = fetch(sg->op[i], ints, fp);
					h = mix(h, hval(sg->op[i].kind, a[i]));
				}
			g_ahash = mix(mix(g_ahash, h), (unsigned)g_depth);
			g_asum += h;
		}
		int ev = -1;
		if (g_rec && g_nev < EV_MAX) {
			if (g_nev == g_capev) { g_capev = g_capev ? g_capev * 2 : 1 << 16; g_ev = (Ev *)realloc(g_ev, g_capev * sizeof(Ev)); }
			Ev &e = g_ev[g_nev];
			e.depth = g_depth; e.id = id; e.r = 0;
			for (int i = 0; i < NARG; i++) e.a[i] = a[i];
			ev = (int)g_nev++;
		}
		if (g_depth >= 0 && g_depth < OPEN_MAX) g_open[g_depth] = ev;
		if (++g_depth > g_maxDepth) g_maxDepth = g_depth;
	}
	g_in = 0;
}

extern "C" void ct_exit(void *ret, const size_t *ints, const unsigned char *fp)
{
	if (!g_on || g_in || curTid() != g_mainTid) return;
	g_in = 1;
	unsigned id = lookup((size_t)ret - g_base);
	if (id && g_depth > 0) {
		g_depth--;
		if (g_args) {
			const Sig *sg = findSig(id);
			if (sg && !(sg->flags & 3) && sg->ret.kind) {
				unsigned r = fetch(sg->ret, ints, fp);
				unsigned h = mix(mix(0x9e3779b9u, id), hval(sg->ret.kind, r));
				g_ahash = mix(g_ahash, h);
				g_asum += h;
				int ev = g_depth < OPEN_MAX ? g_open[g_depth] : -1;
				if (ev >= 0 && (unsigned)ev < g_nev && g_ev[ev].id == id) g_ev[ev].r = r;
			}
		}
	}
	g_in = 0;
}

#if defined(__GNUC__) && !defined(_MSC_VER)
// clang / GCC (build option --cc clang --calltrace): -finstrument-functions calls these after a function's prologue / before its
// epilogue with the function's own address; the call sequence is recorded (no arguments: the registers are not the call's any more).
extern "C" __attribute__((no_instrument_function)) void __cyg_profile_func_enter(void *fn, void *site) { (void)site; ct_enter(fn, 0, 0); }
extern "C" __attribute__((no_instrument_function)) void __cyg_profile_func_exit(void *fn, void *site) { (void)site; ct_exit(fn, 0, 0); }
#elif !defined(_WIN64)
// stub frame (u32 words from the pushfd): 0 eflags, 1 edi, 2 esi, 3 ebp, 4 esp, 5 ebx, 6 edx, 7 ecx, 8 eax, 9 return address into the
// traced function, 10 its return address into the caller, 11.. the caller's stack arguments (_penter is the first instruction of every
// /Gh function on x86). xmm0-7 are saved around the C hook.
extern "C" void __declspec(naked) __cdecl _penter(void)
{
	__asm {
		pushad
		pushfd
		mov ebx, esp
		sub esp, 128
		movdqu [esp], xmm0
		movdqu [esp + 16], xmm1
		movdqu [esp + 32], xmm2
		movdqu [esp + 48], xmm3
		movdqu [esp + 64], xmm4
		movdqu [esp + 80], xmm5
		movdqu [esp + 96], xmm6
		movdqu [esp + 112], xmm7
		push 0
		push ebx
		push dword ptr [ebx + 36]
		call ct_enter
		add esp, 12
		movdqu xmm0, [esp]
		movdqu xmm1, [esp + 16]
		movdqu xmm2, [esp + 32]
		movdqu xmm3, [esp + 48]
		movdqu xmm4, [esp + 64]
		movdqu xmm5, [esp + 80]
		movdqu xmm6, [esp + 96]
		movdqu xmm7, [esp + 112]
		add esp, 128
		popfd
		popad
		ret
	}
}
// _pexit: eax / edx / st0 carry the return value; st0 is copied (fst, no pop) into the fp block {double, int valid} only when the x87
// stack is not empty (fxam: C3 C0 = 1 1, C2 = 0 means empty), so the FPU state of a void / int function is untouched
extern "C" void __declspec(naked) __cdecl _pexit(void)
{
	__asm {
		pushad
		pushfd
		mov ebx, esp
		sub esp, 144
		movdqu [esp + 16], xmm0
		movdqu [esp + 32], xmm1
		movdqu [esp + 48], xmm2
		movdqu [esp + 64], xmm3
		movdqu [esp + 80], xmm4
		movdqu [esp + 96], xmm5
		movdqu [esp + 112], xmm6
		movdqu [esp + 128], xmm7
		mov dword ptr [esp + 8], 0
		fxam
		fnstsw ax
		and ax, 4500h
		cmp ax, 4100h
		je no_st0
		fst qword ptr [esp]
		mov dword ptr [esp + 8], 1
	no_st0:
		mov eax, esp
		push eax
		push ebx
		push dword ptr [ebx + 36]
		call ct_exit
		add esp, 12
		movdqu xmm0, [esp + 16]
		movdqu xmm1, [esp + 32]
		movdqu xmm2, [esp + 48]
		movdqu xmm3, [esp + 64]
		movdqu xmm4, [esp + 80]
		movdqu xmm5, [esp + 96]
		movdqu xmm6, [esp + 112]
		movdqu xmm7, [esp + 128]
		add esp, 144
		popfd
		popad
		ret
	}
}
#endif   // x64: port/provisional/calltrace_x64.asm

static void writeEvent(const Ev &e)
{
	const Sig *sg = g_args ? findSig(e.id) : 0;
	if (!sg || (sg->flags & 1)) { fprintf(g_f, "%d %08x\n", e.depth, e.id); return; }
	static const char tag[] = "-chilbpsffv";
	char buf[128];
	int n = 0;
	for (int k = 0; k < sg->nops && k < NARG; k++)
		n += sprintf(buf + n, "%s%c%x", k ? "," : "", tag[sg->op[k].kind <= 10 ? sg->op[k].kind : 0], e.a[k]);
	if (!n) { buf[0] = '.'; buf[1] = 0; }
	if (sg->ret.kind) fprintf(g_f, "%d %08x %s =%c%x\n", e.depth, e.id, buf, tag[sg->ret.kind <= 10 ? sg->ret.kind : 0], e.r);
	else fprintf(g_f, "%d %08x %s\n", e.depth, e.id, buf);
}

static void frameEnd(int label)
{
	fprintf(g_f, "F %d %u %08x %d %08x %08x %08x\n", label, g_n, g_hash, g_maxDepth, g_sum, g_ahash, g_asum);
	if (g_rec) {
		for (unsigned i = 0; i < g_nev; i++) writeEvent(g_ev[i]);
		for (unsigned i = 0; i < g_nStrNew; i++) fprintf(g_f, "# S %x %s\n", g_strs[g_strNew[i]].h, g_strs[g_strNew[i]].t);
		g_nStrNew = 0;
		if (g_nev >= EV_MAX) fprintf(g_f, "# truncated at %u calls\n", EV_MAX);
	}
	fflush(g_f);
	g_nev = 0; g_n = 0; g_sum = 0; g_hash = 2166136261u; g_asum = 0; g_ahash = 2166136261u; g_maxDepth = g_depth;
	for (int i = 0; i < OPEN_MAX; i++) g_open[i] = -1;
}

static void ctHook(void)
{
	g_in = 1;                                      // nothing below (this chain calls game code) is recorded
	frameEnd(port_frames_presented());
	g_label = port_frames_presented() + 1;
	g_rec = g_label >= g_from && g_label <= g_to;
	if (g_prev) g_prev();
	g_in = 0;
}

static void port_calltrace_exit(void);

static void loadSigs(const char *path)
{
	FILE *sf = fopen(path, "rb");
	unsigned ns = 0;
	if (!sf || fread(&ns, 4, 1, sf) != 1) {
		fprintf(stderr, "calltrace: no argument plans %s (calltrace_map.py)\n", path);
		if (sf) fclose(sf);
		return;
	}
	g_sigs = (Sig *)malloc(ns * sizeof(Sig));
	ns = (unsigned)fread(g_sigs, sizeof(Sig), ns, sf);
	fclose(sf);
	unsigned cap = 1;
	while (cap < ns * 2) cap <<= 1;
	g_sigTab = (unsigned *)calloc(cap, sizeof(unsigned));
	g_sigMask = cap - 1;
	for (unsigned i = 0; i < ns; i++) {
		unsigned h = (g_sigs[i].id * 2654435761u) & g_sigMask;
		while (g_sigTab[h]) h = (h + 1) & g_sigMask;
		g_sigTab[h] = i + 1;
	}
	g_args = 1;
}

extern "C" void port_calltrace_init(void)
{
	const char *e = getenv("PORT_CALLTRACE");
	if (!e || !*e) return;
	char path[MAX_PATH];
	strncpy(path, e, sizeof path - 1); path[sizeof path - 1] = 0;
	char *comma = strrchr(path, ',');
	if (comma) {
		int a, b;
		if (sscanf(comma + 1, "%d-%d", &a, &b) == 2) { g_from = a; g_to = b; *comma = 0; }
	}
	char exe[MAX_PATH];
	GetModuleFileNameA(NULL, exe, sizeof exe);
	char *dot = strrchr(exe, '.');
	if (!dot) return;
	const char *ea = getenv("PORT_CALLTRACE_ARGS");
#if defined(__GNUC__) && !defined(_MSC_VER)
	ea = 0;                                        // clang hooks: no argument capture
#endif
	if (ea && *ea && *ea != '0') {
		strcpy(dot, ".ctsig");
		loadSigs(exe);
	}
	strcpy(dot, ".ctmap");
	FILE *m = fopen(exe, "rb");
	if (!m) { fprintf(stderr, "calltrace: no id map %s (calltrace_map.py)\n", exe); return; }
	unsigned n = 0;
	if (fread(&n, 4, 1, m) != 1) { fclose(m); return; }
	g_ranges = (Range *)malloc(n * sizeof(Range));
	g_nranges = (unsigned)fread(g_ranges, sizeof(Range), n, m);
	fclose(m);
	g_f = fopen(path, "w");
	if (!g_f) return;
	fprintf(g_f, "# calltrace v1 arch=%s window=%d-%d ranges=%u args=%d ctmap=%s\n", sizeof(void *) == 8 ? "x64" : "x86", g_from, g_to, g_nranges, g_args, exe);
	g_base = (size_t)GetModuleHandleA(NULL);
	g_mainTid = curTid();
	g_hash = g_ahash = 2166136261u;
	for (int i = 0; i < OPEN_MAX; i++) g_open[i] = -1;
	g_label = 1;                                   // the first present makes port_frames_presented 1
	g_rec = g_label >= g_from && g_label <= g_to;
	g_prev = port_present_hook;
	port_present_hook = &ctHook;
	g_on = 1;
	atexit(port_calltrace_exit);
}

static void port_calltrace_exit(void)
{
	if (!g_on) return;
	g_on = 0;
	if (g_n) {                                     // the shutdown after the last present: one more block with the fixed label 999999
		fprintf(g_f, "# partial block after the last present (shutdown) of frame %d: label 999999\n", g_label);
		g_in = 1;
		frameEnd(999999);
	}
	fprintf(g_f, "# unknown-range hits %u\n", g_unknown);
	fclose(g_f);
}
