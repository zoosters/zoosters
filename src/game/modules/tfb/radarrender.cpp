#include <math.h>
#include "modules/tfb/radarrender.h"
#include "modules/tfb/camera_views.h"
#include "tfb/render/shadow.h"
#include "rw/rwcore.h"
#include "modules/tfb/gamecamera.h"

// vtables of the two bases and the BIGCIRCLE texture it shares with ShadowRender
// GLOBAL: 0x5d4d48 RadarRender::vftable_for_CEventHandler
// GLOBAL: 0x5d4d38 RadarRender::vftable_for_CAttributeHandler
// 0x442940
// 0x4426d0
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x6223d0 g_bigCircleTexture
// GLOBAL: 0x6223e8 g_radarRender
// GLOBAL: 0x6034dc g_radarRenderClassId
// GLOBAL: 0x6223ac g_radarHeight
// GLOBAL: 0x6223b0 g_radarWidth
// GLOBAL: 0x6034cc g_radarRange
// GLOBAL: 0x6034c4 g_radarSize
// GLOBAL: 0x5caea4 g_radarAspect
// the blip draw methods are called by Draw with `this` in ecx, as the game does
// GLOBAL: 0x4422b0 ?Draw@RadarRender@@QAEXXZ
// GLOBAL: 0x441e40 RadarRenderToCamera

// The game's RenderWare is nothrow (its headers are extern "C"): cl 13.10 emits no exception state around these calls.

void CameraGetPosition(RwV3d *out);

float CameraGetHeading();             // 0x41e6b0 the camera's heading in degrees

// 0x441fb0
RadarRender::RadarRender(const RWS::CAttributePacket &) : m_alpha(0.6f), m_unk70(7)
{
	m_control = 0;
	m_unk94 = 0.15f;
	m_blips = 0;
	m_numBlips = 0;
	m_unkA4 = 0;
	g_radarRender = this;
	g_bigCircleTexture = RwTextureRead("BIGCIRCLE", 0);
}

// 0x442070
RadarRender::~RadarRender()
{
	if (g_bigCircleTexture) {
		RwTextureDestroy(g_bigCircleTexture);
		g_bigCircleTexture = 0;
	}
	g_radarRender = 0;
}

// Attribute 3 is the only one the radar reads: the "radar enable display" switch.
// 0x442850
void RadarRender::HandleAttributes(const RWS::CAttributePacket &packet)
{
	RWS::CAttributeHandler::HandleAttributes(packet);
	for (RWS::CAttributeCommandIterator it(packet, g_radarRenderClassId); !it.IsFinished(); ++it) {
		if (it->GetCommandId() == 3)
			m_control = it->GetAs_RwInt32();
	}
}

// On the render message (while the radar is enabled): sets the radar camera's frame buffer to a sub raster of the main camera's
// (unless the radar has a raster of its own), draws the blips, draws the own raster over the screen, then forgets the blips.
#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_rect(float W, float H, float *x, float *w);   // port/fixes/widescreen.cpp
#endif
// 0x4426e0
void RadarRender::HandleEvents(RWS::CMsg &msg)
{
	if (m_control != 0 && msg.Id == m_evtRender.pEventId) {
		RwCamera *camera = (RwCamera *)msg.pData;
		if (m_target == 0) {
			RwRaster *parent = CameraView(camera)->frameBuffer;
			float w = (float)RasterView(parent)->width;
			float h = (float)RasterView(parent)->height;
			RwRect r;
			r.x = (int)(w * m_screenX);
			r.y = (int)(h * m_screenY);
			float t = g_radarSize * w;
			r.w = (int)(t * g_radarAspect);
			if ((float)(r.x + r.w) > w)
				r.w = (int)w - r.x;
			r.h = (int)(g_radarSize * h);
			if ((float)(r.y + r.h) > h)
				r.h = (int)h - r.y;
#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only (port/fixes/widescreen.cpp): square on any screen, at its side edge
			{
				float x = w * m_screenX, rw = g_radarSize * w * g_radarAspect;
				port_wide2d_rect(w, h, &x, &rw);
				r.x = (int)x;
				r.w = (int)rw;
			}
#endif
			RwRasterSubRaster(m_subRaster, parent, &r);
			CameraView(m_camera)->frameBuffer = m_subRaster;
		}
		Draw();
		if (m_target)
			RadarRenderToCamera(m_target, &m_screenX, camera, m_alpha);
		m_prevPosition = m_position;
		m_position.z = 0;
		m_position.y = 0;
		m_position.x = 0;
		m_numBlips = 0;
		m_unkA4 = 0;
	}
}

// 0x442110
void RadarRender::DrawBlipDot(RadarBlip *blip, float x, float y)
{
	x = g_radarWidth * x - 1.5f;
	y = g_radarHeight * y - 1.5f;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);   // no texture
	RadarDrawRect(x, y, x + 3.0f, y + 3.0f, blip->m_color);
}

// 0x442190
void RadarRender::DrawBlipCone(RadarBlip *blip, float x, float y)
{
	float radius = blip->m_distance / g_radarRange;
	float halfDeg = blip->m_coneAngle * 0.5f;
	float halfRad = halfDeg * 0.017453292f;
	float tipRadius = (blip->m_coneAngle * 0.0046023726f + 1.0f) * radius;
	float dirRad = (blip->m_heading - CameraGetHeading()) * 0.017453292f;
	float x1 = x - (float)sin(halfRad + dirRad) * radius;
	float y1 = y - (float)cos(halfRad + dirRad) * radius;
	float x2 = x - (float)sin(dirRad) * tipRadius;
	float y2 = y - (float)cos(dirRad) * tipRadius;
	float x3 = x - (float)sin(dirRad - halfRad) * radius;
	float y3 = y - (float)cos(dirRad - halfRad) * radius;
	RwRaster *raster = g_bigCircleTexture ? TextureView(g_bigCircleTexture)->raster : 0;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
	RadarDrawCone(x, y, x1, y1, x2, y2, x3, y3, blip->m_coneAngle, blip->m_color);
}

// A cone wider than 90 degrees is drawn as several narrower ones (one to three). The radar is oriented with the camera heading: the blip's
// offset is rotated by it
// and drawn relative to the radar's centre (0.5, 0.5).
// 0x4422b0
void RadarRender::Draw()
{
	RwV3d pos;
	CameraGetPosition(&pos);
	m_position = pos;
	if (m_target != 0) {
		RwRasterPushContext(m_target);
		RwRGBA c;
		c.blue = 0;
		c.green = 0;
		c.red = 0;
		c.alpha = (unsigned char)(int)(m_alpha * 255.0f);
		RwRasterClear(RwRGBAToPixel(&c, RasterView(m_target)->format << 8));
		RwRasterPopContext();
	}
	if (!RwCameraBeginUpdate(m_camera))
		return;
	SaveRenderStates();
	SetRadarRenderStates();
	RwRasterView *frameBuffer = RasterView(CameraView(m_camera)->frameBuffer);
	g_radarWidth = (float)frameBuffer->width;
	g_radarHeight = (float)frameBuffer->height;
	if (m_target == 0) {
		RwRGBA c;
		c.blue = 0;
		c.green = 0;
		c.red = 0;
		c.alpha = (unsigned char)(int)(m_alpha * 255.0f);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
		RadarDrawRect(0, 0, g_radarWidth, g_radarHeight, *(int *)&c);
	}
	for (int i = 0; i < m_numBlips; i++) {
		RadarBlip *b = &m_blips[i];
		float dx = b->m_position.x - pos.x;
		float dz = b->m_position.z - pos.z;
		float dist = sqrt(dx * dx + dz * dz) / g_radarRange;
		if (dist > 1.0f)
			continue;
		float ang = (float)atan2(-dz, dx);
		float r = ang - (CameraGetHeading() + 90.0f) * 0.017453292f;
		float screenX = 0.5f - sin(r) * dist;
		float screenY = 0.5f - cos(r) * dist;
		if (b->m_kind == 0) {
			if ((pos.z - b->m_position.z) * (pos.z - b->m_position.z) + (pos.x - b->m_position.x) * (pos.x - b->m_position.x) < g_radarRange * g_radarRange)   // skip when K^2 <= sum (strict)
				DrawBlipDot(b, screenX, screenY);
		} else if (b->m_kind == 1) {
			if (!((pos.z - b->m_position.z) * (pos.z - b->m_position.z) + (pos.x - b->m_position.x) * (pos.x - b->m_position.x) < g_radarRange * g_radarRange))   // skip when K^2 <= sum
				continue;
			if (b->m_coneAngle <= 90.0f) {
				DrawBlipCone(b, screenX, screenY);
			} else if (b->m_coneAngle <= 180.0f) {
				RadarBlip t = *b;
				t.m_coneAngle = t.m_coneAngle * 0.5f;
				float h = t.m_coneAngle * 0.5f;
				t.m_heading = b->m_heading - h;
				DrawBlipCone(&t, screenX, screenY);
				t.m_heading = h + b->m_heading;
				DrawBlipCone(&t, screenX, screenY);
			} else {
				RadarBlip t = *b;
				t.m_heading = b->m_heading + 45.0f;
				t.m_coneAngle = 90.0f;
				DrawBlipCone(&t, screenX, screenY);
				t.m_heading = b->m_heading - 45.0f;
				DrawBlipCone(&t, screenX, screenY);
				float rr = b->m_coneAngle > 360.0f ? 360.0f : b->m_coneAngle;
				float q = (rr - 180.0f) * 0.25f;
				t.m_coneAngle = q * 0.5f;
				t.m_heading = q + b->m_heading + 90.0f;
				DrawBlipCone(&t, screenX, screenY);
				t.m_heading = b->m_heading - 90.0f - q;
				DrawBlipCone(&t, screenX, screenY);
			}
		}
	}
	RestoreRenderStates();
	RwCameraEndUpdate(m_camera);
}
