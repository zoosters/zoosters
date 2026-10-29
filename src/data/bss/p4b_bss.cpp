// Zero-initialised game globals (.bss and zeroed .data) that src/game declares extern, defined by name so the normal build links them.
// Types are the declarations' (struct tags only; layouts are local stand-ins where a by-value struct is needed).

#include "rw/rwcore.h"                          // RwV3d, RwLLLink, RwFrame, RwIm2DVertex (src/game)
#include "rw/rpworld.h"                         // RpWorld
#include "framework/core/ceventhandler.h"       // CEventId, CMsg
#include "tfb/level/level.h"                    // RWS::CLevel (the world statics)
#include "framework/startup/win32/input/pads.h"   // PadState
#include "framework/toolkits/audio/audio.h"       // RWS::RwsAudio::VirtualVoiceSlot
#include "tfb/hud/hud2d.h"                        // FontCacheEntry
#include "modules/tfb/camera_views.h"             // RwFrustumPlaneView
#include "framework/mainloop/gameclock.h"         // FrameRate
#include "modules/tfb/savefile.h"                 // SaveFileOverlapped
#include "script/ops_set.h"                       // Script::g_subsetTable
#include "script/symbolregistry.h"                // SymbolVec
namespace RWS { namespace Audio { class AudioGlobalMixer; } }
namespace RWS { class CEventId; }
namespace RWS { class CMsg; }
namespace RWS { namespace FX { class CFXMotionBlur; } }
namespace RWS { namespace TFB { class CTFBWorld; } }
namespace RWS { namespace TFB { class GameActor; } }
namespace RWS { namespace TFB { class GameCamera; } }
namespace RWS { namespace TFB { class GameController; } }
namespace RWS { namespace TFB { class GameSprite; } }
namespace RWS { namespace TFB { class SpriteManager; } }
namespace RWS { namespace TFB { class TFBShadowCamera; } }
namespace Script { class MessageActor; }
namespace Script { class OpCode; }
namespace Script { class OpContext; }
namespace Script { class ReferenceSet; }
namespace Script { class ScriptReference; }
namespace Script { class UserReference; }
namespace TFB { class MasterVertAnimation; }
namespace RWS { namespace RwsAudio { struct VirtualVoiceSlot; } }
namespace RWS { namespace TFB { struct CollisionPair; } }
namespace RWS { namespace TFB { struct Voice; } }
namespace Script { struct AssetEntry; }
namespace Script { struct PoolNodeW; }
struct StickyKeys { unsigned w[2]; };
struct RpSkinGlobals { int w[3]; };
struct TFBSystemHook { unsigned q[22]; };
// cl 13.10 aligns every zero-initialised .bss variable to 4 B, but the game packs byte flags (g_padStickUp/Down/Left/Right ...). In
// .data they sit at their natural alignment, as datacheck compares the distances between neighbours. The normal build keeps them in .bss.
// Normal build only: the address-bound port and datacheck bind these globals by game address (link_skeleton gap blocks), and
// compiling them there broke views such as g_table428 = g_scriptAssets + 8.
#include "script/w2cregistry.h"                 // W2cRegistry (DAT_0061f874); the datacheck build does not see the normal block

// GLOBAL: 0x61798c g_gameController
// ALONE: g_gameController used by name only
class RWS::TFB::GameController * g_gameController = 0;

// GLOBAL: 0x6179f8 g_pads
// ALONE: g_pads used by name only
struct PadState g_pads[4];

// GLOBAL: 0x617b98 g_controlsManager
// ALONE: g_controlsManager used by name only
class ControlsManager * g_controlsManager = 0;

// GLOBAL: 0x617b9c g_ctlSelB
// ALONE: g_ctlSelB used by name only
int g_ctlSelB = 0;

// GLOBAL: 0x617ba0 g_ctlBuffers
// ALONE: g_ctlBuffers used by name only
struct CtlBuf * g_ctlBuffers[9];

// GLOBAL: 0x617bc4 g_xboxController
// ALONE: g_xboxController used by name only
class CXboxStndController * g_xboxController = 0;

// GLOBAL: 0x617bc8 g_mouseSens
// ALONE: g_mouseSens used by name only
float g_mouseSens = 0;

// GLOBAL: 0x617bcc g_camDirectTurn
// ALONE: g_camDirectTurn used by name only
bool g_camDirectTurn = 0;

// GLOBAL: 0x617bcd p4b_pad_617bcd
// ALONE: p4b_pad_617bcd padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bcd = 0;

// GLOBAL: 0x617bce p4b_pad_617bce
// ALONE: p4b_pad_617bce padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bce = 0;

// GLOBAL: 0x617bcf p4b_pad_617bcf
// ALONE: p4b_pad_617bcf padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bcf = 0;

// GLOBAL: 0x617bd0 g_ctlFlag617bd0
// ALONE: g_ctlFlag617bd0 used by name only
int g_ctlFlag617bd0 = 0;

// GLOBAL: 0x617be8 g_ctlLoaded
// ALONE: g_ctlLoaded used by name only
unsigned char g_ctlLoaded = 0;

// GLOBAL: 0x617be9 p4b_pad_617be9
// ALONE: p4b_pad_617be9 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617be9 = 0;

// GLOBAL: 0x617bea p4b_pad_617bea
// ALONE: p4b_pad_617bea padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bea = 0;

// GLOBAL: 0x617beb p4b_pad_617beb
// ALONE: p4b_pad_617beb padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617beb = 0;

// GLOBAL: 0x617bec g_rumbleB
// ALONE: g_rumbleB used by name only
float g_rumbleB = 0;

// GLOBAL: 0x617bf0 g_rumbleA
// ALONE: g_rumbleA used by name only
float g_rumbleA = 0;

// GLOBAL: 0x617bf4 g_padHeld
// ALONE: g_padHeld used by name only
unsigned char g_padHeld = 0;

// GLOBAL: 0x617bf5 p4b_pad_617bf5
// ALONE: p4b_pad_617bf5 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bf5 = 0;

// GLOBAL: 0x617bf6 p4b_pad_617bf6
// ALONE: p4b_pad_617bf6 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bf6 = 0;

// GLOBAL: 0x617bf7 p4b_pad_617bf7
// ALONE: p4b_pad_617bf7 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bf7 = 0;

// GLOBAL: 0x617bf8 g_audioMixerFlag
// ALONE: g_audioMixerFlag used by name only
unsigned char g_audioMixerFlag = 0;

// GLOBAL: 0x617bf9 p4b_pad_617bf9
// ALONE: p4b_pad_617bf9 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bf9 = 0;

// GLOBAL: 0x617bfa p4b_pad_617bfa
// ALONE: p4b_pad_617bfa padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bfa = 0;

// GLOBAL: 0x617bfb p4b_pad_617bfb
// ALONE: p4b_pad_617bfb padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617bfb = 0;

// GLOBAL: 0x617bfc g_ctlCounter
// ALONE: g_ctlCounter used by name only
int g_ctlCounter = 0;

// GLOBAL: 0x617c00 g_audioGlobalMixer
// ALONE: g_audioGlobalMixer used by name only
class RWS::Audio::AudioGlobalMixer * g_audioGlobalMixer = 0;

// GLOBAL: 0x617c18 g_audioNextUpdate
// ALONE: g_audioNextUpdate used by name only
unsigned int g_audioNextUpdate = 0;

// GLOBAL: 0x617cb0 g_motionBlur
// ALONE: g_motionBlur used by name only
class RWS::FX::CFXMotionBlur * g_motionBlur = 0;

// GLOBAL: 0x617cd0 g_rsSavedE
// ALONE: g_rsSavedE used by name only
int g_rsSavedE = 0;

// GLOBAL: 0x617cd4 g_rsSaved9
// ALONE: g_rsSaved9 used by name only
int g_rsSaved9 = 0;

// GLOBAL: 0x617cd8 g_rsSaved2
// ALONE: g_rsSaved2 used by name only
int g_rsSaved2 = 0;

// GLOBAL: 0x617cdc g_rsSaved14
// ALONE: g_rsSaved14 used by name only
int g_rsSaved14 = 0;

// GLOBAL: 0x617ce0 g_rsSavedB
// ALONE: g_rsSavedB used by name only
int g_rsSavedB = 0;

// GLOBAL: 0x617ce4 g_rsSavedA
// ALONE: g_rsSavedA used by name only
int g_rsSavedA = 0;

// GLOBAL: 0x617ce8 g_rsSaved8
// ALONE: g_rsSaved8 used by name only
int g_rsSaved8 = 0;

// GLOBAL: 0x617cec g_rsSaved6
// ALONE: g_rsSaved6 used by name only
int g_rsSaved6 = 0;

// GLOBAL: 0x617cf0 g_rsSavedC
// ALONE: g_rsSavedC used by name only
int g_rsSavedC = 0;

// GLOBAL: 0x617cf4 g_rsSaved7
// ALONE: g_rsSaved7 used by name only
int g_rsSaved7 = 0;

// GLOBAL: 0x617cf8 g_rsSaved1
// ALONE: g_rsSaved1 used by name only
void * g_rsSaved1 = 0;   // saved texture raster; a pointer because RwRenderStateGet stores 8 bytes on x64

// GLOBAL: 0x617d04 g_d3dDevice411
// ALONE: g_d3dDevice411 used by name only
struct DevVt * g_d3dDevice411 = 0;

// GLOBAL: 0x617d1c g_bloomTexture
// ALONE: g_bloomTexture used by name only
struct DevVt * g_bloomTexture = 0;

// GLOBAL: 0x617d40 g_bloomHeight
// ALONE: g_bloomHeight used by name only
unsigned int g_bloomHeight = 0;

// GLOBAL: 0x617d44 g_bloomWidth
// ALONE: g_bloomWidth used by name only
unsigned int g_bloomWidth = 0;

// GLOBAL: 0x617d50 g_pixelFogEnd
// ALONE: g_pixelFogEnd used by name only
float g_pixelFogEnd = 0;

// GLOBAL: 0x617d54 g_f617d54
// ALONE: g_f617d54 used by name only
float g_f617d54 = 0;

// GLOBAL: 0x617d58 p4b_pad_617d58
// ALONE: p4b_pad_617d58 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d58 = 0;

// GLOBAL: 0x617d59 p4b_pad_617d59
// ALONE: p4b_pad_617d59 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d59 = 0;

// GLOBAL: 0x617d5a p4b_pad_617d5a
// ALONE: p4b_pad_617d5a padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d5a = 0;

// GLOBAL: 0x617d5b p4b_pad_617d5b
// ALONE: p4b_pad_617d5b padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d5b = 0;

// GLOBAL: 0x617d5c g_pixelFogEnable
// ALONE: g_pixelFogEnable used by name only
unsigned char g_pixelFogEnable = 0;

// GLOBAL: 0x617d5d p4b_pad_617d5d
// ALONE: p4b_pad_617d5d padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d5d = 0;

// GLOBAL: 0x617d5e p4b_pad_617d5e
// ALONE: p4b_pad_617d5e padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d5e = 0;

// GLOBAL: 0x617d5f p4b_pad_617d5f
// ALONE: p4b_pad_617d5f padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d5f = 0;

// GLOBAL: 0x617d60 g_pixelFogColor
// ALONE: g_pixelFogColor used by name only
unsigned char g_pixelFogColor[4];

// GLOBAL: 0x617d6c g_f617d6c
// ALONE: g_f617d6c used by name only
float g_f617d6c = 0;

// GLOBAL: 0x617d74 g_i617d74
// ALONE: g_i617d74 used by name only
int g_i617d74 = 0;

// GLOBAL: 0x617d78 g_i617d78
// ALONE: g_i617d78 used by name only
int g_i617d78 = 0;

// GLOBAL: 0x617d84 g_i617d84
// ALONE: g_i617d84 used by name only
int g_i617d84 = 0;

// GLOBAL: 0x617d88 g_i617d88
// ALONE: g_i617d88 used by name only
int g_i617d88 = 0;

// GLOBAL: 0x617d8c g_i617d8c
// ALONE: g_i617d8c used by name only
int g_i617d8c = 0;

// GLOBAL: 0x617d90 g_pixelFogDist
// ALONE: g_pixelFogDist used by name only
float g_pixelFogDist = 0;

// GLOBAL: 0x617d94 p4b_pad_617d94
// ALONE: p4b_pad_617d94 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d94 = 0;

// GLOBAL: 0x617d95 g_bloomEnable
// ALONE: g_bloomEnable used by name only
unsigned char g_bloomEnable = 0;

// GLOBAL: 0x617d96 p4b_pad_617d96
// ALONE: p4b_pad_617d96 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d96 = 0;

// GLOBAL: 0x617d97 p4b_pad_617d97
// ALONE: p4b_pad_617d97 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617d97 = 0;

// GLOBAL: 0x617d98 g_pixelCutoff
// ALONE: g_pixelCutoff used by name only
int g_pixelCutoff = 0;

// GLOBAL: 0x617e78 g_planeList413
// ALONE: g_planeList413 used by name only
struct OccluderNode * g_planeList413 = 0;

// GLOBAL: 0x617e7c g_cranePromptShown
// ALONE: g_cranePromptShown used by name only
unsigned char g_cranePromptShown = 0;

// GLOBAL: 0x617e7d p4b_pad_617e7d
// ALONE: p4b_pad_617e7d padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617e7d = 0;

// GLOBAL: 0x617e7e p4b_pad_617e7e
// ALONE: p4b_pad_617e7e padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617e7e = 0;

// GLOBAL: 0x617e7f p4b_pad_617e7f
// ALONE: p4b_pad_617e7f padding, keeps the game's distance between neighbours
unsigned char p4b_pad_617e7f = 0;

// GLOBAL: 0x617e80 g_spriteManager
// ALONE: g_spriteManager used by name only
class RWS::TFB::SpriteManager * g_spriteManager = 0;

// GLOBAL: 0x617e84 g_spriteGeneration
// ALONE: g_spriteGeneration used by name only
int g_spriteGeneration = 0;

// GLOBAL: 0x617f34 g_sprFlagStart
// ALONE: g_sprFlagStart used by name only
unsigned char g_sprFlagStart = 0;

// GLOBAL: 0x617f35 g_sprFlagGame
// ALONE: g_sprFlagGame used by name only
unsigned char g_sprFlagGame = 0;

// GLOBAL: 0x617f36 g_sprFlag2P
// ALONE: g_sprFlag2P used by name only
unsigned char g_sprFlag2P = 0;

// GLOBAL: 0x617f37 g_sprFlagMenu
// ALONE: g_sprFlagMenu used by name only
unsigned char g_sprFlagMenu = 0;

// GLOBAL: 0x617f38 g_sprPassCount
// ALONE: g_sprPassCount used by name only
int g_sprPassCount = 0;

// GLOBAL: 0x617fc0 g_soundStreamCount
// ALONE: g_soundStreamCount used by name only
int g_soundStreamCount = 0;

// GLOBAL: 0x617fc4 g_musicVolume
// ALONE: g_musicVolume used by name only
int g_musicVolume = 0;

// GLOBAL: 0x617fc8 g_effectsVolume
// ALONE: g_effectsVolume used by name only
int g_effectsVolume = 0;

// GLOBAL: 0x617fcc g_envType
// ALONE: g_envType used by name only
int g_envType = 0;

// GLOBAL: 0x617fd0 g_voiceList
// ALONE: g_voiceList used by name only
struct RWS::TFB::Voice * g_voiceList = 0;

// GLOBAL: 0x617fe8 g_soundStreamPath
// ALONE: g_soundStreamPath used by name only
char const * g_soundStreamPath = 0;

// GLOBAL: 0x617fec g_musicStreamPath
// ALONE: g_musicStreamPath used by name only
char const * g_musicStreamPath = 0;

// GLOBAL: 0x618010 g_musicPathBuffer
// ALONE: g_musicPathBuffer used by name only
char g_musicPathBuffer[0x80];

// GLOBAL: 0x618090 g_soundPathBuffer
// ALONE: g_soundPathBuffer used by name only
char g_soundPathBuffer[0x80];

// GLOBAL: 0x618110 g_renderQueueHead
// ALONE: g_renderQueueHead used by name only
struct RenderQueueNode * g_renderQueueHead = 0;

// GLOBAL: 0x618114 g_renderQueueNodes
// ALONE: g_renderQueueNodes used by name only
struct RenderQueueNode * * g_renderQueueNodes = 0;

// GLOBAL: 0x618118 g_renderQueueCount
// ALONE: g_renderQueueCount used by name only
unsigned g_renderQueueCount = 0;

// GLOBAL: 0x61811c g_renderQueueFreeList
// ALONE: g_renderQueueFreeList used by name only
struct RwFreeList * g_renderQueueFreeList = 0;

// GLOBAL: 0x618120 g_renderQueueCapacity
// ALONE: g_renderQueueCapacity used by name only
unsigned int g_renderQueueCapacity = 0;

// GLOBAL: 0x618124 g_renderQueueRefCount
// ALONE: g_renderQueueRefCount used by name only
unsigned int g_renderQueueRefCount = 0;

// GLOBAL: 0x61813c g_pixelFogCacheD
// ALONE: g_pixelFogCacheD used by name only
float g_pixelFogCacheD = 0;

// GLOBAL: 0x618140 g_pixelFogCacheC
// ALONE: g_pixelFogCacheC used by name only
float g_pixelFogCacheC = 0;

// GLOBAL: 0x618144 g_pixelFogCacheB
// ALONE: g_pixelFogCacheB used by name only
float g_pixelFogCacheB = 0;

// GLOBAL: 0x618148 g_pixelFogCacheA
// ALONE: g_pixelFogCacheA used by name only
float g_pixelFogCacheA = 0;

// GLOBAL: 0x61814c g_fogCacheKey
// ALONE: g_fogCacheKey used by name only
unsigned int g_fogCacheKey = 0;

// GLOBAL: 0x618150 g_fogColorAlpha
// ALONE: g_fogColorAlpha used by name only
unsigned int g_fogColorAlpha = 0;

// GLOBAL: 0x61818c g_cameraFollowActor
// ALONE: g_cameraFollowActor used by name only
class RWS::TFB::GameActor * g_cameraFollowActor = 0;

// GLOBAL: 0x618190 g_camActorPrev
// ALONE: g_camActorPrev used by name only
class RWS::TFB::GameActor * g_camActorPrev = 0;

// GLOBAL: 0x618194 g_camObjPrev
// ALONE: g_camObjPrev used by name only
void * g_camObjPrev = 0;

// GLOBAL: 0x6181b0 g_camPitchHold
// ALONE: g_camPitchHold used by name only
int g_camPitchHold = 0;

// GLOBAL: 0x6181b4 g_camYawHold
// ALONE: g_camYawHold used by name only
int g_camYawHold = 0;

// GLOBAL: 0x6181b8 g_camPitchDeferred
// ALONE: g_camPitchDeferred used by name only
bool g_camPitchDeferred = 0;

// GLOBAL: 0x6181b9 p4b_pad_6181b9
// ALONE: p4b_pad_6181b9 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_6181b9 = 0;

// GLOBAL: 0x6181ba p4b_pad_6181ba
// ALONE: p4b_pad_6181ba padding, keeps the game's distance between neighbours
unsigned char p4b_pad_6181ba = 0;

// GLOBAL: 0x6181bb p4b_pad_6181bb
// ALONE: p4b_pad_6181bb padding, keeps the game's distance between neighbours
unsigned char p4b_pad_6181bb = 0;

// GLOBAL: 0x6181bc g_camHAnim
// ALONE: g_camHAnim used by name only
struct RpHAnimHierarchy * g_camHAnim = 0;

// GLOBAL: 0x6181c0 g_camModeAux
// ALONE: g_camModeAux used by name only
int g_camModeAux = 0;

// GLOBAL: 0x6181c4 g_camFade
// ALONE: g_camFade used by name only
float g_camFade = 0;

// GLOBAL: 0x6181c8 g_camAddKind
// ALONE: g_camAddKind used by name only
int g_camAddKind = 0;

// GLOBAL: 0x6181cc g_camMode
// ALONE: g_camMode used by name only
int g_camMode = 0;

// GLOBAL: 0x6181d0 g_camValueD0
// ALONE: g_camValueD0 used by name only
float g_camValueD0 = 0;

// GLOBAL: 0x6181d4 g_camSnapTgt
// ALONE: g_camSnapTgt used by name only
__declspec(align(4)) struct RwV3d g_camSnapTgt = {};

// GLOBAL: 0x6181e0 g_camSnapPos
// ALONE: g_camSnapPos used by name only
__declspec(align(4)) struct RwV3d g_camSnapPos = {};

// GLOBAL: 0x6181ec g_camTgt
// ALONE: g_camTgt used by name only
__declspec(align(4)) struct RwV3d g_camTgt = {};

// GLOBAL: 0x6181f8 g_camTgtCur
// ALONE: g_camTgtCur used by name only
__declspec(align(4)) struct RwV3d g_camTgtCur = {};

// GLOBAL: 0x618204 g_camTgtPrev
// ALONE: g_camTgtPrev used by name only
__declspec(align(4)) struct RwV3d g_camTgtPrev = {};

// GLOBAL: 0x618210 g_camPos
// ALONE: g_camPos used by name only
__declspec(align(4)) struct RwV3d g_camPos = {};

// GLOBAL: 0x61821c g_camPosCur
// ALONE: g_camPosCur used by name only
__declspec(align(4)) struct RwV3d g_camPosCur = {};

// GLOBAL: 0x618228 g_camPosPrev
// ALONE: g_camPosPrev used by name only
__declspec(align(4)) struct RwV3d g_camPosPrev = {};

// GLOBAL: 0x618238 g_camBlk
// ALONE: g_camBlk used by name only
struct RwFrustumPlaneView g_camBlk[6];

// GLOBAL: 0x6182b0 g_buttonTexture
// ALONE: g_buttonTexture used by name only
struct RwTexture * g_buttonTexture = 0;

// GLOBAL: 0x6182b4 g_camSnapPending
// ALONE: g_camSnapPending used by name only
bool g_camSnapPending = 0;

// GLOBAL: 0x6182b5 g_camShadowPass
// ALONE: g_camShadowPass used by name only
bool g_camShadowPass = 0;

// GLOBAL: 0x6182b6 p4b_pad_6182b6
// ALONE: p4b_pad_6182b6 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_6182b6 = 0;

// GLOBAL: 0x6182b7 p4b_pad_6182b7
// ALONE: p4b_pad_6182b7 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_6182b7 = 0;

// GLOBAL: 0x6182b8 g_camTimeLeft
// ALONE: g_camTimeLeft used by name only
float g_camTimeLeft = 0;

// GLOBAL: 0x6182bc g_camTimeTotal
// ALONE: g_camTimeTotal used by name only
float g_camTimeTotal = 0;

// GLOBAL: 0x6182c0 g_camTimeStart
// ALONE: g_camTimeStart used by name only
float g_camTimeStart = 0;

// GLOBAL: 0x6182c4 g_camAddAmount
// ALONE: g_camAddAmount used by name only
float g_camAddAmount = 0;

// GLOBAL: 0x6182c8 g_cameraObject
// ALONE: g_cameraObject used by name only
class RWS::TFB::GameCamera * g_cameraObject = 0;

// GLOBAL: 0x6182cc g_camAnim
// ALONE: g_camAnim used by name only
struct RtAnimAnimation * g_camAnim = 0;

// GLOBAL: 0x6182d0 g_gameCamDst
// ALONE: g_gameCamDst used by name only
struct RwCamera * g_gameCamDst = 0;

// GLOBAL: 0x6182d4 g_gameCamValid
// ALONE: g_gameCamValid used by name only
bool g_gameCamValid = 0;

// GLOBAL: 0x6182ec g_camPosBaseY
// ALONE: g_camPosBaseY used by name only
float g_camPosBaseY = 0;

// GLOBAL: 0x6182f0 g_camTgtBaseY
// ALONE: g_camTgtBaseY used by name only
float g_camTgtBaseY = 0;

// GLOBAL: 0x6182f4 g_shadowCamera
// ALONE: g_shadowCamera used by name only
class RWS::TFB::TFBShadowCamera * g_shadowCamera = 0;

// GLOBAL: 0x618308 g_corpusCount
// ALONE: g_corpusCount used by name only
int g_corpusCount = 0;

// GLOBAL: 0x61830c g_corpusGravityDt2
// ALONE: g_corpusGravityDt2 used by name only
float g_corpusGravityDt2 = 0;

// GLOBAL: 0x618310 g_corpusGravityDt
// ALONE: g_corpusGravityDt used by name only
float g_corpusGravityDt = 0;

// GLOBAL: 0x618314 g_frameRate
// ALONE: g_frameRate used by name only
float g_frameRate = 0;

// GLOBAL: 0x618318 g_frameDt
// ALONE: g_frameDt used by name only
float g_frameDt = 0;

// GLOBAL: 0x618368 g_tickSum
// ALONE: g_tickSum used by name only
unsigned int g_tickSum = 0;

// GLOBAL: 0x61836c g_tickBase
// ALONE: g_tickBase used by name only
unsigned int g_tickBase = 0;

// GLOBAL: 0x618378 g_table428Count
// ALONE: g_table428Count used by name only
namespace Script { unsigned int g_table428Count = 0; }

// GLOBAL: 0x618390 Script::g_scriptAssets
// ALONE: Script::g_scriptAssets used by name only

// GLOBAL: 0x618398 g_table428
// ALONE: g_table428 used by name only
struct TableEntry428 * g_table428 = 0;

// GLOBAL: 0x61f464 DAT_0061f464
// ALONE: DAT_0061f464 used by name only
int DAT_0061f464 = 0;   // actor serial counter (GetActorSerialCounter)

// GLOBAL: 0x61f514 g_findSubOwner
// ALONE: g_findSubOwner used by name only
class Script::ScriptReference * g_findSubOwner = 0;

// GLOBAL: 0x61f658 g_scriptPC
// ALONE: g_scriptPC used by name only
int g_scriptPC = 0;

// GLOBAL: 0x61f65c g_curScript
// ALONE: g_curScript used by name only
class Script::OpContext * g_curScript = 0;

// GLOBAL: 0x61f660 Script::g_subsetTable
// ALONE: g_subsetTable pointers filled by RegisterSubset (g_skipA at 0x61f668 is slot 2 in the code) (g_skipA at 0x61f668 is slot 2 in the code)
Script::ScriptSubset * Script::g_subsetTable[25];

// GLOBAL: 0x61f668 g_skipA
// ALONE: g_skipA used by name only
class Script::ReferenceSet * g_skipA = 0;

// GLOBAL: 0x61f6c4 g_userRefSlots
// ALONE: g_userRefSlots used by name only
class Script::UserReference * g_userRefSlots[8];

// GLOBAL: 0x61f6f0 g_opcodeProtoGuard
// ALONE: g_opcodeProtoGuard used by name only
unsigned int g_opcodeProtoGuard = 0;

// GLOBAL: 0x61f870 g_scriptLastType
// ALONE: g_scriptLastType used by name only
class Script::ScriptReference * g_scriptLastType = 0;

// GLOBAL: 0x61f874 DAT_0061f874
// ALONE: DAT_0061f874 used by name only
W2cRegistry * DAT_0061f874 = 0;   // as declared in script/w2cregistry.h

// GLOBAL: 0x61f878 g_symRegistryY
// ALONE: g_symRegistryY the registry's vector pointer, zero until ResolveSymbol creates the registry
SymbolVec * g_symRegistryY = 0;

// GLOBAL: 0x61f87c g_tfbParseEnd
// ALONE: g_tfbParseEnd used by name only
int g_tfbParseEnd = 0;

// GLOBAL: 0x61f880 g_tfbParseOp
// ALONE: g_tfbParseOp used by name only
class Script::OpCode * g_tfbParseOp = 0;

// GLOBAL: 0x61f884 g_tfbLoadArg
// ALONE: g_tfbLoadArg used by name only
unsigned int g_tfbLoadArg = 0;

// GLOBAL: 0x61f888 g_tfbReadPos
// ALONE: g_tfbReadPos used by name only
int g_tfbReadPos = 0;

// GLOBAL: 0x61f88c g_tfbReadBuf
// ALONE: g_tfbReadBuf used by name only
unsigned char * g_tfbReadBuf = 0;

// GLOBAL: 0x61f890 g_scriptPool
// ALONE: g_scriptPool used by name only
char * g_scriptPool = 0;

// GLOBAL: 0x61f89c g_tfbPending
// ALONE: g_tfbPending used by name only
void * g_tfbPending = 0;

// GLOBAL: 0x61f8a0 g_tfbPending2
// ALONE: g_tfbPending2 used by name only
int g_tfbPending2 = 0;

// GLOBAL: 0x61f8a4 g_tfbPending3
// ALONE: g_tfbPending3 used by name only
int g_tfbPending3 = 0;

// GLOBAL: 0x61f8a8 g_typedNameBuf
// ALONE: g_typedNameBuf used by name only
char g_typedNameBuf[0x100];

// GLOBAL: 0x61f9bc g_cutSceneCounter
// ALONE: g_cutSceneCounter used by name only
int g_cutSceneCounter = 0;

// GLOBAL: 0x61fc00 g_cutSceneStackIndex
// ALONE: g_cutSceneStackIndex used by name only
int g_cutSceneStackIndex = 0;

// GLOBAL: 0x61ff58 g_camLookAt
// ALONE: g_camLookAt used by name only
__declspec(align(4)) struct RwV3d g_camLookAt = {};

// GLOBAL: 0x61ff68 g_cam61ff68
// ALONE: g_cam61ff68 used by name only
class Script::ScriptReference * g_cam61ff68 = 0;

// GLOBAL: 0x61ff6c g_cam61ff6c
// ALONE: g_cam61ff6c used by name only
class Script::ScriptReference * g_cam61ff6c = 0;

// GLOBAL: 0x620168 g_nodeFreeList
// ALONE: g_nodeFreeList used by name only
struct FreeNode * g_nodeFreeList = 0;

// GLOBAL: 0x620170 Script::g_nodePool
// ALONE: Script::g_nodePool used by name only

// GLOBAL: 0x621f10 g_scriptSender
// ALONE: g_scriptSender used by name only
class Script::ScriptReference * g_scriptSender = 0;

// GLOBAL: 0x621f14 g_scriptSelf
// ALONE: g_scriptSelf used by name only
class Script::ScriptReference * g_scriptSelf = 0;

// GLOBAL: 0x621f18 g_scriptOwner
// ALONE: g_scriptOwner used by name only
class Script::ScriptReference * g_scriptOwner = 0;

// GLOBAL: 0x621f1c g_forEachItem
// ALONE: g_forEachItem used by name only
class Script::ScriptReference * g_forEachItem = 0;

// GLOBAL: 0x621f20 g_scriptVariable
// ALONE: g_scriptVariable used by name only
class Script::ScriptReference * g_scriptVariable = 0;

// GLOBAL: 0x621f24 g_symCountBase
// ALONE: g_symCountBase used by name only
int g_symCountBase = 0;

// GLOBAL: 0x621f28 g_typeInitCount
// ALONE: g_typeInitCount used by name only
int g_typeInitCount = 0;

// GLOBAL: 0x621f2c g_scriptOuterSelf
// ALONE: g_scriptOuterSelf used by name only
class Script::ScriptReference * g_scriptOuterSelf = 0;

// GLOBAL: 0x621f30 g_scriptOuterOwner
// ALONE: g_scriptOuterOwner used by name only
class Script::ScriptReference * g_scriptOuterOwner = 0;

// GLOBAL: 0x621f34 g_voiceDefaultRef
// ALONE: g_voiceDefaultRef used by name only
class Script::ScriptReference * g_voiceDefaultRef = 0;

// GLOBAL: 0x621f38 g_typeSymCount
// ALONE: g_typeSymCount used by name only
int g_typeSymCount = 0;

// GLOBAL: 0x622000 g_forEachSet
// ALONE: g_forEachSet used by name only
class Script::ScriptReference * g_forEachSet = 0;

// GLOBAL: 0x622004 g_scriptSubset
// ALONE: g_scriptSubset used by name only
class Script::ScriptReference * g_scriptSubset = 0;

// GLOBAL: 0x6221ac g_normalizedAngle
// ALONE: g_normalizedAngle used by name only
float g_normalizedAngle = 0;

// GLOBAL: 0x6221b0 g_levelZone
// ALONE: g_levelZone used by name only
int g_levelZone = 0;

// GLOBAL: 0x6221b4 g_playMode
// ALONE: g_playMode used by name only
int g_playMode = 0;

// GLOBAL: 0x6221b8 g_vsyncFlipCount
// ALONE: g_vsyncFlipCount used by name only
int g_vsyncFlipCount = 0;

// GLOBAL: 0x62239c Script::g_zeroVec3
// ALONE: Script::g_zeroVec3 used by name only
namespace Script { float g_zeroVec3[3]; }

// GLOBAL: 0x6223ac g_radarHeight
// ALONE: g_radarHeight used by name only
float g_radarHeight = 0;

// GLOBAL: 0x6223b0 g_radarWidth
// ALONE: g_radarWidth used by name only
float g_radarWidth = 0;

// GLOBAL: 0x6223b4 g_rsFog
// ALONE: g_rsFog used by name only
int g_rsFog = 0;

// GLOBAL: 0x6223b8 g_radarZTest
// ALONE: g_radarZTest used by name only
int g_radarZTest = 0;

// GLOBAL: 0x6223bc g_radarZWrite
// ALONE: g_radarZWrite used by name only
int g_radarZWrite = 0;

// GLOBAL: 0x6223c0 g_rsVertexAlpha
// ALONE: g_rsVertexAlpha used by name only
int g_rsVertexAlpha = 0;

// GLOBAL: 0x6223c4 g_rsDestBlend
// ALONE: g_rsDestBlend used by name only
int g_rsDestBlend = 0;

// GLOBAL: 0x6223c8 g_rsSrcBlend
// ALONE: g_rsSrcBlend used by name only
int g_rsSrcBlend = 0;

// GLOBAL: 0x6223cc g_rsCullMode
// ALONE: g_rsCullMode used by name only
int g_rsCullMode = 0;

// GLOBAL: 0x6223d0 g_bigCircleTexture
// ALONE: g_bigCircleTexture used by name only
struct RwTexture * g_bigCircleTexture = 0;

// GLOBAL: 0x6223e8 g_radarRender
// ALONE: g_radarRender used by name only
class RadarRender * g_radarRender = 0;

// GLOBAL: 0x622400 g_shadowBuffer
// ALONE: g_shadowBuffer used by name only
void * g_shadowBuffer = 0;

// GLOBAL: 0x622404 g_shadowRender
// ALONE: g_shadowRender used by name only
class ShadowRender * g_shadowRender = 0;

// GLOBAL: 0x622408 g_headShadowTexture
// ALONE: g_headShadowTexture used by name only
struct RwTexture * g_headShadowTexture = 0;

// GLOBAL: 0x622410 g_shadowCasterPipeline
// ALONE: g_shadowCasterPipeline used by name only
struct RxPipeline * g_shadowCasterPipeline = 0;

// GLOBAL: 0x622414 g_shadowTexture
// ALONE: g_shadowTexture used by name only
struct RwTexture * g_shadowTexture = 0;

// GLOBAL: 0x622418 g_shadowPipeline
// ALONE: g_shadowPipeline used by name only
struct RxPipeline * g_shadowPipeline = 0;

// GLOBAL: 0x622430 g_shadowPlaneD
// ALONE: g_shadowPlaneD used by name only
float g_shadowPlaneD = 0;

// GLOBAL: 0x622434 g_shadowDir
// ALONE: g_shadowDir used by name only
__declspec(align(4)) struct RwV3d g_shadowDir = {};

// GLOBAL: 0x622440 g_shadowAt
// ALONE: g_shadowAt used by name only
__declspec(align(4)) struct RwV3d g_shadowAt = {};

// GLOBAL: 0x62244c g_shadowPos
// ALONE: g_shadowPos used by name only
__declspec(align(4)) struct RwV3d g_shadowPos = {};

// GLOBAL: 0x622458 g_shadowPassTexture
// ALONE: g_shadowPassTexture used by name only
struct RwTexture * g_shadowPassTexture = 0;

// GLOBAL: 0x62245c g_shadowStrength
// ALONE: g_shadowStrength used by name only
float g_shadowStrength = 0;

// GLOBAL: 0x622480 g_saveOverlapped
// ALONE: g_saveOverlapped used by name only
__declspec(align(4)) struct SaveFileOverlapped g_saveOverlapped = {};

// GLOBAL: 0x622498 g_savePath
// ALONE: g_savePath used by name only
char g_savePath[0x80];

// GLOBAL: 0x622518 g_saveBuf
// ALONE: g_saveBuf used by name only
void * g_saveBuf = 0;

// GLOBAL: 0x62251c g_saveSize
// ALONE: g_saveSize used by name only
int g_saveSize = 0;

// GLOBAL: 0x622520 g_saveMode
// ALONE: g_saveMode used by name only
int g_saveMode = 0;

// GLOBAL: 0x622524 g_memCardReady
// ALONE: g_memCardReady used by name only
unsigned char g_memCardReady = 0;

// GLOBAL: 0x622525 g_writeBusy
// ALONE: g_writeBusy used by name only
unsigned char g_writeBusy = 0;

// GLOBAL: 0x622526 g_readBusy
// ALONE: g_readBusy used by name only
unsigned char g_readBusy = 0;

// GLOBAL: 0x622527 p4b_pad_622527
// ALONE: p4b_pad_622527 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_622527 = 0;

// GLOBAL: 0x622528 g_saveOffset
// ALONE: g_saveOffset used by name only
unsigned int g_saveOffset = 0;

// GLOBAL: 0x622538 g_saveBuffer
// ALONE: g_saveBuffer used by name only
char g_saveBuffer[0x4000];

// GLOBAL: 0x626538 g_memCard
// ALONE: g_memCard used by name only
class MemCard * g_memCard = 0;

// GLOBAL: 0x626550 g_memCardLastTick
// ALONE: g_memCardLastTick used by name only
unsigned int g_memCardLastTick = 0;

// GLOBAL: 0x626554 g_langIndex
// ALONE: DAT_00626554 used by name only
int g_langIndex = 3;

// GLOBAL: 0x626560 g_pairPool
// ALONE: g_pairPool used by name only

// GLOBAL: 0x629440 g_pairList
// ALONE: g_pairList used by name only
struct RWS::TFB::CollisionPair * g_pairList = 0;

// GLOBAL: 0x629470 g_audioStreams
// ALONE: g_audioStreams used by name only
struct RwaStream * g_audioStreams[4];

// GLOBAL: 0x629480 g_audioVoices
// ALONE: g_audioVoices used by name only
struct RwaOsVoice * g_audioVoices[15];

// GLOBAL: 0x6294c0 g_audioVirtualVoices
// ALONE: g_audioVirtualVoices used by name only
struct RWS::RwsAudio::VirtualVoiceSlot g_audioVirtualVoices[50];

// GLOBAL: 0x629650 g_audioOutput
// ALONE: g_audioOutput used by name only
struct RwaOsOutput * g_audioOutput = 0;

// GLOBAL: 0x629654 g_audioListener
// ALONE: g_audioListener used by name only
struct RwaListener * g_audioListener = 0;

// GLOBAL: 0x629658 g_audioWaveDicts
// ALONE: g_audioWaveDicts used by name only
struct RwaWaveDict * g_audioWaveDicts[16];

// GLOBAL: 0x629698 g_audioRefCount
// ALONE: g_audioRefCount used by name only
unsigned int g_audioRefCount = 0;

// GLOBAL: 0x62969c g_audioCoreOpen
// ALONE: g_audioCoreOpen used by name only
int g_audioCoreOpen = 0;

// GLOBAL: 0x6296a8 g_fontTxd
// ALONE: g_fontTxd used by name only
struct RwTexDictionary * g_fontTxd = 0;

// GLOBAL: 0x6296ec g_levelClump
// ALONE: g_levelClump used by name only
struct RpClump * g_levelClump = 0;

// GLOBAL: 0x6296f0 g_fogFlag
// ALONE: g_fogFlag used by name only
int g_fogFlag = 0;

// GLOBAL: 0x6296f4 g_fogParamB
// ALONE: g_fogParamB used by name only
float g_fogParamB = 0;

// GLOBAL: 0x6296f8 g_fogParamA
// ALONE: g_fogParamA used by name only
float g_fogParamA = 0;

// GLOBAL: 0x6296fc g_levelFogParam
// ALONE: g_levelFogParam used by name only
float g_levelFogParam = 0;

// GLOBAL: 0x629700 g_fogMode
// ALONE: g_fogMode used by name only
int g_fogMode = 0;

// GLOBAL: 0x629704 g_vertFogColor
// ALONE: g_vertFogColor used by name only
unsigned int g_vertFogColor = 0;

// GLOBAL: 0x629708 g_vertFogDist
// ALONE: g_vertFogDist used by name only
float g_vertFogDist = 0;

// GLOBAL: 0x62970c g_farClip
// ALONE: g_farClip used by name only
float g_farClip = 0;

// GLOBAL: 0x629710 g_nearClip
// ALONE: g_nearClip used by name only
float g_nearClip = 0;

// GLOBAL: 0x629714 g_clumpList
// ALONE: g_clumpList used by name only
__declspec(align(4)) struct RwLLLink g_clumpList = {};

// GLOBAL: 0x62971c g_lightList
// ALONE: g_lightList used by name only
__declspec(align(4)) struct RwLLLink g_lightList = {};

// GLOBAL: 0x629724 RWS::CLevel::sm_pOpaqueWorld
// ALONE: RWS::CLevel::sm_pOpaqueWorld used by name only
struct RpWorld * RWS::CLevel::sm_pOpaqueWorld = 0;

// GLOBAL: 0x629728 RWS::CLevel::sm_pTranslucentWorld
// ALONE: RWS::CLevel::sm_pTranslucentWorld used by name only
struct RpWorld * RWS::CLevel::sm_pTranslucentWorld = 0;

// GLOBAL: 0x62972c RWS::CLevel::sm_pActiveWorld
// ALONE: RWS::CLevel::sm_pActiveWorld used by name only
struct RpWorld * RWS::CLevel::sm_pActiveWorld = 0;

// GLOBAL: 0x629730 RWS::CLevel::sm_pPreOpaqueWorld
// ALONE: RWS::CLevel::sm_pPreOpaqueWorld used by name only
struct RpWorld * RWS::CLevel::sm_pPreOpaqueWorld = 0;

// GLOBAL: 0x629734 RWS::CLevel::sm_pSkyWorld
// ALONE: RWS::CLevel::sm_pSkyWorld used by name only
struct RpWorld * RWS::CLevel::sm_pSkyWorld = 0;

// GLOBAL: 0x629738 RWS::CLevel::sm_pFogWorld
// ALONE: RWS::CLevel::sm_pFogWorld used by name only
struct RpWorld * RWS::CLevel::sm_pFogWorld = 0;

// GLOBAL: 0x62973c g_yonObject
// ALONE: g_yonObject used by name only
class RWS::TFB::CTFBWorld * g_yonObject = 0;

// GLOBAL: 0x629740 g_levelWorldsReady
// ALONE: g_levelWorldsReady used by name only
int g_levelWorldsReady = 0;

// GLOBAL: 0x6297c0 g_textDictBufferEnd
// ALONE: g_textDictBufferEnd used by name only
char * g_textDictBufferEnd = 0;

// GLOBAL: 0x6297c4 g_textDictBuffer
// ALONE: g_textDictBuffer used by name only
char * g_textDictBuffer = 0;

// GLOBAL: 0x6297d8 g_stringIdPairs
// ALONE: g_stringIdPairs used by name only
unsigned short g_stringIdPairs[0x320];

// GLOBAL: 0x629e18 g_stringIdLevels
// ALONE: g_stringIdLevels used by name only
unsigned short g_stringIdLevels[64];

// GLOBAL: 0x629e58 g_stringIdCount
// ALONE: g_stringIdCount used by name only
unsigned short g_stringIdCount = 0;

// GLOBAL: 0x629e5a p4b_pad_629e5a
// ALONE: p4b_pad_629e5a padding, keeps the game's distance between neighbours
unsigned char p4b_pad_629e5a = 0;

// GLOBAL: 0x629e5b p4b_pad_629e5b
// ALONE: p4b_pad_629e5b padding, keeps the game's distance between neighbours
unsigned char p4b_pad_629e5b = 0;

// GLOBAL: 0x629e5c p4b_pad_629e5c
// ALONE: p4b_pad_629e5c padding, keeps the game's distance between neighbours
unsigned char p4b_pad_629e5c = 0;

// GLOBAL: 0x629e5d p4b_pad_629e5d
// ALONE: p4b_pad_629e5d padding, keeps the game's distance between neighbours
unsigned char p4b_pad_629e5d = 0;

// GLOBAL: 0x629e5e p4b_pad_629e5e
// ALONE: p4b_pad_629e5e padding, keeps the game's distance between neighbours
unsigned char p4b_pad_629e5e = 0;

// GLOBAL: 0x629e5f p4b_pad_629e5f
// ALONE: p4b_pad_629e5f padding, keeps the game's distance between neighbours
unsigned char p4b_pad_629e5f = 0;

// GLOBAL: 0x629e60 g_fontCache
// ALONE: g_fontCache used by name only
struct FontCacheEntry g_fontCache[10];

// GLOBAL: 0x629ed8 g_hudBrush
// ALONE: g_hudBrush used by name only
struct Rt2dBrush * g_hudBrush = 0;

// GLOBAL: 0x629edc g_hudScreenA
// ALONE: g_hudScreenA used by name only
float g_hudScreenA = 0;

// GLOBAL: 0x629ee0 g_hudScreenB
// ALONE: g_hudScreenB used by name only
float g_hudScreenB = 0;

// GLOBAL: 0x629ee4 g_hudScreenC
// ALONE: g_hudScreenC used by name only
float g_hudScreenC = 0;

// GLOBAL: 0x629ee8 g_hudScreenD
// ALONE: g_hudScreenD used by name only
float g_hudScreenD = 0;

// GLOBAL: 0x629eec g_hudScreenE
// ALONE: g_hudScreenE used by name only
float g_hudScreenE = 0;

// GLOBAL: 0x629ef0 g_hudScreenH
// ALONE: g_hudScreenH used by name only
float g_hudScreenH = 0;

// GLOBAL: 0x629ef4 g_hudScreenW
// ALONE: g_hudScreenW used by name only
float g_hudScreenW = 0;

// GLOBAL: 0x629ef8 g_hudQuad
// ALONE: g_hudQuad used by name only
int g_hudQuad[8];

// GLOBAL: 0x629f18 g_hudQuadState
// ALONE: g_hudQuadState used by name only
int g_hudQuadState = 0;

// GLOBAL: 0x629f1c g_fontCacheCount
// ALONE: g_fontCacheCount used by name only
int g_fontCacheCount = 0;

// GLOBAL: 0x629f30 g_hudText
// ALONE: g_hudText used by name only
char g_hudText[0x108];

// GLOBAL: 0x62a038 g_hudVerts
// ALONE: g_hudVerts used by name only
struct RwIm2DVertex g_hudVerts[4];

// GLOBAL: 0x62a0b0 g_vertAnimSortColours
// ALONE: g_vertAnimSortColours used by name only
int const * g_vertAnimSortColours = 0;

// GLOBAL: 0x62a0b4 g_masterVertAnimList
// ALONE: g_masterVertAnimList used by name only
class TFB::MasterVertAnimation * g_masterVertAnimList = 0;

// GLOBAL: 0x62a0dc g_rsBlendSrc
// ALONE: g_rsBlendSrc used by name only
int g_rsBlendSrc = 0;

// GLOBAL: 0x62a0e0 g_rsBlendDst
// ALONE: g_rsBlendDst used by name only
int g_rsBlendDst = 0;

// GLOBAL: 0x62a0e4 g_rsAlphaRef
// ALONE: g_rsAlphaRef used by name only
int g_rsAlphaRef = 0;

// GLOBAL: 0x62a0e8 g_rsAlphaFunc
// ALONE: g_rsAlphaFunc used by name only
int g_rsAlphaFunc = 0;

// GLOBAL: 0x62a0ec g_rsCull
// ALONE: g_rsCull used by name only
int g_rsCull = 0;

// GLOBAL: 0x62a0f4 g_rsReset
// ALONE: g_rsReset used by name only
int g_rsReset = 0;

// GLOBAL: 0x62a0f8 g_tfbUvScroll
// ALONE: g_tfbUvScroll used by name only
__declspec(align(4)) struct RwV3d g_tfbUvScroll = {};

// GLOBAL: 0x62a124 g_secStatMeshes
// ALONE: g_secStatMeshes used by name only
int g_secStatMeshes = 0;

// GLOBAL: 0x62a128 g_secStatFaded
// ALONE: g_secStatFaded used by name only
int g_secStatFaded = 0;

// GLOBAL: 0x62a12c g_secStatIndices
// ALONE: g_secStatIndices used by name only
int g_secStatIndices = 0;

// GLOBAL: 0x62a130 g_secStatFadedIndices
// ALONE: g_secStatFadedIndices used by name only
int g_secStatFadedIndices = 0;

// GLOBAL: 0x62a134 g_secFadeOn
// ALONE: g_secFadeOn used by name only
unsigned char g_secFadeOn = 0;

// GLOBAL: 0x62a135 g_secFadeHide
// ALONE: g_secFadeHide used by name only
unsigned char g_secFadeHide = 0;

// GLOBAL: 0x62a136 p4b_pad_62a136
// ALONE: p4b_pad_62a136 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a136 = 0;

// GLOBAL: 0x62a137 p4b_pad_62a137
// ALONE: p4b_pad_62a137 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a137 = 0;

// GLOBAL: 0x62a138 g_binkName
// ALONE: g_binkName used by name only
char const * g_binkName = 0;

// GLOBAL: 0x62a13c g_binkClearCount
// ALONE: g_binkClearCount used by name only
int g_binkClearCount = 0;

// GLOBAL: 0x62a140 g_binkSavedFlip
// ALONE: g_binkSavedFlip used by name only
int g_binkSavedFlip = 0;

// GLOBAL: 0x62a144 g_binkSavedRtCam
// ALONE: g_binkSavedRtCam used by name only
struct RwCamera * g_binkSavedRtCam = 0;

// GLOBAL: 0x62a148 g_binkCam
// ALONE: g_binkCam used by name only
struct RwCamera * g_binkCam = 0;

// GLOBAL: 0x62a14c g_binkRaster
// ALONE: g_binkRaster used by name only
struct RwRaster * g_binkRaster = 0;

// GLOBAL: 0x62a150 g_bink
// ALONE: g_bink used by name only
struct BINK * g_bink = 0;

// GLOBAL: 0x62a168 g_loadSpriteAge
// ALONE: g_loadSpriteAge used by name only
int g_loadSpriteAge[15];

// GLOBAL: 0x62a1a4 g_loadSpriteTex
// ALONE: g_loadSpriteTex used by name only
struct RwTexture * g_loadSpriteTex[15];

// GLOBAL: 0x62a1e0 g_loadStartTime
// ALONE: g_loadStartTime used by name only
unsigned int g_loadStartTime = 0;

// GLOBAL: 0x62a1e4 g_loadDuration
// ALONE: g_loadDuration used by name only
unsigned int g_loadDuration = 0;

// GLOBAL: 0x62a1e8 g_loadOfsX
// ALONE: g_loadOfsX used by name only
float g_loadOfsX = 0;

// GLOBAL: 0x62a1ec g_loadOfsY
// ALONE: g_loadOfsY used by name only
float g_loadOfsY = 0;

// GLOBAL: 0x62a1f0 g_shadowSkip
// ALONE: g_shadowSkip used by name only
unsigned char g_shadowSkip = 0;

// GLOBAL: 0x62a1f1 p4b_pad_62a1f1
// ALONE: p4b_pad_62a1f1 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a1f1 = 0;

// GLOBAL: 0x62a1f2 p4b_pad_62a1f2
// ALONE: p4b_pad_62a1f2 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a1f2 = 0;

// GLOBAL: 0x62a1f3 p4b_pad_62a1f3
// ALONE: p4b_pad_62a1f3 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a1f3 = 0;

// GLOBAL: 0x62a1f4 p4b_pad_62a1f4
// ALONE: p4b_pad_62a1f4 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a1f4 = 0;

// GLOBAL: 0x62a1f5 p4b_pad_62a1f5
// ALONE: p4b_pad_62a1f5 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a1f5 = 0;

// GLOBAL: 0x62a1f6 p4b_pad_62a1f6
// ALONE: p4b_pad_62a1f6 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a1f6 = 0;

// GLOBAL: 0x62a1f7 p4b_pad_62a1f7
// ALONE: p4b_pad_62a1f7 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a1f7 = 0;

// GLOBAL: 0x62a1f8 DAT_0062a1f8
// ALONE: DAT_0062a1f8 used by name only
char DAT_0062a1f8 = 0;

// GLOBAL: 0x62a2f8 g_nextLevel
// ALONE: g_nextLevel used by name only
int g_nextLevel = 0;

// GLOBAL: 0x62a308 g_loadLastDraw
// ALONE: g_loadLastDraw used by name only
unsigned int g_loadLastDraw = 0;

// GLOBAL: 0x62a380 TFBClockStart
// ALONE: TFBClockStart used by name only
int TFBClockStart = 0;

// GLOBAL: 0x62a384 TFBClockSum
// ALONE: TFBClockSum used by name only
int TFBClockSum = 0;

// GLOBAL: 0x62a388 TFBClockBase
// ALONE: TFBClockBase used by name only
int TFBClockBase = 0;

// GLOBAL: 0x62a390 g_shotDir
// ALONE: g_shotDir used by name only
char g_shotDir[0x100];

// GLOBAL: 0x62a490 g_shotRequest
// ALONE: g_shotRequest used by name only
unsigned char g_shotRequest = 0;

// GLOBAL: 0x62a491 p4b_pad_62a491
// ALONE: p4b_pad_62a491 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a491 = 0;

// GLOBAL: 0x62a492 p4b_pad_62a492
// ALONE: p4b_pad_62a492 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a492 = 0;

// GLOBAL: 0x62a493 p4b_pad_62a493
// ALONE: p4b_pad_62a493 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a493 = 0;

// GLOBAL: 0x62a494 g_shotNo
// ALONE: g_shotNo used by name only
int g_shotNo = 0;

// GLOBAL: 0x62a498 g_fpsShown
// ALONE: g_fpsShown used by name only
unsigned char g_fpsShown = 0;

// GLOBAL: 0x62a499 g_cutSceneVoicesActive
// ALONE: g_cutSceneVoicesActive used by name only
unsigned char g_cutSceneVoicesActive = 0;

// GLOBAL: 0x62a49a g_fpsCreated
// ALONE: g_fpsCreated used by name only
unsigned char g_fpsCreated = 0;

// GLOBAL: 0x62a49c p4b_pad_62a49c
// ALONE: p4b_pad_62a49c padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a49c = 0;

// GLOBAL: 0x62a49d p4b_pad_62a49d
// ALONE: p4b_pad_62a49d padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a49d = 0;

// GLOBAL: 0x62a49e p4b_pad_62a49e
// ALONE: p4b_pad_62a49e padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a49e = 0;

// GLOBAL: 0x62a49f p4b_pad_62a49f
// ALONE: p4b_pad_62a49f padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a49f = 0;

// GLOBAL: 0x62a4a0 p4b_pad_62a4a0
// ALONE: p4b_pad_62a4a0 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a0 = 0;

// GLOBAL: 0x62a4a1 p4b_pad_62a4a1
// ALONE: p4b_pad_62a4a1 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a1 = 0;

// GLOBAL: 0x62a4a2 p4b_pad_62a4a2
// ALONE: p4b_pad_62a4a2 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a2 = 0;

// GLOBAL: 0x62a4a3 p4b_pad_62a4a3
// ALONE: p4b_pad_62a4a3 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a3 = 0;

// GLOBAL: 0x62a4a4 p4b_pad_62a4a4
// ALONE: p4b_pad_62a4a4 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a4 = 0;

// GLOBAL: 0x62a4a5 p4b_pad_62a4a5
// ALONE: p4b_pad_62a4a5 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a5 = 0;

// GLOBAL: 0x62a4a6 p4b_pad_62a4a6
// ALONE: p4b_pad_62a4a6 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a6 = 0;

// GLOBAL: 0x62a4a7 p4b_pad_62a4a7
// ALONE: p4b_pad_62a4a7 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a4a7 = 0;

// GLOBAL: 0x62a4a8 g_fpsCounter
// ALONE: g_fpsCounter used by name only
__declspec(align(4)) struct FrameRate g_fpsCounter = {};

// GLOBAL: 0x62a4b8 TFBHook
// ALONE: TFBHook used by name only
__declspec(align(4)) struct TFBSystemHook TFBHook = {};

// GLOBAL: 0x62a520 g_ctlSelA
// ALONE: g_ctlSelA used by name only
int g_ctlSelA = 0;

// GLOBAL: 0x62a524 RWS::MainLoop::Render::g_pCamera
// ALONE: RWS::MainLoop::Render::g_pCamera used by name only
namespace RWS { namespace MainLoop { namespace Render { struct RwCamera * g_pCamera = 0; } } }

// GLOBAL: 0x62a528 RWS::MainLoop::Render::sm_frame_count
// ALONE: RWS::MainLoop::Render::sm_frame_count used by name only
namespace RWS { namespace MainLoop { namespace Render { int sm_frame_count = 0; } } }

// GLOBAL: 0x62a57c g_loadRaster
// ALONE: g_loadRaster used by name only
struct RwRaster * g_loadRaster = 0;

// GLOBAL: 0x62a59c g_distClip
// ALONE: g_distClip used by name only
unsigned char g_distClip = 0;

// GLOBAL: 0x62a59d p4b_pad_62a59d
// ALONE: p4b_pad_62a59d padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a59d = 0;

// GLOBAL: 0x62a59e p4b_pad_62a59e
// ALONE: p4b_pad_62a59e padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a59e = 0;

// GLOBAL: 0x62a59f p4b_pad_62a59f
// ALONE: p4b_pad_62a59f padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a59f = 0;

// GLOBAL: 0x62a5a0 g_distance
// ALONE: g_distance used by name only
int g_distance = 0;

// GLOBAL: 0x62a5a8 g_audioDevice
// ALONE: g_audioDevice used by name only
struct CfgObj * g_audioDevice = 0;

// GLOBAL: 0x62a5ac g_dict
// ALONE: g_dict used by name only
struct TextDict * g_dict = 0;

// GLOBAL: 0x62a5b0 g_wndWindowed
// ALONE: g_wndWindowed used by name only
unsigned char g_wndWindowed = 0;

// GLOBAL: 0x62a5b1 g_wndRequestFlag
// ALONE: g_wndRequestFlag used by name only
unsigned char g_wndRequestFlag = 0;

// GLOBAL: 0x62a5b2 p4b_pad_62a5b2
// ALONE: p4b_pad_62a5b2 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5b2 = 0;

// GLOBAL: 0x62a5b3 p4b_pad_62a5b3
// ALONE: p4b_pad_62a5b3 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5b3 = 0;

// GLOBAL: 0x62a5b4 g_wndRequestA
// ALONE: g_wndRequestA used by name only
int g_wndRequestA = 0;

// GLOBAL: 0x62a5b8 g_wndRequestB
// ALONE: g_wndRequestB used by name only
int g_wndRequestB = 0;

// GLOBAL: 0x62a5bc g_padStickUp
// ALONE: g_padStickUp used by name only
unsigned char g_padStickUp = 0;

// GLOBAL: 0x62a5bd g_padStickDown
// ALONE: g_padStickDown used by name only
unsigned char g_padStickDown = 0;

// GLOBAL: 0x62a5be g_padStickLeft
// ALONE: g_padStickLeft used by name only
unsigned char g_padStickLeft = 0;

// GLOBAL: 0x62a5bf g_padStickRight
// ALONE: g_padStickRight used by name only
unsigned char g_padStickRight = 0;

// GLOBAL: 0x62a5c0 g_unlockLevels
// ALONE: g_unlockLevels used by name only
unsigned char g_unlockLevels = 0;

// GLOBAL: 0x62a5c1 g_cfgQuitOnInput
// ALONE: g_cfgQuitOnInput used by name only
unsigned char g_cfgQuitOnInput = 0;

// GLOBAL: 0x62a5c2 g_cfgKeyReqA
// ALONE: g_cfgKeyReqA used by name only
unsigned char g_cfgKeyReqA = 0;

// GLOBAL: 0x62a5c3 g_cfgKeyReqB
// ALONE: g_cfgKeyReqB used by name only
unsigned char g_cfgKeyReqB = 0;

// GLOBAL: 0x62a5c4 p4b_pad_62a5c4
// ALONE: p4b_pad_62a5c4 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5c4 = 0;

// GLOBAL: 0x62a5c5 p4b_pad_62a5c5
// ALONE: p4b_pad_62a5c5 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5c5 = 0;

// GLOBAL: 0x62a5c6 p4b_pad_62a5c6
// ALONE: p4b_pad_62a5c6 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5c6 = 0;

// GLOBAL: 0x62a5c7 p4b_pad_62a5c7
// ALONE: p4b_pad_62a5c7 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5c7 = 0;

// GLOBAL: 0x62a5c8 g_resWidth
// ALONE: g_resWidth used by name only
unsigned int g_resWidth = 0;

// GLOBAL: 0x62a5cc g_resHeight
// ALONE: g_resHeight used by name only
unsigned int g_resHeight = 0;

// GLOBAL: 0x62a5d0 g_mainLoopEntered
// ALONE: g_mainLoopEntered used by name only
unsigned char g_mainLoopEntered = 0;

// GLOBAL: 0x62a5d1 g_mouseButtonDown
// ALONE: g_mouseButtonDown used by name only
unsigned char g_mouseButtonDown = 0;

// GLOBAL: 0x62a5d2 g_cursorShown
// ALONE: g_cursorShown used by name only
unsigned char g_cursorShown = 0;

// GLOBAL: 0x62a5d3 p4b_pad_62a5d3
// ALONE: p4b_pad_62a5d3 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5d3 = 0;

// GLOBAL: 0x62a5d4 g_cursorSprite
// ALONE: g_cursorSprite used by name only
class RWS::TFB::GameSprite * g_cursorSprite = 0;

// GLOBAL: 0x62a5d8 g_cursorSaved
// ALONE: g_cursorSaved used by name only
unsigned char g_cursorSaved = 0;

// GLOBAL: 0x62a5d9 p4b_pad_62a5d9
// ALONE: p4b_pad_62a5d9 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5d9 = 0;

// GLOBAL: 0x62a5da p4b_pad_62a5da
// ALONE: p4b_pad_62a5da padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5da = 0;

// GLOBAL: 0x62a5db p4b_pad_62a5db
// ALONE: p4b_pad_62a5db padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62a5db = 0;

// GLOBAL: 0x62a5dc DAT_0062a5dc
// ALONE: DAT_0062a5dc used by name only
void * DAT_0062a5dc = 0;

// GLOBAL: 0x62a604 g_strPool
// ALONE: g_strPool used by name only
struct StrPool * g_strPool = 0;

// GLOBAL: 0x62a7b0 _rpCollisionGeometryDataOffset
// ALONE: _rpCollisionGeometryDataOffset used by name only
// defined in port/shim/rwshim.cpp under its C name (rw/rwcore.h declares it extern "C")

// GLOBAL: 0x62ac18 RwEngineInstance
// ALONE: RwEngineInstance used by name only
// defined in port/shim/rwshim.cpp under its C name (rw/rwcore.h declares it extern "C")

// GLOBAL: 0x62dc30 g_wdScratch
// ALONE: g_wdScratch used by name only
void * g_wdScratch = 0;

// GLOBAL: 0x62dc34 g_wdScratchSize
// ALONE: g_wdScratchSize used by name only
int g_wdScratchSize = 0;

// GLOBAL: 0x62ed28 g_stickyOff
// ALONE: g_stickyOff used by name only
__declspec(align(4)) struct StickyKeys g_stickyOff = {};

// GLOBAL: 0x62ed30 g_platformLists
// ALONE: g_platformLists used by name only
struct PlatformNode * g_platformLists[255];

// GLOBAL: 0x62f12c g_stickySaved
// ALONE: g_stickySaved used by name only
__declspec(align(4)) struct StickyKeys g_stickySaved = {};

// GLOBAL: 0x62f138 g_mainWnd
// ALONE: g_mainWnd used by name only
void * g_mainWnd = 0;

// GLOBAL: 0x62f13c g_hInstance
// ALONE: g_hInstance used by name only
void * g_hInstance = 0;

// GLOBAL: 0x62f140 g_quitRequested
// ALONE: g_quitRequested used by name only
unsigned char g_quitRequested = 0;

// GLOBAL: 0x62f141 g_frameReady
// ALONE: g_frameReady used by name only
unsigned char g_frameReady = 0;

// GLOBAL: 0x62f142 p4b_pad_62f142
// ALONE: p4b_pad_62f142 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62f142 = 0;

// GLOBAL: 0x62f143 p4b_pad_62f143
// ALONE: p4b_pad_62f143 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62f143 = 0;

// GLOBAL: 0x62f144 g_wndBorderX
// ALONE: g_wndBorderX used by name only
int g_wndBorderX = 0;

// GLOBAL: 0x62f14c g_screenSaverTimeout
// ALONE: g_screenSaverTimeout used by name only
unsigned int g_screenSaverTimeout = 0;

// GLOBAL: 0x62f150 g_pumpCounter
// ALONE: g_pumpCounter used by name only
unsigned int g_pumpCounter = 0;

// GLOBAL: 0x62f154 g_mouseSpeed
// ALONE: g_mouseSpeed used by name only
int g_mouseSpeed = 0;

// GLOBAL: 0x62f158 g_pDirectInput8Create
// ALONE: g_pDirectInput8Create used by name only
struct DiGuid; struct IDirectInput8A;
long (__stdcall * g_pDirectInput8Create)(void *, unsigned int, const struct DiGuid *, struct IDirectInput8A **, void *) = 0;

// GLOBAL: 0x62f15c g_inputManager
// ALONE: g_inputManager used by name only
class InputManager * g_inputManager = 0;

// GLOBAL: 0x62f160 g_hDInput8
// ALONE: g_hDInput8 used by name only
void * g_hDInput8 = 0;

// GLOBAL: 0x62f164 g_dinputLoaded
// ALONE: g_dinputLoaded used by name only
unsigned char g_dinputLoaded = 0;

// GLOBAL: 0x62f165 p4b_pad_62f165
// ALONE: p4b_pad_62f165 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62f165 = 0;

// GLOBAL: 0x62f166 p4b_pad_62f166
// ALONE: p4b_pad_62f166 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62f166 = 0;

// GLOBAL: 0x62f167 p4b_pad_62f167
// ALONE: p4b_pad_62f167 padding, keeps the game's distance between neighbours
unsigned char p4b_pad_62f167 = 0;

// GLOBAL: 0x62f168 g_mouseSmoothY
// ALONE: g_mouseSmoothY used by name only
int g_mouseSmoothY = 0;

// GLOBAL: 0x62f16c g_mouseSmoothX
// ALONE: g_mouseSmoothX used by name only
int g_mouseSmoothX = 0;

// GLOBAL: 0x62f170 g_padPrev
// ALONE: g_padPrev used by name only
unsigned int g_padPrev = 0;

// GLOBAL: 0x62f174 g_ioMutex
// ALONE: g_ioMutex used by name only
void * g_ioMutex = 0;

// GLOBAL: 0x62f178 g_ioThread
// ALONE: g_ioThread used by name only
void * g_ioThread = 0;

// GLOBAL: 0x62f17c g_reqCounter
// ALONE: g_reqCounter used by name only
unsigned int g_reqCounter = 0;

// GLOBAL: 0x62f180 g_pfnCancelIo
// ALONE: g_pfnCancelIo used by name only
void * g_pfnCancelIo = 0;

// GLOBAL: 0x62f1d8 g_pfnGetOverlappedResult
// ALONE: g_pfnGetOverlappedResult used by name only
void * g_pfnGetOverlappedResult = 0;

// GLOBAL: 0x62f1dc g_pfnCloseHandle
// ALONE: g_pfnCloseHandle used by name only
int (__stdcall * g_pfnCloseHandle)(void *h) = 0;

// GLOBAL: 0x62f1e0 g_dsoundObj
// ALONE: g_dsoundObj used by name only
struct DirectSound * g_dsoundObj = 0;

// GLOBAL: 0x62f208 _rpSkinGlobals
// ALONE: _rpSkinGlobals used by name only
__declspec(align(4)) struct RpSkinGlobals _rpSkinGlobals = {};

// GLOBAL: 0x62f214 g_audioValue214
// ALONE: g_audioValue214 used by name only
int g_audioValue214 = 0;

// GLOBAL: 0x62fec8 RWS::iMsgDeleteEventHandler
// ALONE: RWS::iMsgDeleteEventHandler used by name only
namespace RWS { CEventId iMsgDeleteEventHandler; }

// GLOBAL: 0x62fed0 RWS::iMsgDeleteEntity
// ALONE: RWS::iMsgDeleteEntity used by name only
namespace RWS { CEventId iMsgDeleteEntity; }

// GLOBAL: 0x62fed8 RWS::iMsgOnDeleteEntity
// ALONE: RWS::iMsgOnDeleteEntity used by name only
namespace RWS { CEventId iMsgOnDeleteEntity; }

// GLOBAL: 0x631f10 _rxPipelineGlobalsOffset
// ALONE: _rxPipelineGlobalsOffset used by name only
// defined in port/shim/rwshim.cpp under its C name (rw/rwcore.h declares it extern "C")

