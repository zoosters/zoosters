// Per-actor shadows: ShadowPass::Visit decides what to draw, ShadowProject projects the shadow map (or the BIGCIRCLE ring) onto the world, ShadowDrawHead draws the far-away blob, ShadowPipelineRender is the sector pipeline's render callback.
#include <float.h>
#include "tfb/render/shadow.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/gameactor.h"
#include "modules/tfb/shadowrender.h"
#include "modules/tfb/tfbshadowcamera.h"
#include "tfb/corpus/corpus.h"
#include "tfb/render/tfbmaterial.h"
#include "rw/rpcollis.h"
#include "rw/rwd3d8.h"
#include "rw/rpsectorview.h"
#include "tfb/render/shadow_pass_views.h"

using namespace RWS::TFB;

// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x5ca758 g_twelveW3a
// GLOBAL: 0x5d4d70 DAT_005d4d70
// GLOBAL: 0x5d4d7c DAT_005d4d7c
// GLOBAL: 0x62a5c8 g_resWidth
// GLOBAL: 0x62a5cc g_resHeight
// GLOBAL: 0x622458 g_shadowPassTexture
// GLOBAL: 0x62245c g_shadowStrength
// GLOBAL: 0x62244c g_shadowPos
// GLOBAL: 0x622440 g_shadowAt
// GLOBAL: 0x622430 g_shadowPlaneD
// GLOBAL: 0x62a8d4 cameraExtOffset
// GLOBAL: 0x464ef0 ?CEventIdDtorBare@@YAXXZ

extern const float g_floatZero;
extern const float g_twelveW3a;  // 12.0f: feet to world units
extern const RwV3d DAT_005d4d70;  // (1, 0, 0)
extern const RwV3d DAT_005d4d7c;  // (0, 1, 0)
extern unsigned g_resWidth;  // screen size in pixels
extern unsigned g_resHeight;
extern RwTexture *g_shadowPassTexture;  // the texture ShadowPipelineRender draws (set by SetShadowProjection)
extern float g_shadowStrength;  // its alpha: FLT_MAX for the BIGCIRCLE ring, else 0..1 (negative: inverted)
extern RwV3d g_shadowPos;  // position of the shadow camera (SetShadowProjection)
extern RwV3d g_shadowAt;  // its view direction
extern float g_shadowPlaneD;  // -(pos . at)
extern int cameraExtOffset;  // offset of the world plugin's data in an RwCamera
float LerpFloat(float t, float a, float b);
void CEventIdDtorBare();  // the folded empty function (an empty destructor call of the game's source)

// D3D8 values the pipeline callback sets.
enum ShadowD3D {
	D3DRS_ALPHATESTENABLE = 15,
	D3DRS_SRCBLEND = 19,
	D3DRS_DESTBLEND = 20,
	D3DRS_TEXTUREFACTOR = 60,
	D3DRS_CLIPPING = 136,
	D3DTS_TEXTURE0 = 16,
	D3DTSS_COLOROP = 1,
	D3DTSS_COLORARG1 = 2,
	D3DTSS_COLORARG2 = 3,
	D3DTSS_ALPHAOP = 4,
	D3DTSS_ALPHAARG1 = 5,
	D3DTSS_ALPHAARG2 = 6,
	D3DTSS_TEXCOORDINDEX = 11,
	D3DTSS_TEXTURETRANSFORMFLAGS = 24,
	D3DTOP_SELECTARG1 = 2,
	D3DTOP_MODULATE = 4,
	D3DTA_DIFFUSE = 0,
	D3DTA_CURRENT = 1,
	D3DTA_TEXTURE = 2,
	D3DTA_TFACTOR = 3,
	D3DTA_TEXTURE_COMPLEMENT = 0x12,
	D3DBLEND_ONE = 2,
	D3DBLEND_SRCALPHA = 5,
	D3DBLEND_INVSRCALPHA = 6,
	D3DTTFF_COUNT2 = 2,
	D3DTSS_TCI_CAMERASPACEPOSITION = 0x20000
};

// The point the actor casts its shadow from: where it stands (the ground contact of its body), else its own position.
static const RwV3d *GroundPoint(GameActor *actor)
{
	return actor->m_corpus ? &actor->m_corpus->m_contactPoint : &actor->m_matrix.pos;
}

// A player-controlled body in the air (or just landed) keeps the shadow camera at the fixed pitch / heading.
static bool HasFixedShadowDirection(GameActor *actor)
{
	Corpus *corpus = actor->m_corpus;
	return (corpus->m_flags & CORPUS_PLAYER_CONTROLLED) && ((corpus->m_flags & CORPUS_AIRBORNE) || corpus->m_stateTime < 0.125f);
}

// Fits the shadow camera (m_shadowCamera) around the actor's bounding sphere, culls it against the main camera, then draws the BIGCIRCLE ring (second message: actors visible on radar, radius = cone length), the detailed shadow (first message: the actor tree into the shadow map, ShadowProject onto the world) or, far away, the head blob.
// 0x4437f0
void ShadowPass::Visit(Collider *collider)
{
	GameActor *actor = static_cast<GameActor *>(collider);   // the game's `collider ? collider - 0x98 : 0`
	RwSphere sph;
	RwV3d pos;
	RwV2d viewWindow;
	bool big = false;
	if (g_bigCircleTexture && (actor->m_params.m_flags & ACTOR_VISIBLE_ON_RADAR)) {
		sph.radius = actor->m_params.m_coneLength * g_twelveW3a;
		if (sph.radius > g_floatZero)
			big = true;
	}
	if (m_detailed) {
		if (big)
			return;
		sph.radius = 0.0f;
		ShadowActorTreeSphere(actor, &sph);
		if (sph.radius == g_floatZero)
			return;
		if (HasFixedShadowDirection(actor)) {
			float v = actor->m_corpus->m_stateTime * 8.0f;
			float a, b;
			if (actor->m_corpus->m_flags & CORPUS_AIRBORNE) {
				a = g_shadowCamera->m_pitch;
				b = 90.0f;
			} else {
				a = 90.0f;
				b = g_shadowCamera->m_pitch;
			}
			if (v < 1.0f)
				b = LerpFloat(1.0f - v, a, b);
			RwFrame *frame = RwCameraGetFrame(m_shadowCamera);
			RwFrameRotate(frame, &DAT_005d4d70, b, rwCOMBINEREPLACE);
			RwFrameRotate(frame, &DAT_005d4d7c, g_shadowCamera->m_heading, rwCOMBINEPOSTCONCAT);
		}
		if (sph.radius > 120.0f && (actor->m_corpus->m_flags & CORPUS_PLAYER_CONTROLLED)) {
			viewWindow.x = viewWindow.y = 120.0f;
		} else {
			float len = RwV3dLength((const RwV3d *)actor->m_worldBox.m_extent);
			if (len > sph.radius)
				sph.radius = len;
			viewWindow.x = viewWindow.y = sph.radius + 4.0f;
		}
	} else {
		if (!big)
			return;
		RwFrame *frame = RwCameraGetFrame(m_shadowCamera);
		RwFrameRotate(frame, &DAT_005d4d70, 90.0f, rwCOMBINEREPLACE);
		RwFrameRotate(frame, &DAT_005d4d7c, actor->GetFacing(), rwCOMBINEPOSTCONCAT);
		Vec3 *p = actor->GetVector(actor);
		sph.center.x = p->x;
		sph.center.y = p->y;
		sph.center.z = p->z;
		viewWindow.y = sph.radius * 0.5f;
		viewWindow.x = sph.radius;
		float h = *(volatile float *)&viewWindow.y;   // the game stores h and reloads it: the products use the rounded value
		RwV3d *up = &RwCameraGetFrame(m_shadowCamera)->modelling.up;
		float tx = h * up->x;
		float ty = h * up->y;
		float tz = h * up->z;
		sph.center.x += tx;
		sph.center.y += ty;
		sph.center.z += tz;
	}
	RwV3d *at = &RwCameraGetFrame(m_shadowCamera)->modelling.at;
	Corpus *corpus = actor->m_corpus;
	const RwV3d *top;
	const RwV3d *ground;
	if (corpus) {
		top = &corpus->m_lastFrame.pos;
		ground = &corpus->m_contactPoint;
	} else {
		top = &actor->m_matrix.pos;
		ground = &actor->m_matrix.pos;
	}
	// the sphere is stretched along the shadow camera's view direction so that it reaches the ground
	float h = ((top->y - ground->y) + sph.radius) * (-1.0f / at->y) * 0.5f;
	sph.radius = sph.radius + h;
	volatile float fty = h * at->y;      // the game rounds the y/z products to float (stored temporaries), the x product stays extended
	volatile float ftz = h * at->z;
	sph.center.x += h * at->x;
	sph.center.y += fty;
	sph.center.z += ftz;
	bool skipRender = false;
	if (!RwCameraFrustumTestSphere(m_camera, &sph)) {
		if (!m_detailed)
			return;
		skipRender = true;   // detailed + not visible skips the render, straight to the tail
	} else if (m_detailed) {   // far away and small on screen: no detailed shadow
		{
			float s = sph.radius * 0.9f;
			RwV3d d;
			volatile float dxv = s * m_lightDir.x;   // the game stores d.x, d.y, d.z as floats (rounded) before adding/subtracting c
			d.x = dxv;
			d.y = s * m_lightDir.y;
			d.z = s * m_lightDir.z;
			RwV3d q[2];
			q[0].x = d.x + sph.center.x;
			q[0].y = d.y + sph.center.y;
			q[0].z = d.z + sph.center.z;
			q[1].x = sph.center.x - d.x;
			q[1].y = sph.center.y - d.y;
			q[1].z = sph.center.z - d.z;
			RwV3dTransformPoints(q, q, 2, &CameraView(m_camera)->viewMatrix);
			bool skip = false;
			if (q[0].z > 1800.0f && q[1].z > 1800.0f) {
				float W = (float)g_resWidth;
				float H = (float)g_resHeight;
				float i0 = 1.0f / q[0].z;
				float x0 = i0 * W * q[0].x;
				float y0 = i0 * H * q[0].y;
				float i1 = 1.0f / q[1].z;
				float x1 = i1 * W * q[1].x;
				float y1 = i1 * H * q[1].y;
				RwV3d right = RwCameraGetFrame(m_camera)->modelling.right;
				float s2 = sph.radius * 0.9f;
				volatile float rx = s2 * right.x;   // game: the three products are stored as floats first
				volatile float ry = s2 * right.y;
				volatile float rz = s2 * right.z;
				RwV3d e[2];
				e[0].x = rx + sph.center.x;
				e[0].y = ry + sph.center.y;
				e[0].z = rz + sph.center.z;
				e[1].x = sph.center.x - rx;
				e[1].y = sph.center.y - ry;
				e[1].z = sph.center.z - rz;
				RwV3dTransformPoints(e, e, 2, &CameraView(m_camera)->viewMatrix);
				if (e[0].z > 1800.0f && e[1].z > 1800.0f) {
					float j0 = 1.0f / e[0].z;
					float j1 = 1.0f / e[1].z;
					float ex0 = j0 * W * e[0].x;
					float ey0 = j0 * H * e[0].y;
					float ex1 = j1 * W * e[1].x;
					float ey1 = j1 * H * e[1].y;
					float dx = x0 - x1;
					float dy = y0 - y1;
					float ex = ex0 - ex1;
					float ey = ey0 - ey1;
					if ((dx * dx + dy * dy) * (ex * ex + ey * ey) < 10000.0f)
						skip = true;
				}
			}
			skipRender = skip;
		}
	}
	if (!skipRender) {
		RwV3d cameraPos;
		volatile float cpx = sph.radius * at->x;   // game: x and y products stored as floats, z product stays extended
		volatile float cpy = sph.radius * at->y;
		cameraPos.x = sph.center.x - cpx;
		cameraPos.y = sph.center.y - cpy;
		cameraPos.z = sph.center.z - sph.radius * at->z;
		RwFrame *shadowFrame = RwCameraGetFrame(m_shadowCamera);
		RwV3d delta;
		delta.x = cameraPos.x - shadowFrame->modelling.pos.x;
		delta.y = cameraPos.y - shadowFrame->modelling.pos.y;
		delta.z = cameraPos.z - shadowFrame->modelling.pos.z;
		RwFrameTranslate(shadowFrame, &delta, rwCOMBINEPOSTCONCAT);
		RwCameraSetViewWindow(m_shadowCamera, &viewWindow);
		RwCameraSetFarClipPlane(m_shadowCamera, sph.radius + sph.radius);
		if (!m_detailed) {
			ShadowProject(g_bigCircleTexture, m_camera, m_shadowCamera, FLT_MAX);
			return;
		}
		RwV3d toActor;
		RwFrame *cameraFrame = RwCameraGetFrame(m_camera);
		toActor.x = sph.center.x - cameraFrame->modelling.pos.x;
		toActor.y = sph.center.y - cameraFrame->modelling.pos.y;
		toActor.z = sph.center.z - cameraFrame->modelling.pos.z;
		bool nearFlag = true;
		if (!(RwV3dLength(&toActor) - sph.radius * 1.5f < 480.0f))
			nearFlag = false;
		SetShadowDirection(&sph.center);
		if (g_headShadowTexture && RwV3dLength(&toActor) > 1200.0f) {
			if (RwCameraBeginUpdate(m_camera)) {
				pos.x = sph.center.x;
				pos.y = GroundPoint(actor)->y + 2.0f;
				pos.z = sph.center.z;
				float size = (actor->m_worldBox.m_extent[2] + actor->m_worldBox.m_extent[0]) * 0.5f;
				if (sph.radius < size)
					size = sph.radius;
				ShadowDrawHead(&pos, size, g_headShadowTexture, g_shadowRender->m_softness);
				RwCameraEndUpdate(m_camera);
			}
		} else {
			g_shadowRender->RenderShadowMap(m_shadowCamera, actor, nearFlag);
			ShadowProject(g_shadowTexture, m_camera, m_shadowCamera, g_shadowRender->m_softness);
		}
	}
	if (HasFixedShadowDirection(actor)) {
		RwFrame *frame = RwCameraGetFrame(m_shadowCamera);
		RwFrameRotate(frame, &DAT_005d4d70, g_shadowCamera->m_pitch, rwCOMBINEREPLACE);
		RwFrameRotate(frame, &DAT_005d4d7c, g_shadowCamera->m_heading, rwCOMBINEPOSTCONCAT);
	}
}

// Draws the blob `texture` flat on the ground: a quad of 2 * size around `position` at its height, alpha `strength`, in the current camera (Im3D, alpha-tested, no depth write).
// 0x4432c0 ShadowDrawHead
void ShadowDrawHead(const RwV3d *position, float size, RwTexture *texture, float strength)
{
	RwIm3DVertex v[4];
	unsigned char a = (unsigned char)(int)(strength * 255.0f);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, TextureView(texture)->raster);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONGREATER);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void *)2);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)1);
	unsigned col = (unsigned)a << 24;
	v[0].u = 0.0f;  v[0].v = 0.0f;  v[0].color = col;
	v[1].u = 1.0f;  v[1].v = 0.0f;  v[1].color = col;
	float y = position->y;
	float x = position->x;
	float xm = x - size;
	v[0].objVertex.x = xm;
	v[0].objVertex.y = y;
	float zm = position->z - size;
	v[0].objVertex.z = zm;
	float xp = size + x;
	v[1].objVertex.x = xp;
	v[1].objVertex.y = y;
	v[1].objVertex.z = zm;
	v[2].objVertex.x = xm;
	v[2].objVertex.y = y;
	v[2].color = col;
	float zp = size + position->z;
	v[2].objVertex.z = zp;
	v[3].color = col;
	v[2].u = 0.0f;
	v[3].objVertex.y = y;
	v[3].objVertex.z = zp;
	v[2].v = 1.0f;
	v[3].objVertex.x = xp;
	v[3].u = 1.0f;
	v[3].v = 1.0f;
	if (RwIm3DTransform(v, 4, 0, rwIM3D_VERTEXUV | rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA)) {
		RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);
		RwIm3DEnd();
	}
}

// The texture matrix of the projection: the shadow camera's view window as a texture transform of the current camera (camera space -> shadow camera space -> [0, 1]), set as D3D texture transform 0. Also stores the texture, strength and the shadow camera's position, view direction and plane constant for the pipeline callback.
// 0x4445b0
void SetShadowProjection(RwTexture *texture, RwCamera *shadowCamera, float strength, float depthOffset)
{
	RwMatrix ltm, inv, res;
	g_shadowPassTexture = texture;
	g_shadowStrength = strength;
	ltm = *RwFrameGetLTM(RwCameraGetFrame(RwCameraGetCurrentCamera()));
	ltm.right.x = -ltm.right.x;
	ltm.right.y = -ltm.right.y;
	ltm.right.z = -ltm.right.z;
	RwMatrix *shadowMatrix = &RwCameraGetFrame(shadowCamera)->modelling;
	g_shadowPos = shadowMatrix->pos;
	g_shadowAt = shadowMatrix->at;
	// temporaries make cl sum (x + y) + z, which is the game's (y + x) + z bit for bit; the plain y, x, z expression sums (z + x) + y
	float tx = g_shadowPos.x * g_shadowAt.x, ty = g_shadowPos.y * g_shadowAt.y, tz = g_shadowPos.z * g_shadowAt.z;
	g_shadowPlaneD = -((ty + tx) + tz);
	RwMatrixInvert(&inv, shadowMatrix);
	RwV3d scale;
	scale.x = -0.5f / CameraView(shadowCamera)->viewWindow.x;
	scale.y = -0.5f / CameraView(shadowCamera)->viewWindow.y;
	scale.z = 1.0f / (depthOffset + CameraView(shadowCamera)->viewWindow.x);
	RwMatrixScale(&inv, &scale, rwCOMBINEPOSTCONCAT);
	RwV3d offset;
	offset.x = 0.5f;
	offset.y = 0.5f;
	offset.z = 0.0f;
	RwMatrixTranslate(&inv, &offset, rwCOMBINEPOSTCONCAT);
	RwMatrixMultiply(&res, &ltm, &inv);
	// as a D3DMATRIX: the fourth column (RW keeps the flags and padding words there) is 0, 0, 0, 1
	RwReal *d3d = (RwReal *)&res;
	d3d[3] = 0.0f;
	d3d[7] = 0.0f;
	d3d[11] = 0.0f;
	d3d[15] = 1.0f;
	RwD3D8SetTransform(D3DTS_TEXTURE0, &res);
}

// Projects `texture` (seen by `shadowCamera`) onto the world sectors that both cameras see, by drawing them with the shadow pipeline between BeginUpdate / EndUpdate of `camera`.
// 0x4431b0
void ShadowProject(RwTexture *texture, RwCamera *camera, RwCamera *shadowCamera, float strength)
{
	RwFrameGetLTM(RwCameraGetFrame(camera))->flags = 3;
	if (RwCameraBeginUpdate(camera)) {
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)1);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)0);
		SetShadowProjection(texture, shadowCamera, strength, 0.0f);
		RpWorldCameraExtView *ext = (RpWorldCameraExtView *)((char *)camera + cameraExtOffset);
		void **cur = ext->sectors;
		int n = ext->count;
		if (n) {
			int cnt = n;
			do {
				RpWorldSectorView *sector = (RpWorldSectorView *)*cur;
				if (sector->numTriangles > 0) {
					RpWorldCameraExtView *shadowExt = (RpWorldCameraExtView *)((char *)shadowCamera + cameraExtOffset);
					void **p = shadowExt->sectors;
					void *x = sector;
					for (int i = shadowExt->count; i != 0; --i) {   // x becomes 0 when the shadow camera sees the sector too
						void *e = *p;
						if (e == x) {
							x = 0;
							e = 0;
						}
						++p;
						if (e == 0)
							break;
					}
					if (x == 0)
						RxPipelineExecute(g_shadowPipeline, sector, 1);
				}
				++cur;
			} while (--cnt);
		}
		CEventIdDtorBare();
		RwCameraEndUpdate(camera);
	}
}

// The render callback of the shadow pipeline: sets the D3D8 states for the projected texture (colour = the strength as alpha, or a fixed 0x40FFFFFF for the BIGCIRCLE ring), then draws every mesh of the sector whose material does not have m_skipShadow set.
// 0x444770
void ShadowPipelineRender(RwResEntry *entry, void *sector, RwUInt8 type, RwUInt32 flags)
{
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)0);
	RwD3D8SetRenderState(D3DRS_CLIPPING, RwD3D8CameraIsBBoxFullyInsideFrustum(RwCameraGetCurrentCamera(),
	                                                                          &((RpWorldSectorView *)sector)->tightBoundingBox) ? 0 : 1);
	RwD3D8SetPixelShader(0);
	int saved;
	RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)&saved);
	RwD3D8SetTexture(g_shadowPassTexture, 0);
	RwD3D8SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	RwD3D8SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RwD3D8SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	int color;
	if (g_shadowStrength == FLT_MAX) {
		RwD3D8SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		RwD3D8SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		color = 0x40FFFFFF;
		RwD3D8SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	} else {
		if (!g_detailedShadows && g_shadowSkip)
			return;
		float v;
		if (g_shadowStrength > 0.0f) {
			RwD3D8SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			v = g_shadowStrength * 255.0f;
		} else {
			RwD3D8SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE_COMPLEMENT);
			v = g_shadowStrength * -255.0f;
		}
		color = (int)v << 24;
	}
	RwD3D8SetRenderState(D3DRS_TEXTUREFACTOR, color);
	RwD3D8SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	RwD3D8SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	RwD3D8SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	RwD3D8SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	RwD3D8SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	RxD3D8ResEntryView *e = (RxD3D8ResEntryView *)entry;
	RwD3D8SetVertexShader(e->meshes[0].vertexShader);
	void *lastVB = (void *)-1;
	for (int n = e->numMeshes, i = 0; n > 0; n--, i++) {
		RxD3D8InstanceDataView *m = &e->meshes[i];
		if (TFBMaterialGetData(m->material)->m_skipShadow == 0) {
			if (lastVB != m->vertexBuffer) {
				RwD3D8SetStreamSource(0, m->vertexBuffer, m->stride);
				lastVB = m->vertexBuffer;
			}
			if (m->indexBuffer) {
				RwD3D8SetIndices(m->indexBuffer, m->baseIndex);
				RwD3D8DrawIndexedPrimitive(m->primType, 0, m->numVertices, 0, m->numIndices);
			} else {
				RwD3D8DrawPrimitive(m->primType, m->baseIndex, m->numVertices);
			}
		}
	}
	RwD3D8SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	RwD3D8SetTexture(0, 0);
	RwD3D8SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	RwD3D8SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
}
