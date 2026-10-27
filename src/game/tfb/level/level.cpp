#include "tfb/level/level.h"
#include "modules/tfb/ctfbworld.h"
#include "tfb/render/yonbox.h"
#include "rw/rpworld.h"
#include "tfb/render/tfbclump.h"
#include "tfb/render/tfblight.h"
#include "tfb/render/atomicflags.h"
#include "tfb/level/level_views.h"

// GLOBAL: 0x629724 RWS::CLevel::sm_pOpaqueWorld
// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld
// GLOBAL: 0x629728 RWS::CLevel::sm_pTranslucentWorld
// GLOBAL: 0x629730 RWS::CLevel::sm_pPreOpaqueWorld
// GLOBAL: 0x629734 RWS::CLevel::sm_pSkyWorld
// GLOBAL: 0x629738 RWS::CLevel::sm_pFogWorld
// GLOBAL: 0x629740 g_levelWorldsReady
// GLOBAL: 0x62971c g_lightList
// GLOBAL: 0x629714 g_clumpList
// GLOBAL: 0x6296ec g_levelClump
// GLOBAL: 0x60297c g_levelFarDefault
// GLOBAL: 0x629710 g_nearClip
// GLOBAL: 0x62970c g_farClip
// GLOBAL: 0x629700 g_fogMode
// GLOBAL: 0x629704 g_vertFogColor
// GLOBAL: 0x629708 g_vertFogDist
// GLOBAL: 0x6296fc g_levelFogParam
// GLOBAL: 0x6221b0 g_levelZone
// GLOBAL: 0x6296f4 g_fogParamB
// GLOBAL: 0x6296f8 g_fogParamA
// GLOBAL: 0x62a59c g_distClip
// GLOBAL: 0x62a5a0 g_distance
// GLOBAL: 0x62973c g_yonObject

using RWS::CLevel;

ZOOSTERS_NOINLINE int GetCurrentLevel();   // 0x460c20 (noinline: SetFogFarClip / SetFogPlane call it as a function)
void WorldRemoveAllObjects(RpWorld *world);   // 0x44f230

enum { kClumpInWorldLink = offsetof(RpClump, inWorldLink) };
static RpClump *ClumpFromWorldLink(RwLLLink *link)
{
	return (RpClump *)((char *)link - kClumpInWorldLink);
}

// Teardown callback of a sector's atomics: takes the atomic out of its world and queues its clump (unless marked or already queued) to be filed again when the worlds are ready.
// 0x44d820
RpAtomic *LevelUnworldAtomicCB(RpAtomic *atomic, void *data)
{
	RpWorldRemoveAtomic(RpAtomicGetWorld(atomic), atomic);
	RpClump *clump = RpAtomicGetClump(atomic);
	if (clump) {
		if (!clump->inWorldLink.next && !TFBPlugin_B1_TestFlags(clump, 2)) {
			clump->inWorldLink.next = g_clumpList.next;
			clump->inWorldLink.prev = &g_clumpList;
			g_clumpList.next->prev = &clump->inWorldLink;
			g_clumpList.next = &clump->inWorldLink;
		}
	}
	return atomic;
}

// 0x44d890
RpWorldSector *LevelUnworldSectorCB(RpWorldSector *sector, void *data)
{
	RpWorldSectorForAllAtomics(sector, LevelUnworldAtomicCB, data);
	return sector;
}

// 0x44d7c0
RpLight *LevelMoveLightCB(RpLight *light, void *data)
{
	RpWorld *world = (RpWorld *)data;
	RpWorldRemoveLight(RpLightGetWorld(light), light);
	if (world) {
		RpWorldAddLight(world, light);
		return light;
	}
	RwLLLink *node = RpLightGetWorldLink(light);
	node->next = g_lightList.next;
	RpLightGetWorldLink(light)->prev = &g_lightList;
	g_lightList.next->prev = node;
	g_lightList.next = node;
	return light;
}

// 0x44d8b0
void LevelDestroyWorlds(bool keepOpaque)
{
	if (!keepOpaque) {
		RpWorld *w = CLevel::sm_pOpaqueWorld;
		if (w) {
			RpWorldForAllWorldSectors(w, LevelUnworldSectorCB, 0);
			RpWorldForAllLights(w, LevelMoveLightCB, 0);
			WorldRemoveAllObjects(w);
			CLevel::sm_pOpaqueWorld = 0;
		}
		w = CLevel::sm_pTranslucentWorld;
		if (w) {
			RpWorldForAllWorldSectors(w, LevelUnworldSectorCB, 0);
			RpWorldForAllLights(w, LevelMoveLightCB, 0);
			WorldRemoveAllObjects(w);
			CLevel::sm_pTranslucentWorld = 0;
		}
		if (CLevel::sm_pFogWorld) {
			WorldRemoveAllObjects(CLevel::sm_pFogWorld);
			CLevel::sm_pFogWorld = 0;
		}
		if (CLevel::sm_pPreOpaqueWorld) {
			WorldRemoveAllObjects(CLevel::sm_pPreOpaqueWorld);
			CLevel::sm_pPreOpaqueWorld = 0;
		}
	}
	RpWorld *w = CLevel::sm_pActiveWorld;
	if (w) {
		RpWorldForAllLights(w, LevelMoveLightCB, 0);
		WorldRemoveAllObjects(w);
		CLevel::sm_pActiveWorld = 0;
	}
	if (CLevel::sm_pSkyWorld) {
		WorldRemoveAllObjects(CLevel::sm_pSkyWorld);
		CLevel::sm_pSkyWorld = 0;
	}
	g_levelWorldsReady = 0;
	g_levelFarDefault = -1.0f;
}

// Every atomic of the clump is filed into the opaque world or the yon box.
// 0x44da10
void RWS::CLevel::AddClump(RpClump *clump)
{
	if (g_levelWorldsReady) {
		clump->inWorldLink.prev = 0;
		clump->inWorldLink.next = 0;
		RpWorld *w = RpClumpGetWorld(clump);
		if (w)
			RpWorldRemoveClump(w, clump);
		RpClumpForAllAtomics(clump, FileAtomicInLevelWorld, w);
	} else if (!clump->inWorldLink.next) {
		clump->inWorldLink.next = g_clumpList.next;
		clump->inWorldLink.prev = &g_clumpList;
		g_clumpList.next->prev = &clump->inWorldLink;
		g_clumpList.next = &clump->inWorldLink;
	}
}

// 0x44da90
RpWorld *RWS::CLevel::AddLight(RpLight *light, int level)
{
	RpWorld *w = 0;
	TFBPlugin_C2_SetLevel(light, level);
	if (g_levelWorldsReady) {
		if (TFBPlugin_C2_GetLevel(light) == 3) {
			w = CLevel::sm_pActiveWorld;
		} else {
			w = CLevel::sm_pOpaqueWorld;
			if (!w)
				w = CLevel::sm_pTranslucentWorld;
		}
		if (w)
			RpWorldAddLight(w, light);
	} else if (!RpLightGetWorldLink(light)->next) {
		RpLightGetWorldLink(light)->next = g_lightList.next;
		RpLightGetWorldLink(light)->prev = &g_lightList;
		g_lightList.next->prev = RpLightGetWorldLink(light);
		g_lightList.next = RpLightGetWorldLink(light);
	}
	return w;
}

// 0x44d780
RpWorld *RWS::CLevel::AddAtomic(RpAtomic *atomic)
{
	if (g_levelClump) {
		RpClumpAddAtomic(g_levelClump, atomic);
		if (g_levelWorldsReady)
			RpWorldAddAtomic(RpClumpGetWorld(g_levelClump), atomic);
	}
	return 0;
}

// 0x44db20
void LevelAddPendingLights()
{
	if (g_lightList.next != &g_lightList) {
		do {
			RpLight *light = RpLightFromWorldLink(g_lightList.next);
			LevelListRemove(RpLightGetWorldLink(light));
			RpWorld *w = RpLightGetWorld(light);
			if (w)
				RpWorldRemoveLight(w, light);
			if (TFBPlugin_C2_GetLevel(light) == 3) {
				w = CLevel::sm_pActiveWorld;
			} else {
				w = CLevel::sm_pOpaqueWorld;
				if (!w)
					w = CLevel::sm_pTranslucentWorld;
			}
			if (w)
				RpWorldAddLight(w, light);
		} while (g_lightList.next != &g_lightList);
	}
}

// 0x44dba0
void LevelWorldsSetReady()
{
	if (!g_levelWorldsReady && CLevel::sm_pOpaqueWorld && CLevel::sm_pActiveWorld) {
		LevelAddPendingLights();
		if (g_clumpList.next != &g_clumpList) {
			do {
				RpClump *clump = ClumpFromWorldLink(g_clumpList.next);
				LevelListRemove(&clump->inWorldLink);
				clump->inWorldLink.prev = 0;
				clump->inWorldLink.next = 0;
				RpWorld *w = RpClumpGetWorld(clump);
				if (w)
					RpWorldRemoveClump(w, clump);
				RpClumpForAllAtomics(clump, FileAtomicInLevelWorld, w);
			} while (g_clumpList.next != &g_clumpList);
		}
		g_levelWorldsReady = 1;
	}
}

// 0x44dc30
void LevelResetWorlds()
{
	LevelDestroyWorlds(false);
	g_lightList.next = &g_lightList;
	g_lightList.prev = &g_lightList;
	g_clumpList.next = &g_clumpList;
	g_clumpList.prev = &g_clumpList;
	if (g_levelClump)
		RWS::CLevel::AddClump(g_levelClump);
	g_nearClip = 15.0f;
	g_farClip = 20000.0f;
	g_fogMode = 1;
	g_vertFogColor = 0;
	g_levelFogParam = 0;
	g_vertFogDist = 0;
	g_levelZone = 0;
}

// 0x44d9b0
void LevelHideClump()
{
	if (g_levelClump)
		ClumpSetIsVisible(g_levelClump, false, -1.0f, -1.0f);
}

// 0x44d9d0
void LevelShowClump()
{
	if (g_levelClump)
		ClumpSetIsVisible(g_levelClump, true, -1.0f, -1.0f);
}

// 0x44d9f0
void LevelDestroyClump()
{
	if (g_levelClump) {
		RpClumpDestroy(g_levelClump);
		g_levelClump = 0;
	}
}

// 0x44d300
float SetFarClip(float distance)
{
	g_farClip = distance;
	return distance;
}

// 0x44d310
void SetVertFogDistance(float distance)
{
	g_vertFogDist = distance;
}

// 0x44d320
void SetVertFogColor(unsigned argb)
{
	g_vertFogColor = argb;
}

// 0x44d330
void SetLevelFogParam(float value)
{
	g_levelFogParam = value;
}

// 0x44d340
float SetFogFarClip(float v)
{
	float c;
	return g_fogParamA = (g_distClip && GetCurrentLevel() != 1 && GetCurrentLevel() != 3 && GetCurrentLevel() != 8 &&
	                      GetCurrentLevel() != 0xb && GetCurrentLevel() != 0xe && GetCurrentLevel() != 0xd &&
	                      GetCurrentLevel() != 0xf && (c = v / (3.0f - (float)g_distance * 0.02f)) < v) ? c : v;
}

// 0x44d3c0
float SetFogPlane(float v)
{
	if (v == -1.0f)
		return g_fogParamA;
	if (g_distClip && GetCurrentLevel() != 1 && GetCurrentLevel() != 3 && GetCurrentLevel() != 8 && GetCurrentLevel() != 0xb &&
	    GetCurrentLevel() != 0xe && GetCurrentLevel() != 0xd && GetCurrentLevel() != 0xf) {
		float c = v / ((3.0f - (float)g_distance * 0.02f) * 4.0f);
		if (c < v)
			g_fogParamA = c;
		else
			g_fogParamA = v;
		return g_fogParamB;
	}
	*(int *)&g_fogParamB = *(int *)&v;
	return g_fogParamB;
}

static inline void WorldBounds(RwBBox *out, const RpWorld *world)
{
	const RpWorldView *w = (const RpWorldView *)world;
	out->inf.x = w->boundingBox.inf.x + w->worldOrigin.x;
	out->inf.y = w->boundingBox.inf.y + w->worldOrigin.y;
	out->inf.z = w->boundingBox.inf.z + w->worldOrigin.z;
	out->sup.x = w->boundingBox.sup.x + w->worldOrigin.x;
	out->sup.y = w->boundingBox.sup.y + w->worldOrigin.y;
	out->sup.z = w->boundingBox.sup.z + w->worldOrigin.z;
}

static inline void BoxUnion(RwBBox *box, const RwBBox *t)
{
	if (box->inf.x > t->inf.x)
		box->inf.x = t->inf.x;
	if (box->inf.y > t->inf.y)
		box->inf.y = t->inf.y;
	if (box->inf.z > t->inf.z)
		box->inf.z = t->inf.z;
	if (box->sup.x < t->sup.x)
		box->sup.x = t->sup.x;
	if (box->sup.y < t->sup.y)
		box->sup.y = t->sup.y;
	if (box->sup.z < t->sup.z)
		box->sup.z = t->sup.z;
}

// Atomics whose clump has the yon flag go to the yon box (built on first use from the union of the opaque, translucent and pre-opaque world bounds); the others go into the opaque world. Does nothing for the opaque world's own atomics (data == the opaque world).
// 0x44d490
RpAtomic *FileAtomicInLevelWorld(RpAtomic *atomic, void *data)
{
	RpWorld *opaque = CLevel::sm_pOpaqueWorld;
	if (opaque != data) {
		RpWorld *w = RpAtomicGetWorld(atomic);
		if (w)
			RpWorldRemoveAtomic(w, atomic);
		if (RpAtomicGetClump(atomic) && TFBPlugin_B1_TestFlags(RpAtomicGetClump(atomic), LEVEL_CLUMP_YON)) {
			if (!g_yonObject->m_yonBox) {
				RwBBox box;
				RwBBox t;
				WorldBounds(&box, CLevel::sm_pOpaqueWorld);
				if (CLevel::sm_pTranslucentWorld)
					WorldBounds(&t, CLevel::sm_pTranslucentWorld);
				BoxUnion(&box, &t);
				if (CLevel::sm_pPreOpaqueWorld)
					WorldBounds(&t, CLevel::sm_pPreOpaqueWorld);
				BoxUnion(&box, &t);
				g_yonObject->m_yonBox = new YonBox(&box);
			}
			g_yonObject->m_yonBox->AddAtomic(atomic);
		} else if (opaque) {
			RpWorldAddAtomic(opaque, atomic);
		}
	}
	return atomic;
}

ZOOSTERS_NOINLINE void LevelResetWorlds();
extern int g_currentLevel;

// 0x44dcc0
void LevelResetWorlds_thunk()
{
	LevelResetWorlds();
}

// 0x460c20
int GetCurrentLevel()
{
	return g_currentLevel;
}
