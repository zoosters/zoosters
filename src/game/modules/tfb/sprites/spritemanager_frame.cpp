#include <string.h>
#include "tfb/level/level.h"
#include "modules/tfb/sprites/spritemanager.h"
#include "modules/tfb/camera_views.h"
#include "rw/rpworld.h"
#include "framework/mainloop/gameclock.h"

// GLOBAL: 0x617e80 g_spriteManager
// GLOBAL: 0x617e84 g_spriteGeneration
// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld
// GLOBAL: 0x617f38 g_sprPassCount
// GLOBAL: 0x617f37 g_sprFlagMenu
// GLOBAL: 0x617f36 g_sprFlag2P
// GLOBAL: 0x617f35 g_sprFlagGame
// GLOBAL: 0x617f34 g_sprFlagStart
// GLOBAL: 0x617e7c g_cranePromptShown
// GLOBAL: 0x62a5c2 g_cfgKeyReqA
// GLOBAL: 0x62a5c3 g_cfgKeyReqB
// GLOBAL: 0x618318 g_frameDt
// GLOBAL: 0x464ef0 ??1Hud2DObject@@QAE@XZ
extern int g_sprPassCount;
extern unsigned char g_sprFlagMenu, g_sprFlag2P, g_sprFlagGame, g_sprFlagStart, g_cranePromptShown;
extern unsigned char g_cfgKeyReqA, g_cfgKeyReqB;
extern float g_frameDt;

int GetCurrentLevel();
void Pad_SelectControlBuffer(int idx);
void CameraDrawOverlay(RwCamera *cam);                    // cdecl; the callee ignores the argument
int *CutScene_StackSlot(int delta);
float TFBClock_GetRawSeconds();

#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only (port/fixes/widescreen.cpp): HUD elements near a side edge move to that edge
extern float g_hudScreenB, g_hudScreenC, g_hudScreenD, g_hudScreenE, g_hudScreenH, g_hudScreenW;
extern "C" void port_wide2d_sprite(const char *name, float left, float width, float top, float height);
extern "C" void port_wide2d_hud_y(float H, float *C, float *E);
extern "C" void port_wide2d_anchor(float left, float width, float top, float height);
extern "C" void port_wide2d_reset(void);
extern "C" void port_wide2d_scan_begin(void);
extern "C" void port_wide2d_scan(const char *name);
extern "C" void port_wide2d_trace(const char *name);
extern "C" void port_wide2d_hud(float W, float H, float *B, float *D);
#endif

#ifdef ZOOSTERS_OPTIONS   // in-game options (port/options/): the Options page of the pause menu is a C++ screen
extern "C" void port_options_frame(void);
extern "C" int port_options_is_menu_sprite(const char *name);
#endif

namespace RWS { namespace TFB {

// Near-match of 0x415330: layout and registers only; the game computes `raised` (setg bl) from the cmp shared with the je.
// The letterbox alpha follows the cut-scene state (fade in outside one, out in one). Then every listed sprite is resolved; when
// the list is dirty it is rebuilt from the active set, sorted by priority key.
void SpriteManager::Update(RwCamera *camera)
{
#ifdef ZOOSTERS_OPTIONS
	port_options_frame();   // port/options/pages/pause_options.cpp: runs the options screen before the sprites are resolved
#endif
	int cutState = *CutScene_StackSlot(0);
	if (cutState != 3) {
		if (cutState < 2) {
			if (TFBClock_GetRawSeconds() < 0.1f)
				m_letterboxAlpha = 255;
			m_letterboxAlpha += 4;
			if (m_letterboxAlpha > 255)
				m_letterboxAlpha = 255;
		} else if (m_letterboxAlpha > 0) {
			m_letterboxAlpha -= 7;
			if (m_letterboxAlpha < 0)
				m_letterboxAlpha = 0;
		}
	}
	GameSprite **head = &m_renderList;
	GameSprite *spr = *head;
	if (spr != (GameSprite *)-1) {
		for (; spr; spr = spr->m_renderNext) {
			spr->Resolve(camera, 0);
			if (spr->m_extra)
				spr->m_extra->Advance(g_frameDt);
		}
	} else {
		int gen = g_spriteGeneration;
		gen++;
		m_renderList = 0;
		g_spriteGeneration = gen;
		Script::SetNode *cur = Script::TFB_GetActiveSpriteSet()->m_head;
		Script::SetNode *next;
		for (; cur; cur = next) {
			next = cur;
			Script::TFB_GetActiveSpriteSet()->Next(Script::SET_FOLLOWING, &next);
			spr = static_cast<GameSprite *>(cur->m_object);
			if (!spr->Resolve(camera, 0)) {
				spr->m_renderNext = (GameSprite *)-1;
				continue;
			}
			spr->m_renderNext = *head;
			int prio = spr->m_drawPriority;
			int oldPrio = spr->m_priorityKey;
			if (prio == oldPrio) {
				*head = spr;
				if (spr->m_extra)
					spr->m_extra->Advance(g_frameDt);
			} else {
				spr->m_priorityKey = prio;
				unsigned char raised = prio > oldPrio;
				Script::TFB_GetActiveSpriteSet()->RemoveNode(cur);
				cur = Script::TFB_GetActiveSpriteSet()->Insert(spr, (float)spr->m_priorityKey);
				if (raised) {
					Script::TFB_GetActiveSpriteSet()->Next(Script::SET_FOLLOWING, &cur);
					if (cur != next)
						continue;
				}
				GameSprite *first = *head;
				if (first == 0 || spr->m_priorityKey >= first->m_priorityKey) {
					*head = spr;
					if (spr->m_extra)
						spr->m_extra->Advance(g_frameDt);
				} else {
					GameSprite **pp = head;
					for (GameSprite *nx = (*pp)->m_renderNext; nx && spr->m_priorityKey < nx->m_priorityKey; nx = (*pp)->m_renderNext)
						pp = &(*pp)->m_renderNext;
					spr->m_renderNext = (*pp)->m_renderNext;
					(*pp)->m_renderNext = spr;
				}
			}
		}
	}
}

// 0x4140f0
// Two black bars, each 15% of the raster height times alpha / 255, as 2D quads.
void SpriteManager::DrawLetterbox(RwCamera *camera)
{
	const RwCameraView *cam = CameraView(camera);
	RwIm2DVertex v[4];
	float height = (float)((const RwRasterView *)cam->frameBuffer)->height;
	float rhw = 1.0f / cam->nearPlane;
	float width = (float)((const RwRasterView *)cam->frameBuffer)->width;
	unsigned colour = (unsigned)m_letterboxAlpha << 24;
	float barHeight = ((float)m_letterboxAlpha * 0.003921569f) * (height * 0.15f);
	v[0].x = -0.5f; v[0].z = RwIm2DGetNearScreenZ(); v[0].rhw = rhw; v[0].emissiveColor = colour;
	v[1].x = width; v[1].z = RwIm2DGetNearScreenZ(); v[1].rhw = rhw; v[1].emissiveColor = colour;
	v[2].x = -0.5f; v[2].z = RwIm2DGetNearScreenZ(); v[2].rhw = rhw; v[2].emissiveColor = colour;
	v[3].x = width; v[3].z = RwIm2DGetNearScreenZ(); v[3].rhw = rhw; v[3].emissiveColor = colour;
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONALWAYS);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	v[0].y = barHeight - 0.5f;
	v[1].y = barHeight - 0.5f;
	v[2].y = -0.5f;
	v[3].y = -0.5f;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	v[0].y = height;
	v[1].y = height;
	v[2].y = height - barHeight;
	v[3].y = height - barHeight;
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, v, 4);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONGREATER);
}

// 0x415150
// Fades the caption in over 100 ms (m_caption) and out over 500 ms (m_shownCaption), drawn with a function-local text object.
void SpriteManager::DrawCaption()
{
	const char *name = m_caption;
	float alpha = 1.0f;
	if (!name) {
		name = m_shownCaption;
		if (!name)
			return;
		alpha -= (float)((unsigned)TFBClock_GetRawTime() - m_fadeStart) * 0.002f;
		if (alpha <= 0.0f) {
			m_shownCaption = 0;
			return;
		}
	} else if (!m_shownCaption) {
		alpha = (float)((unsigned)TFBClock_GetRawTime() - m_fadeStart) * 0.01f;
		if (alpha >= 1.0f) {
			alpha = 1.0f;
			m_shownCaption = name;
		}
	}
	static Hud2DObject s;
	s.m_flags = 2;
	s.m_font = Hud2D::GetFont(0, "tsa16");
	s.m_boxWidth = 500.0f;
	s.m_boxHeight = 400.0f;
	s.m_anchor = 5;
	s.m_drawY = 440.0f;
	s.m_drawTint.red = 0xa0;
	s.m_drawTint.green = 0x80;
	s.m_drawTint.blue = 0;
	s.m_drawAlpha = alpha;
	s.SetFormat(name);
	float ext[2] = { 32.0f, 32.0f };
	s.MeasureText(&ext[0], &ext[1]);
	s.m_width = ext[0];
	s.m_height = ext[1];
	s.GetAnchorOffset(&s.m_offsetX, 5);
	s.DrawText();
}

// Level 15 (the main menu): two prompt sprites are moved down a little (the game patches their y position by bits).
static void AdjustMainMenuPrompts(GameSprite *spr)
{
	const char *name = spr->m_name;
	if (name && strcmp(name, "prompt_main_menu") == 0 && spr->m_format && strcmp(spr->m_format, "& Pick Action") == 0) {
		if (*(int *)&spr->m_drawY == 0x43D80000)   // 432.0 -> 450.0
			*(int *)&spr->m_drawY = 0x43E10000;
	} else if (name && strcmp(name, "prompt_bkgnd_new") == 0) {
		if (*(int *)&spr->m_drawY == 0x43DB0000 && spr->m_scalePct.x == 0x242)   // 438.0 -> 456.0
			*(int *)&spr->m_drawY = 0x43E40000;
	}
}

// Every tenth pass the visible sprites decide which pad control buffer the game uses (Render's end): a menu frame, the 2-player
// prompt, a gunner game, "Press Start". False when the crane prompt shows: the pass stops drawing there.
static bool NoteControlSprite(GameSprite *spr)
{
	if (g_sprPassCount != 0 || g_sprFlagMenu != 0)
		return true;
	const char *nm = spr->m_name;
	if (nm) {
		const char *menu[16] = { "menu_frame_top", "menu_master", "game_menu_master", "Shop_menu_master", "Quit_Confirmation_Msg",
			"Match_Confirmation_Msg", "Msg_Box_Bot", "AUTO_Msg_Box_Top", "menu_checklist_bkTop", "Menu_Slider_Bkgnd",
			"MEM_Title_menu_title", "MEM_New_Game_title", "MEM_Load_Game_title", "MEM_prompt_select", "Rave_Prompt_O_InstReplay", "" };
		for (int i = 0; i < 16; i++) {
			if (strcmp(nm, menu[i]) == 0) {
				g_sprFlagMenu = 1;
				break;
			}
		}
		if (strcmp(nm, "Rave_2P_2ControllerIn") == 0)
			g_sprFlag2P = 1;
#ifdef ZOOSTERS_OPTIONS
		if (port_options_is_menu_sprite(nm))   // port/options/: our screen's sprites keep the menu control buffer and the cursor
			g_sprFlagMenu = 1;
#endif
	}
	if (g_sprFlagMenu || g_sprFlagGame)
		return true;
	if (nm) {
		const char *game[2] = { "Crosshairs 3D", "Penguin_Gunner" };
		for (int i = 0; i < 2; i++) {
			if (strcmp(nm, game[i]) == 0) {
				g_sprFlagGame = 1;
				return true;
			}
		}
	}
	if (g_sprFlagStart)
		return true;
	if (nm) {
		const char *start[1] = { "Press Start" };
		for (int i = 0; i < 1; i++) {
			if (strcmp(nm, start[i]) == 0) {
				g_sprFlagStart = 1;
				return true;
			}
		}
	}
	if (!g_cranePromptShown && nm && strcmp(nm, "Prompt_Crane_Button") == 0) {
		g_cranePromptShown = 1;
		return false;
	}
	return true;
}

static void DrawSprite(GameSprite *spr)
{
#ifdef ZOOSTERS_WIDESCREEN
	const GameSprite *root = spr;   // HUD scale anchors a sprite group as one: the root of the position-inheritance chain (m_inheritFlags 1)
	for (int depth = 0; depth < 8 && root->m_parent && (root->m_inheritFlags & 1); depth++)
		root = root->m_parent;
	// a 3D billboard (type 3, e.g. the minigame's head icon in Mini_IconFrame) has no box of its own: HUD-scaled as a point, or it keeps
	// the game's place while its frame shrinks toward the corner (_rt2dCTMGet folds the element's mapping into the model's frame)
	float boxH = spr->m_type == 3 && spr->m_height == 0.0f ? 1.0f : spr->m_height;
	port_wide2d_sprite(spr->m_name, spr->m_drawX + spr->m_drawScaleX * spr->m_offsetX, spr->m_width,
	                   spr->m_drawY + spr->m_drawScaleY * spr->m_offsetY, boxH);
	port_wide2d_anchor(root->m_drawX + root->m_drawScaleX * root->m_offsetX, root->m_width, root->m_drawY + root->m_drawScaleY * root->m_offsetY,
	                   root->m_height);
	port_wide2d_hud(g_hudScreenW, g_hudScreenH, &g_hudScreenB, &g_hudScreenD);
	port_wide2d_hud_y(g_hudScreenH, &g_hudScreenC, &g_hudScreenE);   // HUD scale: y around the element's anchor (1:1 unless scaled)
	port_wide2d_trace(spr->m_name);   // PORT_SPRITE_TRACE
#endif
	switch (spr->m_type) {
	case 0:
		if (spr->m_texture) {
			if (spr->ComputeScreenQuad())
				spr->DrawQuad();
		}
		break;
	case 1:
	case 2:
		if (spr->m_font && spr->m_format) {
			if (spr->ComputeScreenQuad())
				spr->DrawText();
		}
		break;
	case 3:
		if (spr->m_extra)
			spr->DrawBillboard();
		break;
	}
#ifdef ZOOSTERS_WIDESCREEN
	port_wide2d_reset();
	port_wide2d_hud(g_hudScreenW, g_hudScreenH, &g_hudScreenB, &g_hudScreenD);
	port_wide2d_hud_y(g_hudScreenH, &g_hudScreenC, &g_hudScreenE);
#endif
}

// Two parts with the camera overlay (fades) between them: in a cut scene (slot 3) only sprites above the cut-scene priority are drawn over
// the overlay.
// Then the caption and the pad control buffer choice.
// 0x415560
void SpriteManager::Render()
{
	RpWorld *world = RWS::CLevel::sm_pActiveWorld;
	if (!world)
		return;
	RwCamera *cam = g_spriteManager ? g_spriteManager->m_camera : 0;
	RpWorldAddCamera(world, cam);
	if (g_sprPassCount++ == 10) {
		g_sprFlagGame = 0;
		g_sprFlag2P = 0;
		g_sprFlagMenu = 0;
		g_sprFlagStart = 0;
		g_cranePromptShown = 0;
		g_sprPassCount = 0;
	}
	Hud2D::BeginFrame(cam);
#ifdef ZOOSTERS_WIDESCREEN   // menu or HUD from the sprites this pass draws (port/fixes/widescreen.cpp)
	port_wide2d_scan_begin();
	for (GameSprite *s = m_renderList; s; s = s->m_renderNext)
		port_wide2d_scan(s->m_name);
#endif
	if (m_letterboxAlpha)
		DrawLetterbox(cam);
	int minPrio = 0;
	if (*CutScene_StackSlot(0) == 3) {
		CameraDrawOverlay(cam);
		minPrio = 0xC0000001;
	}
	GameSprite *spr = m_renderList;
	for (;;) {
		while (spr && spr->m_priorityKey >= minPrio) {
			if (GetCurrentLevel() == 15)
				AdjustMainMenuPrompts(spr);
			if (!NoteControlSprite(spr))
				break;
			DrawSprite(spr);
			spr = spr->m_renderNext;
		}
		if (minPrio == 0xC0000001)
			break;
		minPrio = 0xC0000001;
		if (*CutScene_StackSlot(0) != 3)
			CameraDrawOverlay(cam);
	}
	if (*CutScene_StackSlot(0) != 3)
		DrawCaption();
	Hud2D::EndFrame(cam);
	RpWorldRemoveCamera(world, cam);
	if (g_sprFlagMenu) {
		Pad_SelectControlBuffer(1);
		return;
	}
	if (g_sprFlagGame) {
		Pad_SelectControlBuffer(6);
		return;
	}
	g_cfgKeyReqA = g_sprFlagStart;
	g_cfgKeyReqB = g_sprFlag2P;
	if (GetCurrentLevel() == 14)
		Pad_SelectControlBuffer(2);
	else if (GetCurrentLevel() == 5)
		Pad_SelectControlBuffer(3);
	else if (GetCurrentLevel() == 12)
		Pad_SelectControlBuffer(5);
	else if (GetCurrentLevel() == 13)
		Pad_SelectControlBuffer(4);
	else
		Pad_SelectControlBuffer(0);
}

}}
