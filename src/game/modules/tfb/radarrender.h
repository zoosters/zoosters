// Not in the PS2 class list: base order assumed as ShadowRender / GameController.
// Vtables 0x5d4d48 (CEventHandler part: ~ 0x442940, HandleEvents 0x4426e0) and 0x5d4d38 (CAttributeHandler part at +8:
// thunk 0x4426d0, HandleAttributes 0x442850, base, base). HandleAttributes lives in the secondary vtable only, so the
// compiler (cl 13.10 as the game's) compiles its body with `this` = the CAttributeHandler sub-object, no thunk.
#ifndef ZOOSTERS_MODULES_TFB_RADARRENDER_H
#define ZOOSTERS_MODULES_TFB_RADARRENDER_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"

struct RadarBlip {
	int m_kind;           // +0x00 0 dot (3x3 pixels), 1 cone
	RwV3d m_position;     // +0x04 world position (x and z are drawn)
	int m_color;          // +0x10 packed RGBA, red in the low byte
	float m_heading;      // +0x14 degrees: the direction of the cone
	float m_coneAngle;    // +0x18 degrees: the cone spans the heading +- half of it
	float m_distance;     // +0x1c length of the cone in world units
};
ZOOSTERS_ASSERT_SIZE(RadarBlip, 0x20);

class RadarRender : public RWS::CEventHandler, public RWS::CAttributeHandler, public RWS::LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(RadarRender);
	RadarRender(const RWS::CAttributePacket &attr);
	~RadarRender();
	virtual void HandleEvents(RWS::CMsg &msg);
	virtual void HandleAttributes(const RWS::CAttributePacket &attr);
	void Draw();
	void DrawBlipDot(RadarBlip *blip, float x, float y);   // kind 0, a 3x3 quad at (x, y) in 0..1 radar coordinates
	void DrawBlipCone(RadarBlip *blip, float x, float y);   // kind 1, the cone with the BIGCIRCLE texture

	char m_pad24[0x48];                 // +0x24 unused
	float m_alpha;                      // +0x6c opacity of the radar background, 0..1 (0.6; Draw scales it by 255)
	int m_unk70;  // +0x70 7 from the constructor; never read
	RWS::CEventId m_evtRender;          // +0x74 the message that triggers the radar render (HandleEvents; the main camera's)
	RwRaster *m_target;                 // +0x7c the raster the radar is drawn into first (0: only the sub raster is set up)
	RwRaster *m_subRaster;              // +0x80 sub raster of the main camera's frame buffer the radar camera draws into
	RwCamera *m_camera;                 // +0x84 the radar camera
	int m_control;                      // +0x88 script "radar enable display" (ExposeRadarControl); attribute 3. 0: no radar
	float m_screenX;                    // +0x8c position of the radar as a fraction of the screen (RadarRenderToCamera reads x, y as a pair)
	float m_screenY;                    // +0x90
	float m_unk94;  // +0x94 0.15 from the constructor; never read
	int m_unk98;  // +0x98 never accessed
	RadarBlip *m_blips;                 // +0x9c the blip array (left by the actors each frame)
	int m_numBlips;                     // +0xa0 entries used (cleared after each render)
	int m_unkA4;  // +0xa4 cleared by the constructor and after each render; never read
	RwV3d m_position;                   // +0xa8 the camera position of this frame's Draw (CameraGetPosition)
	RwV3d m_prevPosition;               // +0xb4 the camera position of the previous frame
};
ZOOSTERS_ASSERT_SIZE(RadarRender, 0xc0);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_alpha, 0x6c);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_evtRender, 0x74);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_target, 0x7c);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_camera, 0x84);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_control, 0x88);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_screenX, 0x8c);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_blips, 0x9c);
ZOOSTERS_ASSERT_OFFSET(RadarRender, m_position, 0xa8);

extern RadarRender *g_radarRender;   // the instance (0 while there is none)
extern const char *g_radarRenderClassId;

// The 2D drawing of the radar (radarrender_draw.cpp). The radar is drawn in pixels of the radar camera's frame buffer: x * width.
extern float g_radarRange;   // 3600.0: world units from the camera to the edge of the radar
extern float g_radarSize;   // 0.15: the radar's height as a fraction of the screen's
extern const float g_radarAspect;   // 0.75: its width is size * aspect of the screen's width
extern float g_radarWidth;   // width of the radar camera's frame buffer in pixels (set by Draw / RadarRenderToCamera)
extern float g_radarHeight;   // its height
int RadarDrawTexturedQuad(float left, float top, float right, float bottom, float screenZ, float recipZ, float uv);
int RadarDrawRect(float left, float top, float right, float bottom, int rgba);   // an untextured quad
int RadarDrawCone(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float angle, int rgba);
// Draws `target` over the screen at the radar's position in the camera `camera` (0x441e40; the game ignores the 4th argument, the
// opaque alpha 0xFF is handed to RadarDrawTexturedQuad in eax).
void RadarRenderToCamera(RwRaster *target, float *position, RwCamera *camera, float alpha);
void SaveRenderStates();
void SetRadarRenderStates();
void RestoreRenderStates();

#endif
