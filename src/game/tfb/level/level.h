// The level's RenderWare worlds, the pending lights and clumps, and the clip / fog state. RWS::CLevel files atomics, clumps and lights into the worlds.
#ifndef ZOOSTERS_TFB_LEVEL_LEVEL_H
#define ZOOSTERS_TFB_LEVEL_LEVEL_H

#include "zoosters/base.h"
#include "framework/core/rwsguid.h"
#include "framework/core/resourceref.h"
#include "rw/rpworld.h"

extern RpClump *g_levelClump;
extern RwLLLink g_lightList;  // lights (RpLight::inWorld) waiting for the worlds
extern RwLLLink g_clumpList;  // clumps (RpClump::inWorldLink) waiting for the worlds
extern int g_levelWorldsReady;  // nonzero once the opaque and active world exist and the lists were flushed

extern float g_nearClip;
extern float g_farClip;
extern float g_levelFarDefault;  // far clip the level started with (-1 until command 3 sets it)
extern int g_fogMode;  // 0 off, else the RwFogType
extern int g_fogFlag;                     // 0x6296f0 nonzero when RenderFogPass sets the camera's fog plane
extern unsigned g_vertFogColor;  // D3DCOLOR
extern float g_vertFogDist;
extern float g_levelFogParam;  // handed to RwRenderState FOGDENSITY
extern float g_fogParamA;  // far clip of the fog pass (SetFogFarClip)
extern float g_fogParamB;  // fog plane of the fog pass (SetFogPlane)
extern int g_levelZone;  // ScriptSystem LevelZone
// Shrink fog and pixel-fog distances on the levels outside {1, 3, 8, 11, 13, 14, 15}: divided by 3 - g_distance * 0.02. Purpose unknown (a video option?).
extern unsigned char g_distClip;
extern int g_distance;

// The TFB clump flag that marks distant (yon) geometry: AddClumpResource sets it, FileAtomicInLevelWorld hands such atomics to the yon box.
enum { LEVEL_CLUMP_YON = 2 };

// RpLight's world link sits at +0x34; the game unlinks and relinks it there.
struct RpLightView {
	RwObjectHasFrameView object;
	RwReal radius;
	RwRGBAReal color;
	RwReal minusCosAngle;
	RwLLLink worldSectorsInLight;
	RwLLLink inWorld;
};
ZOOSTERS_ASSERT_OFFSET(RpLightView, inWorld, 0x34);
enum { kLightInWorldLink = offsetof(RpLightView, inWorld) };
static inline RwLLLink *RpLightGetWorldLink(RpLight *light)
{
	return &((RpLightView *)light)->inWorld;
}
static inline RpLight *RpLightFromWorldLink(RwLLLink *link)
{
	return (RpLight *)((char *)link - kLightInWorldLink);
}
// RW's list helpers: unlink a link / put it first.
static inline void LevelListRemove(RwLLLink *link)
{
	link->prev->next = link->next;
	link->next->prev = link->prev;
}
static inline void LevelListAddFirst(RwLLLink *list, RwLLLink *link)
{
	link->next = list->next;
	link->prev = list;
	list->next->prev = link;
	list->next = link;
}

// A resource reference in a level behaviour's attribute data: the resource type, then the id text GetGuid parses (hex groups, as ModelResAttr::ToGuid).
struct LevelResourceRef {
	int m_type;
	char m_idText[0x20];
	void GetGuid(RWS::RWSGUID *out) const;   // 0x4500b0
};
inline void LevelResourceRef::GetGuid(RWS::RWSGUID *out) const { ((const RWS::ResourceRef *)this)->GetGuid(out); }

namespace RWS {

// The game's versions of the CLevel functions (AddLight takes the world level).
class CLevel {
public:
	// CTFBWorld command 0 stores its resource in the world slot of the type given in the attribute. The first three are stock CLevel; the rest are the game's additions, named by their role in CTFBWorld::RenderFrame.
	static RpWorld *sm_pOpaqueWorld;  // type 0: the level geometry (PVS data: the sector walk follows it); lights go here
	static RpWorld *sm_pTranslucentWorld;  // type 1: rendered back to front after the opaque pass (RenderTransparentPass)
	static RpWorld *sm_pSkyWorld;  // type 2: rendered first with the camera at the origin (RenderSkyPass)
	static RpWorld *sm_pActiveWorld;  // type 3: receives the lights of level 3 (TFBLightData::m_level)
	static RpWorld *sm_pFogWorld;  // type 5: rendered with the fog far clip g_fogParamA (RenderFogPass)
	static RpWorld *sm_pPreOpaqueWorld;  // type 6: rendered front to back before the opaque world (RenderFrame)
	static RpWorld *GetOpaqueWorld(void) { return sm_pOpaqueWorld; }

	// Adds the atomic to the level clump (and its world once ready). Always returns 0.
	static RpWorld *AddAtomic(RpAtomic *atomic);
	// Files a clump into its worlds, or the pending list.
	static void AddClump(RpClump *clump);
	// Sets the light's level and files it into the active world (level 3) or the opaque world; pending while none exists.
	static RpWorld *AddLight(RpLight *light, int level);
};

}  // namespace RWS

float SetFarClip(float distance);  // g_farClip = distance
void SetVertFogDistance(float distance);
void SetVertFogColor(unsigned argb);
void SetLevelFogParam(float value);
float SetFogFarClip(float distance);  // g_fogParamA, scaled down on the levels the settings above shrink
float SetFogPlane(float distance);  // g_fogParamB (-1: only returns g_fogParamA)

void LevelHideClump();
void LevelShowClump();
void LevelDestroyClump();
// The sky and active world are always destroyed (a LevelHub does that for its level); the opaque, translucent, fog and pre-opaque worlds only when keepOpaque is false (the CTFBWorld teardown).
void LevelDestroyWorlds(bool keepOpaque);
void LevelAddPendingLights();
void LevelWorldsSetReady();
void LevelResetWorlds();

// RpWorldForAllLights callback: moves the light into `data` (an RpWorld), or the pending list when none.
RpLight *LevelMoveLightCB(RpLight *light, void *data);
// RpClumpForAllAtomics callback: files an atomic in the opaque world, or hands it to the yon box when its clump is marked LEVEL_CLUMP_YON.
RpAtomic *FileAtomicInLevelWorld(RpAtomic *atomic, void *data);

#endif
