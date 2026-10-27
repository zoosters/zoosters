// Port side of the d3dcap draw-call capture. PORT_D3DCAP=<frame>:<path> records every draw of presented frame <frame> (the draws made
// while g_presented == frame - 1, as PORT_CAPTURE / "s<frame>" shows) at the IDirect3DDevice9 level, below librw's state cache, into
// the TSV of src/common/d3dcap_core.h that the stock game's capture (src/fixture/d3dcap8.cpp) also writes.
// Uses the C interface of d3d9.h (IDirect3DDevice9Vtbl) for the vtable slot numbers, so no rw.h here; the device pointer and texture
// names come through port_d3d_device / port_d3dcap_texname in rwrender.cpp.
#define CINTERFACE
#include <windows.h>
#include <d3d9.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/d3dcap_core.h"

extern "C" FILE *port_log;
extern "C" void *port_d3d_device(void);
extern "C" const char *port_d3dcap_texname(void *d3dtex, char *buf, int n);

namespace {

#define SLOT(name) ((int)(offsetof(IDirect3DDevice9Vtbl, name) / sizeof(void *)))
enum { kMaxSlot = 128 };

d3dcap::Recorder g_rec;
extern "C" int port_d3dcap_ndraw(void) { return g_rec.ndraw; }   // draw index for trace lines
void *g_orig[kMaxSlot];
void **g_vt;
IDirect3DDevice9 *g_dev;
int g_patched;
int g_frame = -2;       // PORT_D3DCAP frame, -1 = off
char g_path[512];
int g_done;

template <class R, class... A> R Orig(int slot, void *dev, A... a)
{
	typedef R(__stdcall * Fn)(void *, A...);
	return reinterpret_cast<Fn>(g_orig[slot])(dev, a...);
}

const char *NameOf(void *t, char *buf, int n)
{
	const char *nm = port_d3dcap_texname(t, buf, n);
	if (nm)
		return nm;
#ifdef _MSC_VER
	__try {
#endif
		IDirect3DTexture9 *tx = (IDirect3DTexture9 *)t;
		D3DSURFACE_DESC d;
		if (SUCCEEDED(tx->lpVtbl->GetLevelDesc(tx, 0, &d))) {
			// as on the stock side: "WxH:fmt<D3DFORMAT>:levels"
			_snprintf(buf, n, "%ux%u:fmt%u:%lu", d.Width, d.Height, (unsigned)d.Format, (unsigned long)tx->lpVtbl->GetLevelCount(tx));
			buf[n - 1] = 0;
			return buf;
		}
#ifdef _MSC_VER
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {     // not a live texture (MSVC builds only, no SEH elsewhere)
	}
#endif
	return 0;
}

// ---- wrappers
HRESULT __stdcall W_SetRenderState(void *d, DWORD s, DWORD v) { g_rec.SetRS(s, v); return Orig<HRESULT>(SLOT(SetRenderState), d, s, v); }
HRESULT __stdcall W_SetTextureStageState(void *d, DWORD st, DWORD ty, DWORD v) { g_rec.SetTSS(st, ty, v); return Orig<HRESULT>(SLOT(SetTextureStageState), d, st, ty, v); }
HRESULT __stdcall W_SetSamplerState(void *d, DWORD st, DWORD ty, DWORD v) { g_rec.SetSampler9(st, ty, v); return Orig<HRESULT>(SLOT(SetSamplerState), d, st, ty, v); }
HRESULT __stdcall W_SetTexture(void *d, DWORD st, void *t) { g_rec.SetTexture(st, t); return Orig<HRESULT>(SLOT(SetTexture), d, st, t); }
HRESULT __stdcall W_SetTransform(void *d, DWORD ty, const float *m)
{
	if (ty == 256 && m)
		g_rec.SetWorld(m);
	return Orig<HRESULT>(SLOT(SetTransform), d, ty, m);
}
HRESULT __stdcall W_SetMaterial(void *d, const float *m)
{
	if (m)
		g_rec.SetMaterial(m);
	return Orig<HRESULT>(SLOT(SetMaterial), d, m);
}
HRESULT __stdcall W_LightEnable(void *d, DWORD i, BOOL on) { g_rec.LightEnable(i, on); return Orig<HRESULT>(SLOT(LightEnable), d, i, on); }
// librw's shader path keeps the world matrix in c4..c7 (VSLOC_world; D3D row layout, translation in the 4th row) and the
// material colour in c12 (VSLOC_matColor): recorded as "wt" and "matD" (matA / matE stay unknown)
HRESULT __stdcall W_SetVertexShaderConstantF(void *d, UINT start, const float *data, UINT count)
{
	if (data && start <= 4 && start + count >= 8)
		g_rec.SetWorld(data + (4 - start) * 4);
	if (data && start <= 12 && start + count >= 13)
		g_rec.SetMatDiffuseOnly(data + (12 - start) * 4);
	return Orig<HRESULT>(SLOT(SetVertexShaderConstantF), d, start, data, count);
}
HRESULT __stdcall W_SetVertexShader(void *d, void *vs)
{
	g_rec.SetShaders(vs != 0, g_rec.ps);
	return Orig<HRESULT>(SLOT(SetVertexShader), d, vs);
}
HRESULT __stdcall W_SetPixelShader(void *d, void *ps)
{
	g_rec.SetShaders(g_rec.vs, ps != 0);
	return Orig<HRESULT>(SLOT(SetPixelShader), d, ps);
}
void DescribeDecl(IDirect3DVertexDeclaration9 *decl, char *out, int n)
{
	_snprintf(out, n, "decl:?");
	if (!decl)
		return;
	D3DVERTEXELEMENT9 el[MAXD3DDECLLENGTH + 1];
	UINT cnt = 0;
	if (FAILED(decl->lpVtbl->GetDeclaration(decl, el, &cnt)))
		return;
	static const char kUse[] = "PWINSTABFRCFDS";  // POSITION BLENDWEIGHT BLENDINDICES NORMAL PSIZE TEXCOORD TANGENT BINORMAL TESSFACTOR POSITIONT COLOR FOG DEPTH SAMPLE
	int len = _snprintf(out, n, "decl:");
	for (UINT i = 0; i + 1 < cnt && len > 0 && len < n - 6; i++) {
		unsigned u = el[i].Usage;
		len += _snprintf(out + len, n - len, "%c%u", u < sizeof(kUse) - 1 ? kUse[u] : '?', (unsigned)el[i].UsageIndex);
	}
	out[n - 1] = 0;
}
HRESULT __stdcall W_SetVertexDeclaration(void *d, IDirect3DVertexDeclaration9 *decl)
{
	char s[96];
	if (decl)
		DescribeDecl(decl, s, sizeof(s));
	else
		strcpy(s, "none");
	g_rec.SetVertexFormat(s);
	return Orig<HRESULT>(SLOT(SetVertexDeclaration), d, decl);
}
HRESULT __stdcall W_SetFVF(void *d, DWORD fvf)
{
	char s[40];
	_snprintf(s, sizeof(s), "fvf:0x%X", (unsigned)fvf);
	g_rec.SetVertexFormat(s);
	return Orig<HRESULT>(SLOT(SetFVF), d, fvf);
}
HRESULT __stdcall W_SetStreamSource(void *d, UINT stream, void *vb, UINT off, UINT stride)
{
	if (stream == 0)
		g_rec.SetStride(stride);
	return Orig<HRESULT>(SLOT(SetStreamSource), d, stream, vb, off, stride);
}
HRESULT __stdcall W_SetRenderTarget(void *d, DWORD idx, void *rt)
{
	g_rec.Marker("SetRenderTarget idx=%lu rt=%p", (unsigned long)idx, rt);
	return Orig<HRESULT>(SLOT(SetRenderTarget), d, idx, rt);
}
HRESULT __stdcall W_SetDepthStencilSurface(void *d, void *zs)
{
	g_rec.Marker("SetDepthStencilSurface zs=%p", zs);
	return Orig<HRESULT>(SLOT(SetDepthStencilSurface), d, zs);
}
HRESULT __stdcall W_SetViewport(void *d, const DWORD *vp)
{
	if (vp)
		g_rec.Marker("SetViewport x=%lu y=%lu w=%lu h=%lu", (unsigned long)vp[0], (unsigned long)vp[1], (unsigned long)vp[2], (unsigned long)vp[3]);
	return Orig<HRESULT>(SLOT(SetViewport), d, vp);
}
HRESULT __stdcall W_Clear(void *d, DWORD count, const void *rects, DWORD flags, DWORD color, float z, DWORD stencil)
{
	g_rec.Marker("Clear count=%lu flags=0x%lX color=0x%08lX z=%g stencil=%lu", (unsigned long)count, (unsigned long)flags, (unsigned long)color, z, (unsigned long)stencil);
	return Orig<HRESULT>(SLOT(Clear), d, count, rects, flags, color, z, stencil);
}
HRESULT __stdcall W_BeginScene(void *d) { g_rec.Marker("BeginScene"); return Orig<HRESULT>(SLOT(BeginScene), d); }
HRESULT __stdcall W_EndScene(void *d) { g_rec.Marker("EndScene"); return Orig<HRESULT>(SLOT(EndScene), d); }
HRESULT __stdcall W_DrawPrimitive(void *d, DWORD type, UINT start, UINT pc)
{
	g_rec.Draw("DrawPrimitive", type, pc, d3dcap::Recorder::PrimVerts(type, pc), 0);
	return Orig<HRESULT>(SLOT(DrawPrimitive), d, type, start, pc);
}
HRESULT __stdcall W_DrawIndexedPrimitive(void *d, DWORD type, INT base, UINT minV, UINT numV, UINT startIdx, UINT pc)
{
	g_rec.Draw("DrawIndexedPrimitive", type, pc, numV, 0);
	return Orig<HRESULT>(SLOT(DrawIndexedPrimitive), d, type, base, minV, numV, startIdx, pc);
}
HRESULT __stdcall W_DrawPrimitiveUP(void *d, DWORD type, UINT pc, const void *data, UINT stride)
{
	g_rec.Draw("DrawPrimitiveUP", type, pc, d3dcap::Recorder::PrimVerts(type, pc), stride);
	return Orig<HRESULT>(SLOT(DrawPrimitiveUP), d, type, pc, data, stride);
}
HRESULT __stdcall W_DrawIndexedPrimitiveUP(void *d, DWORD type, UINT minV, UINT numV, UINT pc, const void *idx, DWORD fmt, const void *vdata, UINT stride)
{
	g_rec.Draw("DrawIndexedPrimitiveUP", type, pc, numV, stride);
	return Orig<HRESULT>(SLOT(DrawIndexedPrimitiveUP), d, type, minV, numV, pc, idx, fmt, vdata, stride);
}

struct PatchDef {
	int slot;
	void *fn;
};
#define P(name, fn) {SLOT(name), (void *)&fn}
const PatchDef kPatches[] = {
    P(SetRenderState, W_SetRenderState),
    P(SetTextureStageState, W_SetTextureStageState),
    P(SetSamplerState, W_SetSamplerState),
    P(SetTexture, W_SetTexture),
    P(SetTransform, W_SetTransform),
    P(SetMaterial, W_SetMaterial),
    P(LightEnable, W_LightEnable),
    P(SetVertexShaderConstantF, W_SetVertexShaderConstantF),
    P(SetVertexShader, W_SetVertexShader),
    P(SetPixelShader, W_SetPixelShader),
    P(SetVertexDeclaration, W_SetVertexDeclaration),
    P(SetFVF, W_SetFVF),
    P(SetStreamSource, W_SetStreamSource),
    P(SetRenderTarget, W_SetRenderTarget),
    P(SetDepthStencilSurface, W_SetDepthStencilSurface),
    P(SetViewport, W_SetViewport),
    P(Clear, W_Clear),
    P(BeginScene, W_BeginScene),
    P(EndScene, W_EndScene),
    P(DrawPrimitive, W_DrawPrimitive),
    P(DrawIndexedPrimitive, W_DrawIndexedPrimitive),
    P(DrawPrimitiveUP, W_DrawPrimitiveUP),
    P(DrawIndexedPrimitiveUP, W_DrawIndexedPrimitiveUP),
};
#undef P

void SetSlot(int slot, void *fn)
{
	DWORD old;
	VirtualProtect(&g_vt[slot], sizeof(void *), PAGE_READWRITE, &old);
	g_vt[slot] = fn;
	VirtualProtect(&g_vt[slot], sizeof(void *), old, &old);
}
void Patch()
{
	g_vt = (void **)g_dev->lpVtbl;
	for (size_t i = 0; i < sizeof(kPatches) / sizeof(kPatches[0]); i++) {
		g_orig[kPatches[i].slot] = g_vt[kPatches[i].slot];
		SetSlot(kPatches[i].slot, kPatches[i].fn);
	}
	g_patched = 1;
}
void Unpatch()
{
	if (!g_patched)
		return;
	for (size_t i = 0; i < sizeof(kPatches) / sizeof(kPatches[0]); i++)
		SetSlot(kPatches[i].slot, g_orig[kPatches[i].slot]);
	g_patched = 0;
}

void Seed()
{
	IDirect3DDevice9 *d = g_dev;
	g_rec.ClearShadow();
	for (int i = 0; i < d3dcap::kNumRs; i++) {
		DWORD v = 0;
		if (SUCCEEDED(d->lpVtbl->GetRenderState(d, (D3DRENDERSTATETYPE)d3dcap::kRs[i].id, &v)))
			g_rec.SetRS(d3dcap::kRs[i].id, v);
	}
	for (DWORD s = 0; s < 2; s++) {
		for (int i = 0; i < d3dcap::kNumTss; i++) {
			DWORD v = 0;
			if (SUCCEEDED(d->lpVtbl->GetTextureStageState(d, s, (D3DTEXTURESTAGESTATETYPE)d3dcap::kTss[i].id, &v)))
				g_rec.SetTSS(s, d3dcap::kTss[i].id, v);
		}
		static const unsigned types[5] = {1, 2, 5, 6, 7};
		for (int k = 0; k < 5; k++) {
			DWORD v = 0;
			if (SUCCEEDED(d->lpVtbl->GetSamplerState(d, s, (D3DSAMPLERSTATETYPE)types[k], &v)))
				g_rec.SetSampler9(s, types[k], v);
		}
		IDirect3DBaseTexture9 *t = 0;
		if (SUCCEEDED(d->lpVtbl->GetTexture(d, s, &t)) && t) {
			g_rec.SetTexture(s, t);
			t->lpVtbl->Release(t);
		}
	}
	D3DMATRIX m;
	if (SUCCEEDED(d->lpVtbl->GetTransform(d, (D3DTRANSFORMSTATETYPE)256, &m)))
		g_rec.SetWorld((const float *)&m);
	D3DMATERIAL9 mat;
	if (SUCCEEDED(d->lpVtbl->GetMaterial(d, &mat))) {
		float f[17];
		memcpy(f, &mat.Diffuse, 16);
		memcpy(f + 4, &mat.Ambient, 16);
		memcpy(f + 8, &mat.Specular, 16);
		memcpy(f + 12, &mat.Emissive, 16);
		f[16] = mat.Power;
		g_rec.SetMaterial(f);
	}
	for (DWORD i = 0; i < 8; i++) {
		BOOL on = 0;
		if (SUCCEEDED(d->lpVtbl->GetLightEnable(d, i, &on)))
			g_rec.LightEnable(i, on);
	}
	IDirect3DVertexShader9 *vs = 0;
	IDirect3DPixelShader9 *ps = 0;
	if (SUCCEEDED(d->lpVtbl->GetVertexShader(d, &vs))) {
		if (vs)
			vs->lpVtbl->Release(vs);
		g_rec.SetShaders(vs != 0, 0);
	}
	if (SUCCEEDED(d->lpVtbl->GetPixelShader(d, &ps))) {
		if (ps)
			ps->lpVtbl->Release(ps);
		g_rec.SetShaders(g_rec.vs, ps != 0);
	}
	IDirect3DVertexDeclaration9 *decl = 0;
	DWORD fvf = 0;
	char s[96] = "?";
	d->lpVtbl->GetFVF(d, &fvf);
	if (SUCCEEDED(d->lpVtbl->GetVertexDeclaration(d, &decl)) && decl) {
		DescribeDecl(decl, s, sizeof(s));
		decl->lpVtbl->Release(decl);
	}
	else if (fvf)
		_snprintf(s, sizeof(s), "fvf:0x%X", (unsigned)fvf);
	g_rec.SetVertexFormat(s);
	IDirect3DVertexBuffer9 *vb = 0;
	UINT off = 0, stride = 0;
	if (SUCCEEDED(d->lpVtbl->GetStreamSource(d, 0, &vb, &off, &stride))) {
		g_rec.SetStride(stride);
		if (vb)
			vb->lpVtbl->Release(vb);
	}
}

}  // namespace

// PORT_D3DCAP=<frame>:<path>. Read during static initialisation; the game changes directory before the first frame, so a relative path
// resolves against the start directory.
static void ReadCapEnv()
{
	const char *e = getenv("PORT_D3DCAP");
	g_frame = -1;
	if (e) {
		const char *c = strchr(e, ':');
		if (c) {
			g_frame = atoi(e);
			if (!GetFullPathNameA(c + 1, sizeof(g_path), g_path, NULL))
				strncpy(g_path, c + 1, sizeof(g_path) - 1);
		}
	}
}
static struct CapEnvAtStart { CapEnvAtStart() { ReadCapEnv(); } } g_capEnvAtStart;

// Called by RwCameraShowRaster after the present; `presented` = frames presented so far.
extern "C" void port_d3dcap_after_present(int presented)
{
	if (g_frame == -2)
		ReadCapEnv();
	if (g_frame < 0 || g_done)
		return;
	if (presented == g_frame - 1) {
		g_dev = (IDirect3DDevice9 *)port_d3d_device();
		if (!g_dev)
			return;
		Seed();
		g_rec.Begin("port", g_frame, NameOf);
		Patch();
	}
	else if (presented == g_frame && g_patched) {
		Unpatch();
		int ok = g_rec.End(g_path);
		g_done = 1;
		if (port_log) {
			fprintf(port_log, "d3dcap: frame %d: %d draws, %d markers -> %s (%s)\n", g_frame, g_rec.ndraw, g_rec.nmarker, g_path, ok ? "ok" : "write failed");
			fflush(port_log);
		}
	}
}
