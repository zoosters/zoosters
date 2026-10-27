// 2D HUD layer: Hud2DObject (the text / sprite / billboard object every GameSprite embeds) and the Hud2D free functions.
#ifndef ZOOSTERS_TFB_HUD_HUD2D_H
#define ZOOSTERS_TFB_HUD_HUD2D_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rttoolkits.h"

struct FontCacheEntry {              // capacity 10
	const char *m_name;
	Rt2dFont *m_font;
	int m_refCount;
};
extern FontCacheEntry g_fontCache[10];
extern int g_fontCacheCount;
extern Rt2dBrush *g_hudBrush;
extern char g_hudText[];

// Hud2DObject::m_anchor: which point of the object's box its position gives (screen y grows downwards).
enum Hud2DAnchor {
	HUD2D_ANCHOR_TOP_LEFT = 0,
	HUD2D_ANCHOR_TOP_RIGHT = 1,
	HUD2D_ANCHOR_TOP_CENTRE = 2,
	HUD2D_ANCHOR_BOTTOM_LEFT = 3,
	HUD2D_ANCHOR_BOTTOM_RIGHT = 4,
	HUD2D_ANCHOR_BOTTOM_CENTRE = 5,
	HUD2D_ANCHOR_MIDDLE_LEFT = 6,
	HUD2D_ANCHOR_MIDDLE_RIGHT = 7,
	HUD2D_ANCHOR_CENTRE = 8
};

// Text / sprite object; second base of GameSprite, also used on its own (the SpriteManager's caption). The m_draw* members are the per-frame state GameSprite::Resolve computes from the m_* inputs and the parent sprite. Flags: 1 / 2 = fixed box size, 4 = "%d" in the format, 0x10 = "%t".
struct Hud2DObject {
	unsigned m_flags;
	RwTexture *m_texture;
	Rt2dFont *m_font;
	RpClump *m_clump;                // clump of a model sprite (DrawBillboard reads its frame and atomic list)
	const char *m_format;            // the sprite's text
	unsigned char m_cullBack;        // back faces culled, else front faces
	char m_pad15[3];
	int m_anchor;                    // anchor / alignment: the kind GetAnchorOffset takes (= the sprite's justification)
	float m_posX;                    // screen x, or world x
	float m_posY;                    // FLT_MAX: screen sprite; else world y (ProjectToScreen)
	float m_posZ;                    // screen y, or world z
	float m_scaleX;                  // scale (percent * 0.01)
	float m_scaleY;
	float m_rotation;
	float m_boxWidth;                // max width (text box; texture size; fixed size)
	float m_boxHeight;
	union {
		RwRGBA m_tint;               // R G B A
		int m_tintPacked;            // the same four bytes as one int (R in the low byte)
	};
	float m_roll, m_tilt, m_facing;  // model rotation, degrees (DrawBillboard rotates about z / x / y)
	float m_offsetX;                 // anchor offset (GetAnchorOffset out)
	float m_offsetY;
	unsigned char m_drawVisible;
	char m_pad55[3];
	float m_drawX;                   // resolved position (640x480 virtual screen)
	float m_drawY;
	float m_drawScaleX;              // resolved scale
	float m_drawScaleY;
	float m_drawRotation;
	float m_drawAlpha;               // resolved alpha (0..1)
	float m_width;                   // size times resolved scale
	float m_height;
	RwRGBA m_drawTint;               // resolved colour (its alpha byte is not used)
	int m_drawPriority;
	int m_value;                     // value for %d / %t
	float m_distanceScale;           // distance fade of a world sprite

	Hud2DObject() { Init(); }
	~Hud2DObject();  // (empty; the game's copy is shared with CEventId's destructor)

	void Init();
	void SetTexture(RwTexture *texture);
	void SetCullBack(unsigned char v);
	void SetSizeFromTexture();
	bool ComputeScreenQuad();  // true = visible
	void DrawQuad();
	void DrawBillboard();
	void SetFormat(const char *fmt);
	void SetSpriteByName(const char *name);
	const char *FormatTimeHMS();
	void GetAnchorOffset(float *out, int anchor);
	void DrawText();
	float MeasureText(float *w, float *h);  // returns a scale in st0
};
ZOOSTERS_ASSERT_SIZE(Hud2DObject, 0x88);
ZOOSTERS_ASSERT_OFFSET(Hud2DObject, m_anchor, 0x18);
ZOOSTERS_ASSERT_OFFSET(Hud2DObject, m_posX, 0x1c);
ZOOSTERS_ASSERT_OFFSET(Hud2DObject, m_tint, 0x3c);
ZOOSTERS_ASSERT_OFFSET(Hud2DObject, m_offsetX, 0x4c);
ZOOSTERS_ASSERT_OFFSET(Hud2DObject, m_drawVisible, 0x54);
ZOOSTERS_ASSERT_OFFSET(Hud2DObject, m_drawTint, 0x78);
ZOOSTERS_ASSERT_OFFSET(Hud2DObject, m_distanceScale, 0x84);

namespace Hud2D {
void DestroyFontCache();
void Close();
void Open();
Rt2dFont *GetFont(Rt2dFont *res, const char *name);
void ReleaseFont(const char *name);
void SetRenderStates();
bool BeginFrame(RwCamera *cam);
RwCamera *EndFrame(RwCamera *cam);
RwTexture *LoadSpriteTexture(const char *name);
#ifdef ZOOSTERS_OPTIONS
void RescaleFonts(float ratio);
#endif
}

#endif
