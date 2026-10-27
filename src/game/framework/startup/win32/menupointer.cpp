// The mouse in the menus: the window procedure hands every mouse event to MenuPointer_Update.
#include "framework/startup/win32/appstate.h"
#include "modules/tfb/sprites/gamesprite.h"
#include "modules/tfb/sprites/spritemanager_api.h"
#include "rw/rttoolkits.h"
#include "script/ops_message.h"
#include "script/ops_set.h"
#include <float.h>
#include <string.h>
#include "zoosters/os.h"

struct RECT {
	long left, top, right, bottom;
};
struct POINT {
	long x, y;
};
static inline int GetClientRect(void *wnd, RECT *rect)
{
	int w = 0, h = 0;
	int ok = zoosters::os::ClientSize(wnd, &w, &h);
	rect->left = rect->top = 0;
	rect->right = w;
	rect->bottom = h;
	return ok;
}
// Half-open: left / top inside, right / bottom outside.
static inline int PtInRect(const RECT *rect, POINT point)
{
	return point.x >= rect->left && point.x < rect->right && point.y >= rect->top && point.y < rect->bottom;
}
// GLOBAL: 0x62a5bc g_padStickUp
// GLOBAL: 0x62a5be g_padStickLeft
// GLOBAL: 0x62a5bf g_padStickRight
extern unsigned char g_padStickUp; // one-shot stick presses the controller code reads
extern unsigned char g_padStickLeft;
extern unsigned char g_padStickRight;

using RWS::TFB::GameSprite;

static const float kPerScreenUnitX = 0.0015625f;    // 1 / 640: 640 x 480 sprite units to client pixels
static const float kPerScreenUnitY = 0.0020833334f; // 1 / 480
static const int kTintHighlighted = (int)0xFFC0C0C0; // the toggle's chosen side
static const int kTintDimmed = (int)0xFF404040;
static const int kShopListHash = 0x64EE407F;         // name hash of the shop's menu list

// Visible, and its parent too where it inherits visibility.
static bool IsShown(const GameSprite *sprite)
{
	return sprite->m_visible && (!sprite->m_parent || !(sprite->m_inheritFlags & 8) || sprite->m_parent->m_visible);
}

static void ClientSize(int *width, int *height)
{
	RECT client;
	GetClientRect(g_mainWnd, &client);
	*width = client.right - client.left;
	*height = client.bottom - client.top;
}

// The position less none, all or half of the size (anchors right / bottom and centre).
static int AnchorEdge(float position, int size, int part)
{
	if (part == 0)
		return (int)position;
	return (int)(position - (part == 1 ? size : size / 2));
}

// Hit test against the text box of a menu button sprite. No text, and the "VIBRATION" option, never hit. A button with three child
// sprites takes a wider box (150 more on each side, 5 above and below).
// 0x463080
bool HudButtonHitTest(int x, int y, GameSprite *button)
{
	int clientWidth, clientHeight;
	ClientSize(&clientWidth, &clientHeight);
	float posX = button->m_drawX * (float)clientWidth * kPerScreenUnitX;
	float posY = button->m_drawY * (float)clientHeight * kPerScreenUnitY;
	const char *text = button->FormatTimeHMS();
	if (!text || strcmp(text, "VIBRATION") == 0)
		return false;
	float fontHeight = Rt2dFontGetHeight(button->m_font);
	int textHeight = (int)(fontHeight * (float)clientHeight * kPerScreenUnitY);
	int textWidth = (int)(Rt2dFontGetStringWidth(button->m_font, text, fontHeight) * (float)clientWidth * kPerScreenUnitX);
	// Hud2DAnchor 0..8: rows top / bottom / middle, columns left / right / centre
	static const int kHorizontalPart[9] = { 0, 1, 2, 0, 1, 2, 0, 1, 2 };
	static const int kVerticalPart[9] = { 0, 0, 0, 1, 1, 1, 2, 2, 2 };
	int anchor = button->m_justification;
	RECT rect;
	rect.left = 0; // the game leaves left / top unset for an anchor past 8 (none exists)
	rect.top = 0;
	if (anchor <= HUD2D_ANCHOR_CENTRE) {
		rect.left = AnchorEdge(posX, textWidth, kHorizontalPart[anchor]);
		rect.top = AnchorEdge(posY, textHeight, kVerticalPart[anchor]);
	}
	rect.right = rect.left + textWidth;
	rect.bottom = rect.top + textHeight;
	if (button->m_childRefs == 3) {
		rect.left -= 150;
		rect.right += 150;
		rect.top -= 5;
		rect.bottom += 5;
	}
	POINT point;
	point.x = x;
	point.y = y;
	return PtInRect(&rect, point) != 0;
}

static void MoveCursor(int x, int y)
{
	int clientWidth, clientHeight;
	ClientSize(&clientWidth, &clientHeight);
	g_cursorSprite->m_posY = FLT_MAX;
	g_cursorSprite->m_posX = (float)(x * 640 / clientWidth);
	g_cursorSprite->m_posZ = (float)(y * 480 / clientHeight);
	Script::InvalidateSprite(g_cursorSprite);
}

// While a memory-card error is shown, the next click or key quits (the window procedure reads g_cfgQuitOnInput).
static void CheckCardErrors()
{
	static const char *const kErrors[] = { "MEM Error no space", "MEM_create_failed", "MEM error unknown", "MEM error access" };
	for (int i = 0; i < 4; i++) {
		GameSprite *error = FindSpriteByName(kErrors[i]);
		if (error && IsShown(error))
			g_cfgQuitOnInput = 1;
	}
}

// The pointer's position across the slider box sets the slider variable (0..100); a stick press in the direction it moved lets the
// menu script follow. True when the pointer is on the slider.
static bool DragSlider(GameSprite *slider, int x, int y, Script::ScriptReference *owner)
{
	int clientWidth, clientHeight;
	ClientSize(&clientWidth, &clientHeight);
	int height = (int)(slider->m_boxHeight * slider->m_scaleY * (float)clientHeight * kPerScreenUnitY);
	int width = (int)(slider->m_boxWidth * slider->m_scaleX * (float)clientWidth * kPerScreenUnitX);
	RECT rect;
	rect.left = (int)((float)clientWidth * slider->m_drawX * kPerScreenUnitX - (float)(width / 2));
	rect.top = (int)((float)clientHeight * slider->m_drawY * kPerScreenUnitY - (float)(height / 2));
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
	POINT point;
	point.x = x;
	point.y = y;
	if (!PtInRect(&rect, point))
		return false;
	Script::UserReference *value = g_userRefSlots[MENUVAR_SLIDER];
	float before = value->GetFloat(owner);
	value->SetValue((int)((x - rect.left) * 100 / (rect.right - rect.left)), owner);   // (int): a long argument is ambiguous between the int and float overloads outside cl 13.10
	float after = value->GetFloat(owner);
	if (after > before)
		g_padStickRight = 1;
	else if (after < before)
		g_padStickLeft = 1;
	return true;
}

static void ChooseToggleSide(GameSprite *marker, GameSprite *chosen, GameSprite *on, GameSprite *off, int value,
                             Script::ScriptReference *owner)
{
	marker->m_posX = chosen->m_posX;
	marker->m_posY = FLT_MAX;
	g_userRefSlots[MENUVAR_TOGGLE]->SetValue(value, owner);
	on->m_tintPacked = value ? kTintHighlighted : kTintDimmed;
	off->m_tintPacked = value ? kTintDimmed : kTintHighlighted;
	Script::InvalidateSprite(on);
	Script::InvalidateSprite(off);
}

// Flags the change for the menu script.
static void SelectItem(int list, int item, Script::ScriptReference *owner)
{
	Script::UserReference *selection = g_userRefSlots[MENUVAR_SELECTION + list];
	if (selection->GetInt(owner) == item)
		return;
	selection->SetValue(item, owner);
	Script::UserReference *changed = g_userRefSlots[MENUVAR_SELECTION_CHANGED + list];
	if (changed)
		changed->SetValue(1, owner);
}

// The four items sit in fixed bands of the screen.
static void SelectShopItem(const GameSprite *item, Script::ScriptReference *owner)
{
	static const float kBands[4][2] = { { 117.0f, 161.0f }, { 192.0f, 236.0f }, { 267.0f, 311.0f }, { 342.0f, 386.0f } };
	for (int i = 0; i < 4; i++) {
		if (item->m_drawY >= kBands[i][0] && item->m_drawY <= kBands[i][1]) {
			SelectItem(0, i + 1, owner);
			return;
		}
	}
}

// The sprites of each live subset are items 1, 2, ...; a list stops at its first hidden item, and the shop's back button only
// counts in its usual place. The item under the pointer is selected in all lists that exist.
static bool PointAtListItem(int x, int y, Script::ScriptReference *owner)
{
	for (int i = 0; i < Script::g_subsetTableSize; i++) {
		Script::ScriptSubset *subset = Script::g_subsetTable[i];
		if (!subset)
			continue;
		int item = 1;
		for (Script::SetNode *node = subset->m_head; node; node = node->m_next, item++) {
			GameSprite *sprite = static_cast<GameSprite *>(node->m_object);
			if (item == 1 && sprite->m_name && strcmp(sprite->m_name, "Shop_menu_back") == 0 && sprite->m_drawY != 104.0f)
				break;
			if (!IsShown(sprite))
				break;
			if (!HudButtonHitTest(x, y, sprite))
				continue;
			if ((uintptr_t)subset->GetName() == (uintptr_t)(unsigned)kShopListHash) {
				SelectShopItem(sprite, owner);
				return true;
			}
			SelectItem(0, item, owner);
			if (g_userRefSlots[MENUVAR_SELECTION + 1])
				SelectItem(1, item, owner);
			if (g_userRefSlots[MENUVAR_SELECTION + 2])
				SelectItem(2, item, owner);
			return true;
		}
	}
	return false;
}

static bool ButtonHit(GameSprite *button, int x, int y)
{
	return button && IsShown(button) && HudButtonHitTest(x, y, button);
}

// The brighter of the two frame sprites marks the highlighted button; if that is the other one, a stick-up press moves the highlight.
static void HighlightDialogueButton(const char *ownFrame, const char *otherFrame)
{
	GameSprite *own = FindSpriteByName(ownFrame);
	GameSprite *other = FindSpriteByName(otherFrame);
	if (own && other && other->m_tintPacked > own->m_tintPacked)
		g_padStickUp = 1;
}

// Two layouts: MEM_yes/no_* and MEM_sel_*.
static bool PointAtDialogueButton(int x, int y)
{
	if (ButtonHit(FindSpriteByName("MEM_yes/no_okay"), x, y))
		return true;
	if (ButtonHit(FindSpriteByName("MEM_sel_okay"), x, y))
		return true;
	if (ButtonHit(FindSpriteByName("MEM_yes/no_yes"), x, y)) {
		HighlightDialogueButton("MEM_yes/no_frame_00", "MEM_yes/no_frame_01");
		return true;
	}
	if (ButtonHit(FindSpriteByName("MEM_yes/no_no"), x, y)) {
		HighlightDialogueButton("MEM_yes/no_frame_01", "MEM_yes/no_frame_00");
		return true;
	}
	if (ButtonHit(FindSpriteByName("MEM_sel_yes"), x, y)) {
		HighlightDialogueButton("MEM_sel_frame_0", "MEM_sel_frame_1");
		return true;
	}
	if (ButtonHit(FindSpriteByName("MEM_sel_no"), x, y)) {
		HighlightDialogueButton("MEM_sel_frame_1", "MEM_sel_frame_0");
		return true;
	}
	return false;
}

// A pad button glyph in a text (* ^ & @) presses that button (1, 3, 0, 2) through the window request the controller code reads.
static void ClickButtonGlyph(int x, int y)
{
	for (Script::SetNode *node = Script::TFB_GetActiveSpriteSet()->m_head; node; node = node->m_next) {
		GameSprite *sprite = static_cast<GameSprite *>(node->m_object);
		const char *text = sprite->FormatTimeHMS();
		if (!text || !IsShown(sprite) || !HudButtonHitTest(x, y, sprite))
			continue;
		int button;
		if (strchr(text, '*'))
			button = 1;
		else if (strchr(text, '^'))
			button = 3;
		else if (strchr(text, '&'))
			button = 0;
		else if (strchr(text, '@'))
			button = 2;
		else
			continue;
		g_wndRequestA = button;
		g_wndRequestB = 0;
		g_wndRequestFlag = 1;
		return;
	}
}

#ifdef ZOOSTERS_WIDESCREEN
extern "C" int port_wide2d_mouse_x(int x, int clientW, int clientH);   // port/fixes/widescreen.cpp
#endif

// down: button held; click: a press or release. The first widget on the screen takes the event: the slider with the button held,
// else the toggle, else the menu lists; then the dialogue buttons. Returns 1 when the pointer is on one of them (on a release the
// window procedure then presses confirm).
// The game also looks up menu_list_more_up / _down and doesn't use them.
// 0x463640
int MenuPointer_Update(int x, int y, bool down, bool click)
{
#ifdef ZOOSTERS_WIDESCREEN   // the pointer as a pixel of the stretched 640 x 480 grid
	{
		int clientWidth, clientHeight;
		ClientSize(&clientWidth, &clientHeight);
		x = port_wide2d_mouse_x(x, clientWidth, clientHeight);
	}
#endif
	if (g_cursorSprite)
		MoveCursor(x, y);
	Script::ScriptReference owner;
	GameSprite *slider = FindSpriteByName("Menu_Slider_Marks");
	GameSprite *on = FindSpriteByName("Menu_Options_On");
	GameSprite *off = FindSpriteByName("Menu_Options_Off");
	GameSprite *marker = FindSpriteByName("Menu_Toggle_Marker");
	CheckCardErrors();
	if (down && slider && IsShown(slider)) {
		if (DragSlider(slider, x, y, &owner))
			return 1;
	} else if (on && IsShown(on)) {
		if (HudButtonHitTest(x, y, on)) {
			ChooseToggleSide(marker, on, on, off, 1, &owner);
			return 1;
		}
		if (HudButtonHitTest(x, y, off)) {
			ChooseToggleSide(marker, off, on, off, 0, &owner);
			return 1;
		}
	} else if (g_userRefSlots[MENUVAR_SELECTION] && g_userRefSlots[MENUVAR_SELECTION_CHANGED]) {
		if (PointAtListItem(x, y, &owner))
			return 1;
	}
	if (PointAtDialogueButton(x, y))
		return 1;
	if (!down && click)
		ClickButtonGlyph(x, y);
	return 0;
}
