// Game state for files that include librw (<rw.h>) and so cannot include the game's headers: shim/rwcore.cpp, rwshim.cpp,
// rwaudio.cpp, debugui_rw.cpp, debugui_freecam.cpp. game_access.cpp implements the accessors against the real headers, so the
// compiler checks each against the game's declaration. Plain float / int / bool globals need no accessor: declare them extern.
#ifndef PORT_GAME_ACCESS_H
#define PORT_GAME_ACCESS_H

struct RpWorld;
struct RwCamera;
struct OccluderNode;
class ShadowRender;
namespace RWS { namespace TFB { class TFBShadowCamera; } }

// RWS::CLevel's world slots (src/game/tfb/level/level.h)
RpWorld *&PortGame_OpaqueWorld();        // 0x629724 sm_pOpaqueWorld
RpWorld *&PortGame_TranslucentWorld();   // 0x629728 sm_pTranslucentWorld
RpWorld *&PortGame_ActiveWorld();        // 0x62972c sm_pActiveWorld
RpWorld *&PortGame_SkyWorld();           // 0x629734 sm_pSkyWorld
RpWorld *&PortGame_FogWorld();           // 0x629738 sm_pFogWorld

RwCamera *&PortGame_CamDst();                                // 0x6182d0 g_gameCamDst: the RwCamera the game renders the 3D view through
RWS::TFB::TFBShadowCamera *&PortGame_ShadowCamera();        // 0x6182f4 g_shadowCamera
ShadowRender *&PortGame_ShadowRender();                      // 0x622404 g_shadowRender
OccluderNode *&PortGame_PlaneList();                         // 0x617e78 g_planeList413 (first node; the list is linked through the first word)
void *PortGame_SymRegistry();                                // 0x61f878 g_symRegistryY (the vector of 8-byte slots: obj|tag, key); 0 before the first fill
void *PortGame_ClassMap();                                   // 0x5522b0 RWS::CClassFactory::GetClassMap: the std::map of the game's CRegistrar

#endif
