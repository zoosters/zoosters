// Mouse pointer sprite ("bx_MousePointer") show / hide, and the pad control buffer selection that drives it.
#include "framework/startup/win32/input/inputmanager.h"
#include "framework/startup/win32/appstate.h"
#include "modules/tfb/sprites/spritemanager.h"
#include "zoosters/os.h"

struct RectC {
	long left, top, right, bottom;
};
struct PointC {
	long x, y;
};
static inline int GetClientRect(void *wnd, RectC *rc)
{
	int w = 0, h = 0;
	int ok = zoosters::os::ClientSize(wnd, &w, &h);
	rc->left = rc->top = 0;
	rc->right = w;
	rc->bottom = h;
	return ok;
}
static inline int GetCursorPos(PointC *pt)
{
	int x = 0, y = 0;
	int ok = zoosters::os::CursorPos(&x, &y);
	pt->x = x;
	pt->y = y;
	return ok;
}

// 0x462eb0
void Cursor_Show(unsigned char show)
{
	if (g_cursorShown == show)
		return;
	g_cursorShown = show;
	if (show) {
		RectC rc;
		GetClientRect(g_mainWnd, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		if (!g_cursorSprite) {
			g_cursorSprite = SpriteMgr_CreateTextSprite("bx_MousePointer", "", 0.0f, 0.0f, 40960.0f / w, 30720.0f / h);
			g_cursorSprite->SetSpriteByName("MOUSE_POINTER");
			g_cursorSprite->m_type = 0;
			g_cursorSprite->m_tint.red = 0x80;
			g_cursorSprite->m_tint.green = 0x80;
			g_cursorSprite->m_tint.blue = 0x80;
			g_cursorSprite->m_tint.alpha = 0xff;
		}
		PointC pt;
		GetCursorPos(&pt);
		g_cursorSprite->m_visible = g_cursorShown;
		int y = pt.y * 480 / h;
		Hud2DObject &hud = *g_cursorSprite;
		int x = pt.x * 640 / w;
		hud.m_posY = 3.4028235e+38f;
		hud.m_posX = (float)x;
		hud.m_posZ = (float)y;
		Script::InvalidateSprite(g_cursorSprite);
	} else {
		if (g_cursorSprite)
			RemoveSpriteByName("bx_MousePointer");
		g_cursorSprite = 0;
	}
}

// 0x463030
void Cursor_HideSave()
{
	g_cursorSaved = g_cursorShown;
	if (g_cursorShown) {
		g_cursorShown = 0;
		if (g_cursorSprite)
			RemoveSpriteByName("bx_MousePointer");
		g_cursorSprite = 0;
	}
}

// 0x463070
void Cursor_Restore()
{
	Cursor_Show(g_cursorSaved);
}

extern int g_ctlSelA;                    // GLOBAL: 0x62a520 g_ctlSelA
extern int g_ctlSelB;                    // GLOBAL: 0x617b9c g_ctlSelB
extern CtlBuf *g_ctlBuffers[9];          // GLOBAL: 0x617ba0 g_ctlBuffers

// The first call only latches idx; a repeat installs the buffer and sets the cursor visibility from its m_showCursor.
// 0x461db0
void Pad_SelectControlBuffer(int idx)
{
	if (g_ctlSelA != idx) {
		g_ctlSelA = idx;
		return;
	}
	if (idx != g_ctlSelB) {
		g_inputManager->SetPlayerContext(0, g_ctlBuffers[idx]);
		g_ctlSelB = idx;
		Cursor_Show(g_ctlBuffers[idx]->m_showCursor);
	}
}
