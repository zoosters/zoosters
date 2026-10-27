// Constructors and destructor inline what the original source got from ScriptSprite / Placement / Hud2DObject; the order of their stores is
// the game's.
#include <float.h>
#include <math.h>
#include "modules/tfb/sprites/spritemanager.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/ctfbmodel.h"

void *MemAlloc(size_t size);

// GLOBAL: 0x617e80 g_spriteManager
// GLOBAL: 0x617e84 g_spriteGeneration
// GLOBAL: 0x618318 g_frameDt
// GLOBAL: 0x5ca5a8 g_floatZero
// GLOBAL: 0x464ef0 ??1Hud2DObject@@QAE@XZ
// GLOBAL: 0x450620 ?Init@Hud2DObject@@QAEXXZ
// 0x415dc0
extern float g_frameDt;
extern const float g_floatZero;

namespace RWS { namespace TFB {

// Bit pattern of FLT_MAX: a sprite whose m_posY holds it is a screen sprite (position = screen x / y), else a world position for ProjectToScreen.
static const int kScreenSpriteMarker = 0x7F7FFFFF;

// The model size is in the raw copy of the model's first attribute command (+0x274 / +0x278).

// ---- construction

// 0x415c90
GameSprite::GameSprite(SpriteObject *owner)
{
	m_owner = owner;
	m_fixedWidth = 32.0f;
	m_fixedHeight = 32.0f;
	m_tint.blue = 0x80;
	m_tint.green = 0x80;
	m_tint.red = 0x80;
	m_refCount = 0;
	m_name = 0;
	m_spawnSource = 0;
	m_extra = 0;
	m_parentName = 0;
	m_inheritFlags = 0;
	m_parentAnchor = 0;
	m_parent = 0;
	m_childRefs = 0;
	m_textBuf = 0;
	m_alwaysVisible = 0;
	m_visible = 1;
	m_scaleByDistance = 1;
	m_resolveGeneration = -1;
	m_renderNext = (GameSprite *)-1;
	m_tint.alpha = 0xff;
}

// 0x415de0
// The clone keeps the source's looks but starts unlinked.
GameSprite::GameSprite(const GameSprite *src) : Hud2DObject(*(const Hud2DObject *)src)
{
	m_refCount = 0;
	m_name = 0;
	m_owner = src->m_owner;
	m_spawnSource = 0;
	m_visible = src->m_visible;
	m_extra = 0;
	m_scaleByDistance = src->m_scaleByDistance;
	m_fixedWidth = src->m_fixedWidth;
	m_fixedHeight = src->m_fixedHeight;
	m_parentName = src->m_parentName;
	m_inheritFlags = src->m_inheritFlags;
	m_parentAnchor = src->m_parentAnchor;
	m_resolveGeneration = -1;
	m_renderNext = (GameSprite *)-1;
	m_parent = 0;
	m_childRefs = 0;
	m_tint.alpha = 0xff;
	m_tint.blue = 0x80;
	m_tint.green = 0x80;
	m_tint.red = 0x80;
	m_type = src->m_type;
	m_justification = src->m_justification;
	m_pad4e = src->m_pad4e;
	m_priority = src->m_priority;
	m_dataValue = src->m_dataValue;
	m_scaleX = src->m_scaleX;
	m_scaleY = src->m_scaleY;
	m_visible = src->m_visible;
	m_priorityKey = src->m_priorityKey;
	((Hud2DObject *)this)->m_flags = ((const Hud2DObject *)src)->m_flags;   // redundant, the copy constructor already copied it
	GameSprite *parent = src->m_parent;
	if (parent) {
		parent->m_childRefs++;
		parent->AdjustRefCount(1);
	}
	if (m_parent) {
		m_parent->m_childRefs--;
		m_parent->AdjustRefCount(-1);
	}
	m_parent = parent;
	m_textBuf = 0;
	m_alwaysVisible = 0;
}

// 0x415fe0
// The name is kept; the text goes into a 255 byte buffer the caller fills (`text` is unused).
GameSprite::GameSprite(const char *name, const char *text)
{
	m_name = name;
	m_fixedWidth = 32.0f;
	m_fixedHeight = 32.0f;
	m_refCount = 0;
	m_owner = 0;
	m_spawnSource = 0;
	m_visible = 1;
	m_extra = 0;
	m_scaleByDistance = 1;
	m_parentName = 0;
	m_inheritFlags = 0;
	m_parentAnchor = 0;
	m_renderNext = 0;
	m_parent = 0;
	m_tint.blue = 0x80;
	m_tint.green = 0x80;
	m_tint.red = 0x80;
	m_tint.alpha = 0xff;
	m_textBuf = (char *)MemAlloc(0xff);
	m_alwaysVisible = 1;
}

// ---- destruction

// 0x415bb0
GameSprite::~GameSprite()
{
	if (m_parent) {
		m_parent->m_childRefs--;
		m_parent->AdjustRefCount(-1);
	}
	SpriteExtra *extra = m_extra;
	m_parent = 0;
	if (extra) {
		delete extra;
		m_extra = 0;
		m_clump = 0;
	}
	if (m_spawnSource && m_spawnSource != this)
		m_texture = 0;   // a clone shares the original's texture, which it does not release
	delete m_textBuf;
}

// ---- Placement overrides

// 0x414220
int GameSprite::AdjustRefCount(int delta)
{
	m_refCount += delta;
	if (m_refCount == 0) {
		delete this;
		return 0;
	}
	return m_refCount;
}

// 0x4142a0
Script::ScriptReference *GameSprite::GetSpawnSource()
{
	if (m_spawnSource && m_spawnSource != this)
		return m_spawnSource;
	return 0;
}

// 0x414400
bool GameSprite::SetFacing(float v, int)
{
	m_facing = v;
	return true;
}

// 0x414420
bool GameSprite::SetTilt(float v, Script::ScriptReference *)
{
	m_tilt = v;
	return true;
}

// 0x414440
bool GameSprite::SetRoll(float v, Script::ScriptReference *)
{
	m_roll = v;
	return true;
}

// 0x4143f0
float GameSprite::GetFacing()
{
	return m_facing;
}

// 0x414410
float GameSprite::GetTilt()
{
	return m_tilt;
}

// 0x414430
float GameSprite::GetRoll()
{
	return m_roll;
}

// 0x414470
bool GameSprite::PlayAnimation(int index)
{
	if (m_extra && index >= -1)
		return m_extra->PlaySlot(index, g_frameDt);
	return false;
}

// 0x4144a0
// Reads (and, for value >= 0, writes) the model state's slot map; returns the old entry.
int GameSprite::AccessBehavior(int index, int value)
{
	if (m_extra)
		return m_extra->ExchangeSlotIndex(index, value);
	return 0;
}

// 0x4144d0
// NOTE: slot 18 stores the vector into `owner`, not into this (the game's accessor ignores this).
void GameSprite::SetVector(const Vec3 *p, Script::ScriptReference *owner)
{
	Vec3 t = *p;
	Vec3 *d = (Vec3 *)&static_cast<GameSprite *>(owner)->m_posX;
	*d = t;
}

// 0x414450
void GameSprite::SetFont(const char *name)
{
	if (name)
		m_font = Hud2D::GetFont(0, name);
}

// 0x414250
void GameSprite::SetParent(GameSprite *parent)
{
	if (parent) {
		parent->m_childRefs++;
		parent->AdjustRefCount(1);
	}
	if (m_parent) {
		m_parent->m_childRefs--;
		m_parent->AdjustRefCount(-1);
	}
	m_parent = parent;
}

// 0x4143c0
// True while the sprite is in play: an original, or a clone that its original still lists.
char GameSprite::Control(bool)
{
	if (m_spawnSource) {
		if (m_spawnSource == this)
			return 1;
		if (m_spawnSource->m_20.FindNode(this))   // the clone set of the original (the Placement's second set)
			return 1;
	}
	return 0;
}

// 0x4142c0
void GameSprite::Remove()
{
	GameSprite *source = m_spawnSource;
	if (source) {
		StopVoicesForObject(this, 1);
		if (source != this) {
			if (source->m_20.Remove(this))
				source->AdjustRefCount(-1);
		} else {
			Script::TFB_GetRemovedSpriteSet()->Insert(this, FLT_MAX);
			m_spawnSource = 0;
		}
		if (Script::TFB_GetActiveSpriteSet()->Remove(this) && m_renderNext != (GameSprite *)-1 && g_spriteManager)
			g_spriteManager->MarkRenderListDirty();
	}
}

// NEARMISS: 0x414350 RWS::TFB::GameSprite::Reset (DIFF, 106 B vs 112 B)
// The game pads the `m_spawnSource = this` store to 16 B with a jmp + lea after the `m_renderList = -1` block; ours falls through.
void GameSprite::Reset()
{
	if (m_spawnSource) {
		if (m_spawnSource != this) {
			Remove();
			return;
		}
		if (Script::TFB_GetActiveSpriteSet()->FindNode(this))
			return;
	} else {
		if (Script::TFB_GetRemovedSpriteSet()->Remove(this)) {
			if (g_spriteManager)
				g_spriteManager->MarkRenderListDirty();
		}
		m_spawnSource = this;
	}
	Script::TFB_GetActiveSpriteSet()->Insert(this, (float)m_priorityKey);
}

// 0x416110
// Clones, places at `position` (a Vec3 *), registers in the active set.
intptr_t GameSprite::Spawn(intptr_t position, int)
{
	GameSprite *c = new GameSprite(this);
	if (m_extra)
		c->SetModel(m_extra->m_model);
	c->m_spawnSource = this;
	c->SetVector((const Vec3 *)position, c);
	c->m_scaleX = m_scaleX;
	c->m_scalePct.x = (short)(int)(m_scaleX * 100.0f);
	c->m_scaleY = m_scaleY;
	c->m_scalePct.y = (short)(int)(m_scaleY * 100.0f);
	c->m_boxWidth = m_boxWidth;
	c->m_boxHeight = m_boxHeight;
	Script::TFB_GetActiveSpriteSet()->Insert(c, (float)c->m_priorityKey);
	if (g_spriteManager)
		g_spriteManager->MarkRenderListDirty();
	return (intptr_t)c;
}

// 0x414cd0
void GameSprite::SetModel(CTFBModel *model)
{
	if (m_extra) {
		delete m_extra;
		m_extra = 0;
		m_clump = 0;
	}
	if (model) {
		m_extra = new SpriteExtra(model);
		m_extra->PlaySlot(0, 0.0f);
		m_clump = m_extra->m_clump.ptr();
	}
}

// ---- screen position and extents

// Near-match of 0x4144f0: the game passes lea [esi+70h] without keeping it in edi for the final m_posX store.
// 0x4144f0
// World position -> camera space -> 640x480 screen (x, FLT_MAX, y). Returns the distance fade (1 up to 360 units, 0 at 2400).
float GameSprite::ProjectToScreen(RwCamera *camera)
{
	RwV3d p;
	RwV3dTransformPoint(&p, (const RwV3d *)&m_posX, &CameraView(camera)->viewMatrix);
	if (p.z < 0.0f)
		return 0.0f;
	float scale = 1.0f;
	if (p.z != 0.0f) {
		if (p.z > 360.0f) {
			scale = 0.0f;
			if (p.z < 2400.0f)
				scale = 1.0f - p.z * 0.00041666668f;
		}
		float inv = 1.0f / p.z;
		p.x = p.x * inv * 640.0f;
		p.y = inv * p.y * 480.0f;
	}
	m_posX = p.x;
	m_posY = FLT_MAX;
	m_posZ = p.y;
	return scale;
}

// 0x4145d0
void GameSprite::UpdateExtents(char reset)
{
	m_value = m_dataValue;
	if (reset) {
		m_drawScaleY = 1.0f;
		m_drawScaleX = 1.0f;
	}
	float h = 32.0f;
	float w = 32.0f;
	int kind = m_type;
	if (kind == 3) {   // a 3D model: its extents
		if (m_extra) {
			w = m_extra->m_model->Width() * 12.0f;
			h = m_extra->m_model->Height() * 12.0f;
		}
	} else if (kind == 1 || kind == 2) {   // text: its measured size
		if (m_font && m_format) {
			float textScale = MeasureText(&w, &h);
			if (textScale != 1.0f) {
				if (!(m_flags & 2))
					m_drawScaleX *= textScale;
				else
					m_drawScaleY *= textScale;
			}
		}
	} else {   // a quad (and unknown kinds): the box
		h = m_boxHeight;
		w = m_boxWidth;
	}
	m_width = w;
	m_height = h;
	GetAnchorOffset(&m_offsetX, m_justification);
	m_width = m_drawScaleX * m_width;
	m_height = m_drawScaleY * m_height;
}

// 0x414d80
// Resolves the draw state from the sprite's own inputs and, through the inherit flags, its parent. resolveOnly: the call made for a parent
// (transform only, no early visibility exits).
char GameSprite::Resolve(RwCamera *camera, char resolveOnly)
{
	if (m_alwaysVisible) {
		m_drawVisible = 1;
		m_visible = 1;
	}
	if (m_resolveGeneration == g_spriteGeneration)
		return m_drawVisible;
	m_resolveGeneration = g_spriteGeneration;
	if (m_parent == 0 && m_parentName != 0 && m_inheritFlags != 0)
		SetParent(FindSpriteByName(m_parentName));
	m_drawVisible = m_visible;
	if (resolveOnly == 0) {
		if (m_parent != 0 && (m_inheritFlags & 8)) {
			if (!m_parent->Resolve(camera, 0)) {
				m_drawVisible = 0;
				return 0;
			}
		} else if (m_visible == 0) {
			return 0;
		}
	}
	m_drawScaleY = 1.0f;
	m_drawScaleX = 1.0f;
	m_drawAlpha = 1.0f;
	m_drawTint.red = m_tint.red;
	int prio = m_priority;
	m_drawY = 0.0f;
	m_drawX = 0.0f;
	m_drawRotation = 0.0f;
	m_drawTint.green = m_tint.green;
	m_drawTint.blue = m_tint.blue;
	m_drawPriority = prio;
	if (prio > 0x7f)
		m_drawPriority = prio - 254;   // the game subtracts 254 (not 256) to make the priority byte signed
	float anchorOff[2];
	anchorOff[1] = 0.0f;
	anchorOff[0] = 0.0f;
	if (m_parent) {
		m_parent->Resolve(camera, 1);
		unsigned inherit = m_inheritFlags;
		if (inherit & 0x20)
			m_drawPriority += m_parent->m_drawPriority;
		if (inherit & 4) {
			m_drawTint.red = m_parent->m_drawTint.red;
			m_drawTint.green = m_parent->m_drawTint.green;
			m_drawTint.blue = m_parent->m_drawTint.blue;
		}
		if (m_drawVisible && (inherit & 8))
			m_drawVisible = m_parent->m_drawVisible;
		if (inherit & 1) {
			m_drawX = m_parent->m_drawX;
			m_drawY = m_parent->m_drawY;
			float parentAnchor[2];
			m_parent->GetAnchorOffset(anchorOff, m_parent->m_justification);
			m_parent->GetAnchorOffset(parentAnchor, m_parentAnchor);
			anchorOff[0] = anchorOff[0] - parentAnchor[0];
			anchorOff[1] = anchorOff[1] - parentAnchor[1];
		}
		if (m_inheritFlags & 0x10)
			m_drawRotation = m_parent->m_drawRotation;
		if (m_inheritFlags & 2) {
			m_drawScaleX = m_parent->m_drawScaleX;
			m_drawScaleY = m_parent->m_drawScaleY;
		}
		if (m_inheritFlags & 0x40)
			m_drawAlpha = m_parent->m_drawAlpha;
	}
	if (*(int *)&m_posY != kScreenSpriteMarker)
		m_distanceScale = ProjectToScreen(camera);
	if (m_distanceScale <= g_floatZero)
		m_drawVisible = 0;
	anchorOff[0] = m_posX * m_drawScaleX + anchorOff[0];
	anchorOff[1] = m_posZ * m_drawScaleY + anchorOff[1];
	if (m_drawRotation == g_floatZero) {
		m_drawX = anchorOff[0] + m_drawX;
		m_drawY = anchorOff[1] + m_drawY;
	} else {
		float negRot = -m_drawRotation;
		float rad = negRot * 0.017453292f;
		float sn = (float)sin(rad);
		float cs = (float)cos(rad);
		m_drawX = anchorOff[0] * cs - sn * anchorOff[1] + m_drawX;
		m_drawY = sn * anchorOff[0] + cs * anchorOff[1] + m_drawY;
	}
	if (m_scaleByDistance) {
		m_drawScaleX = m_drawScaleX * m_distanceScale;
		m_drawScaleY = m_drawScaleY * m_distanceScale;
	}
	m_drawScaleX = m_scaleX * m_drawScaleX;
	m_drawScaleY = m_scaleY * m_drawScaleY;
	m_drawRotation = m_rotation + m_drawRotation;
	m_drawAlpha = ((float)m_tint.alpha * 0.003921569f) * m_drawAlpha;  // alpha byte scaled first, then times m_drawAlpha (the game's rounding)
	UpdateExtents(0);
	return m_drawVisible;
}

}}
