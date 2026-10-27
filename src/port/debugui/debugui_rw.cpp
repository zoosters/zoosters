// Debug overlay platform side (ZOOSTERS_DEBUGUI): the Dear ImGui context, its renderer on librw's Im2D (any backend; only the
// scissor rectangle is backend specific) and the window input it needs (fed by the port's window procedure, window_w32.cpp /
// window_sdl.cpp). Follows librw's skeleton/imgui/imgui_impl_rw.cpp, with ImGui 1.92 texture requests
// (ImGuiBackendFlags_RendererHasTextures) instead of the legacy font atlas upload. the game's keyboard only while an ImGui text
// field has the focus). Closed, the panels marked "pin" stay on screen without input.
#ifndef ZOOSTERS_DEBUGUI
#error debugui_rw.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include "debugui_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef RW_GL3
#include "gl/glad/glad.h"   // glScissor (librw's loader)
#endif
#ifdef RW_D3D9
#include <d3d9.h>          // before rw.h: its d3d/rwd3d.h then declares d3d::d3ddevice (the scissor rectangle)
#endif
#include <rw.h>
#include "imgui.h"
#include "debugui.h"
extern "C" int port_platform_display_scale(float *scale);   // port/provisional/window_*.cpp

extern "C" FILE *port_log;
extern void *g_mainWnd;   // GLOBAL: 0x62f138 g_mainWnd
#include "../provisional/game_access.h"
#define g_gameCamDst (PortGame_CamDst())   // 0x6182d0: the RwCamera the game renders the 3D view through
extern bool g_gameCamValid;   // GLOBAL: 0x6182d4 g_gameCamValid
extern "C" int port_dbg_hide[PORT_DBG_PASSES] = { 0 };

static bool g_ready, g_open;
static LARGE_INTEGER g_qpf, g_last;
static rw::RWDEVICE::Im2DVertex *g_verts;
static int g_nverts;
static unsigned short g_idx[10000];
// 3D lines queued by the panels this frame (port_debugui_line), drawn with Im3D before the 2D overlay
struct DbgLine { float a[3], b[3]; unsigned rgba; };
static DbgLine *g_lines;
static int g_nlines, g_capLines;
extern "C" int port_debugui_xray = 0;
static float g_mouseScaleX = 1.0f, g_mouseScaleY = 1.0f;   // client pixels -> frame buffer pixels
static char g_iniPath[MAX_PATH];

static void init(void)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.BackendRendererName = "zoosters_librw_im2d";
#ifdef _WIN32
	io.BackendPlatformName = "zoosters_win32";
#else
	io.BackendPlatformName = "zoosters_sdl";
#endif
	io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
	if (port_debugui_file("debugui.ini", g_iniPath, sizeof g_iniPath)) io.IniFilename = g_iniPath;
	if (getenv("PORT_DEBUGUI_SHOW")) io.IniFilename = 0;   // scripted shots: default layout, and the user's file stays as it is
	port_debugui_panels_settings();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().Alpha = 0.95f;
	{   // HiDPI: the overlay draws in frame-buffer pixels; scaled by the display's pixels per point (Retina 2, Windows 150 % = 1.5)
		float scale = 1.0f;
		if (port_platform_display_scale(&scale) && scale > 1.0f) {
			ImGui::GetStyle().ScaleAllSizes(scale);
			io.FontGlobalScale = scale;
		}
	}
	QueryPerformanceFrequency(&g_qpf);
	QueryPerformanceCounter(&g_last);
	const char *op = getenv("PORT_DEBUGUI_OPEN");          // test aid: start with the overlay open (screenshots of scripted runs)
	g_open = op && op[0] == '1';
	for (const char *h = getenv("PORT_DEBUGUI_HIDE"); h && *h; ) {   // test aid: "0,1" = start with those passes hidden (PortDbgPass)
		int k = atoi(h);
		if (k >= 0 && k < PORT_DBG_PASSES) port_dbg_hide[k] = 1;
		h = strchr(h, ','); if (h) h++;
	}
	g_ready = true;
	if (port_log) fprintf(port_log, "debugui: ImGui %s ready (F1 = overlay; settings %s)\n", IMGUI_VERSION, g_iniPath);
}

// ---- textures (the font atlas): RGBA32 pixels -> an rw::Raster through an rw::Image (librw converts to the device format)
static rw::Raster *makeRaster(ImTextureData *tex)
{
	rw::Image *img = rw::Image::create(tex->Width, tex->Height, 32);
	img->allocate();
	for (int y = 0; y < tex->Height; y++)
		memcpy(img->pixels + img->stride * y, (unsigned char *)tex->GetPixels() + tex->GetPitch() * y, tex->Width * 4);
	rw::Raster *r = rw::Raster::createFromImage(img);
	img->destroy();
	return r;
}

static void updateTexture(ImTextureData *tex)
{
	if (tex->Status == ImTextureStatus_WantCreate || tex->Status == ImTextureStatus_WantUpdates) {
		// whole-texture upload for both (the atlas changes rarely: new glyph sizes)
		rw::Raster *old = (rw::Raster *)(uintptr_t)tex->GetTexID();
		rw::Raster *r = tex->Format == ImTextureFormat_RGBA32 ? makeRaster(tex) : 0;
		if (old && tex->Status == ImTextureStatus_WantUpdates) old->destroy();
		tex->SetTexID((ImTextureID)(uintptr_t)r);
		tex->SetStatus(ImTextureStatus_OK);
	} else if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0) {
		rw::Raster *r = (rw::Raster *)(uintptr_t)tex->GetTexID();
		if (r) r->destroy();
		tex->SetTexID(ImTextureID_Invalid);
		tex->SetStatus(ImTextureStatus_Destroyed);
	}
}

static void setClip(const ImVec4 &c, bool on)
{
#ifdef RW_D3D9
	if (on) {
		RECT r = { (LONG)c.x, (LONG)c.y, (LONG)c.z, (LONG)c.w };
		rw::d3d::d3ddevice->SetScissorRect(&r);
	}
	rw::d3d::d3ddevice->SetRenderState(D3DRS_SCISSORTESTENABLE, on ? 1 : 0);
#elif defined(RW_GL3)
	if (on) {                   // ImGui clips in frame-buffer pixels from the top left; GL's scissor counts from the bottom left
		GLint vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		glScissor((GLint)c.x, vp[3] - (GLint)c.w, (GLsizei)(c.z - c.x), (GLsizei)(c.w - c.y));
		glEnable(GL_SCISSOR_TEST);
	} else
		glDisable(GL_SCISSOR_TEST);
#else
	(void)c; (void)on;          // other librw backends: no clipping yet (overflowing text in scrolled regions)
#endif
}

// A file the overlay writes: next to the exe on Windows (debugui.ini, recordings, captures, as before), the user folder elsewhere.
extern "C" int port_user_file(const char *name, char *buf, int size);   // port/provisional/userdir.cpp
const char *port_debugui_file(const char *name, char *out, int size)
{
#ifdef _WIN32
	char exe[MAX_PATH];
	DWORD n = GetModuleFileNameA(0, exe, MAX_PATH);
	char *slash = n ? strrchr(exe, '\\') : 0;
	if (!slash) return 0;
	*slash = 0;
	_snprintf(out, size - 1, "%s\\%s", exe, name);
	out[size - 1] = 0;
	return out;
#else
	return port_user_file(name, out, size) ? out : 0;
#endif
}

static void renderDrawData(ImDrawData *dd, rw::Camera *cam)
{
	if (dd->Textures)
		for (ImTextureData *tex : *dd->Textures)
			if (tex->Status != ImTextureStatus_OK) updateTexture(tex);
	if (dd->TotalVtxCount <= 0 || dd->DisplaySize.x <= 0.0f) return;
	if (g_nverts < dd->TotalVtxCount) {
		delete[] g_verts;
		g_nverts = dd->TotalVtxCount + 5000;
		g_verts = new rw::RWDEVICE::Im2DVertex[g_nverts];
	}
	float nearZ = rw::im2d::GetNearZ(), recipZ = 1.0f / cam->nearPlane;
	rw::RWDEVICE::Im2DVertex *dst = g_verts;
	for (const ImDrawList *list : dd->CmdLists) {
		for (const ImDrawVert &v : list->VtxBuffer) {
			dst->setScreenX(v.pos.x); dst->setScreenY(v.pos.y); dst->setScreenZ(nearZ);
			dst->setCameraZ(cam->nearPlane); dst->setRecipCameraZ(recipZ);
			dst->setColor(v.col & 0xff, v.col >> 8 & 0xff, v.col >> 16 & 0xff, v.col >> 24 & 0xff);
			dst->setU(v.uv.x, recipZ); dst->setV(v.uv.y, recipZ);
			dst++;
		}
	}
	static const rw::int32 saved[] = { rw::VERTEXALPHA, rw::SRCBLEND, rw::DESTBLEND, rw::ZTESTENABLE, rw::ZWRITEENABLE, rw::TEXTUREADDRESSU,
	                                   rw::TEXTUREADDRESSV, rw::TEXTUREFILTER, rw::CULLMODE, rw::FOGENABLE, rw::ALPHATESTFUNC };
	rw::uint32 old[sizeof saved / sizeof saved[0]];
	for (int i = 0; i < (int)(sizeof saved / sizeof saved[0]); i++) old[i] = rw::GetRenderState(saved[i]);
	void *oldRaster = rw::GetRenderStatePtr(rw::TEXTURERASTER);
	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	rw::SetRenderState(rw::ZTESTENABLE, 0);
	rw::SetRenderState(rw::ZWRITEENABLE, 0);
	rw::SetRenderState(rw::CULLMODE, rw::CULLNONE);
	rw::SetRenderState(rw::FOGENABLE, 0);
	rw::SetRenderState(rw::ALPHATESTFUNC, rw::ALPHAALWAYS);
	rw::SetRenderState(rw::TEXTUREADDRESSU, rw::Texture::CLAMP);
	rw::SetRenderState(rw::TEXTUREADDRESSV, rw::Texture::CLAMP);
	rw::SetRenderState(rw::TEXTUREFILTER, rw::Texture::LINEAR);
	int base = 0;
	for (const ImDrawList *list : dd->CmdLists) {
		for (const ImDrawCmd &cmd : list->CmdBuffer) {
			if (cmd.UserCallback) { if (cmd.UserCallback != ImDrawCallback_ResetRenderState) cmd.UserCallback(list, &cmd); continue; }
			if (cmd.ClipRect.z <= cmd.ClipRect.x || cmd.ClipRect.w <= cmd.ClipRect.y) continue;
			rw::SetRenderStatePtr(rw::TEXTURERASTER, (rw::Raster *)(uintptr_t)cmd.GetTexID());
			setClip(cmd.ClipRect, true);
			// librw's Im2D takes at most 10000 vertices and 10000 indices per call (d3dimmed.cpp NUMVERTICES / NUMINDICES; a larger
			// call draws nothing): whole triangles in chunks, each chunk's indices rebased to the lowest vertex it uses
			const ImDrawIdx *idx = list->IdxBuffer.Data + cmd.IdxOffset;
			rw::RWDEVICE::Im2DVertex *verts = g_verts + base + cmd.VtxOffset;
			for (unsigned done = 0; done < cmd.ElemCount; ) {
				unsigned n = cmd.ElemCount - done < 9999 ? cmd.ElemCount - done : 9999;
				unsigned lo = 0xffff, hi = 0;
				for (unsigned i = 0; i < n; i++) { unsigned v = idx[done + i]; if (v < lo) lo = v; if (v > hi) hi = v; }
				if (hi - lo < 10000) {
					for (unsigned i = 0; i < n; i++) g_idx[i] = (unsigned short)(idx[done + i] - lo);
					rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, verts + lo, (rw::int32)(hi - lo + 1), g_idx, (rw::int32)n);
				}
				done += n;
			}
		}
		base += list->VtxBuffer.Size;
	}
	setClip(ImVec4(), false);
	for (int i = 0; i < (int)(sizeof saved / sizeof saved[0]); i++) rw::SetRenderState(saved[i], old[i]);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, oldRaster);
}

extern "C" void port_debugui_line(const float *a, const float *b, unsigned rgba)
{
	if (g_nlines == g_capLines) {
		int cap = g_capLines ? g_capLines * 2 : 4096;
		if (cap > 262144) return;                              // a sane bound: the overlay never needs more
		DbgLine *n = new DbgLine[cap];
		if (g_lines) { memcpy(n, g_lines, sizeof(DbgLine) * g_nlines); delete[] g_lines; }
		g_lines = n; g_capLines = cap;
	}
	DbgLine &l = g_lines[g_nlines++];
	memcpy(l.a, a, sizeof l.a); memcpy(l.b, b, sizeof l.b); l.rgba = rgba;
}

// the queued lines as Im3D line lists in world space (chunks: librw's Im3D buffer holds 10000 vertices); z test on unless x-ray,
// no z writes, untextured, no fog
static void renderLines(void)
{
	if (!g_nlines) return;
	static rw::RWDEVICE::Im3DVertex v[9998];
	static const rw::int32 saved[] = { rw::VERTEXALPHA, rw::SRCBLEND, rw::DESTBLEND, rw::ZTESTENABLE, rw::ZWRITEENABLE, rw::CULLMODE, rw::FOGENABLE };
	rw::uint32 old[sizeof saved / sizeof saved[0]];
	for (int i = 0; i < (int)(sizeof saved / sizeof saved[0]); i++) old[i] = rw::GetRenderState(saved[i]);
	void *oldRaster = rw::GetRenderStatePtr(rw::TEXTURERASTER);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, 0);
	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	rw::SetRenderState(rw::ZTESTENABLE, port_debugui_xray ? 0 : 1);
	rw::SetRenderState(rw::ZWRITEENABLE, 0);
	rw::SetRenderState(rw::CULLMODE, rw::CULLNONE);
	rw::SetRenderState(rw::FOGENABLE, 0);
	for (int first = 0; first < g_nlines; first += 4999) {
		int n = g_nlines - first < 4999 ? g_nlines - first : 4999;
		for (int i = 0; i < n; i++) {
			const DbgLine &l = g_lines[first + i];
			for (int e = 0; e < 2; e++) {
				rw::RWDEVICE::Im3DVertex &d = v[2 * i + e];
				const float *p = e ? l.b : l.a;
				d.setX(p[0]); d.setY(p[1]); d.setZ(p[2]);
				d.setNormalX(0.0f); d.setNormalY(1.0f); d.setNormalZ(0.0f);
				d.setColor(l.rgba >> 24 & 0xff, l.rgba >> 16 & 0xff, l.rgba >> 8 & 0xff, l.rgba & 0xff);
				d.setU(0.0f); d.setV(0.0f);
			}
		}
		rw::im3d::Transform(v, 2 * n, 0, rw::im3d::VERTEXXYZ | rw::im3d::VERTEXRGBA);
		rw::im3d::RenderPrimitive(rw::PRIMTYPELINELIST);
		rw::im3d::End();
	}
	for (int i = 0; i < (int)(sizeof saved / sizeof saved[0]); i++) rw::SetRenderState(saved[i], old[i]);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, oldRaster);
	g_nlines = 0;
}

extern "C" void port_debugui_present(void *camera)
{
	rw::Camera *cam = (rw::Camera *)camera;
	if (!cam || !cam->frameBuffer) return;
	port_debugui_camtour_tick();                          // debugui_camtour.cpp: cut log / camera tour shots (the frame before the overlay)
	if (!g_ready) init();
	ImGuiIO &io = ImGui::GetIO();
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	io.DeltaTime = (float)(now.QuadPart - g_last.QuadPart) / (float)g_qpf.QuadPart;
	if (io.DeltaTime <= 0.0f) io.DeltaTime = 1.0f / 1000.0f;
	g_last = now;
	io.DisplaySize = ImVec2((float)cam->frameBuffer->width, (float)cam->frameBuffer->height);
	RECT rc;
	if (g_mainWnd && GetClientRect((HWND)g_mainWnd, &rc) && rc.right > 0 && rc.bottom > 0) {
		g_mouseScaleX = io.DisplaySize.x / (float)rc.right;
		g_mouseScaleY = io.DisplaySize.y / (float)rc.bottom;
	}
	io.MouseDrawCursor = g_open;
	ImGui::NewFrame();
	port_debugui_panels(g_open);
	ImGui::Render();
	if (g_nlines) {                                       // 3D lines through the game's 3D camera (the presenting camera is not it)
		rw::Camera *view = g_gameCamDst && g_gameCamValid ? (rw::Camera *)g_gameCamDst : cam;
		port_debugui_camera_begin(view);                  // the free view when it is on (the shim's camBegin is not on this path)
		view->beginUpdate();
		renderLines();
		view->endUpdate();
		port_debugui_camera_end(view);
	}
	cam->beginUpdate();                                   // Im2D needs a current camera (as testTriangle in rwcore.cpp)
	renderDrawData(ImGui::GetDrawData(), cam);
	cam->endUpdate();
	port_debugui_camera_restore();                        // free camera: the game camera's own frame back before the next logic step
}

// ---- input: Win32 window messages -> ImGui events
static ImGuiKey vkToKey(WPARAM vk)
{
	if (vk >= '0' && vk <= '9') return (ImGuiKey)(ImGuiKey_0 + (vk - '0'));
	if (vk >= 'A' && vk <= 'Z') return (ImGuiKey)(ImGuiKey_A + (vk - 'A'));
	if (vk >= VK_F1 && vk <= VK_F12) return (ImGuiKey)(ImGuiKey_F1 + (vk - VK_F1));
	switch (vk) {
	case VK_TAB: return ImGuiKey_Tab;
	case VK_LEFT: return ImGuiKey_LeftArrow;
	case VK_RIGHT: return ImGuiKey_RightArrow;
	case VK_UP: return ImGuiKey_UpArrow;
	case VK_DOWN: return ImGuiKey_DownArrow;
	case VK_PRIOR: return ImGuiKey_PageUp;
	case VK_NEXT: return ImGuiKey_PageDown;
	case VK_HOME: return ImGuiKey_Home;
	case VK_END: return ImGuiKey_End;
	case VK_INSERT: return ImGuiKey_Insert;
	case VK_DELETE: return ImGuiKey_Delete;
	case VK_BACK: return ImGuiKey_Backspace;
	case VK_SPACE: return ImGuiKey_Space;
	case VK_RETURN: return ImGuiKey_Enter;
	case VK_ESCAPE: return ImGuiKey_Escape;
	case VK_SHIFT: return ImGuiKey_LeftShift;
	case VK_CONTROL: return ImGuiKey_LeftCtrl;
	case VK_MENU: return ImGuiKey_LeftAlt;
	case VK_OEM_MINUS: return ImGuiKey_Minus;
	case VK_OEM_PERIOD: return ImGuiKey_Period;
	case VK_OEM_COMMA: return ImGuiKey_Comma;
	}
	return ImGuiKey_None;
}

extern "C" int port_debugui_wndproc(void *hwnd, unsigned msg, unsigned long long wp, long long lp, long long *result)
{
	(void)hwnd; (void)result;
	if (!g_ready) return 0;
	ImGuiIO &io = ImGui::GetIO();
	switch (msg) {
	case WM_KEYDOWN: case WM_SYSKEYDOWN:
		if (wp == VK_F1) { if (!(lp & (1 << 30))) g_open = !g_open; return 1; }   // auto-repeat ignored
		// fall through
	case WM_KEYUP: case WM_SYSKEYUP: {
		if (wp == VK_F1) return 1;
		bool down = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
		io.AddKeyEvent(ImGuiMod_Ctrl, (GetKeyState(VK_CONTROL) & 0x8000) != 0);
		io.AddKeyEvent(ImGuiMod_Shift, (GetKeyState(VK_SHIFT) & 0x8000) != 0);
		io.AddKeyEvent(ImGuiMod_Alt, (GetKeyState(VK_MENU) & 0x8000) != 0);
		ImGuiKey k = vkToKey((WPARAM)wp);
		if (k != ImGuiKey_None) io.AddKeyEvent(k, down);
		return g_open && io.WantCaptureKeyboard;
	}
	case WM_CHAR:
		if (wp > 0 && wp < 0x10000) io.AddInputCharacterUTF16((unsigned short)wp);
		return g_open && io.WantCaptureKeyboard;
	case WM_MOUSEMOVE:
		io.AddMousePosEvent((float)(short)LOWORD(lp) * g_mouseScaleX, (float)(short)HIWORD(lp) * g_mouseScaleY);
		return g_open && io.WantCaptureMouse;
	case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_MBUTTONDOWN: case WM_MBUTTONUP: {
		int b = (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) ? 0 : (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) ? 1 : 2;
		bool down = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
		if (!g_open) return 0;                                 // closed: the HUD takes no clicks
		io.AddMouseButtonEvent(b, down);
		return io.WantCaptureMouse;
	}
	case WM_MOUSEWHEEL:
		if (!g_open) return 0;
		io.AddMouseWheelEvent(0.0f, (float)(short)HIWORD(wp) / (float)WHEEL_DELTA);
		return io.WantCaptureMouse;
	case WM_KILLFOCUS:
		io.AddFocusEvent(false);
		return 0;
	case WM_SETFOCUS:
		io.AddFocusEvent(true);
		return 0;
	}
	return 0;
}

void port_debugui_set_open(int open) { g_open = open != 0; }   // Studio mode opens the overlay
extern "C" int port_debugui_blocks_keyboard(void) { return g_ready && ((g_open && ImGui::GetIO().WantCaptureKeyboard) || port_debugui_freecam_keys()); }
extern "C" int port_debugui_blocks_mouse(void) { return g_ready && (g_open || port_debugui_freecam_keys()); }
