// Shared recorder of the "d3dcap" draw-call capture, used by the stock game (src/fixture/d3dcap8.cpp, D3D8) and the port
// (port/shim/d3dcap.cpp, D3D9) so both write the same TSV. Plain C++ without the STL (shims compile against the MSVC 7.1 CRT headers):
// a shadow copy of the device state fed from the side's intercepted setters, and one TSV line per draw. Knows no Direct3D.
#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace d3dcap {

// name of the texture behind a device texture pointer (any non-empty string without blanks), 0 = unknown
typedef const char *(*NameFn)(void *tex, char *buf, int n);

struct RsDef {
	int id;
	const char *name;
	char fmt;  // d decimal, x hex, f float bits
};
// the render states recorded (D3D8 and D3D9 use the same numbers for all of them)
static const RsDef kRs[] = {
    {7, "ZENABLE", 'd'},      {14, "ZWRITE", 'd'},     {23, "ZFUNC", 'd'},       {27, "ABLEND", 'd'},     {19, "SRCBLEND", 'd'},
    {20, "DSTBLEND", 'd'},    {171, "BLENDOP", 'd'},   {15, "ATEST", 'd'},       {24, "AREF", 'd'},       {25, "AFUNC", 'd'},
    {22, "CULL", 'd'},        {28, "FOG", 'd'},        {34, "FOGCOLOR", 'x'},    {137, "LIGHTING", 'd'},  {60, "TFACTOR", 'x'},
    {141, "COLORVERTEX", 'd'}, {145, "DIFFSRC", 'd'},  {146, "SPECSRC", 'd'},    {147, "AMBSRC", 'd'},    {148, "EMISSRC", 'd'},
    {139, "AMBIENT", 'x'},    {29, "SPECULAR", 'd'},   {9, "SHADE", 'd'},        {35, "FOGTABLE", 'd'},   {140, "FOGVERTEX", 'd'},
    {36, "FOGSTART", 'f'},    {37, "FOGEND", 'f'},     {38, "FOGDENSITY", 'f'},  {168, "COLORWRITE", 'd'}, {52, "STENCIL", 'd'},
    {143, "NORMALIZE", 'd'},  {26, "DITHER", 'd'},     {136, "CLIPPING", 'd'},
};
static const int kNumRs = (int)(sizeof(kRs) / sizeof(kRs[0]));

// texture stage states recorded (D3DTSS_*, same numbers in D3D8 and D3D9)
struct TssDef {
	int id;
	const char *name;
};
static const TssDef kTss[] = {{1, "cop"}, {2, "ca1"}, {3, "ca2"}, {4, "aop"}, {5, "aa1"}, {6, "aa2"}, {11, "tci"}, {24, "ttf"}};
static const int kNumTss = (int)(sizeof(kTss) / sizeof(kTss[0]));
static const char *const kSmpNames[5] = {"au", "av", "mag", "min", "mip"};  // normalised sampler states
static const int kStages = 2;                                              // stages 0 and 1 are recorded

struct Recorder {
	NameFn nameFn;
	char *buf;
	size_t len, cap;
	int active;
	int ndraw, nmarker;
	char side[16];
	// shadow
	unsigned rs[256];
	unsigned char rsv[256];
	unsigned tss[4][40];
	unsigned char tssv[4][40];
	unsigned smp[4][5];
	unsigned char smpv[4][5];
	void *tex[4];
	float world[16];
	int worldValid;
	float matD[4], matA[4], matE[4];
	int matValid;
	unsigned lightMask[4];  // 128 lights
	char vfmt[96];
	unsigned stride;
	int vs, ps;
	// name cache
	struct Cached {
		void *p;
		char name[56];
	};
	Cached cache[512];
	int ncache;

	void Out(const char *fmt, ...)
	{
		char tmp[4096];
		va_list ap;
		va_start(ap, fmt);
		int n = _vsnprintf(tmp, sizeof(tmp) - 1, fmt, ap);
		va_end(ap);
		if (n < 0 || n >= (int)sizeof(tmp) - 1)
			n = (int)sizeof(tmp) - 1;
		tmp[n] = 0;
		if (len + (size_t)n + 1 > cap) {
			cap = (cap + (size_t)n + 1) * 2;
			buf = (char *)realloc(buf, cap);
		}
		memcpy(buf + len, tmp, (size_t)n);
		len += (size_t)n;
		buf[len] = 0;
	}

	void ClearShadow()
	{
		memset(rsv, 0, sizeof(rsv));
		memset(tssv, 0, sizeof(tssv));
		memset(smpv, 0, sizeof(smpv));
		memset(tex, 0, sizeof(tex));
		memset(lightMask, 0, sizeof(lightMask));
		worldValid = matValid = 0;
		strcpy(vfmt, "?");
		stride = 0;
		vs = ps = 0;
		ncache = 0;
	}

	// Start a capture: header lines. `frame` is the presented-frame number of the captured frame (informational).
	void Begin(const char *sideName, int frame, NameFn fn)
	{
		nameFn = fn;
		traceSets = 1;
		len = 0;
		if (buf)
			buf[0] = 0;
		strncpy(side, sideName, sizeof(side) - 1);
		side[sizeof(side) - 1] = 0;
		ndraw = nmarker = 0;
		Out("# d3dcap 1\n# side=%s\n# frame=%d\n", side, frame);
		Out("#cols\tkind\tidx\tcall\tprim\tpc\tvc\tvfmt\tstride\tvs\tps\ttex0\ttex1\twt\tnlights\tmatD\tmatA\tmatE");
		for (int i = 0; i < kNumRs; i++)
			Out("\t%s", kRs[i].name);
		for (int s = 0; s < kStages; s++)
			for (int i = 0; i < kNumTss; i++)
				Out("\ts%d.%s", s, kTss[i].name);
		for (int s = 0; s < kStages; s++)
			for (int i = 0; i < 5; i++)
				Out("\ts%d.%s", s, kSmpNames[i]);
		Out("\tcaller\n");
		active = 1;
	}

	// ---- setters (the sides call these from their wrappers, also before Begin to seed the shadow) ----
	void SetRS(unsigned state, unsigned v)
	{
		if (state < 256) {
			if (active && traceSets && (state == 19 || state == 20 || state == 23 || state == 27) && (!rsv[state] || rs[state] != v))
				Marker("SetRS %u=%u", state, v);  // changes of SRCBLEND / DESTBLEND / ZFUNC / ALPHABLENDENABLE (d3dcap.md)
			rs[state] = v;
			rsv[state] = 1;
		}
	}
	void SetTSS8(unsigned stage, unsigned type, unsigned v)  // D3D8: the sampler types are texture stage states
	{
		int k = type == 13 ? 0 : type == 14 ? 1 : type == 16 ? 2 : type == 17 ? 3 : type == 18 ? 4 : -1;
		if (k >= 0)
			SetSampler(stage, k, v);
		else
			SetTSS(stage, type, v);
	}
	void SetTSS(unsigned stage, unsigned type, unsigned v)
	{
		if (stage < 4 && type < 40) {
			tss[stage][type] = v;
			tssv[stage][type] = 1;
		}
	}
	void SetSampler(unsigned stage, int kind, unsigned v)  // kind: 0 addrU 1 addrV 2 mag 3 min 4 mip
	{
		if (stage < 4 && kind >= 0 && kind < 5) {
			smp[stage][kind] = v;
			smpv[stage][kind] = 1;
		}
	}
	void SetSampler9(unsigned stage, unsigned type, unsigned v)  // D3DSAMP_*
	{
		int k = type == 1 ? 0 : type == 2 ? 1 : type == 5 ? 2 : type == 6 ? 3 : type == 7 ? 4 : -1;
		SetSampler(stage, k, v);
	}
	void SetTexture(unsigned stage, void *t)
	{
		if (stage < 4)
			tex[stage] = t;
	}
	void SetWorld(const float *m16)  // D3DMATRIX layout (row vectors, translation in the 4th row)
	{
		memcpy(world, m16, sizeof(world));
		worldValid = 1;
	}
	void SetMaterial(const float *m17)  // D3DMATERIAL: diffuse, ambient, specular, emissive (RGBA floats), power
	{
		memcpy(matD, m17, 16);
		memcpy(matA, m17 + 4, 16);
		memcpy(matE, m17 + 12, 16);
		matValid = 1;
	}
	void SetMatDiffuseOnly(const float *c4)  // a side that only knows the diffuse material colour (shader constant)
	{
		memcpy(matD, c4, 16);
		matValid = 2;
	}
	void LightEnable(unsigned i, int on)
	{
		if (i < 128) {
			if (on)
				lightMask[i >> 5] |= 1u << (i & 31);
			else
				lightMask[i >> 5] &= ~(1u << (i & 31));
		}
	}
	void SetVertexFormat(const char *desc) { strncpy(vfmt, desc, sizeof(vfmt) - 1), vfmt[sizeof(vfmt) - 1] = 0; }
	void SetStride(unsigned s) { stride = s; }
	// The return addresses of the code that issued the next draw that lie in [lo, hi) (comma separated hex, innermost
	// first; informational, the diff tool ignores the column). Only where the side defines D3DCAP_CALLERS (needs
	// RtlCaptureStackBackTrace); the port leaves it "-".
	char caller[96];
	int traceSets;  // 1: markers for changes of the blend / z-function states (set by the side; Marker is declared below)
	void CaptureCaller(unsigned lo, unsigned hi)
	{
#ifdef D3DCAP_CALLERS
		void *fr[24];
		int n = (int)RtlCaptureStackBackTrace(1, 24, fr, 0);
		int len = 0;
		caller[0] = 0;
		for (int i = 0; i < n && len < (int)sizeof(caller) - 12; i++) {
			unsigned a = (unsigned)(size_t)fr[i];
			if (a >= lo && a < hi)
				len += _snprintf(caller + len, sizeof(caller) - len, "%s%X", len ? "," : "", a);
		}
#else
		(void)lo;
		(void)hi;
#endif
	}
	void SetShaders(int v, int p)
	{
		vs = v;
		ps = p;
	}

	// ---- output ----
	static unsigned PackColor(const float *c)
	{
		unsigned a = 0;
		for (int i = 0; i < 4; i++) {
			float f = c[i] < 0 ? 0 : c[i] > 1 ? 1 : c[i];
			a = a << 8 | (unsigned)(f * 255.0f + 0.5f);
		}
		return a;  // c = r,g,b,a -> 0xRRGGBBAA
	}
	static void Val(char *o, int n, int valid, char fmt, unsigned v)
	{
		if (!valid)
			_snprintf(o, n, "?");
		else if (fmt == 'x')
			_snprintf(o, n, "0x%08X", v);
		else if (fmt == 'f') {
			float f;
			memcpy(&f, &v, 4);
			_snprintf(o, n, "%g", f);
		}
		else
			_snprintf(o, n, "%u", v);
		o[n - 1] = 0;
	}
	const char *TexName(void *t, char *o, int n)
	{
		if (!t)
			return "-";
		for (int i = 0; i < ncache; i++)
			if (cache[i].p == t)
				return strcpy(o, cache[i].name);
		char tmp[64] = "";
		const char *nm = nameFn ? nameFn(t, tmp, sizeof(tmp)) : 0;
		if (!nm || !nm[0]) {
			_snprintf(tmp, sizeof(tmp), "ptr%p", t);
			nm = tmp;
		}
		char clean[56];
		int j = 0;
		for (; nm[j] && j < 55; j++)
			clean[j] = (nm[j] == ' ' || nm[j] == '\t' || nm[j] == '\n') ? '_' : nm[j];
		clean[j] = 0;
		if (ncache < 512) {
			cache[ncache].p = t;
			strcpy(cache[ncache].name, clean);
			ncache++;
		}
		_snprintf(o, n, "%s", clean);
		o[n - 1] = 0;
		return o;
	}

	static const char *PrimName(unsigned p) { return p == 1 ? "POINTLIST" : p == 2 ? "LINELIST" : p == 3 ? "LINESTRIP" : p == 4 ? "TRILIST" : p == 5 ? "TRISTRIP" : p == 6 ? "TRIFAN" : "?"; }
	static unsigned PrimVerts(unsigned type, unsigned prims)
	{
		switch (type) {
		case 1: return prims;
		case 2: return prims * 2;
		case 3: return prims + 1;
		case 4: return prims * 3;
		case 5:
		case 6: return prims + 2;
		}
		return 0;
	}

	void Marker(const char *fmt, ...)
	{
		if (!active)
			return;
		char tmp[512];
		va_list ap;
		va_start(ap, fmt);
		int n = _vsnprintf(tmp, sizeof(tmp) - 1, fmt, ap);
		va_end(ap);
		tmp[n < 0 ? (int)sizeof(tmp) - 1 : n] = 0;
		Out("M\t%d\t%s\n", ndraw, tmp);
		nmarker++;
	}

	// One draw. vcount: NumVertices for the indexed calls, the vertex count implied by the primitive count otherwise.
	// strideOverride: the UP calls pass their stride (0 = the stream 0 stride of SetStreamSource).
	void Draw(const char *call, unsigned prim, unsigned pcount, unsigned vcount, unsigned strideOverride)
	{
		if (!active)
			return;
		char t0[64], t1[64], v[40];
		int nl = 0;
		for (int i = 0; i < 4; i++)
			for (unsigned m = lightMask[i]; m; m >>= 1)
				nl += (int)(m & 1);
		Out("D\t%d\t%s\t%s\t%u\t%u\t%s\t%u\t%d\t%d\t%s\t%s", ndraw, call, PrimName(prim), pcount, vcount, vfmt,
		    strideOverride ? strideOverride : stride, vs, ps, TexName(tex[0], t0, sizeof(t0)), TexName(tex[1], t1, sizeof(t1)));
		if (worldValid)
			Out("\t%.3f,%.3f,%.3f", world[12], world[13], world[14]);
		else
			Out("\t?");
		Out("\t%d", nl);
		if (matValid == 2)
			Out("\t0x%08X\t?\t?", PackColor(matD));
		else if (matValid)
			Out("\t0x%08X\t0x%08X\t0x%08X", PackColor(matD), PackColor(matA), PackColor(matE));
		else
			Out("\t?\t?\t?");
		for (int i = 0; i < kNumRs; i++) {
			Val(v, sizeof(v), rsv[kRs[i].id], kRs[i].fmt, rs[kRs[i].id]);
			Out("\t%s", v);
		}
		for (int s = 0; s < kStages; s++)
			for (int i = 0; i < kNumTss; i++) {
				Val(v, sizeof(v), tssv[s][kTss[i].id], 'd', tss[s][kTss[i].id]);
				Out("\t%s", v);
			}
		for (int s = 0; s < kStages; s++)
			for (int i = 0; i < 5; i++) {
				Val(v, sizeof(v), smpv[s][i], 'd', smp[s][i]);
				Out("\t%s", v);
			}
		Out("\t%s\n", caller[0] ? caller : "-");
		caller[0] = 0;
		ndraw++;
	}

	// Finish: write the file. Returns 1 on success.
	int End(const char *path)
	{
		active = 0;
		Out("# draws=%d markers=%d\n", ndraw, nmarker);
		FILE *f = fopen(path, "wb");
		if (!f)
			return 0;
		fwrite(buf, 1, len, f);
		fclose(f);
		return 1;
	}
};

}  // namespace d3dcap
