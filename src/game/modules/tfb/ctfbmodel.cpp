#include <stdint.h>
#include <stdio.h>
#include "framework/core/csystemcommands.h"
#include <string.h>
#include "modules/tfb/ctfbmodel.h"
#include "framework/core/cresourcemanager.h"
#include "script/symbolregistry.h"
#include "rw/rpplugins.h"
#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"
#include "script/runtime.h"

// vtable and the base id of the system commands
// GLOBAL: 0x5d4e5c CTFBModel::vftable
// 0x445690
// GLOBAL: 0x4500b0 ?ToGuid@ModelResAttr@@QBE_NPAVRWSGUID@RWS@@@Z
// GLOBAL: 0x603548 g_ctfbModelClassId
// GLOBAL: 0x611634 RWS::CSystemCommands::pCSystemCommandsClassID
// GLOBAL: 0x43e6f0 FUN_0043e6f0
// GLOBAL: 0x61f878 g_symRegistryY
// GLOBAL: 0x61f65c g_curScript
// GLOBAL: 0x602ea0 g_globalSymbolCount
// GLOBAL: 0x5d4ed8 g_surfaceNames

extern const char *g_ctfbModelClassId;           // 0x603548
int FUN_0043e6f0(const char *s);                 // 0x43e6f0 folded `return 0`
extern int g_globalSymbolCount;                    // 0x602ea0 the first registry index the footstep search starts at
extern const char *g_surfaceNames[SURFACE_COUNT];   // 0x5d4ed8 "rock", "dirt", "water", "sand", "grass", "mud", "wood", "hard"
int SymbolRegistry_GetSize();                    // 0x432cb0 number of registry entries

extern "C" FILE *port_log;
// The mouth / eye animation commands index m_mouthAnims[9] / m_eyeAnims[4] with the stream's number unchecked, and Gloria's
// model (kingofny) has eye animations 4..9. In the game's 32-bit layout the stores land on: 4 m_haveEyeAnims, 5..8
// m_eyeBoneIds (recomputed later), 9 m_surfacesResolved = low byte of the animation address (nonzero), so
// ResolveSurfaceEffects never runs and Gloria has no footprints. With 8-byte pointers the same indices reach m_surfaces and
// rock footsteps near her enclosure crash.
// Vanilla: the store goes to the field the game's store hits. ZOOSTERS_FIX_BUGS: dropped (footprints_gloria_rock.dff ships
// in kingofny.stream)
static void PortAnimOutOfRange(CTFBModel *m, const char *table, unsigned x86Table, int index, const void *anim, const char *resName)
{
	static int logged;
	if (port_log && logged++ < 32)
		fprintf(port_log, "CTFBModel: %s animation %d outside the table ('%s')\n", table, index, resName ? resName : "?");
#ifndef ZOOSTERS_FIX_BUGS
	unsigned off = x86Table + 4u * (unsigned)index;
	int low = (int)((uintptr_t)anim & 0x7fffffff);   // a 32-bit user-space address in the game
	if (index < 0)
		return;
	if (off >= 0x24 && off < 0x48)
		m->m_mouthAnims[(off - 0x24) / 4] = (RtAnimAnimation *)anim;
	else if (off == 0x48)
		m->m_haveMouthAnims = (unsigned char)low;
	else if (off == 0x4c)
		m->m_mouthBoneId = low;
	else if (off >= 0x50 && off < 0x60)
		m->m_eyeAnims[(off - 0x50) / 4] = (RtAnimAnimation *)anim;
	else if (off == 0x60)
		m->m_haveEyeAnims = (unsigned char)low;
	else if (off >= 0x64 && off < 0x74)
		m->m_eyeBoneIds[(off - 0x64) / 4] = low;
	else if (off == 0x74)
		m->m_surfacesResolved = 1;
	else if (port_log)   // m_surfaces and on: an animation filed as a dust clump / sound would crash the game too; the data never gets here
		fprintf(port_log, "CTFBModel: %s animation %d would land at +0x%x: dropped\n", table, index, off);
#else
	(void)m; (void)x86Table; (void)anim;
#endif
}

enum CTFBModelAttribute {
	MODEL_ATTR_SLOT_GUID = 0,       // a ModelResAttr: slot index and the animation resource of that slot
	MODEL_ATTR_RAW_PARAMS = 1,      // 0xd4 bytes copied as they are
	MODEL_ATTR_NAME = 2,            // a string
	MODEL_ATTR_SLOT = 3,            // a ModelResAttr with a slot's flags and blend frames
	MODEL_ATTR_NUM_ENTRIES = 4,
	MODEL_ATTR_ENTRY = 5,           // a 0x40-byte ModelEntry
	MODEL_ATTR_MOUTH_ANIM = 6,      // a ModelResAttr: the mouth animation of a slot
	MODEL_ATTR_EYE_ANIM = 7         // a ModelResAttr: the eye animation of a slot
};

// 0x444de0
CTFBModel::CTFBModel(const RWS::CAttributePacket &)
    : m_numEntries(0), m_entries(0), m_haveMouthAnims(0), m_mouthBoneId(-1), m_haveEyeAnims(0), m_surfacesResolved(0)
{
	m_name = 0;
	m_clumpId.Clear();
	memset(m_mouthAnims, 0, sizeof(m_mouthAnims));
	memset(m_eyeAnims, 0, sizeof(m_eyeAnims));
	m_eyeBoneIds[0] = -1;
	m_eyeBoneIds[1] = -1;
	m_eyeBoneIds[2] = -1;
	m_eyeBoneIds[3] = -1;
	memset(m_surfaces, 0, sizeof(m_surfaces));
}

// 0x444cc0
CTFBModel::~CTFBModel()
{
	if (m_entries)
		RwFree(m_entries);
}

// 0x444d30
int SurfaceHoldsFootprint(int surface)
{
	return surface != SURFACE_WATER;
}

// kind predicate with a byte-indexed jump table: the game table has three entries (case 0, the false group, default), read from
// the byte index at 0x444d6c; kinds 1/3/4/5 are not listed (they take default)
// 0x444d40
bool SurfaceRaisesDust(int surface)
{
	switch (surface) {
	case SURFACE_ROCK:
		return true;
	case SURFACE_WATER:
	case SURFACE_WOOD:
	case SURFACE_HARD:
		return false;
	default:
		return true;
	}
}

// 0x444d80
int FootprintLifetimeMs(int surface)
{
	if (surface > 0) {
		if (surface <= SURFACE_WATER || surface == SURFACE_MUD)
			return 1000;
	}
	return 0;
}

// 0x444da0
RpClump *CTFBModel::CloneClump()
{
	const char *type = 0;
	const void *obj = RWS::CResourceManager::FindById(&m_clumpId, &type, 0, 0);
	if (obj)
		return RpClumpClone((RpClump *)obj);
	return 0;
}

// Takes the first resource of the packet's system commands (command 0) that is a clump as m_clumpId; false if there is none.
// 0x444ec0
bool CTFBModel::FindClumpId(const RWS::CAttributePacket &attr)
{
	RWS::CAttributeCommandIterator it(attr, RWS::CSystemCommands::pCSystemCommandsClassID);
	while (!it.IsFinished()) {
		if (it->GetCommandId() == 0) {
			const ModelGuid *guid = static_cast<const ModelGuid *>(it->GetAs_void_ptr());
			const char *type = 0;
			const void *obj = RWS::CResourceManager::FindById(guid, &type, 0, 0);
			if (obj && !rwstrcmp("rwID_CLUMP", type)) {
				m_clumpId = *guid;
				return true;
			}
		}
		++it;
	}
	return false;
}

// Atomic callback: the mouth bone id is the int user data "TFB_MESH.MOUTH_BONE_ID" of the atomic's geometry; stops at the first.
// 0x444ad0
RpAtomic *GetMouthBoneIdCB(RpAtomic *atomic, void *data)
{
	RpGeometry *geom = RpAtomicGetGeometry(atomic);
	if (!geom)
		return atomic;
	int count = RpGeometryGetUserDataArrayCount(geom);
	for (int i = 0; i < count; i++) {
		RpUserDataArray *arr = RpGeometryGetUserDataArray(geom, i);
		const char *name = RpUserDataArrayGetName(arr);
		if (strstr(name, "TFB_MESH.MOUTH_BONE_ID") && RpUserDataArrayGetNumElements(arr) > 0 &&
		    RpUserDataArrayGetFormat(arr) == rpINTUSERDATA) {
			*(int *)data = RpUserDataArrayGetInt(arr, 0);
			return 0;
		}
	}
	return atomic;
}

// Atomic callback: the four eye bone ids are the int user data "TFB_MESH.EUL / EUR / ELL / ELR_BONE_ID" of the geometry; `data` is
// the int[4] to fill (eye upper / lower, left / right).
// 0x444b60
RpAtomic *GetEyeBoneIdsCB(RpAtomic *atomic, void *data)
{
	RpGeometry *geom = RpAtomicGetGeometry(atomic);
	if (!geom)
		return atomic;
	int *out = (int *)data;
	int count = RpGeometryGetUserDataArrayCount(geom);
	for (int i = 0; i < count; i++) {
		RpUserDataArray *arr = RpGeometryGetUserDataArray(geom, i);
		const char *name = RpUserDataArrayGetName(arr);
		if (strstr(name, "TFB_MESH.EUL_BONE_ID") && RpUserDataArrayGetNumElements(arr) > 0 && RpUserDataArrayGetFormat(arr) == rpINTUSERDATA)
			out[0] = RpUserDataArrayGetInt(arr, 0);
		if (strstr(name, "TFB_MESH.EUR_BONE_ID") && RpUserDataArrayGetNumElements(arr) > 0 && RpUserDataArrayGetFormat(arr) == rpINTUSERDATA)
			out[1] = RpUserDataArrayGetInt(arr, 0);
		if (strstr(name, "TFB_MESH.ELL_BONE_ID") && RpUserDataArrayGetNumElements(arr) > 0 && RpUserDataArrayGetFormat(arr) == rpINTUSERDATA)
			out[2] = RpUserDataArrayGetInt(arr, 0);
		if (strstr(name, "TFB_MESH.ELR_BONE_ID") && RpUserDataArrayGetNumElements(arr) > 0 && RpUserDataArrayGetFormat(arr) == rpINTUSERDATA)
			out[3] = RpUserDataArrayGetInt(arr, 0);
	}
	return atomic;
}

// Disabled (below the bar): only the stack frame differs from the game (sub esp,4A4h, eight 16-byte guid temporaries at esp+28..98); ours is
// 0x484/0x4A4
// depending on the name size, in a different slot order. The declaration order of the guids does not move the slots.
void CTFBModel::HandleAttributes(const RWS::CAttributePacket &attr)
{
	char name[1024];   // the game's buffer is 1024 bytes (esp+0xB0 in the 0x4A4 frame)
	RWS::CAttributeCommandIterator it(attr, g_ctfbModelClassId);
	name[0] = 0;
	for (; !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case MODEL_ATTR_NAME: {
			const char *s = it->GetAs_RwChar_ptr();
			strcpy(name, s);
			m_name = (const char *)(intptr_t)FUN_0043e6f0(name);
			break;
		}
		case MODEL_ATTR_SLOT_GUID: {
			const ModelResAttr *a = static_cast<const ModelResAttr *>(it->GetAs_void_ptr());
			ModelGuid t;
			a->ToGuid(&t);
			ModelGuid g = t;
			m_modelState.SetSlotAnimation(a->m_index, g);
			break;
		}
		case MODEL_ATTR_MOUTH_ANIM: {
			const ModelResAttr *a = static_cast<const ModelResAttr *>(it->GetAs_void_ptr());
			ModelGuid t;
			a->ToGuid(&t);
			ModelGuid g = t;
			const char *resType;
			const char *resName;
			const void *anim = RWS::CResourceManager::FindById(&g, &resType, 0, &resName);
			if (anim) {
				if (rwstrcmp("rwID_HANIMANIMATION", resType) == 0) {
					if ((unsigned)a->m_index >= 9)
						PortAnimOutOfRange(this, "mouth", 0x24, a->m_index, anim, resName);
					else
					m_mouthAnims[a->m_index] = (RtAnimAnimation *)anim;
					m_haveMouthAnims = 1;
				}
			}
			break;
		}
		case MODEL_ATTR_EYE_ANIM: {
			const ModelResAttr *a = static_cast<const ModelResAttr *>(it->GetAs_void_ptr());
			ModelGuid t;
			a->ToGuid(&t);
			ModelGuid g = t;
			const char *resType;
			const char *resName;
			const void *anim = RWS::CResourceManager::FindById(&g, &resType, 0, &resName);
			if (anim) {
				if (rwstrcmp("rwID_HANIMANIMATION", resType) == 0) {
					if ((unsigned)a->m_index >= 4)
						PortAnimOutOfRange(this, "eye", 0x50, a->m_index, anim, resName);
					else
					m_eyeAnims[a->m_index] = (RtAnimAnimation *)anim;
					m_haveEyeAnims = 1;
				}
			}
			break;
		}
		case MODEL_ATTR_SLOT: {
			const ModelResAttr *a = static_cast<const ModelResAttr *>(it->GetAs_void_ptr());
			ModelGuid t;
			a->ToGuid(&t);
			ModelGuid g = t;
			m_modelState.SetSlot(a->m_index, g, a->m_2c, a->m_30);
			break;
		}
		case MODEL_ATTR_RAW_PARAMS: {
			const RWS::CAttributeCommand *cmd = it.operator->();
			m_rawParams = *static_cast<const ModelRaw *>(cmd->GetAs_void_ptr());
			break;
		}
		case MODEL_ATTR_NUM_ENTRIES: {
			int count = it->GetAs_RwInt32();
			m_numEntries = count;
			if (count)
				m_entries = (ModelEntry *)RwMalloc(count << 6, 0x30000);
			break;
		}
		case MODEL_ATTR_ENTRY: {
			const ModelEntry *src = static_cast<const ModelEntry *>(it->GetAs_void_ptr());
			if (m_entries) {
				unsigned idx = src->m_index;
				if (idx < (unsigned)m_numEntries) {
					ModelEntry *entry = &m_entries[idx];
					memcpy(entry, src, sizeof(ModelEntry));
					entry->m_index = idx;
					if (src->m_kind < 2) {
						entry->m_fov = 90.0f;
						entry->m_smoothPercent = 15;
						entry->m_unk38 = 30.0f;
						entry->m_unk3c = 45.0f;
						entry->m_roll = 0;
					}
				}
			}
			break;
		}
		}
	}
	m_modelState.FillEmptySlots();
	if (m_clumpId.m_Data[0] != 0 || FindClumpId(attr) && m_clumpId.m_Data[0] != 0) {
		const char *resType = 0;
		const void *clump = RWS::CResourceManager::FindById(&m_clumpId, &resType, 0, 0);
		if (clump) {
			int idx = -1;
			RpClumpForAllAtomics((RpClump *)clump, GetMouthBoneIdCB, &idx);
			m_mouthBoneId = idx;
			RpClumpForAllAtomics((RpClump *)clump, GetEyeBoneIdsCB, m_eyeBoneIds);
		}
	}
}

// A script object of the Sound type as the registry search reads it: its +4 points 0x1c bytes into the sound, whose name pointer is
// at +0x20 (the CTFBSound behind the script object; the audio package owns it).
#include "modules/tfb/ctfbsound.h"
typedef Script::ScriptSoundObj RegisteredSoundView;
ZOOSTERS_ASSERT_OFFSET_AT(ScriptSoundObj_link, offsetof(Script::ScriptSoundObj, m_link), 4);   // the game build's RegisteredSoundView::m_sound
// the SoundLink base of a CTFBSound is at +0x1c (the game build: m_sound - 0x1c), its m_name at +0x20: CAttributeHandler is 0x1c bytes, SoundLink 4
typedef char zoosters_soundlink_at_1c[ZOOSTERS_LAYOUT_CHECK(sizeof(RWS::CAttributeHandler) == 0x1c && sizeof(RWS::TFB::SoundLink) == 4 && offsetof(RWS::TFB::CTFBSound, m_name) == 0x20) ? 1 : -1];

// A footstep sound object named "footsteps_<base>_<surface>_<a..d>": filed under its surface and letter (the first match).
// `tmp` holds "footsteps_<base>_" (len characters); the surface suffix is written behind it for each try.
static void FileStepSound(SurfaceEffects *surfaces, char *tmp, int len, const char *soundName, RegisteredSoundView *sound)
{
	for (int i = 0; i < SURFACE_COUNT; i++) {
		for (unsigned j = 0; j < 4; j++) {
			char *full = tmp + len;
			sprintf(full, "%s_%c", g_surfaceNames[i], 'a' + j);
			// the second argument is tmp (the whole "footsteps_<base>_<surf>_<c>"), not tmp+len
			if (_stricmp(tmp, soundName) == 0) {
				surfaces[i].m_steps[j] = (Script::ScriptReference *)sound;
				return;
			}
		}
	}
}

// Finds, once, what the model brings for each surface: the footprint clump "footprints_<model>_<surface>.dff" and the footstep
// sounds "footsteps_<model>_<surface>_<a..d>" (script Sound objects, searched in the symbol registry by their name prefix).
// <model> is the clump resource's name up to its first '_', in lower case.
// 0x4453c0
void CTFBModel::ResolveSurfaceEffects()
{
	if (m_surfacesResolved)
		return;
	m_surfacesResolved = 1;
	const char *name = 0;
	if (!RWS::CResourceManager::FindById(&m_clumpId, 0, 0, &name) || !name)
		return;
	const char *dot = strstr(name, ".dff");
	if (!dot)
		return;
	char base[0x400];
	int n = dot - name;
	strncpy(base, name, n);
	base[n] = 0;
	char *us = strchr(base, '_');
	if (us)
		*us = 0;
	_strlwr(base);
	char tmp[0x400];
	for (int i = 0; i < SURFACE_COUNT; i++) {
		sprintf(tmp, "footprints_%s_%s.dff", base, g_surfaceNames[i]);
		m_surfaces[i].m_dust = (RpClump *)RWS::CResourceManager::FindByName(tmp, 0, 0, 0);
	}
	int len = sprintf(tmp, "footsteps_%s_", base);
	for (int idx = g_globalSymbolCount; idx < SymbolRegistry_GetSize(); idx++) {
		RegisteredSoundView *o;
		if (idx >= 0x3fc0d) {
			o = (RegisteredSoundView *)g_curScript->m_locals[idx - 0x3fc0d];
		} else {
			uintptr_t v = (*g_symRegistryY)[idx].obj;
			if (v & 1)
				continue;
			o = (RegisteredSoundView *)(v & ~(uintptr_t)1);
		}
		// the registry slot holds any script object: test its type as a ScriptReference, only then it is a ScriptSoundObj
		if (!o || static_cast<Script::ScriptReference *>(o)->GetType() != TFBType_Sound())
			continue;
		RWS::TFB::CTFBSound *sound = o->GetSound();
		if (!sound)
			continue;
		if (_strnicmp(tmp, sound->m_name, len) != 0)
			continue;
		FileStepSound(m_surfaces, tmp, len, sound->m_name, o);
	}
}
