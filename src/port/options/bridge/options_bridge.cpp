// ZOOSTERS_OPTIONS bridge: the game-facing half of the in-game options (options_bridge.h). Uses game classes directly:
// SpriteManager / GameSprite / Hud2DObject (run-time sprites, like the mouse pointer in framework/cursor.cpp), CtlMap of the
// active input context, GameController (input block mask), CfgObj (settings store).
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "options_bridge.h"
#include "rw/rwcore.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/sprites/spritemanager.h"
#include "modules/tfb/gamecontroller.h"
#include "framework/startup/win32/appstate.h"
#include "framework/mainloop/mainloop.h"
#include "tfb/hud/hud2d.h"
#include "zoosters/settings.h"
#include "framework/startup/win32/cfgobj.h"
#include "framework/startup/win32/input/inputmanager.h"
#include "framework/startup/win32/input/ctlbuf.h"
#include "script/types/scriptcontroller.h"
#include "script/types/scriptsound.h"
#include "script/symbolregistry.h"
#include "script/ops_set.h"
#include "modules/tfb/ctfbsound.h"
#include "framework/core/streamhandler.h"
#include "framework/core/cresource.h"
#include "../../os/native_path.h"   // PORT_NATIVE_PATH

extern "C" FILE *port_log;
extern "C" int port_platform_pointer(int *x, int *y, int *leftDown);   // port/provisional/window_*.cpp (Win32: GetAsyncKeyState(VK_LBUTTON))
static bool LeftButtonDown()
{
	int x, y, down;
	port_platform_pointer(&x, &y, &down);
	return down != 0;
}
extern RWS::TFB::GameController *g_gameController;   // 0x61798c
extern CtlBuf *g_ctlBuffers[9];                      // 0x617ba0

int SymbolRegistry_GetSize();   // 0x432cb0
using RWS::TFB::GameSprite;

namespace opt_bridge {

void Log(const char *fmt, ...)
{
	if (!port_log) return;
	fprintf(port_log, "options: ");
	va_list ap; va_start(ap, fmt); vfprintf(port_log, fmt, ap); va_end(ap);
	fputc('\n', port_log); fflush(port_log);
}

// ---- sprites ---------------------------------------------------------------------------------------------------------------------------
namespace {
struct Slot {
	GameSprite *sp;
	const char *name;
	const char *tex;     // texture requested (quads), 0 for text
	float texW, texH;    // the texture's size
	bool changed;
};
const int kMaxSprites = 160;
Slot g_slots[kMaxSprites];
int g_nSlots;
bool g_listDirty;

Slot *slot(Sprite s) { return (s > 0 && s <= g_nSlots && g_slots[s - 1].sp) ? &g_slots[s - 1] : 0; }

bool rasterSize(RwTexture *tex, float *w, float *h)
{
	if (!tex) return false;
	RwRasterView *r = (RwRasterView *)((RwTextureView *)tex)->raster;
	if (!r) return false;
	*w = (float)r->width; *h = (float)r->height;
	return true;
}
}  // namespace

Sprite SpriteCreate(const char *name)
{
	if (!g_spriteManager) return 0;
	int at = -1;
	for (int i = 0; i < g_nSlots && at < 0; i++)
		if (g_slots[i].name == name) {
			if (g_slots[i].sp) return i + 1;
			at = i;                              // destroyed earlier: the slot is reused
		}
	if (at < 0) {
		if (g_nSlots >= kMaxSprites) return 0;
		at = g_nSlots;
	}
	GameSprite *sp = SpriteMgr_CreateTextSprite(name, "", 0.0f, 0.0f, 600.0f, 40.0f);
	if (!sp) return 0;
	Slot &s = g_slots[at];
	s.sp = sp; s.name = name; s.tex = 0; s.texW = s.texH = 0; s.changed = true;
	// CreateTextSprite makes an always-visible type 2 sprite with the HUD's red tint: neutral defaults, hidden
	// GameSprite(name, text) leaves these unset (the game's allocator hands out zeroed memory; the port's does not)
	sp->m_childRefs = 0;
	sp->m_resolveGeneration = -1;
	sp->m_renderNext = (GameSprite *)-1;
	sp->m_alwaysVisible = 0;
	sp->m_visible = 0;
	sp->m_type = 2;
	sp->m_tint.red = 0xff; sp->m_tint.green = 0xff; sp->m_tint.blue = 0xff; sp->m_tint.alpha = 0xff;
	// priority stays at CreateTextSprite's 1 (= its priority key): changing it makes SpriteManager::Update re-key the sprite in the active set
	sp->m_justification = ANCHOR_CENTRE; sp->m_anchor = ANCHOR_CENTRE;
	g_listDirty = true;
	if (at == g_nSlots) g_nSlots++;
	return at + 1;
}

void SpriteDestroy(Sprite s)
{
	Slot *sl = slot(s);
	if (!sl) return;
	RemoveSpriteByName(sl->name);
	sl->sp = 0;
	g_listDirty = true;
}

void SpriteSetQuad(Sprite s, const char *texture)
{
	Slot *sl = slot(s);
	if (!sl) return;
	GameSprite *sp = sl->sp;
	if (sp->m_type == 0 && sl->tex && strcmp(sl->tex, texture) == 0) return;
	sp->m_type = 0;
	sp->m_flags &= ~3u;                  // size from the texture (SetSizeFromTexture), no text wrap
	sl->tex = texture;
	sp->SetSpriteByName(texture);
	sp->SetSizeFromTexture();
	if (!rasterSize(sp->m_texture, &sl->texW, &sl->texH)) {
		static int logged; if (logged++ < 8) Log("texture '%s' not found", texture);
		sl->texW = sl->texH = 0;
	}
	sp->m_tint.red = 0x80; sp->m_tint.green = 0x80; sp->m_tint.blue = 0x80; sp->m_tint.alpha = 0xff;
	sl->changed = true; g_listDirty = true;
}

void SpriteSetText(Sprite s, const char *text)
{
	Slot *sl = slot(s);
	if (!sl) return;
	GameSprite *sp = sl->sp;
	if (sp->m_type == 2 && sp->m_format == sp->m_textBuf && strcmp(sp->m_textBuf, text) == 0) return;
	if (sp->m_type != 2) { sp->m_type = 2; sl->tex = 0; sp->m_texture = 0; }
	if (getenv("PORT_OPTIONS_TRACE")) Log("SetText %s buf %p fmt %p '%s' type %d", sl->name, (void *)sp->m_textBuf, (const void *)sp->m_format, text, sp->m_type);
	strncpy(sp->m_textBuf, text, 0xfe); sp->m_textBuf[0xfe] = 0;
	sp->m_format = sp->m_textBuf;
	sl->changed = true; g_listDirty = true;
}

void SpriteSetAnchor(Sprite s, int anchor)
{
	Slot *sl = slot(s);
	if (!sl || sl->sp->m_justification == anchor) return;
	sl->sp->m_justification = (unsigned char)anchor; sl->sp->m_anchor = anchor;
	sl->changed = true;
}

void SpriteSetPos(Sprite s, float x, float y)
{
	Slot *sl = slot(s);
	if (!sl || (sl->sp->m_posX == x && sl->sp->m_posZ == y)) return;
	sl->sp->m_posX = x; sl->sp->m_posZ = y; sl->sp->m_posY = 3.4028235e+38f;   // FLT_MAX: a screen sprite
	sl->changed = true;
}

void SpriteSetScale(Sprite s, float sx, float sy)
{
	Slot *sl = slot(s);
	if (!sl || (sl->sp->m_scaleX == sx && sl->sp->m_scaleY == sy)) return;
	sl->sp->m_scaleX = sx; sl->sp->m_scaleY = sy;
	sl->changed = true;
}

void SpriteSetSize(Sprite s, float w, float h)
{
	Slot *sl = slot(s);
	if (!sl || sl->texW <= 0 || sl->texH <= 0) return;
	SpriteSetScale(s, w / sl->texW, h / sl->texH);
}

void SpriteSetTint(Sprite s, unsigned argb)
{
	Slot *sl = slot(s);
	if (!sl) return;
	GameSprite *sp = sl->sp;
	unsigned char a = (unsigned char)(argb >> 24), r = (unsigned char)(argb >> 16), g = (unsigned char)(argb >> 8), b = (unsigned char)argb;
	if (sp->m_tint.alpha == a && sp->m_tint.red == r && sp->m_tint.green == g && sp->m_tint.blue == b) return;
	sp->m_tint.alpha = a; sp->m_tint.red = r; sp->m_tint.green = g; sp->m_tint.blue = b;
	sl->changed = true;
}

void SpriteSetPriority(Sprite s, int priority)
{
	// the game's priority byte: 0..127 as is (drawn first = behind), above 127 means byte - 254 (negative = in front); higher draws earlier
	unsigned char b = (unsigned char)(priority < 0 ? priority + 254 : priority);
	Slot *sl = slot(s);
	if (!sl || sl->sp->m_priority == b) return;
	sl->sp->m_priority = b;
	sl->changed = true; g_listDirty = true;
}

void SpriteSetVisible(Sprite s, bool visible)
{
	Slot *sl = slot(s);
	if (!sl || (sl->sp->m_visible != 0) == visible) return;
	sl->sp->m_visible = visible ? 1 : 0;
	sl->changed = true; g_listDirty = true;
}

bool SpriteBounds(Sprite s, float *x, float *y, float *w, float *h)
{
	Slot *sl = slot(s);
	if (!sl || !sl->sp->m_visible || !sl->sp->m_drawVisible || sl->sp->m_resolveGeneration != g_spriteGeneration) return false;
	GameSprite *sp = sl->sp;
	*x = sp->m_drawX + sp->m_drawScaleX * sp->m_offsetX;
	*y = sp->m_drawY + sp->m_drawScaleY * sp->m_offsetY;
	*w = sp->m_width; *h = sp->m_height;
	return true;
}

bool TextureSize(const char *texture, float *w, float *h)
{
	struct Cached { char name[40]; float w, h; };
	static Cached cache[16]; static int n;
	for (int i = 0; i < n; i++)
		if (_stricmp(cache[i].name, texture) == 0) { *w = cache[i].w; *h = cache[i].h; return cache[i].w > 0; }
	RwTexture *t = Hud2D::LoadSpriteTexture(texture);   // the level's dictionary (RwTextureRead); the reference is kept: the textures stay resident anyway
	float tw = 0, th = 0;
	rasterSize(t, &tw, &th);
	if (n < 16) { strncpy(cache[n].name, texture, 39); cache[n].name[39] = 0; cache[n].w = tw; cache[n].h = th; n++; }
	*w = tw; *h = th;
	return tw > 0;
}

void DebugDump()
{
	for (int i = 0; i < g_nSlots; i++) {
		GameSprite *sp = g_slots[i].sp;
		if (!sp || !sp->m_visible) continue;
		Log("  %s type %d tex %s pos %.1f,%.1f scale %.3f,%.3f box %.1f,%.1f draw %.1f,%.1f dscale %.3f,%.3f ext %.1f,%.1f off %.1f,%.1f prio %d tint %02x%02x%02x%02x alpha %.2f text '%s'",
		    g_slots[i].name, sp->m_type, g_slots[i].tex ? g_slots[i].tex : "-", sp->m_posX, sp->m_posZ, sp->m_scaleX, sp->m_scaleY, sp->m_boxWidth, sp->m_boxHeight,
		    sp->m_drawX, sp->m_drawY, sp->m_drawScaleX, sp->m_drawScaleY, sp->m_width, sp->m_height, sp->m_offsetX, sp->m_offsetY, sp->m_drawPriority,
		    sp->m_tint.alpha, sp->m_tint.red, sp->m_tint.green, sp->m_tint.blue, sp->m_drawAlpha, sp->m_type == 2 ? sp->m_textBuf : "");
	}
}

void SpritesFlush()
{
	if (!g_spriteManager) return;
	if (g_listDirty) { Script::InvalidateSprite(0); g_listDirty = false; }
	for (int i = 0; i < g_nSlots; i++) {
		if (g_slots[i].changed && g_slots[i].sp) Script::InvalidateSprite(g_slots[i].sp);
		g_slots[i].changed = false;
	}
}

// ---- input -----------------------------------------------------------------------------------------------------------------------------
namespace {
struct NavState { bool down; int held; bool event; };
NavState g_nav[NAV_COUNT];
bool g_capture, g_primed;
int g_noMenuCtxLogged;

CtlMap *menuMap()
{
	if (!g_inputManager || !g_ctlBuffers[1]) return 0;
	CtlBuf *c = g_inputManager->m_contexts[0];
	if (c != g_ctlBuffers[1]) return 0;     // buffer 1 is the menu mapping; other contexts bind other actions to these numbers
	return &c->m_map;
}

void setBlock(bool on)
{
	if (!g_gameController) return;
	g_gameController->m_blockMask = on ? 0xFFFF : 0;       // GetInputValue ignores a blocked input
	g_gameController->m_blockUntil = on ? 0x7FFFFFFF : 0;  // the post-tick release check clears bits 0..5 only after this time
}
}  // namespace

void InputCapture(bool on)
{
	g_capture = on;
	g_primed = false;
	setBlock(on);
}


void InputPoll()
{
	if (g_capture) setBlock(true);
	CtlMap *m = menuMap();
	bool d[NAV_COUNT] = { false, false, false, false, false, false };
	if (m) {
		d[NAV_UP] = m->GetValue(0x10) > 0.5f || m->GetValue(0x18) > 0.5f;
		d[NAV_DOWN] = m->GetValue(0x11) > 0.5f || m->GetValue(0x19) > 0.5f;
		d[NAV_LEFT] = m->GetValue(0x12) > 0.5f || m->GetValue(0x1a) > 0.5f;
		d[NAV_RIGHT] = m->GetValue(0x13) > 0.5f || m->GetValue(0x1b) > 0.5f;
		d[NAV_ACCEPT] = m->GetValue(4) > 0.5f;
		d[NAV_BACK] = m->GetValue(7) > 0.5f;
	} else if (g_capture && g_noMenuCtxLogged++ < 3) {
		Log("InputPoll: the active input context is not the menu buffer");
	}
	for (int i = 0; i < NAV_COUNT; i++) {
		NavState &n = g_nav[i];
		n.event = false;
		if (!g_primed) { n.down = d[i]; n.held = 0; continue; }   // keys already held when the screen opens do nothing until released
		if (d[i] && !n.down) { n.event = true; n.held = 0; }
		else if (d[i] && i <= NAV_RIGHT) {
			n.held++;
			if (n.held >= 20 && (n.held - 20) % 3 == 0) n.event = true;   // auto repeat of the arrows
		}
		n.down = d[i];
	}
	g_primed = true;
}

bool InputAnyHeld() { for (int i = 0; i < NAV_COUNT; i++) if (g_nav[i].down) return true; return false; }
bool InputPressed(int nav) { return nav >= 0 && nav < NAV_COUNT && g_nav[nav].event; }

void InputInjectBack()
{
	g_wndRequestA = 3;       // pad button Y = the script's "triangle button" (the click path of menupointer.cpp injects A = X button)
	g_wndRequestB = 0;
	g_wndRequestFlag = 1;    // read by ControlsManager::Poll as g_wndRequestFlag (one-shot)
}

MouseState Mouse()
{
	static float lastX = -1, lastY = -1;
	static bool wasDown;
	MouseState m = { false, 0, 0, false, false, false };
	if (!g_cursorSprite || !g_cursorShown) { wasDown = LeftButtonDown(); return m; }
	m.valid = true;
	m.x = g_cursorSprite->m_posX; m.y = g_cursorSprite->m_posZ;
	m.moved = (m.x != lastX || m.y != lastY);
	lastX = m.x; lastY = m.y;
	m.down = LeftButtonDown();
	m.pressed = m.down && !wasDown;
	wasDown = m.down;
	return m;
}

// The pause script plays these through "play sound UI_Move::sound" (MediaPlay::Execute -> ScriptSoundObj::Play(actor) of the
// symbol the level's CTFBSound entity registered under its lower-case name); the same call from here, with no actor (voices only
// use it to follow a position).
namespace {
Script::ScriptSoundObj *findSound(const char *lowerName)
{
	if (!g_symRegistryY) return 0;
	Script::ScriptReference *proto = TFBType_Sound();
	int n = SymbolRegistry_GetSize();
	for (int i = 0; i < n && i < 0x3fc0d; i++) {
		uintptr_t o = (*g_symRegistryY)[i].obj;
		if (o & 1) continue;
		Script::ScriptReference *r = (Script::ScriptReference *)o;
		if (!r || r == proto || r->GetType() != proto) continue;
		Script::ScriptSoundObj *so = (Script::ScriptSoundObj *)r;
		if (!so->m_link) continue;
		RWS::TFB::CTFBSound *snd = so->GetSound();
		if (snd && snd->m_name && strcmp(snd->m_name, lowerName) == 0) return so;
		if (snd && snd->m_name && getenv("PORT_OPTIONS_TRACE") && strncmp(snd->m_name, "ui", 2) == 0) Log("  sound %s", snd->m_name);
	}
	return 0;
}
}  // namespace

void PlaySound(int which)
{
	static const char *const names[3] = { "ui_select", "ui_accept", "ui_cancel" };
	if (which < 0 || which > 2) return;
	Script::ScriptSoundObj *so = findSound(names[which]);
	static int logged;
	if (logged++ < 6) Log("PlaySound %s -> %s", names[which], so ? "found" : "no such sound in this level");
	if (so) so->Play(0);
}

// ---- mod files
// ----------------------------------------------------------------------------------------------------------------------------
bool ModFile(const char *rel, char *out, int outSize)
{
	char cand[3][520];
	int n = 0;
	const char *env = getenv("PORT_MOD_DIR");
	if (env && *env) _snprintf(cand[n++], 520, "%s\\%s", env, rel);
	_snprintf(cand[n++], 520, "Mod\\%s", rel);
	const char *user = getenv("PORT_USER_DIR");      // <port exe dir>\user (port_run.cpp): its parent is the exe's folder
	if (user && *user) {
		char dir[520]; strncpy(dir, user, 519); dir[519] = 0;
		char *sl = strrchr(dir, '\\'), *sl2 = strrchr(dir, '/');   // either separator (POSIX: <exe dir>/user)
		if (sl2 > sl) sl = sl2;
		if (sl) { *sl = 0; _snprintf(cand[n++], 520, "%s\\Mod\\%s", dir, rel); }
	}
	for (int i = 0; i < n; i++) {
		cand[i][519] = 0;
		char native[1024];
		FILE *f = fopen(PORT_NATIVE_PATH(cand[i], native), "rb");
		if (!f) continue;
		fclose(f);
		strncpy(out, cand[i], outSize - 1); out[outSize - 1] = 0;
		return true;
	}
	return false;
}

// Mod\Levels\options.stream = a texture dictionary, built with --global: a locked resource, so it survives the level resets and
// every level finds the textures by name). Same reader as StartUp::LoadGameDatabaseFile, which only takes paths below the game's
// video dir.
bool LoadModAssets()
{
	static int state;      // 0 not tried, 1 loaded, -1 absent / failed
	if (state) return state > 0;
	char path[520];
	if (!ModFile("Levels\\options.stream", path, sizeof(path))) { state = -1; Log("no Mod\\Levels\\options.stream: text arrows, game slider art only"); return false; }
	RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
	bool ok = false;
	if (stream) {
		ok = RWS::CStreamHandler::ProcessStream(stream);
		RwStreamClose(stream, path);
	}
	state = ok ? 1 : -1;
	Log("mod stream %s: %s", path, ok ? "loaded" : "FAILED");
	int nres = 0;
	for (RWS::CResourceIterator it; ok && !it.IsDone(); ++it, nres++)   // the dictionary must be locked (global flag) to survive the level resets
		if (it->GetName() && _stricmp(it->GetName(), "ModOptions_TXD") == 0) Log("resource ModOptions_TXD lock count %u", it->GetLockRefCount());
	Log("%d resources in the manager", nres);
	return ok;
}

// ---- the pause menu ---------------------------------------------------------------------------------------------------------------------
static const char *const kOptionsPageSprites[] = { "Set_Options_title", "Menu_Sound_Volume", "Menu_Music_Volume", "Menu_Options_Vibrations",
	"Menu_Options_Back", "Menu_Options_Autosave", "Selection Indicator", 0 };

bool PauseOptionsPageShown()
{
	GameSprite *t = FindSpriteByName("Set_Options_title");
	return t && t->m_visible;
}

bool PauseMenuShown()
{
	GameSprite *m = FindSpriteByName("menu_master");
	return m && m->m_visible;
}

// The pause frame: menu_frame_top, menu_frame_bottom and the 18 rows menu_frame_mid_00..17. The options screen hides it and gives
// each part back the visibility it had: rows 12..17 are parked off the frame and hidden by PR_Pause, so restoring them all as
// visible left a stray panel at the right edge of the pause menu.
enum { PAUSE_FRAME_PARTS = 20 };
static GameSprite *pauseFramePart(int i)
{
	char nm[24];
	if (i == 0) return FindSpriteByName("menu_frame_top");
	if (i == 1) return FindSpriteByName("menu_frame_bottom");
	sprintf(nm, "menu_frame_mid_%02d", i - 2);
	return FindSpriteByName(nm);
}
static unsigned char g_frameWas[PAUSE_FRAME_PARTS];   // each part's visibility before the options screen hid it
static bool g_frameSaved;

static void hidePauseFrame()
{
	for (int i = 0; i < PAUSE_FRAME_PARTS; i++) {
		GameSprite *s = pauseFramePart(i);
		if (!g_frameSaved) g_frameWas[i] = s ? s->m_visible : 0;
		if (s) s->m_visible = 0;
	}
	g_frameSaved = true;
	g_listDirty = true;
}

void PauseRestoreFrame()
{
	if (!g_frameSaved)
		return;
	for (int i = 0; i < PAUSE_FRAME_PARTS; i++)
		if (GameSprite *s = pauseFramePart(i))
			s->m_visible = g_frameWas[i];
	g_frameSaved = false;
	g_listDirty = true;
}

void PauseHideOptionsPage()
{
	GameSprite *top = FindSpriteByName("menu_frame_top");
	if (top && (top->m_visible || !g_frameSaved)) hidePauseFrame();
	bool any = false;
	for (int i = 0; kOptionsPageSprites[i]; i++) {
		GameSprite *s = FindSpriteByName(kOptionsPageSprites[i]);
		if (s && s->m_visible) { s->m_visible = 0; any = true; }
	}
	if (any) g_listDirty = true;
}

void PauseShowOptionsTitle()
{
	GameSprite *t = FindSpriteByName("Set_Options_title");
	if (t) { t->m_visible = 1; g_listDirty = true; }
}

// ---- the title menu --------------------------------------------------------------------------------------------------------------------
// The title's list is the script set g_subsetTable[2] (New Game, Load Game, + the rows ReferenceSet::Insert spawns); its title text, the
// selection indicator (re-shown by the script every tick) and the button prompt go with it while our screen is up.
namespace {
const int kTitleMax = 12;
GameSprite *g_titleHidden[kTitleMax];
unsigned char g_titleWas[kTitleMax];
int g_titleCount;
bool g_titleSaved;

int titleSprites(GameSprite **out)
{
	int n = 0;
	Script::ScriptSubset *set = Script::g_subsetTable[2];
	for (Script::SetNode *node = set ? set->m_head : 0; node && n < kTitleMax - 3; node = node->m_next)
		if (node->m_object) out[n++] = static_cast<GameSprite *>(node->m_object);
	static const char *const extra[3] = { "MEM_Title_menu_title", "Selection Indicator", "prompt_main_menu" };
	for (int i = 0; i < 3; i++)
		if (GameSprite *g = FindSpriteByName(extra[i])) out[n++] = g;
	return n;
}
}  // namespace

bool TitleMenuShown()
{
	Script::ScriptSubset *set = Script::g_subsetTable[2];
	GameSprite *head = set && set->m_head ? static_cast<GameSprite *>(set->m_head->m_object) : 0;
	return head && head->m_visible;
}

// The fourth row would sit on the "Pick Action" prompt (440): it moves down to the screen's bottom edge.
void TitleMovePrompt()
{
	GameSprite *p = FindSpriteByName("prompt_main_menu");
	if (!p || p->m_posZ == 462.0f) return;
	p->m_posZ = 462.0f;
	Script::InvalidateSprite(p);
}

void TitleHideMenu()
{
	if (!g_titleSaved) {
		g_titleCount = titleSprites(g_titleHidden);
		for (int i = 0; i < g_titleCount; i++) g_titleWas[i] = g_titleHidden[i]->m_visible;
		g_titleSaved = true;
	}
	bool any = false;
	for (int i = 0; i < g_titleCount; i++)
		if (g_titleHidden[i]->m_visible) { g_titleHidden[i]->m_visible = 0; any = true; }
	if (any) g_listDirty = true;
}

void TitleRestoreMenu()
{
	if (!g_titleSaved) return;
	for (int i = 0; i < g_titleCount; i++) g_titleHidden[i]->m_visible = g_titleWas[i];
	g_titleSaved = false;
	g_listDirty = true;
}

// ---- settings ----------------------------------------------------------------------------------------------------------------------------------
extern "C" void port_gamepad_pulse(float strength, int ms);   // port/provisional/gamepad_*.cpp

int SettingGet(int key)
{
	switch (key) {
	case KEY_MUSIC: return g_musicVolume;
	case KEY_EFFECTS: return g_effectsVolume;
	case KEY_RUMBLE_ON: return g_rumbleOff ? 0 : 1;
	case KEY_SHADOWS: return g_detailedShadows ? 1 : 0;
	case KEY_DIST_CLIP: return g_distClip ? 1 : 0;
	case KEY_DISTANCE: return g_distance;
	}
	return 0;
}

void SettingSet(int key, int v)
{
	switch (key) {
	case KEY_MUSIC: g_musicVolume = v; break;
	case KEY_EFFECTS: g_effectsVolume = v; break;
	case KEY_RUMBLE_ON:
		if (v && g_rumbleOff)
			port_gamepad_pulse(0.6f, 300);   // qol: switching vibration on gives a short buzz, so the player feels it is on
		g_rumbleOff = v ? 0 : 1;
		break;
	case KEY_SHADOWS: g_detailedShadows = v != 0; break;
	case KEY_DIST_CLIP: g_distClip = v != 0; break;
	case KEY_DISTANCE: g_distance = v; break;
	}
}

void SettingCommit(int key, const char *regPath)
{
	int v = SettingGet(key);
	if (key == KEY_RUMBLE_ON) v = v ? 0 : 1;    // the registry value is RumbleOff
	ConfigSetInt(regPath, v);
}

// the store addresses section and name apart: "Settings\\Display\\Resolution" -> ("Settings\\Display", "Resolution")
static bool splitPath(const char *regPath, char *section, int size, const char **name)
{
	const char *b = strrchr(regPath, '\\');
	if (!b || b - regPath >= size) return false;
	memcpy(section, regPath, (size_t)(b - regPath));
	section[b - regPath] = 0;
	*name = b + 1;
	return true;
}
bool ConfigGetString(const char *regPath, char *out, int size)
{
	char section[64];
	const char *name;
	return splitPath(regPath, section, sizeof section, &name) && zoosters::settings::GetString(section, name, out, (size_t)size) == zoosters::settings::Found::Yes;
}
void ConfigSetString(const char *regPath, const char *value)
{
	char section[64];
	const char *name;
	if (splitPath(regPath, section, sizeof section, &name)) zoosters::settings::SetString(section, name, value);
}

// ---- screen
extern "C" int port_camera_resize(void *camera, int w, int h);   // port/shim/rwcore.cpp
bool ScreenSize(int *w, int *h)
{
	RwCamera *cam = RWS::MainLoop::Render::GetCamera();
	RwRaster *r = cam ? RwCameraGetRaster(cam) : 0;
	if (!r) return false;
	*w = RwRasterGetWidth(r);
	*h = RwRasterGetHeight(r);
	return true;
}
void ScreenResized(int w, int h)
{
	RwCamera *cam = RWS::MainLoop::Render::GetCamera();
	if (!cam || !port_camera_resize(cam, w, h)) return;
	if (g_resHeight > 0) Hud2D::RescaleFonts((float)h / (float)g_resHeight);   // fonts were sized by the screen height at load
	g_resWidth = (unsigned)w;
	g_resHeight = (unsigned)h;
}

int ConfigGetInt(const char *regPath, int def) { return g_audioDevice ? g_audioDevice->GetInt(regPath, def) : def; }
void ConfigSetInt(const char *regPath, int value) { if (g_audioDevice) g_audioDevice->SetOption(regPath, value); }

}  // namespace opt_bridge

extern "C" void port_options_load_assets(void) { opt_bridge::LoadModAssets(); }   // called after global.stream (winmain.cpp)
