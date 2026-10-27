// mathlog: one text line per call of a RenderWare math function, same format on the stock side (src/fixture/mathlog.cpp,
// [options] MathLog=<file> MathLogFrames=A-B) and the port (port/shim/rwcore.cpp, PORT_MATHLOG=<file> PORT_MATHLOG_FRAMES=A-B):
//   <trace frame> <function> <argument words> | <result words>
// Words are raw 32-bit values (floats as bits), so `diff` finds the first call whose inputs or outputs differ in any bit. Header-only.
#pragma once
#include <stdio.h>
#include <string.h>

namespace mathlog {

struct Window {
	int lo = -1, hi = -1;
	FILE *f = nullptr;
	bool Parse(const char *path, const char *frames)
	{
		if (!path || !*path)
			return false;
		if (!frames || sscanf(frames, "%d-%d", &lo, &hi) != 2) {
			lo = 0;
			hi = 0x7fffffff;
		}
		f = fopen(path, "w");
		return f != nullptr;
	}
	bool On(int tf) const { return f && tf >= lo && tf <= hi; }
};

// in/out: pointers to n words each (a matrix is 16 words: right, flags, up, pad, at, pad, pos, pad; only the 12 floats and flags
// matter, the pads are written as they are)
inline void Line(Window &w, int tf, const char *fn, const void *in, int nin, const void *out, int nout, const void *caller = nullptr)
{
	char buf[1024];
	int n = _snprintf(buf, sizeof buf, "%d %s", tf, fn);
	const unsigned *a = static_cast<const unsigned *>(in);
	for (int i = 0; i < nin && n < (int)sizeof buf - 12; i++)
		n += _snprintf(buf + n, sizeof buf - n, " %08x", a[i]);
	n += _snprintf(buf + n, sizeof buf - n, " |");
	const unsigned *b = static_cast<const unsigned *>(out);
	for (int i = 0; i < nout && n < (int)sizeof buf - 12; i++)
		n += _snprintf(buf + n, sizeof buf - n, " %08x", b[i]);
	if (caller)   // stock: the game return address (the port's code addresses differ; compare lines up to the '#')
		_snprintf(buf + n, sizeof buf - n, " # %p", caller);
	buf[sizeof buf - 1] = 0;
	fprintf(w.f, "%s\n", buf);
	fflush(w.f);   // the stock game is killed at the end of a fixture run
}

// matrix words without the pads (right xyz flags, up xyz, at xyz, pos xyz = 13 words)
inline void Mat(unsigned *dst, const void *m)
{
	const unsigned *s = static_cast<const unsigned *>(m);
	const int idx[13] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14};
	for (int i = 0; i < 13; i++)
		dst[i] = s[idx[i]];
	dst[3] = 0;   // flags: bookkeeping (RW and librw set them differently); a flag that changes a result shows in the floats
}

}  // namespace mathlog
