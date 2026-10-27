// Shared recorder of the "golden traces", used by the stock game (hook DLL) and the port so both write the same TSV. Plain C++
// without the STL (shims compile against the MSVC 7.1 CRT headers). Parses a watch list (grammar v1), waits for its start condition,
// then writes one TSV line per presented frame. Knows no game: the side supplies the address translator, a fault-safe memory read and
// optionally the CRT rand state.
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace gtrace {

// game address (a static Game.exe VA from the watch list) -> the side's address; 0 = unknown
typedef void *(*XlateFn)(unsigned gameAddr);
// copy n bytes from src to dst; 1 = ok, 0 = the read faulted (the side guards it with SEH)
typedef int (*ReadFn)(void *dst, const void *src, int n);
// the CRT rand state for `crt_rand` fields; 1 = *out set, 0 = not available
typedef int (*RandFn)(unsigned *out);

enum Type { kI8, kU8, kI16, kU16, kI32, kU32, kF32, kF32x3, kF32x4, kPtrNz, kCrtRand, kNumTypes };
static const char *const kTypeNames[kNumTypes] = {"i8", "u8", "i16", "u16", "i32", "u32", "f32", "f32x3", "f32x4", "ptrnz", "crt_rand"};
static const int kTypeSize[kNumTypes] = {1, 1, 2, 2, 4, 4, 4, 12, 16, 4, 0};

enum Op { kEq, kNe, kLt, kLe, kGt, kGe };
static const char *const kOpNames[6] = {"==", "!=", "<", "<=", ">", ">="};

static const int kMaxFields = 320;   // scenario watches merge core + physics + actors + move + audio (about 280 columns)
static const int kMaxOffs = 96;  // a list unrolled 16 deep needs 18 (actors.watch); NPC #65 needs 69 (npc65.watch)
static const int kNameLen = 48;

struct Field {
	char name[kNameLen];
	int type;
	unsigned root;
	int noffs;
	int offs[kMaxOffs];
};

struct Recorder {
	XlateFn xlate;
	ReadFn readmem;
	RandFn randFn;
	FILE *out;
	Field fields[kMaxFields];
	int nfields;
	Field start;
	int startOp;
	__int64 startValue;
	int started;
	unsigned frame;   // trace frame (0 = the first frame in which the start condition holds)
	unsigned pframe;  // calls of Frame = the side's presented frames since Open
	char err[256];    // why Open failed

	// ---- watch-list parsing ----
	int Fail(const char *path, int line, const char *msg, const char *tok)
	{
		_snprintf(err, sizeof(err) - 1, "%s:%d: %s%s%s", path, line, msg, tok ? ": " : "", tok ? tok : "");
		err[sizeof(err) - 1] = 0;
		return 0;
	}

	static int ParseHex(const char *s, int *v)
	{
		int neg = 0;
		if (*s == '-') {
			neg = 1;
			s++;
		}
		if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
			s += 2;
		if (!*s)
			return 0;
		char *e;
		unsigned long u = strtoul(s, &e, 16);
		if (*e)
			return 0;
		*v = neg ? -(int)u : (int)u;
		return 1;
	}

	static int ParseRoot(const char *s, unsigned *v)
	{
		if (s[0] != '0' || (s[1] != 'x' && s[1] != 'X') || !s[2])
			return 0;
		char *e;
		*v = (unsigned)strtoul(s + 2, &e, 16);
		return !*e;
	}

	static int ParseValue(const char *s, __int64 *v)
	{
		char *e;
		int neg = *s == '-';
		const char *p = neg ? s + 1 : s;
		if (!*p)
			return 0;
		unsigned long u = (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) ? strtoul(p + 2, &e, 16) : strtoul(p, &e, 10);
		if (*e)
			return 0;
		*v = neg ? -(__int64)u : (__int64)u;
		return 1;
	}

	static int ValidName(const char *s)
	{
		int n = (int)strlen(s);
		if (n == 0 || n >= kNameLen)
			return 0;
		for (; *s; s++)
			if (!((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') || *s == '_' || *s == '.' ||
			      *s == '[' || *s == ']'))
				return 0;
		return 1;
	}

	// split a line into blank-separated tokens (in place); `#` ends the line
	static int Tokens(char *s, char **tok, int max)
	{
		int n = 0;
		for (;;) {
			while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
				s++;
			if (!*s || *s == '#')
				break;
			if (n == max)
				return max + 1;
			tok[n++] = s;
			while (*s && *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n' && *s != '#')
				s++;
			if (*s == '#') {
				*s = 0;
				break;
			}
			if (*s)
				*s++ = 0;
		}
		return n;
	}

	int ParseWatch(const char *path)
	{
		FILE *f = fopen(path, "rb");
		if (!f)
			return Fail(path, 0, "cannot open", 0);
		char line[1024];
		int ln = 0, haveStart = 0, ok = 1;
		nfields = 0;
		while (ok && fgets(line, sizeof(line), f)) {
			ln++;
			char *tok[4 + kMaxOffs + 2];
			int n = Tokens(line, tok, 4 + kMaxOffs + 1);
			if (n == 0)
				continue;
			if (n > 4 + kMaxOffs) {
				ok = Fail(path, ln, "too many tokens", 0);
				break;
			}
			int isStart = strcmp(tok[0], "start") == 0;
			if (!ValidName(tok[0])) {
				ok = Fail(path, ln, "bad name", tok[0]);
				break;
			}
			if (n < 2) {
				ok = Fail(path, ln, "missing type", 0);
				break;
			}
			int type = -1;
			for (int t = 0; t < kNumTypes; t++)
				if (strcmp(tok[1], kTypeNames[t]) == 0)
					type = t;
			if (type < 0) {
				ok = Fail(path, ln, "unknown type", tok[1]);
				break;
			}
			Field fd;
			memset(&fd, 0, sizeof(fd));
			strcpy(fd.name, tok[0]);
			fd.type = type;
			int last = n;  // tokens [3, last) are offsets
			if (isStart) {
				if (haveStart) {
					ok = Fail(path, ln, "second start line", 0);
					break;
				}
				if (type == kF32 || type == kF32x3 || type == kF32x4 || type == kCrtRand) {
					ok = Fail(path, ln, "start needs an integer type or ptrnz", tok[1]);
					break;
				}
				if (n < 5) {
					ok = Fail(path, ln, "start: <type> <root> [offs] <op> <value>", 0);
					break;
				}
				startOp = -1;
				for (int o = 0; o < 6; o++)
					if (strcmp(tok[n - 2], kOpNames[o]) == 0)
						startOp = o;
				if (startOp < 0) {
					ok = Fail(path, ln, "bad operator", tok[n - 2]);
					break;
				}
				if (!ParseValue(tok[n - 1], &startValue)) {
					ok = Fail(path, ln, "bad value", tok[n - 1]);
					break;
				}
				last = n - 2;
			}
			if (type == kCrtRand) {
				if (n != 2) {
					ok = Fail(path, ln, "crt_rand takes no root or offsets", 0);
					break;
				}
			} else {
				if (last < 3 || !ParseRoot(tok[2], &fd.root)) {
					ok = Fail(path, ln, "root must be a 0x hex address", last >= 3 ? tok[2] : 0);
					break;
				}
				if (last - 3 > kMaxOffs) {
					ok = Fail(path, ln, "too many offsets", 0);
					break;
				}
				for (int i = 3; i < last && ok; i++)
					if (!ParseHex(tok[i], &fd.offs[fd.noffs++]))
						ok = Fail(path, ln, "bad offset", tok[i]);
				if (!ok)
					break;
			}
			if (isStart) {
				start = fd;
				haveStart = 1;
				continue;
			}
			for (int i = 0; i < nfields && ok; i++)
				if (strcmp(fields[i].name, fd.name) == 0)
					ok = Fail(path, ln, "duplicate name", fd.name);
			if (!ok)
				break;
			if (nfields == kMaxFields) {
				ok = Fail(path, ln, "more than 128 fields", 0);
				break;
			}
			fields[nfields++] = fd;
		}
		fclose(f);
		if (ok && !haveStart)
			ok = Fail(path, ln, "no start line", 0);
		return ok;
	}

	// ---- reading ----
	// address of a field's value on this side, 0 = unknown root, null link or unreadable pointer
	void *Resolve(const Field &fd)
	{
		char *p = (char *)xlate(fd.root);
		if (!p)
			return 0;
		for (int i = 0; i < fd.noffs; i++) {
			void *next;
			if (!readmem(&next, p, (int)sizeof(next)) || !next)
				return 0;
			p = (char *)next + fd.offs[i];
		}
		return p;
	}

	// integer value of an integer / ptrnz field; 0 = unreadable
	int ReadInt(const Field &fd, __int64 *v)
	{
		unsigned char b[4];
		void *p = Resolve(fd);
		if (!p || !readmem(b, p, kTypeSize[fd.type]))
			return 0;
		switch (fd.type) {
		case kI8: *v = *(signed char *)b; break;
		case kU8: *v = *(unsigned char *)b; break;
		case kI16: *v = *(short *)b; break;
		case kU16: *v = *(unsigned short *)b; break;
		case kI32: *v = *(int *)b; break;
		case kPtrNz: *v = *(unsigned *)b != 0; break;
		default: *v = *(unsigned *)b; break;
		}
		return 1;
	}

	int StartHolds()
	{
		__int64 v;
		if (!ReadInt(start, &v))
			return 0;
		switch (startOp) {
		case kEq: return v == startValue;
		case kNe: return v != startValue;
		case kLt: return v < startValue;
		case kLe: return v <= startValue;
		case kGt: return v > startValue;
		default: return v >= startValue;
		}
	}

	// %.9g with the exponent in one spelling on every CRT: the 7.1 CRT prints e-007 where a modern one prints e-07; drop the extra
	// leading zeros down to two digits, so both sides' TSVs are byte-comparable
	static void FormatFloat(char *buf, int n, float f)
	{
		_snprintf(buf, n - 1, "%.9g", (double)f);
		buf[n - 1] = 0;
		char *e = strchr(buf, 'e');
		if (!e || (e[1] != '+' && e[1] != '-'))
			return;
		char *d = e + 2;
		while (d[0] == '0' && strlen(d) > 2)
			memmove(d, d + 1, strlen(d));
	}

	void WriteCell(const Field &fd)
	{
		if (fd.type == kCrtRand) {
			unsigned r;
			if (randFn && randFn(&r))
				fprintf(out, "\t%u", r);
			else
				fputs("\t-", out);
			return;
		}
		if (fd.type == kF32 || fd.type == kF32x3 || fd.type == kF32x4) {
			int n = kTypeSize[fd.type] / 4;
			unsigned bits[4];
			void *p = Resolve(fd);
			int ok = p && readmem(bits, p, n * 4);
			for (int i = 0; i < n; i++) {
				if (!ok) {
					fputs("\t-", out);
					continue;
				}
				float fl;
				memcpy(&fl, &bits[i], 4);
				char num[48];
				FormatFloat(num, sizeof(num), fl);
				fprintf(out, "\t%08x:%s", bits[i], num);
			}
			return;
		}
		__int64 v;
		if (!ReadInt(fd, &v))
			fputs("\t-", out);
		else if (fd.type == kI8 || fd.type == kI16 || fd.type == kI32)
			fprintf(out, "\t%d", (int)v);
		else
			fprintf(out, "\t%u", (unsigned)v);
	}

	// ---- the side's interface ----
	// Parse the watch list and create the trace file. side: "stock" or "port"; randFn may be 0. 1 = ok, else err says why.
	int Open(const char *watchPath, const char *outPath, const char *side, XlateFn x, ReadFn r, RandFn rf)
	{
		out = 0;
		xlate = x;
		readmem = r;
		randFn = rf;
		started = 0;
		frame = pframe = 0;
		err[0] = 0;
		if (!ParseWatch(watchPath))
			return 0;
		out = fopen(outPath, "wb");
		if (!out)
			return Fail(outPath, 0, "cannot create", 0);
		fprintf(out, "# gtrace 1 side=%s watch=%s\nframe", side, watchPath);
		static const char *const kAxes[4] = {".x", ".y", ".z", ".w"};
		for (int i = 0; i < nfields; i++) {
			int n = fields[i].type == kF32x3 ? 3 : fields[i].type == kF32x4 ? 4 : 0;
			if (!n)
				fprintf(out, "\t%s", fields[i].name);
			for (int a = 0; a < n; a++)
				fprintf(out, "\t%s%s", fields[i].name, kAxes[a]);
		}
		fputs("\n", out);
		fflush(out);
		return 1;
	}

	// Once per presented frame, after the game's logic for that frame has run.
	void Frame()
	{
		if (!out)
			return;
		pframe++;
		if (!started) {
			if (!StartHolds())
				return;
			started = 1;
			fprintf(out, "# start pframe=%u\n", pframe);
		}
		fprintf(out, "%u", frame++);
		for (int i = 0; i < nfields; i++)
			WriteCell(fields[i]);
		fputs("\n", out);
		fflush(out);  // a crash keeps every frame recorded so far
	}

	void Close()
	{
		if (out)
			fclose(out);
		out = 0;
	}
};

}  // namespace gtrace
