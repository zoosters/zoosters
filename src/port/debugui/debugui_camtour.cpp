// Camera tour and cut log (ZOOSTERS_DEBUGUI): shows what the script cameras see, e.g. the sides of a widescreen build never visible at 4:3.
//   PORT_CAMLOG=1                  log every switch of the game's camera object (`use camera` cuts) with a screenshot 2 and 25 frames
//                                  after the cut, then one every 90 frames while the shot lasts (up to 4 per cut)
//   PORT_CAMTOUR=<frame>[:<hold>]  from presented frame <frame> on, show every camera of the level for <hold> frames (default 45); the
//                                  camera is forced as `use camera` does (CameraUse::Execute 0x438f80, CameraUse_Apply 0x41d4c0) and
//                                  re-forced when a script switches away; screenshots 4 frames in and on the last frame; the window
//                                  closes after the last camera
//   PORT_CAMTOUR_HUD=1             keep Im2D (HUD, letterbox); otherwise hidden, with the back buffer cleared to magenta so pixels no
//                                  geometry covers are countable
//   PORT_CAMTOUR_HIDE=world,atomic,im3d,d3d  hide those passes too
//   PORT_CAMTOUR_KEY=0             keep the level's clear colour
//   PORT_CAMTOUR_ONLY=<a>|<b>...   only cameras whose name contains one of them (shot indices stay the full-list index)
//   PORT_CAMOUT=<dir>              where the shots and cameras.tsv go (default PORT_GTRACE_OUT, else the current directory)
// cameras.tsv: kind, index, frame, shot file, camera name, look-from and look-at names (CameraData attributes 1 and 4), field of view,
// camera animation (1 = it has one), cut-scene stack slot 0, times a script took the camera back during the hold. An external tool
// turns it into a report and contact sheets.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debugui_platform.h"
#include "debugui.h"
#include "modules/tfb/cameradata.h"
#include "script/types/scriptcamera.h"
#include "script/ops_camera.h"

int *CutScene_StackSlot(int delta);                // 0x436c00
extern "C" int port_frames_presented(void);
extern "C" int port_capture_backbuffer(const char *path);
extern "C" void port_capture_this_frame(const char *path);   // rwcore.cpp: the frame buffer toImage path (gl3)
extern void *g_mainWnd;                            // GLOBAL: 0x62f138 g_mainWnd
extern "C" int port_dbg_clear_key = 0;                        // rwcore.cpp RwCameraClear: clear the colour to magenta

using RWS::TFB::CameraData;

static CameraData **g_cams;
static int g_ncams, g_capCams;
void port_dbg_camera_data(CameraData *data, int alive)
{
	for (int i = 0; i < g_ncams; i++)
		if (g_cams[i] == data) {
			if (!alive) { memmove(g_cams + i, g_cams + i + 1, (g_ncams - i - 1) * sizeof *g_cams); g_ncams--; }
			return;
		}
	if (!alive) return;
	if (g_ncams == g_capCams) {
		int cap = g_capCams ? g_capCams * 2 : 256;
		CameraData **n = (CameraData **)realloc(g_cams, cap * sizeof *n);
		if (!n) return;
		g_cams = n; g_capCams = cap;
	}
	g_cams[g_ncams++] = data;
}

static const char *outDir(void)
{
	const char *d = getenv("PORT_CAMOUT");
	if (!d) d = getenv("PORT_GTRACE_OUT");
	return d ? d : ".";
}
static FILE *tsv(void)
{
	static FILE *f;
	if (!f) {
		char p[512];
		_snprintf(p, sizeof p, "%s" DEBUGUI_SEP "cameras.tsv", outDir()); p[sizeof p - 1] = 0;
		CreateDirectoryA(outDir(), 0);
		f = fopen(p, "w");
		if (f) fprintf(f, "kind\tindex\tframe\tshot\tcamera\tlook_from\tlook_at\tfov\tanim\tcutscene\toverrides\n");
	}
	return f;
}
static CameraData *dataOf(Script::ScriptReference *obj)
{
	for (int i = 0; i < g_ncams; i++) if ((Script::ScriptReference *)g_cams[i]->DbgCamera() == obj) return g_cams[i];
	return 0;
}
static const char *nameOf(Script::ScriptReference *obj)
{
	const char *n = obj ? port_debugui_object_name(obj) : 0;
	return n ? n : (obj ? "(unnamed)" : "(none)");
}
static void shot(const char *kind, int index, const char *tag, Script::ScriptReference *obj, int overrides)
{
	char file[128], path[640];
	int frame = port_frames_presented();
	_snprintf(file, sizeof file, "%s_%03d%s_f%05d.png", kind, index, tag, frame); file[sizeof file - 1] = 0;
	_snprintf(path, sizeof path, "%s" DEBUGUI_SEP "%s", outDir(), file); path[sizeof path - 1] = 0;
	if (!port_capture_backbuffer(path)) port_capture_this_frame(path);   // written during this frame's present
	FILE *f = tsv();
	if (!f) return;
	CameraData *d = dataOf(obj);
	Script::ScriptCamera *c = d ? d->DbgCamera() : 0;
	fprintf(f, "%s\t%d\t%d\t%s\t%s\t%s\t%s\t%.1f\t%d\t%d\t%d\n", kind, index, frame, file, nameOf(obj),
	        d && d->DbgLookFrom() ? d->DbgLookFrom() : "", d && d->DbgLookAt() ? d->DbgLookAt() : "", c ? c->m_fov : 0.0f,
	        d && d->GetAnimation() ? 1 : 0, *CutScene_StackSlot(0), overrides);
	fflush(f);
}

// ---- cut log
static void cutLog(void)
{
	static Script::ScriptReference *last;
	static int cutFrame = -1, cuts, shots;
	Script::ScriptReference *cur = GetCameraObject();
	int frame = port_frames_presented();
	if (cur != last) {
		last = cur;
		cutFrame = frame;
		cuts++;
		shots = 0;
	}
	if (cutFrame < 0 || shots >= 4) return;
	int age = frame - cutFrame;
	if (age == 2 || age == 25 || (age > 25 && age % 90 == 25)) {
		shot("cut", cuts, age == 2 ? "a" : age == 25 ? "b" : "c", cur, 0);
		shots++;
	}
}

// ---- tour
static void tour(int start, int hold)
{
	static int idx, overrides, done, began;
	static CameraData *cur;
	int frame = port_frames_presented();
	if (done || frame < start) return;
	if (!began) {
		began = 1;
		port_dbg_clear_key = !(getenv("PORT_CAMTOUR_KEY") && getenv("PORT_CAMTOUR_KEY")[0] == '0');   // 0: the level's own clear colour
		if (!(getenv("PORT_CAMTOUR_HUD") && getenv("PORT_CAMTOUR_HUD")[0] == '1')) port_dbg_hide[PORT_DBG_IM2D] = 1;
		const char *h = getenv("PORT_CAMTOUR_HIDE");     // also hide passes: world, atomic, im3d, d3d (comma separated)
		if (h) {
			if (strstr(h, "world")) port_dbg_hide[PORT_DBG_WORLD] = 1;
			if (strstr(h, "atomic")) port_dbg_hide[PORT_DBG_ATOMIC] = 1;
			if (strstr(h, "im3d")) port_dbg_hide[PORT_DBG_IM3D] = 1;
			if (strstr(h, "d3d")) port_dbg_hide[PORT_DBG_D3D] = 1;
		}
	}
	static int *order, norder = -1;
	if (norder < 0) {                              // PORT_CAMTOUR_ONLY=<name>|<name>...: only cameras whose name contains one of them
		order = (int *)malloc(sizeof(int) * (g_ncams + 1));
		norder = 0;
		const char *only = getenv("PORT_CAMTOUR_ONLY");
		for (int i = 0; i < g_ncams && order; i++) {
			int keep = !only;
			if (only) {
				const char *n = nameOf((Script::ScriptReference *)g_cams[i]->DbgCamera());
				char buf[512];
				strncpy(buf, only, sizeof buf - 1); buf[sizeof buf - 1] = 0;
				for (char *tok = strtok(buf, "|"); tok && !keep; tok = strtok(0, "|"))
					keep = strstr(n, tok) != 0;
			}
			if (keep) order[norder++] = i;
		}
	}
	int k = (frame - start) / hold, t = (frame - start) % hold;
	if (k >= norder) {
		done = 1;
		port_dbg_clear_key = 0;
		port_dbg_hide[PORT_DBG_IM2D] = 0;
		if (g_mainWnd) PostMessageA((HWND)g_mainWnd, WM_CLOSE, 0, 0);
		return;
	}
	int ci = order[k];
	Script::ScriptReference *obj = (Script::ScriptReference *)g_cams[ci]->DbgCamera();
	if (t == 0 || cur != g_cams[ci]) {            // the next camera: as `use camera <cam>, pan, 0, no transition`
		cur = g_cams[ci];
		idx = ci;
		overrides = 0;
		TFBUser_Camera()->SetReferent(obj, 0);
		CameraUse_Apply(0, 0.0f, 0, 0.0f);
		return;
	}
	if (GetCameraObject() != obj) {               // a script took the camera back: force it again
		overrides++;
		TFBUser_Camera()->SetReferent(obj, 0);
	}
	if (t == 4) shot("tour", idx, "a", obj, overrides);
	else if (t == hold - 1) shot("tour", idx, "b", obj, overrides);
}

// Called by port_debugui_present before the overlay draws (the frame as the game rendered it)
void port_debugui_camtour_tick(void)
{
	static int init, log, start = -1, hold = 45;
	if (!init) {
		init = 1;
		const char *l = getenv("PORT_CAMLOG");
		log = l && l[0] == '1';
		const char *t = getenv("PORT_CAMTOUR");
		if (t) {
			start = atoi(t);
			const char *c = strchr(t, ':');
			if (c && atoi(c + 1) > 5) hold = atoi(c + 1);
		}
	}
	if (log) cutLog();
	if (start >= 0) tour(start, hold);
}
