// Stand-ins for game functions on the headless start-up path that are not implemented yet.
#include <stdio.h>
#include "../debugui/debugui.h"   // port_debugui_blocks_keyboard (ZOOSTERS_DEBUGUI builds)
#include "rw/rpcollis.h"
#include <stdlib.h>
#include <string.h>

extern "C" FILE *port_log;

// 0x519d60 GetUserDataPath: port/provisional/userdir.cpp (per-platform user folder)

// 0x5ca250 g_pfnSprintf is not data but Game.exe's import slot USER32!wsprintfW. The save code calls through it, so the slot
// points at a wsprintfW equivalent.
#ifdef _WIN32   // only the address-bound ports call through the slot (wchar_t is 2 bytes there; nothing in a normal build reads it)
#include <stdarg.h>
#include <wchar.h>
static int __cdecl port_wsprintfW(char *buf, const char *fmt, ...)
{
	va_list ap; va_start(ap, fmt);
	int n = vswprintf((wchar_t *)buf, (const wchar_t *)fmt, ap);
	va_end(ap);
	return n;
}
int (__cdecl *g_pfnSprintf)(char *buf, const char *fmt, ...) = port_wsprintfW;
#endif

// 0x51bf30 InputManager DirectInput set-up. Port: no DirectInput devices; the keyboard is served by the provisional Poll below.
// Normal build: the real class, whose members are pointer-sized, so the 32-bit offsets below do not hold.
#include "framework/startup/win32/input/inputmanager.h"
#include "framework/startup/win32/input/ctlbuf.h"

// Keyboard sources (port_keyboard_frame below): the real keyboard while the window has the focus, PORT_KEYS="<frame>:<dik>[,...]"
// (each key held for 6 presented frames; e.g. 0x1c = Return, 0x39 = Space) and PORT_INPUT. The normal build reads them through
// dinput_port.cpp from the game's own Poll; the address-bound port keeps the stand-in Poll below.
// InputManager::Poll 0x51b450, address-bound stand-in (mouse at +8 and the joysticks are not read): previous keys (+0x26e4) =
// current (+0x25e4), current = the keyboard as DirectInput codes (0x80 = down); while the "keys held at a context switch" flag (+1)
// is set, keys equal to the mask (+0x24e4, filled by FUN_00403f70) are suppressed and released ones leave the mask; then every
// controller context (+0x129c8 count, +0x129d0 table) is evaluated (UpdateBindings(ctx + 0x18, i)).
extern "C" int port_frames_presented(void);
extern "C" void port_mouse_update(void *mouseState);   // port/provisional/mouse.cpp (a DIMOUSESTATE2: lX, lY, lZ, rgbButtons)
#include "platform.h"            // port_platform_keyboard: the window backend
#include "modules/tfb/gamecamera.h"   // g_camPos / g_camTgt / g_camPosCur / g_camTgtCur, g_camMode, g_cameraObject, g_gameCamDst, g_floatZero
#include "tfb/level/level.h"          // RWS::CLevel::sm_pActiveWorld
#include "tfb/audio/audioenv.h"        // SetEnvironment (the `env` input line)
#include "game_access.h"             // PortGame_ClassMap (the class map's type is local to the game's cclassfactory.cpp)
extern RwV3d g_camLookAt;            // GLOBAL: 0x61ff58 g_camLookAt (declared in the game's .cpp files only)
extern const float g_negEpsilon;     // GLOBAL: 0x5d4d0c g_negEpsilon (script/comparefloat.cpp)
static void *port_rayCb(void *is, void *sec, void *tri, float dist, void *data)
{
	(void)is; (void)data; const float *n = (const float *)tri; const char *t = (const char *)tri;
	fprintf(port_log, "camray: sector %p tri index %d dist %.4f normal %.3f %.3f %.3f flags %02x" "\n", sec, *(int *)(t + 0x18), dist, n[0], n[1], n[2], ((const unsigned char *)(*(void **)((char *)sec + 4)))[*(int *)(t + 0x18) * 8 + 6]);
	return tri;
}
extern int g_nextLevel;   // GLOBAL: 0x62a2f8 g_nextLevel
extern void *g_mainWnd;   // GLOBAL: 0x62f138 g_mainWnd
// PORT_INPUT=<file>: input script (format shared with src/fixture/gtrace.cpp); `#` starts a comment. <frame> is the absolute decimal
// presented-frame counter (port_frames_presented).
//   <frame> <key> [<held>]  key down in the Poll calls while the counter is in [frame, frame + held), held default 1. <key>: a name
//                           (case-insensitive: every letter, digits 0..9, SPACE RETURN ENTER ESCAPE ESC TAB UP DOWN LEFT RIGHT
//                           LSHIFT LCTRL PGUP PGDN) or a DIK code in hex ("1c", "0x1c"); names win, so "A" is the A key and the
//                           code 0x0a is written "0a"
//   <frame> level <n>       writes g_nextLevel once, at the first Poll with counter >= frame
//   <frame> shot <name>     frame N is saved while present number N + 1 is being made (rwcore.cpp RwCameraShowRaster asks
//                           port_input_shot), as <name>.png next to the PORT_CAPTURE screenshots (or in PORT_GTRACE_OUT)
// PORT_KEYS and PORT_LEVEL keep working next to it. While a recorder is open (PORT_GTRACE) frames count from trace frame 0 for key and
// shot lines; `level` frames stay absolute. A first line `# frames absolute` / `# frames trace` overrides. Events of trace frame 0
// fire at the next Poll, like late events.
extern "C" int port_gtrace_wanted(void);
extern "C" int port_gtrace_base(void);
struct PortInputKey { int frame, held; unsigned dik; };
struct PortInputLevel { int frame, level, done; };
struct PortInputShot { int frame, done; char name[64]; };
struct PortInputEnv { int frame, done, type, level; };
// `<frame> env <type> <level>`: call SetEnvironment(type, level / 100.0f) (0x4177a0: reverb preset 0..23 of g_envPresets, level
// 0..100; escape/mutiny use 23 100). Port only. Fires once, at the first Poll with the frame due, and logs "SetEnvironment(type,
// level)". `<frame> set <0xADDR> <int|byte|float> <value>`: write a game global at that game address (port_gaddr; symbol names
// are resolved by an external script before the file is given to the port). Fires once, like env.
struct PortInputSet { int frame, done, kind; unsigned addr; unsigned bits; };   // kind 1 = byte, 4 = int, 5 = float
// Mouse (port side only): `<frame> mouse <dx> <dy> [frames]` = relative movement of dx,dy mickeys in each of the frames (default
// 1); `<frame> wheel <notches> [frames]` = lZ 120 per notch; `<frame> mbutton <0..7> [frames]` = DirectInput mouse button held (0
// left, 1 right, 2 middle). Read by port_mouse_frame (mouse.cpp) through port_input_mouse, once per presented frame.
struct PortInputMouse { int frame, held, kind, x, y, last; };   // kind 0 = move, 1 = wheel, 2 = button
extern "C" void *port_gaddr(unsigned game_addr);
static int port_inRelative;
static int port_inDue(int frame)
{
	if (!port_inRelative) return frame;
	int b = port_gtrace_base();
	return b >= 0 ? b + frame : 0x7fffffff;
}
static PortInputKey port_inKeys[1024];
static PortInputLevel port_inLevels[64];
static PortInputShot port_inShots[64];
static PortInputEnv port_inEnvs[16];
static PortInputSet port_inSets[256];
static PortInputMouse port_inMouse[1024];
static int port_inNEnvs;
static int port_inNKeys, port_inNLevels, port_inNShots, port_inNSets, port_inNMouse;
static unsigned port_inputKeyCode(const char *s)
{
	static const struct { const char *n; unsigned dik; } names[] = {
		{ "ESCAPE", 0x01 }, { "ESC", 0x01 }, { "RETURN", 0x1c }, { "ENTER", 0x1c }, { "SPACE", 0x39 }, { "TAB", 0x0f },
		{ "UP", 0xc8 }, { "DOWN", 0xd0 }, { "LEFT", 0xcb }, { "RIGHT", 0xcd }, { "LSHIFT", 0x2a }, { "LCTRL", 0x1d }, { "PGUP", 0xc9 }, { "PGDN", 0xd1 } };
	for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++)
		if (_stricmp(s, names[i].n) == 0) return names[i].dik;
	if (s[0] && !s[1]) {
		static const char *rows[3] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };
		static const unsigned starts[3] = { 0x10, 0x1e, 0x2c };
		char u = (char)((s[0] >= 'a' && s[0] <= 'z') ? s[0] - 32 : s[0]);
		for (int r = 0; r < 3; r++) { const char *p = strchr(rows[r], u); if (p) return starts[r] + (unsigned)(p - rows[r]); }
		if (u >= '1' && u <= '9') return 0x01 + (unsigned)(u - '0');
		if (u == '0') return 0x0b;
	}
	char *end = 0; unsigned long v = strtoul(s, &end, 16);
	return (end && end != s && !*end && v > 0 && v < 0x100) ? (unsigned)v : 0x100;     // 0x100 = not a key
}
static void port_inputLoad(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: cannot open %s\n", path); fprintf(stderr, "PORT_INPUT: cannot open %s\n", path); return; }
	char line[512]; int ln = 0;
	port_inRelative = port_gtrace_wanted();
	while (fgets(line, sizeof(line), f)) {
		ln++;
		if (ln == 1) {
			char w1[32] = "", w2[32] = "";
			if (sscanf(line, "# %31s %31s", w1, w2) == 2 && _stricmp(w1, "frames") == 0) {
				if (_stricmp(w2, "absolute") == 0) port_inRelative = 0;
				else if (_stricmp(w2, "trace") == 0) port_inRelative = 1;
			}
		}
		char *h = strchr(line, '#'); if (h) *h = 0;
		char a[64] = "", b[64] = "", c[128] = "";
		int got = sscanf(line, "%63s %63s %127s", a, b, c);
		if (got < 2) continue;                                   // blank / comment / incomplete
		char *end = 0; long frame = strtol(a, &end, 10);
		if (*end || frame < 0) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad frame '%s'\n", path, ln, a); continue; }
		if (_stricmp(b, "level") == 0) {
			if (got < 3 || port_inNLevels >= 64) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad level line\n", path, ln); continue; }
			PortInputLevel &l = port_inLevels[port_inNLevels++]; l.frame = (int)frame; l.level = (int)strtol(c, 0, 0); l.done = 0;
			continue;
		}
		if (_stricmp(b, "shot") == 0) {
			if (got < 3 || port_inNShots >= 64) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad shot line\n", path, ln); continue; }
			PortInputShot &sh = port_inShots[port_inNShots++]; sh.frame = (int)frame; sh.done = 0; strncpy(sh.name, c, sizeof(sh.name) - 1); sh.name[sizeof(sh.name) - 1] = 0;
			continue;
		}
		if (_stricmp(b, "env") == 0) {
			int ty = 0, lv = 0;
			if (sscanf(line, "%*s %*s %d %d", &ty, &lv) != 2 || ty < 0 || ty > 23 || lv < 0 || lv > 100 || port_inNEnvs >= 16) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad env line (env <type 0..23> <level 0..100>)\n", path, ln); continue; }
			PortInputEnv &en = port_inEnvs[port_inNEnvs++]; en.frame = (int)frame; en.done = 0; en.type = ty; en.level = lv;
			continue;
		}
		if (_stricmp(b, "set") == 0) {
			char kind[16] = "", val[64] = "", addr[64] = "";
			if (sscanf(line, "%*s %*s %63s %15s %63s", addr, kind, val) != 3 || port_inNSets >= 256) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad set line\n", path, ln); continue; }
			char *e1 = 0, *e2 = 0; unsigned long ad = strtoul(addr, &e1, 0);
			PortInputSet &st = port_inSets[port_inNSets];
			st.frame = (int)frame; st.done = 0; st.kind = 0; st.addr = (unsigned)ad;
			if (_stricmp(kind, "float") == 0) { float fv = (float)strtod(val, &e2); memcpy(&st.bits, &fv, 4); st.kind = 5; }
			else if (_stricmp(kind, "int") == 0) { st.bits = val[0] == 45 ? (unsigned)strtol(val, &e2, 0) : (unsigned)strtoul(val, &e2, 0); st.kind = 4; }
			else if (_stricmp(kind, "byte") == 0) { st.bits = (unsigned)strtol(val, &e2, 0) & 0xff; st.kind = 1; }
			else e2 = val;
			if (*e1 || !ad || !e2 || *e2 || !val[0] || !st.kind) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad set line (0xADDR int|byte|float value; symbol names are resolved by the runner)\n", path, ln); continue; }
			port_inNSets++;
			continue;
		}
		if (_stricmp(b, "mouse") == 0 || _stricmp(b, "wheel") == 0 || _stricmp(b, "mbutton") == 0) {
			int kind = _stricmp(b, "mouse") == 0 ? 0 : _stricmp(b, "wheel") == 0 ? 1 : 2;
			int v[3] = { 0, 0, 1 }, n = sscanf(line, "%*s %*s %d %d %d", &v[0], &v[1], &v[2]);
			if (n < (kind == 0 ? 2 : 1) || port_inNMouse >= 1024 || (kind == 2 && (v[0] < 0 || v[0] > 7))) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad %s line\n", path, ln, b); continue; }
			PortInputMouse &m = port_inMouse[port_inNMouse++];
			m.frame = (int)frame; m.kind = kind; m.x = v[0]; m.y = kind == 0 ? v[1] : 0; m.last = -1;
			m.held = kind == 0 ? (n >= 3 ? v[2] : 1) : (n >= 2 ? v[1] : 1);
			if (m.held < 1) m.held = 1;
			continue;
		}
		unsigned dik = port_inputKeyCode(b);
		if (dik >= 0x100 || port_inNKeys >= 1024) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s:%d: bad key '%s'\n", path, ln, b); continue; }
		long held = got >= 3 ? strtol(c, 0, 10) : 1; if (held < 1) held = 1;
		PortInputKey &k = port_inKeys[port_inNKeys++]; k.frame = (int)frame; k.held = (int)held; k.dik = dik;
	}
	fclose(f);
	if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: %s: %d key events, %d mouse events, %d level events, %d shots, frames are %s\n", path, port_inNKeys, port_inNMouse, port_inNLevels, port_inNShots,
	                      port_inRelative ? "trace frames (keys, shots; level events absolute)" : "absolute presented frames");
}
static int port_inputEnsure(void)
{
	static const char *path = getenv("PORT_INPUT");
	static int loaded;
	if (!path) return 0;
	if (!loaded) { loaded = 1; port_inputLoad(path); }
	return 1;
}
// rwcore.cpp RwCameraShowRaster: the name of a `shot` due at `frame` (completed presents before this one), once; 0 = none
extern "C" const char *port_input_shot(int frame)
{
	if (!port_inputEnsure()) return 0;
	for (int i = 0; i < port_inNShots; i++)
		if (!port_inShots[i].done && port_inDue(port_inShots[i].frame) <= frame) { port_inShots[i].done = 1; return port_inShots[i].name; }
	return 0;
}
// the scripted mouse of this frame, added to st (lX, lY, lZ, rgbButtons[8]); movement and wheel count once per presented frame
extern "C" void port_input_mouse(int *st)
{
	if (!port_inputEnsure()) return;
	int f = port_frames_presented();
	unsigned char *btn = (unsigned char *)(st + 3);
	for (int i = 0; i < port_inNMouse; i++) {
		PortInputMouse &m = port_inMouse[i];
		int due = port_inDue(m.frame);
		if (due > f || f - due >= m.held) continue;
		if (m.kind == 2) { btn[m.x] = 0x80; continue; }
		if (m.last == f) continue;
		m.last = f;
		if (m.kind == 0) { st[0] += m.x; st[1] += m.y; } else st[2] += m.x * 120;
		static int logged; if (logged++ < 16 && port_log) fprintf(port_log, "game_stubs: PORT_INPUT: mouse %s %d %d at frame %d\n", m.kind ? "wheel" : "move", m.x, m.y, f);
	}
}
static void port_input_apply(unsigned char *cur)
{
	if (!port_inputEnsure()) return;
	int f = port_frames_presented();
	for (int i = 0; i < port_inNKeys; i++)
		if (port_inDue(port_inKeys[i].frame) <= f && f - port_inDue(port_inKeys[i].frame) < port_inKeys[i].held) {
			cur[port_inKeys[i].dik] = 0x80;
			static int logged; if (logged++ < 16 && port_log) fprintf(port_log, "game_stubs: PORT_INPUT: key %02x down at frame %d\n", port_inKeys[i].dik, f);
		}
	for (int i = 0; i < port_inNSets; i++)
		if (!port_inSets[i].done && port_inDue(port_inSets[i].frame) <= f) {
			PortInputSet &st = port_inSets[i]; st.done = 1;
			void *p = port_gaddr(st.addr);
			if (!p) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: frame %d: set 0x%08x: address does not resolve (not a mapped game data address)\n", f, st.addr); continue; }
			if (st.kind == 1) { if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: frame %d: set 0x%08x byte %02x -> %02x\n", f, st.addr, *(unsigned char *)p, st.bits); *(unsigned char *)p = (unsigned char)st.bits; }
			else { float fo; memcpy(&fo, p, 4); if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: frame %d: set 0x%08x %s %08x (%g) -> %08x\n", f, st.addr, st.kind == 5 ? "float" : "int", *(unsigned *)p, (double)fo, st.bits); *(unsigned *)p = st.bits; }
		}
	for (int i = 0; i < port_inNEnvs; i++)
		if (!port_inEnvs[i].done && port_inDue(port_inEnvs[i].frame) <= f) {
			PortInputEnv &en = port_inEnvs[i]; en.done = 1;
			if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: frame %d: SetEnvironment(type %d, level %d%%)\n", f, en.type, en.level);
			SetEnvironment(en.type, (float)en.level / 100.0f);
		}
	for (int i = 0; i < port_inNLevels; i++)
		if (!port_inLevels[i].done && f >= port_inLevels[i].frame) {
			port_inLevels[i].done = 1;
			if (port_log) fprintf(port_log, "game_stubs: PORT_INPUT: frame %d: g_nextLevel %d -> %d\n", f, g_nextLevel, port_inLevels[i].level);
			g_nextLevel = port_inLevels[i].level;
		}
}
// The keyboard of one InputManager poll (dinput_port.cpp and the address-bound Poll below): 256 DIK-indexed bytes, 0x80 = down.
// Real keys only while the window is in front and no input script is given (port_real_input); extended keys (E0 prefix) carry bit
// 7, as DirectInput reports them. Then the scripted keys and the per-poll test switches.
extern "C" int port_real_input(void);
extern "C" void port_keyboard_frame(unsigned char *cur)
{
	memset(cur, 0, 0x100);
#ifdef ZOOSTERS_DEBUGUI
	if (port_real_input() && !port_debugui_blocks_keyboard())   // typing into the debug overlay does not reach the game
#else
	if (port_real_input())
#endif
		port_platform_keyboard(cur);    // the window backend (port/provisional/platform.h)
	static const char *keys = getenv("PORT_KEYS");
	if (keys) {
		int f = port_frames_presented();
		for (const char *p = keys; p && *p; ) {
			int at = 0; unsigned dik = 0;
			if (sscanf(p, "%d:%x", &at, &dik) == 2 && f >= at && f < at + 6 && dik < 0x100) {
				cur[dik] = 0x80;
				static int logged; if (logged++ < 8 && port_log) fprintf(port_log, "game_stubs: Poll: injected key %02x at frame %d\n", dik, f);
			}
			p = strchr(p, ','); if (p) p++;
		}
	}
	port_input_apply(cur);
	{   // PORT_LEVEL="<frame>:<n>[,<frame>:<n>...]": request level n at presented frame f (what the fixture's `level` command does:
		// writes g_nextLevel 0x62a2f8, the level manager 0x4610B0 then loads it); port-only test switch.
		static const char *lv = getenv("PORT_LEVEL");
		if (lv) {
			int f = port_frames_presented();
			for (const char *p = lv; p && *p; ) {
				int at = 0, n = 0;
				if (sscanf(p, "%d:%d", &at, &n) == 2 && f == at) {
					if (port_log) fprintf(port_log, "game_stubs: PORT_LEVEL: frame %d: g_nextLevel %d -> %d\n", f, g_nextLevel, n);
					g_nextLevel = n;
				}
				p = strchr(p, ','); if (p) p++;
			}
		}
	}
	{   // PORT_CAMLOG=<every N frames>: the game camera's globals (position, target, look-at) in the port log
		static const char *cl = getenv("PORT_CAMLOG");
		static int every = cl ? atoi(cl) : 0;
		int f = port_frames_presented();
		if (every > 0 && f % every == 0 && port_log) {
			fprintf(port_log, "camlog: frame %d pos %.1f %.1f %.1f tgt %.1f %.1f %.1f look %.1f %.1f %.1f posCur %.1f %.1f %.1f tgtCur %.1f %.1f %.1f mode %d\n", f,
			        g_camPos.x, g_camPos.y, g_camPos.z, g_camTgt.x, g_camTgt.y, g_camTgt.z, g_camLookAt.x, g_camLookAt.y, g_camLookAt.z,
			        g_camPosCur.x, g_camPosCur.y, g_camPosCur.z, g_camTgtCur.x, g_camTgtCur.y, g_camTgtCur.z, g_camMode);
			fprintf(port_log, "camlog: frame %d g_cameraObject %p g_gameCamDst %p\n", f, g_cameraObject, g_gameCamDst);
			if (f == 316 && port_log) fprintf(port_log, "camlog: g_negEpsilon %g g_floatZero %g\n", g_negEpsilon, g_floatZero);
			if (getenv("PORT_CAMRAY")) {   // test ray: straight down from the camera target (RpCollisionWorldForAllIntersections, line)
				float isect[7] = { g_camTgt.x, g_camTgt.y + 20.0f, g_camTgt.z, g_camTgt.x, g_camTgt.y - 3000.0f, g_camTgt.z, 0 }; *(int *)&isect[6] = 1;
#define PORT_COLL_ARGS (RpWorld *)RWS::CLevel::sm_pActiveWorld, (RpIntersection *)isect, (RpIntersectionCallBackWorldTriangle)port_rayCb, 0
				if (RWS::CLevel::sm_pActiveWorld) RpCollisionWorldForAllIntersections(PORT_COLL_ARGS);
			}
			fflush(port_log);
		}
	}
}

// ---- debugging aid: dump RWS::CClassFactory's class map (std::map<const char*, CRegistrar*>, cl 7.1 layout: head at +4,
// size at +8; node: left +0, parent +4, right +8, key +0xc, value +0x10, isnil +0x15). Called once by the harness (rwcore.cpp).
#include <stdio.h>
extern "C" FILE *port_log;
extern "C" void port_dump_classmap(void)
{
	if (!port_log) return;
	char *m = (char *)PortGame_ClassMap();
	// modern MSVC std::map<>: _Myhead at +0, _Mysize at +P; node: left, parent, right (3 P), _Color, _Isnil, then key / value aligned to P
	// (x86: _Isnil +0xd, key +0x10, value +0x14; x64: +0x19, +0x20, +0x28)
	enum { P = sizeof(void *), MAP_HEAD = 0, MAP_SIZE = P, N_PAR = P, N_RIGHT = 2 * P, N_NIL = 3 * P + 1, N_KEY = 4 * P, N_VAL = 5 * P };
	char *head = *(char **)(m + MAP_HEAD);
	int size = *(int *)(m + MAP_SIZE);
	fprintf(port_log, "classmap: map %p head %p size %d:", (void *)m, (void *)head, size);
	char *n = *(char **)(head + 0);                     // leftmost
	int guard = 0;
	while (n && n != head && !n[N_NIL] && guard++ < 200) {
		const char *key = *(const char **)(n + N_KEY); void **reg = *(void ***)(n + N_VAL);
		fprintf(port_log, " [%s -> %p fn %p]", key ? key : "(null)", (void *)reg, reg ? reg[0] : 0);
		// in-order successor
		char *r = *(char **)(n + N_RIGHT);
		if (!r[N_NIL]) { n = r; while (!(*(char **)(n + 0))[N_NIL]) n = *(char **)(n + 0); }
		else { char *pn; while (!(pn = *(char **)(n + N_PAR))[N_NIL] && n == *(char **)(pn + N_RIGHT)) n = pn; n = pn; }
	}
	fprintf(port_log, "\n"); fflush(port_log);
}

// 0x41f980 FUN_0041f980 (camera probe intersection callback)
