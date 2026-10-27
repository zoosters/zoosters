// PS2 base order: CAttributeHandler@0, LinearAllocationPolicy@28. ONE vtable 0x5d4e5c = [~ 0x445690, HandleAttributes 0x444fa0,
// base, base]; no CEventHandler. Registered size 0x2f0 (RegisterFactory 0x5c6210).
#ifndef ZOOSTERS_MODULES_TFB_CTFBMODEL_H
#define ZOOSTERS_MODULES_TFB_CTFBMODEL_H

#include "tfb/anim/tfbmodelstate.h"   // first: <vector> before the framework headers
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/resourceref.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "rw/rpworld.h"
#include "rw/rtanim.h"

namespace Script {
class ScriptReference;
}

// Parsed from the text of an attribute resource field by 0x4500b0 (strtoul of hex groups).
typedef RWS::RWSGUID ModelGuid;   // the 16-byte resource id
// Attribute field of CTFBModel commands 0, 3, 6, 7: first dword = slot index, text from +5 (guid), more ints at +0x2c/+0x30.
struct ModelResAttr {
	int m_index;
	char m_text[0x28];
	int m_2c, m_30;
	bool ToGuid(ModelGuid *out) const;
};

inline bool ModelResAttr::ToGuid(ModelGuid *out) const { return ((const RWS::ResourceRef *)this)->GetGuid(out); }
struct ModelRaw { int m_d[0x35]; };    // 0xd4 bytes copied as a whole

// The order of g_surfaceNames (0x5d4ed8) and of CTFBModel::m_surfaces.
enum Surface {
	SURFACE_ROCK = 0,
	SURFACE_DIRT = 1,
	SURFACE_WATER = 2,
	SURFACE_SAND = 3,
	SURFACE_GRASS = 4,
	SURFACE_MUD = 5,
	SURFACE_WOOD = 6,
	SURFACE_HARD = 7,
	SURFACE_COUNT = 8
};

struct SurfaceEffects {
	RpClump *m_dust;                            // +0x00 "footprints_<model>_<surface>.dff": the clump cloned as dust where a foot lands
	Script::ScriptReference *m_steps[4];        // +0x04 the footstep sounds "footsteps_<model>_<surface>_<a..d>" (script objects of the Sound type)
};
ZOOSTERS_ASSERT_SIZE(SurfaceEffects, 0x14);

// Command 5 copies it from the attribute and puts its own index at +0xc. Kinds below 2 get fixed values at +0x14 / +0x1c / +0x34..+0x3c.
struct ModelEntry {
	unsigned m_kind;       // +0x00
	char m_pad04[8];
	unsigned m_index;      // +0x0c
	char m_pad10[4];
	int m_smoothPercent;   // +0x14 15 for kinds below 2: the part's follow smoothing in percent (the part constructor stores 1 - this / 100 at part +0x104)
	char m_pad18[4];
	int m_roll;            // +0x1c float bits, 0 for kinds below 2; the part constructor copies it to the part's local roll (+0xb0)
	char m_pad20[0x14];
	float m_fov;           // +0x34 90.0 for kinds below 2; copied to the part's FOV (+0x100)
	float m_unk38;         // +0x38 30.0 for kinds below 2; written by command 5, never read
	float m_unk3c;         // +0x3c 45.0, the same
};
ZOOSTERS_ASSERT_SIZE(ModelEntry, 0x40);

class CTFBModel : public RWS::CAttributeHandler, public RWS::LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(CTFBModel);
	CTFBModel(const RWS::CAttributePacket &attr);
	~CTFBModel();
	virtual void HandleAttributes(const RWS::CAttributePacket &attr);
	bool FindClumpId(const RWS::CAttributePacket &attr);   // takes the first clump resource of the packet's system commands as m_clumpId
	RpClump *CloneClump();
	// Finds the footprint models and footstep sounds of every surface by name (once, on the first call; Corpus::Footsteps).
	void ResolveSurfaceEffects();
	// The model size GameSprite::UpdateExtents reads: the floats at +0x274 / +0x278, dwords 0x17 / 0x18 of the raw copy of command 1
	float Height() const { return *(const float *)&m_rawParams.m_d[0x17]; }
	float Width() const { return *(const float *)&m_rawParams.m_d[0x18]; }

	int m_numEntries;                // +0x1c: entry count of m_entries (command 4)
	ModelEntry *m_entries;           // +0x20: RwMalloc'd array (command 5)
	RtAnimAnimation *m_mouthAnims[9];   // +0x24: the mouth animations by slot (command 6; resources of type rwID_HANIMANIMATION)
	unsigned char m_haveMouthAnims;  // +0x48: set with m_mouthAnims
	int m_mouthBoneId;               // +0x4c: "TFB_MESH.MOUTH_BONE_ID" of the clump's mesh (-1 if none), found by GetMouthBoneIdCB
	RtAnimAnimation *m_eyeAnims[4];  // +0x50: the four eye animations (command 7)
	unsigned char m_haveEyeAnims;    // +0x60
	char m_pad61[3];
	int m_eyeBoneIds[4];             // +0x64: the bone ids "TFB_MESH.EUL / EUR / ELL / ELR_BONE_ID" (-1 = none), found by GetEyeBoneIdsCB
	unsigned char m_surfacesResolved; // +0x74 set on the first call of ResolveSurfaceEffects
	char m_pad75[3];
	SurfaceEffects m_surfaces[SURFACE_COUNT];   // +0x78 .. 0x118, zeroed by the constructor
	ModelGuid m_clumpId;             // +0x118: clump resource id (FindClumpId sets it)
	TFBModelState m_modelState;      // +0x128 .. 0x218: the animations of the model by slot (0xf0 bytes)
	ModelRaw m_rawParams;            // +0x218: raw copy of command 1 (0x35 dwords; GameSprite reads the model size at +0x5c / +0x60 of it)
	const char *m_name;              // +0x2ec: command 2 (a name run through the folded `return 0` at 0x43e6f0: always 0 in the shipped game)
};
ZOOSTERS_ASSERT_SIZE(CTFBModel, 0x2f0);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_numEntries, 0x1c);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_entries, 0x20);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_mouthAnims, 0x24);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_haveMouthAnims, 0x48);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_mouthBoneId, 0x4c);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_eyeAnims, 0x50);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_haveEyeAnims, 0x60);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_eyeBoneIds, 0x64);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_surfacesResolved, 0x74);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_surfaces, 0x78);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_clumpId, 0x118);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_modelState, 0x128);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_rawParams, 0x218);
ZOOSTERS_ASSERT_OFFSET(CTFBModel, m_name, 0x2ec);
ZOOSTERS_ASSERT_OFFSET_AT(CTFBModel_height, offsetof(CTFBModel, m_rawParams.m_d[0x17]), 0x274);   // GameSprite::UpdateExtents' model height
ZOOSTERS_ASSERT_OFFSET_AT(CTFBModel_width, offsetof(CTFBModel, m_rawParams.m_d[0x18]), 0x278);    // its width

int SurfaceHoldsFootprint(int surface);   // every surface but water keeps a footprint's effect
int FootprintLifetimeMs(int surface);   // 1000 for dirt, water and mud, else 0
bool SurfaceRaisesDust(int surface);   // false for water, wood and hard ground

#endif
