// Commands go into locals (and straight into the sprite once it exists); when command 0 found the sprite's own script object (flagA), the tail
// applies the locals, registers the sprite in the active set and marks the manager dirty.
#include <float.h>
#include <stddef.h>
#include <string.h>
#include "script/script.h"
#include "modules/tfb/sprites/spriteobject.h"
#include "modules/tfb/sprites/spritemanager.h"
#include "modules/tfb/camera_views.h"
#include "script/symbolregistry.h"
#include "framework/stringpool.h"
#include "script/runtime.h"
#include "rw/rwcore.h"

// GLOBAL: 0x617e80 g_spriteManager
// GLOBAL: 0x617e84 g_spriteGeneration
// GLOBAL: 0x6029e4 g_spriteObjectClassId
// GLOBAL: 0x4500b0 ?GetGuid@SprResRef@@QBEXPAUSprGuid@@@Z
// GLOBAL: 0x5cc994 g_fmtD
// GLOBAL: 0x5cbf3c RWS::TFB::SpriteObject::vftable
// 0x4176a0
extern const char *g_spriteObjectClassId;
extern const char g_fmtD[];
const char *InternString(const char *s);
int FUN_0043e6f0(const char *s);
char *GetLocalizedString(int id);
int RwTextureDestroy(RwTexture *t);        // 0x4dbcc0

// The 16-byte resource id inside a model attribute field (same GUID as ModelResAttr::ToGuid).
struct SprGuid { unsigned d[4]; };
struct SprResRef {
	void GetGuid(SprGuid *out) const;
};

namespace RWS { namespace TFB {

// 0x416b10
SpriteObject::SpriteObject(const CAttributePacket &)
{
	m_spriteName = 0;
	m_fontName = 0;
	m_data = new GameSprite(this);
	m_data->SetTexture(0);
	m_data->AdjustRefCount(1);
}

// 0x415ae0
SpriteObject::~SpriteObject()
{
	if (m_data->m_texture) {
		RwTextureDestroy(m_data->m_texture);
		m_data->SetTexture(0);
	}
	GameSprite *d = m_data;
	if (d->m_parent) {
		d->m_parent->m_childRefs--;
		d->m_parent->AdjustRefCount(-1);
	}
	d->m_parent = 0;
	d = m_data;
	if (d->m_extra) {
		delete d->m_extra;
		d->m_extra = 0;
		d->m_clump = 0;
	}
	m_data->AdjustRefCount(-1);
}

// Attribute command as the iterator hands it out: CAttributeDataChunk size_, type_, then the data from +8
struct SprCmd {
	int size_;
	int type_;
	union { int data_; float fdata_; };
};
// The game moves the float attribute values with integer instructions (float locals would add x87 loads and stores), so they are kept
// as int bits and stored through int views of the float members.
static inline void SetBits(float &dst, int bits)
{
	*(int *)&dst = bits;
}
static inline void CopyBits(float &dst, const float &src)
{
	*(int *)&dst = *(const int *)&src;
}
#define MARK() do { if (g_spriteManager) g_spriteManager->MarkRenderListDirty(); } while (0)
#define CMD(it) ((SprCmd *)(it).operator->())

// TFB_ObjectFromIndex (0x416220) inlined with a constant userFlag
static inline ::Script::ScriptReference *LookupSpr(int index, char userFlag)
{
	if (index >= 0x3fc0d)
		return g_curScript->m_locals[index - 0x3fc0d];
	SymbolSlot *slot = &(*g_symRegistryY)[index];
	int f = userFlag == 1;
	if (slot->obj & f)
		return 0;
	return (::Script::ScriptReference *)(slot->obj & ~(uintptr_t)1);
}

// Near-match of 0x416bb0 (2584 B vs 2544 B): register allocation and stack slot order.
void SpriteObject::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	CAttributeCommandIterator it(attr, g_spriteObjectClassId);
	int justification = 0;
	int type = 0;
	int rotation = 0;   // float bits (the game moves them with integer instructions)
	int data = -1;
	bool useTexSize = true;            // size from the texture, else fixed (commands 13 / 14)
	bool b16 = true;
	bool f14 = false;
	int roll = 0;
	int tilt = 0;
	int facing = 0;
	int fixedWidth = 0;
	int fixedHeight = 0;
	const char *fmt = 0;
	const char *parentName = 0;        // the name behind the +0x120 parent link (command 21 clears that link)
	bool flagA = false;
	RwV3d pos;
	RwV3d scale;
	int v22, v23;
	for (; !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0:
			m_data->m_name = InternString(it->GetAs_RwChar_ptr());
			{
				GameSprite *d = m_data;
				int idx = ((::Script::ScriptReference *)d)->ResolveSymbol(d->GetType(), m_data->m_name, 0);
				if (idx >= 0) {
					flagA = LookupSpr(idx, 0) == m_data;
					if (!flagA) {
						m_data->AdjustRefCount(-1);
						m_data = (GameSprite *)LookupSpr(idx, 0);
						m_data->AdjustRefCount(1);
						m_data->m_owner = this;
					}
				}
			}
			break;
		case 2:
			m_spriteName = it->GetAs_RwChar_ptr();
			if (*m_spriteName != 0 && *m_spriteName != '<')
				m_data->SetSpriteByName(m_spriteName);
			m_spriteName = (const char *)(intptr_t)FUN_0043e6f0(m_spriteName);
			break;
		case 1:
			type = CMD(it)->data_;
			if (type > 0) {
				data = 0;
				if (type == 1) {
					fmt = g_fmtD;
					m_data->SetFormat(fmt);
				}
			}
			if (m_data) {
				m_data->m_type = (unsigned char)type;
				m_data->m_dataValue = data;
			}
			break;
		case 3:
			if (m_fontName) {
				Hud2D::ReleaseFont(m_fontName);
				m_fontName = 0;
			}
			m_fontName = it->GetAs_RwChar_ptr();
			if (*m_fontName != 0 && *m_fontName != '<') {
				char *dot = strrchr((char *)m_fontName, '.');
				if (dot)
					*dot = 0;
				m_fontName = (char *)InternString(m_fontName);
				m_data->SetFont(m_fontName);
			} else {
				m_fontName = 0;
			}
			break;
		case 4:
			it.operator->();
			break;
		case 25:
			{
				int stringId = CMD(it)->data_;
				if (m_data) {
					if (m_data->m_type == 1)
						fmt = g_fmtD;
					else
						fmt = GetLocalizedString(stringId);
					m_data->SetFormat(fmt);
				}
			}
			break;
		case 5:
			{
				SprCmd *c = CMD(it);
				if (m_data) {
					m_data->m_visible = c->data_ != 0;
					MARK();
				}
			}
			break;
		case 7:
			justification = CMD(it)->data_;
			if (m_data) {
				m_data->m_justification = (unsigned char)justification;
				m_data->m_anchor = justification;
			}
			break;
		case 8:
			rotation = CMD(it)->data_;
			if (m_data)
				SetBits(m_data->m_rotation, rotation);
			break;
		case 9:
			pos = *static_cast<const RwV3d *>(it->GetAs_void_ptr());
			if (m_data) {
				GameSprite *hud = m_data;
				hud->m_posX = pos.x;
				hud->m_posY = FLT_MAX;
				hud->m_posZ = pos.y;
			}
			break;
		case 10:
			scale = *(RwV3d *)&CMD(it)->data_;
			if (m_data) {
				m_data->m_scaleX = scale.x * 0.01f;
				m_data->m_scaleY = scale.y * 0.01f;
				m_data->m_scalePct.x = (short)scale.x;
				m_data->m_scalePct.y = (short)scale.y;
			}
			break;
		case 11:
			{
				SprCmd *c = CMD(it);
				if (m_data) {
					m_data->m_priority = (unsigned char)c->data_;
					MARK();
				}
			}
			break;
		case 12:
			{
				bool b = CMD(it)->data_ != 0;
				useTexSize = b;
				if (m_data) {
					if (b) {
						m_data->m_flags &= ~1;
						m_data->SetSizeFromTexture();
					} else {
						m_data->m_flags |= 1;
						CopyBits(m_data->m_boxWidth, m_data->m_fixedWidth);
						CopyBits(m_data->m_boxHeight, m_data->m_fixedHeight);
					}
				}
			}
			break;
		case 26:
			{
				bool b = CMD(it)->data_ != 0;
				if (m_data) {
					if (!b) {
						m_data->m_flags &= ~2;
					} else {
						m_data->m_flags |= 2;
						CopyBits(m_data->m_boxWidth, m_data->m_fixedWidth);
						CopyBits(m_data->m_boxHeight, m_data->m_fixedHeight);
					}
				}
			}
			break;
		case 13:
			fixedWidth = CMD(it)->data_;
			if (m_data) {
				SetBits(m_data->m_fixedWidth, fixedWidth);
				if (m_data->m_flags & 1)
					CopyBits(m_data->m_boxWidth, m_data->m_fixedWidth);
			}
			break;
		case 14:
			fixedHeight = CMD(it)->data_;
			if (m_data) {
				SetBits(m_data->m_fixedHeight, fixedHeight);
				if (m_data->m_flags & 1)
					CopyBits(m_data->m_boxHeight, m_data->m_fixedHeight);
			}
			break;
		case 15:
			{
				SprGuid tmp;
				const SprResRef *resRef = static_cast<const SprResRef *>(it->GetAs_void_ptr());
				resRef->GetGuid(&tmp);
				SprGuid guid = tmp;
				CAttributeHandler *model = 0;
				if (guid.d[0] || guid.d[1] || guid.d[2] || guid.d[3])
					model = CAttributeHandler::Find(*(RWSGUID *)&guid);
				m_model = model;
				if (model && static_cast<CTFBModel *>(model)->m_clumpId.m_Data[0] != 0) {   // the first word of the clump resource id (+0x118) is set once the model has a clump
					if (m_data)
						m_data->SetModel(static_cast<CTFBModel *>(model));
				} else {
					m_model = 0;
				}
			}
			break;
		case 16:
			{
				bool b = CMD(it)->data_ != 0;
				b16 = b;
				if (m_data)
					m_data->m_scaleByDistance = b;
			}
			break;
		case 17:
			f14 = CMD(it)->data_ != 0;
			if (m_data)
				m_data->SetCullBack(f14);
			break;
		case 18:
			roll = CMD(it)->data_;
			if (m_data)
				SetBits(m_data->m_roll, roll);
			break;
		case 19:
			tilt = CMD(it)->data_;
			if (m_data)
				SetBits(m_data->m_tilt, tilt);
			break;
		case 20:
			facing = CMD(it)->data_;
			if (m_data)
				SetBits(m_data->m_facing, facing);
			break;
		case 21:
			{
				const char *p = it->GetAs_RwChar_ptr();
				if (p && *p != '<' && *p != 0)
					parentName = InternString(p);
				else
					parentName = 0;
				if (m_data) {
					m_data->m_parentName = parentName;
					GameSprite *d = m_data;
					if (d->m_parent) {
						d->m_parent->m_childRefs--;
						d->m_parent->AdjustRefCount(-1);
					}
					d->m_parent = 0;
					MARK();
				}
			}
			break;
		case 22:
			v22 = CMD(it)->data_;
			if (m_data) {
				m_data->m_inheritFlags = v22;
				MARK();
			}
			break;
		case 23:
			v23 = CMD(it)->data_;
			if (m_data)
				m_data->m_parentAnchor = v23;
			break;
		case 24:
			if (m_data) {
				unsigned int tint = CMD(it)->data_;
				m_data->m_tint.red = (unsigned char)tint;
				m_data->m_tint.green = (unsigned char)(tint >> 8);
				m_data->m_tint.blue = (unsigned char)(tint >> 16);
				m_data->m_tint.alpha = (unsigned char)(tint >> 24);
			}
			break;
		case 6:
		default:
			break;
		}
	}
	if (!flagA) {
		Script::InvalidateSprite(m_data);
		return;
	}
	{
		GameSprite *d = m_data;
		d->m_type = (unsigned char)type;
		Hud2DObject *hud = &static_cast<Hud2DObject &>(*d);   // (a reference conversion: no null check)
		hud->m_posX = pos.x;
		hud->m_posZ = pos.y;
		hud->m_posY = FLT_MAX;
		d->m_scaleX = scale.x * 0.01f;
		d->m_scaleY = scale.y * 0.01f;
		d->m_scalePct.x = (short)scale.x;
		d->m_scalePct.y = (short)scale.y;
		hud->SetFormat(d->m_type == 1 ? g_fmtD : fmt);
		SetBits(d->m_fixedWidth, fixedWidth);
		SetBits(d->m_fixedHeight, fixedHeight);
		if (useTexSize) {
			if (hud->m_texture) {
				hud->m_boxWidth = (float)((RwRasterView *)((RwTextureView *)hud->m_texture)->raster)->width;
				hud->m_boxHeight = (float)((RwRasterView *)((RwTextureView *)hud->m_texture)->raster)->height;
			}
		} else {
			SetBits(d->m_boxWidth, fixedWidth);
			SetBits(d->m_boxHeight, fixedHeight);
		}
		d->m_justification = (unsigned char)justification;
		d->m_anchor = justification;
		d->m_dataValue = data;
		d->m_parentAnchor = v23;
		SetBits(d->m_rotation, rotation);
		d->m_parentName = parentName;
		d->m_inheritFlags = v22;
		if (d->m_extra == 0 && m_model) {
			d->m_scaleByDistance = b16;
			hud->SetCullBack(f14);
			SetBits(d->m_tilt, tilt);
			SetBits(d->m_roll, roll);
			SetBits(d->m_facing, facing);
			d->SetModel(static_cast<CTFBModel *>(m_model));
		}
		int priority = d->m_priority;
		d->m_spawnSource = m_data;
		d->m_priorityKey = priority;
		if (priority > 0x7f)
			d->m_priorityKey = priority - 0xfe;          // the game subtracts 254 (add eax,0FFFFFF02h), not 256
		Script::TFB_GetActiveSpriteSet()->Insert(d, (float)d->m_priorityKey);
		MARK();
	}
}


}}

#include "framework/core/resourceref.h"
inline void SprResRef::GetGuid(SprGuid *out) const { ((const RWS::ResourceRef *)this)->GetGuid((RWS::RWSGUID *)out); }
