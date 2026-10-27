// Unit axes for the HUD 2D rotation.
#include "rw/rwcore.h"   // RwV3d (src/game)

// Unnamed 2D/HUD literals around the axes (default names, button sprite and image names).

// GLOBAL: 0x5d5494 g_hudNamesA
extern const char g_hudNamesA[12] = "<DEFAULT>";
// GLOBAL: 0x5d54a0 g_hudNamesA_tail
extern const char g_hudNamesA_tail[16] = "SB_ice_stop.anm";

// X (tilt), Y (facing), Z (roll), passed to RwFrameRotate. Three objects instead of one float[9]: a name cannot alias into
// a src/data object at link time. Kept in address order so they stay adjacent.
// GLOBAL: 0x5d54b0 DAT_005d54b0
extern const RwV3d DAT_005d54b0 = { 1.0f, 0.0f, 0.0f };   // X axis
// GLOBAL: 0x5d54bc DAT_005d54bc
extern const RwV3d DAT_005d54bc = { 0.0f, 1.0f, 0.0f };   // Y axis
// GLOBAL: 0x5d54c8 DAT_005d54c8
extern const RwV3d DAT_005d54c8 = { 0.0f, 0.0f, 1.0f };   // Z axis

extern const char g_strTfbScript[12];

struct HudNamesB
{
	const char *m_typeName;
	float m_k0;
	float m_k1;
	char m_dataImagesCirclePng[24];
	char m_circlesprite[16];
	char m_dataImagesCircleSmallPng[28];
	char m_spriteOButton[16];
	char m_dataImagesSquarePng[24];
	char m_squaresprite[16];
	char m_dataImagesSquareSmallPng[28];
	char m_spriteSquareButton[24];
	char m_dataImagesTrianglePng[28];
	char m_trianglesprite[16];
	char m_dataImagesTriangleSmallPng[32];
	char m_spriteTriangleButton[24];
	char m_dataImagesCrossPng[24];
	char m_xsprite[8];
	char m_dataImagesCrossSmallPng[28];
	char m_spriteXButton[16];
	char m_dataImagesMousePointerPng[32];
	char m_mousePointer[16];
	char m_rwID2dfont[12];
	char m_02d02d02d[16];
};

// ALONE: g_hudNamesB literals reached by value; followed by a vtable block
// GLOBAL: 0x5d54d4 g_hudNamesB
extern const HudNamesB g_hudNamesB = {
	g_strTfbScript,
	2000.0f,
	20.2000008f,
	"data\\Images\\Circle.png",
	"CIRCLESPRITE",
	"data\\Images\\CircleSmall.png",
	"SPRITE_O_BUTTON",
	"Data\\Images\\Square.png",
	"SQUARESPRITE",
	"Data\\Images\\SquareSmall.png",
	"SPRITE_SQUARE_BUTTON",
	"Data\\Images\\Triangle.png",
	"TRIANGLESPRITE",
	"Data\\Images\\TriangleSmall.png",
	"SPRITE_TRIANGLE_BUTTON",
	"Data\\Images\\Cross.png",
	"XSPRITE",
	"Data\\Images\\CrossSmall.png",
	"SPRITE_X_BUTTON",
	"Data\\Images\\MousePointer.png",
	"MOUSE_POINTER",
	"rwID_2DFONT",
	"%02d:%02d:%02d",
};
