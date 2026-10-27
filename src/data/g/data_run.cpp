// The .data run 0x6023c0 and 0x60241c..0x606cc8, in address order. One file because a global whose next game symbol is also
// defined in src/data must sit right behind it in the same object and section (datacheck LAYOUT). Unnamed words behind a global
// are typed tail members (see "typed tails"); other unnamed words stay in the image.
#include "framework/loadscreentex.h"
#include "framework/startup/win32/input/controls.h"
#include "rw/rwaudio.h"
#include "modules/fx/partspray.h"   // RWS::FX::TexPartBuffer::DrawMode
#include "tfb/level/levelmanager.h"
#include <float.h>
// One .data section: without the pragma cl 13.10 moves zero-initialised variables (g_boundsMaxY/Z) into .bss, which breaks the layout.
#if defined(_MSC_VER) || defined(_WIN32)   // COFF section name; only the layout check needs it
#pragma data_seg(".data")
#endif
// ---- ctl/bindings
// Rows of g_ctlBindings, read by Controls_LoadSettings (struct CtlBinding, include/ctl/ctlmap.h, 0x40 B): registry key name, action index
// (m_28), opposite action (m_2c), default of slots 1 and 2 as (device type << 16 | code), and a "fixed" flag per slot (not read from the registry).
// ---- audio/env_presets
// The 24 standard I3DL2 reverb presets in DirectSound DSFXI3DL2Reverb order (DEFAULT .. UNDERWATER); g_envPresets[type] is copied by
// SetEnvironment.
// ---- script/subset_names
// Script subset name hashes.
// ---- script/crc_table
// ---- f/lang_tables
// ---- g/tfb_misc
// Small TFB, anim and language globals; each is followed in the game by unnamed data (left out).
// Prefix of language-specific resource names (w3b_rwres.cpp: strncmp(name, g_langPrefix, 5)). The 104 B between it and g_alphaTag
// are typed here to keep the game's layout: the stream type id row 1,5,1,1,1,2,1,2 (0x0f0f000x) twice, a pointer to
// "FACE_LAYER_FLAGS", the row again and the pointer again.
struct LangPrefixTail {
	unsigned m_rowA[8];
	unsigned m_rowB[8];
	const char *m_faceA;
	unsigned m_rowC[8];
	const char *m_faceB;
};
typedef char lang_size_check_LangPrefixTail[sizeof(void *) != 4 || sizeof(LangPrefixTail) == 0x68 ? 1 : -1];
// ---- rws/core_streams
// Stream pump hook pointer. 0x410380 is the shared empty `ret 4` function; it is written by its FUN_ name because the port defines that
// spelling, and an invented name would resolve to a trap stub.
void __stdcall FUN_00410380(int);

// ---- typed tails
// 0x0f0f0001..5 are the stream type ids of the anim/TFB node readers. The row 1,5,1,1,1,2,1,2 (m_streamTypeIds), pointers to
// "FACE_LAYER_FLAGS" (m_faceLayerFlagsName) and single ids follow most globals of this run; nothing reads them by name.
struct SubRasterCameraIds { const char *m_className; unsigned m_streamTypeIds[8]; const char *m_controllerClassName;
	unsigned m_padToBindings; };   // zero word: padding before the next 8-aligned global
struct ClassIdTags2 { const char *m_className; unsigned m_streamTypeIds[16]; };
struct ClassIdTags { const char *m_className; unsigned m_streamTypeIds[8]; };
struct LevelHubIds { const char *m_className; const char *m_scriptNamesName; };
struct IntTagsFace { int m_value; unsigned m_streamTypeIds[8]; const char *m_faceLayerFlagsName; };
struct FloatTags { float m_value; unsigned m_streamTypeIds[8]; };
struct CrcTableTags { unsigned m_crc[256]; unsigned m_streamTypeIds[8]; };
struct ValueKindNamesTags { const char *m_names[3]; unsigned m_streamTypeIds[8]; };
struct RadarScaleColours { float m_scale; unsigned m_colourA; unsigned m_colourB; unsigned m_colourC; };
struct MemCardStatusBlock { int m_status; const char *m_saveFolderNames[21]; };
struct LangCodesTags { const char *m_codes[18]; unsigned m_streamTypeIds[8]; };
struct AlphaTagBlock { const char *m_substring; unsigned m_streamTypeIds[24]; const char *m_faceLayerFlagsName; unsigned m_streamTypeId; };
struct AnimDefaultPair { int m_nodeTypeId; unsigned m_streamTypeId; };
struct FdMinTail { unsigned m_streamTypeIds[8]; const char *m_faceLayerFlagsName; unsigned m_streamTypeId; };
struct IntTagsFaceId { int m_value; unsigned m_streamTypeIds[8]; const char *m_faceLayerFlagsName; unsigned m_streamTypeId; };
struct TfbWorldOfsBlock { int m_value; unsigned m_streamTypeIds[8]; const char *m_faceLayerFlagsName; unsigned m_streamTypeIdsB[8]; const char *m_faceLayerFlagsNameB; unsigned m_streamTypeId; };
struct CtfbSoundClassIdPad { const char *m_className; unsigned m_padToEnvPresets; };   // zero word: padding before the 8-aligned g_envPresets
struct CurrentLevelPad { int m_level; unsigned m_padToLevelTable; };   // zero word: padding before the 8-aligned g_LevelTable
struct IntTags { int m_value; unsigned m_streamTypeIds[8]; };
struct TagsIds16 { unsigned m_streamTypeIds[16]; };
struct TagsIds8 { unsigned m_streamTypeIds[8]; };
struct TagsFace { unsigned m_streamTypeIds[8]; const char *m_faceLayerFlagsName; };
struct TagsFaceId { unsigned m_streamTypeIds[8]; const char *m_faceLayerFlagsName; unsigned m_streamTypeId; };
struct AlphaTail { unsigned m_streamTypeIds[24]; const char *m_faceLayerFlagsName; unsigned m_streamTypeId; };
struct WorldTail { unsigned m_streamTypeIdsA[8]; const char *m_faceLayerFlagsNameA; unsigned m_streamTypeIdsB[8]; const char *m_faceLayerFlagsNameB; unsigned m_streamTypeId; };
struct SubRasterTail { unsigned m_streamTypeIds[8]; const char *m_controllerClassName; unsigned m_padToBindings; };
struct ShadowTail { float m_f1200; unsigned m_streamTypeIdsA[8]; unsigned m_streamTypeIdsB[8]; };
struct RadarTail { unsigned m_colourA; unsigned m_colourB; unsigned m_colourC; };

// ---- exception type descriptors and the CEntity class id before them (0x6023c0..0x60241c)
#include "rtti_std.h"
// ALONE: s_CEntity C string (8 B incl. the NUL), shared with the CEntity registrar's argument
// GLOBAL: 0x5ca2e0 s_CEntity
extern const char s_CEntity[8] = "CEntity";

// CEntity class id: a pointer to the literal, like g_protoActorClassId (0x602f94). Unread.
// ALONE: g_entityClassId unread by the rest of the tree; the type descriptors of the exception classes follow
// GLOBAL: 0x6023c0 g_entityClassId
const char *g_entityClassId = s_CEntity;
// Type descriptors of std::exception, logic_error and length_error; their records are in rtti_std.cpp.
// GLOBAL: 0x6023c4 g_tdException
RttiTypeDesc<16> g_tdException = { g_typeInfoVftable, 0, ".?AVexception@@" };
// GLOBAL: 0x6023dc g_tdLogicError
RttiTypeDesc<24> g_tdLogicError = { g_typeInfoVftable, 0, ".?AVlogic_error@std@@" };
// GLOBAL: 0x6023fc g_tdLengthError
RttiTypeDesc<24> g_tdLengthError = { g_typeInfoVftable, 0, ".?AVlength_error@std@@" };

// GLOBAL: 0x60241c g_subRasterCameraClassId
const char *g_subRasterCameraClassId = "CSubRasterCamera";
// GLOBAL: 0x602420 g_subRasterCameraClassId_tail
__declspec(align(4)) SubRasterTail g_subRasterCameraClassId_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "GameController", 0 };

// The 16 rows span 0x400 B; the 8 dwords of 0x0f0f000x behind them are not part of this object.
// GLOBAL: 0x602448 g_ctlBindings
__declspec(align(4)) CtlBinding g_ctlBindings[16] = {
		{ "Forward", 1, 2, 0x10011, 0, {0}, 0x100c8, 0, {0} },   // 0
		{ "Backward", 2, 1, 0x1001f, 0, {0}, 0x100d0, 0, {0} },   // 1
		{ "TurnLeft", 3, 4, 0x1001e, 0, {0}, 0x100cb, 0, {0} },   // 2
		{ "TurnRight", 4, 3, 0x10020, 0, {0}, 0x100cd, 0, {0} },   // 3
		{ "Jump", 9, 0, 0x10039, 0, {0}, 0xffff, 0, {0} },   // 4
		{ "Action", 10, 0, 0x2000a, 0, {0}, 0x10014, 0, {0} },   // 5
		{ "Fire", 11, 0, 0x20009, 0, {0}, 0x10021, 0, {0} },   // 6
		{ "Alternate Attack", 12, 0, 0x1002e, 0, {0}, 0x10022, 0, {0} },   // 7
		{ "Show UI", 16, 0, 0x1002a, 0, {0}, 0xffff, 0, {0} },   // 8
		{ "CameraUp", 5, 6, 0x10048, 0, {0}, 0x100c9, 0, {0} },   // 9
		{ "CameraDown", 6, 5, 0x10050, 0, {0}, 0x100d1, 0, {0} },   // 10
		{ "CameraLeft", 7, 8, 0x1004b, 0, {0}, 0xffff, 0, {0} },   // 11
		{ "CameraRight", 8, 7, 0x1004d, 0, {0}, 0xffff, 0, {0} },   // 12
		{ "Pause", 13, 0, 0x10001, 1, {0}, 0xffff, 0, {0} },   // 13
		{ "ResetCamera", 14, 0, 0x1002d, 0, {0}, 0xffff, 0, {0} },   // 14
		{ "TakeScreenshot", 15, 0, 0x10057, 1, {0}, 0xffff, 0, {0} },   // 15
};
// ALONE: g_ctlBindings_tail the 8 stream type ids behind the table, never read through this object; the next global is reached by its own name
// GLOBAL: 0x602848 g_ctlBindings_tail
__declspec(align(4)) unsigned g_ctlBindings_tail[8] = { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 };

// GLOBAL: 0x602868 g_xboxControllerClassId
const char * g_xboxControllerClassId = "CXboxStndController";

// GLOBAL: 0x60286c g_btnMask
unsigned g_btnMask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

// GLOBAL: 0x60288c g_audioGlobalMixerClassId
const char * g_audioGlobalMixerClassId = "AudioGlobalMixer";

// GLOBAL: 0x602890 g_cfxBaseLightClassId
const char * g_cfxBaseLightClassId = "CFXBaseLight";

// GLOBAL: 0x602894 g_cfxColorLightClassId
const char * g_cfxColorLightClassId = "CFXColorLight";

// GLOBAL: 0x602898 g_replicatorGeneratorClassId (the class id of the unused Studio replicator behaviour)
const char *g_replicatorGeneratorClassId = "CFXReplicatorGenerator";
// GLOBAL: 0x60289c pCFXReplicatorGeneratorClassID_tail
__declspec(align(4)) TagsIds16 pCFXReplicatorGeneratorClassID_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x6028dc g_pCFXPartSprayClassID
const char * g_pCFXPartSprayClassID = "CFXPartSpray";

// GLOBAL: 0x6028e0 g_sprayTexU
unsigned g_sprayTexU[7] = { 1, 1, 2, 2, 4, 4, 8 };

// GLOBAL: 0x6028fc g_sprayTexV
unsigned g_sprayTexV[7] = { 1, 2, 2, 4, 4, 8, 8 };

// GLOBAL: 0x602918 g_sprayBlend
RwBlendFunction g_sprayBlend[11] = { rwBLENDZERO, rwBLENDONE, rwBLENDSRCCOLOR, rwBLENDINVSRCCOLOR, rwBLENDSRCALPHA, rwBLENDINVSRCALPHA, rwBLENDDESTALPHA, rwBLENDINVDESTALPHA, rwBLENDDESTCOLOR, rwBLENDINVDESTCOLOR, rwBLENDSRCALPHASAT };

// GLOBAL: 0x602944 g_sprayRenderMode
RWS::FX::TexPartBuffer::DrawMode g_sprayRenderMode[3] = { RWS::FX::TexPartBuffer::DRAW_NO_DEPTH_WRITE, RWS::FX::TexPartBuffer::DRAW_DEPTH_WRITE, RWS::FX::TexPartBuffer::DRAW_TWO_PASS };

// GLOBAL: 0x602950 g_environmentMapClassId (the class id of the unused Studio environment map behaviour)
const char *g_environmentMapClassId = "CFXEnvironmentMap";

// ALONE: g_cfxMotionBlurClassId only read as the class name pointer (handlers_c.cpp CAttributeCommandIterator(&attr, g_cfxMotionBlurClassId)); unnamed literals follow before g_levelFarDefault
// GLOBAL: 0x602954 g_cfxMotionBlurClassId
const char * g_cfxMotionBlurClassId = "CFXMotionBlur";

// GLOBAL: 0x60297c g_levelFarDefault
float g_levelFarDefault = -1.0f;

// GLOBAL: 0x602980 g_dofBluriness
int g_dofBluriness = 50;

// GLOBAL: 0x602984 g_bloomAddFactor
float g_bloomAddFactor = 1.0f;

// GLOBAL: 0x602988 g_bloomHaloSize
float g_bloomHaloSize = 1.0f;

// GLOBAL: 0x60298c g_flag60298c
int g_flag60298c = 1;

// GLOBAL: 0x602990 g_f602990
float g_f602990 = 0.94f;

// GLOBAL: 0x602994 g_f602994
float g_f602994 = 0.99f;

// unnamed int between g_f602994 and g_f60299c (value 1), keeps the game's 8-byte distance
// GLOBAL: 0x602998 DAT_00602998
int DAT_00602998 = 1;

// GLOBAL: 0x60299c g_f60299c
float g_f60299c = 0.095f;

// GLOBAL: 0x6029a0 g_f6029a0
float g_f6029a0 = 0.035f;

// GLOBAL: 0x6029bc g_ctfbCommandClassId
const char *g_ctfbCommandClassId = "CTFBCommand";
// GLOBAL: 0x6029c0 g_ctfbCommandClassId_tail
__declspec(align(4)) TagsIds8 g_ctfbCommandClassId_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x6029e0 g_spriteManagerClassId
const char * g_spriteManagerClassId = "SpriteManager";

// GLOBAL: 0x6029e4 g_spriteObjectClassId
const char *g_spriteObjectClassId = "SpriteObject";
// GLOBAL: 0x6029e8 g_spriteObjectClassId_tail
__declspec(align(4)) TagsIds8 g_spriteObjectClassId_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x602a08 g_envLevel
float g_envLevel = 1.0f;

// GLOBAL: 0x602a0c g_envRoom
int g_envRoom = -1000;

// GLOBAL: 0x602a10 g_ctfbSoundClassId
const char *g_ctfbSoundClassId = "CTFBSound";
// GLOBAL: 0x602a14 g_ctfbSoundClassId_tail
__declspec(align(4)) unsigned g_ctfbSoundClassId_tail = 0;

// GLOBAL: 0x602a18 g_envPresets
__declspec(align(4)) RwaEnvironment g_envPresets[24] = {
	{ -10000, -10000, 0.0f, 1.49f, 0.83f, -2602, 0.007f, 200, 0.011f, 100.0f, 100.0f, 5000.0f },   // 0 DEFAULT
	{ -1000, -100, 0.0f, 1.49f, 0.83f, -2602, 0.007f, 200, 0.011f, 100.0f, 100.0f, 5000.0f },   // 1 GENERIC
	{ -1000, -6000, 0.0f, 0.17f, 0.1f, -1204, 0.001f, 207, 0.002f, 100.0f, 100.0f, 5000.0f },   // 2 PADDEDCELL
	{ -1000, -454, 0.0f, 0.4f, 0.83f, -1646, 0.002f, 53, 0.003f, 100.0f, 100.0f, 5000.0f },   // 3 ROOM
	{ -1000, -1200, 0.0f, 1.49f, 0.54f, -370, 0.007f, 1030, 0.011f, 100.0f, 60.0f, 5000.0f },   // 4 BATHROOM
	{ -1000, -6000, 0.0f, 0.5f, 0.1f, -1376, 0.003f, -1104, 0.004f, 100.0f, 100.0f, 5000.0f },   // 5 LIVINGROOM
	{ -1000, -300, 0.0f, 2.31f, 0.64f, -711, 0.012f, 83, 0.017f, 100.0f, 100.0f, 5000.0f },   // 6 STONEROOM
	{ -1000, -476, 0.0f, 4.32f, 0.59f, -789, 0.02f, -289, 0.03f, 100.0f, 100.0f, 5000.0f },   // 7 AUDITORIUM
	{ -1000, -500, 0.0f, 3.92f, 0.7f, -1230, 0.02f, -2, 0.029f, 100.0f, 100.0f, 5000.0f },   // 8 CONCERTHALL
	{ -1000, 0, 0.0f, 2.91f, 1.3f, -602, 0.015f, -302, 0.022f, 100.0f, 100.0f, 5000.0f },   // 9 CAVE
	{ -1000, -698, 0.0f, 7.24f, 0.33f, -1166, 0.02f, 16, 0.03f, 100.0f, 100.0f, 5000.0f },   // 10 ARENA
	{ -1000, -1000, 0.0f, 10.05f, 0.23f, -602, 0.02f, 198, 0.03f, 100.0f, 100.0f, 5000.0f },   // 11 HANGAR
	{ -1000, -4000, 0.0f, 0.3f, 0.1f, -1831, 0.002f, -1630, 0.03f, 100.0f, 100.0f, 5000.0f },   // 12 CARPETEDHALLWAY
	{ -1000, -300, 0.0f, 1.49f, 0.59f, -1219, 0.007f, 441, 0.011f, 100.0f, 100.0f, 5000.0f },   // 13 HALLWAY
	{ -1000, -237, 0.0f, 2.7f, 0.79f, -1214, 0.013f, 395, 0.02f, 100.0f, 100.0f, 5000.0f },   // 14 STONECORRIDOR
	{ -1000, -270, 0.0f, 1.49f, 0.86f, -1204, 0.007f, -4, 0.011f, 100.0f, 100.0f, 5000.0f },   // 15 ALLEY
	{ -1000, -3300, 0.0f, 1.49f, 0.54f, -2560, 0.162f, -613, 0.088f, 79.0f, 100.0f, 5000.0f },   // 16 FOREST
	{ -1000, -800, 0.0f, 1.49f, 0.67f, -2273, 0.007f, -2217, 0.011f, 50.0f, 100.0f, 5000.0f },   // 17 CITY
	{ -1000, -2500, 0.0f, 1.49f, 0.21f, -2780, 0.3f, -2014, 0.1f, 27.0f, 100.0f, 5000.0f },   // 18 MOUNTAINS
	{ -1000, -1000, 0.0f, 1.49f, 0.83f, -10000, 0.061f, 500, 0.025f, 100.0f, 100.0f, 5000.0f },   // 19 QUARRY
	{ -1000, -2000, 0.0f, 1.49f, 0.5f, -2466, 0.179f, -2514, 0.1f, 21.0f, 100.0f, 5000.0f },   // 20 PLAIN
	{ -1000, 0, 0.0f, 1.65f, 1.5f, -1363, 0.008f, -1153, 0.012f, 100.0f, 100.0f, 5000.0f },   // 21 PARKINGLOT
	{ -1000, -1000, 0.0f, 2.81f, 0.14f, 429, 0.014f, 648, 0.021f, 80.0f, 60.0f, 5000.0f },   // 22 SEWERPIPE
	{ -1000, -4000, 0.0f, 1.49f, 0.1f, -449, 0.007f, 1700, 0.011f, 100.0f, 100.0f, 5000.0f },   // 23 UNDERWATER
};

// GLOBAL: 0x602e98 g_levelHubClassId
const char *g_levelHubClassId = "LevelHub";
// GLOBAL: 0x602e9c g_levelHubClassId_tail
__declspec(align(4)) const char *g_levelHubClassId_tail = "GlobalScriptNames";

// GLOBAL: 0x602ea0 g_globalSymbolCount
int g_globalSymbolCount = -1;
// GLOBAL: 0x602ea4 g_globalSymbolCount_tail
__declspec(align(4)) TagsFace g_globalSymbolCount_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS" };

// GLOBAL: 0x602ec8 g_ctfbWorldClassId
const char * g_ctfbWorldClassId = "CTFBWorld";

// unnamed int (5) between g_ctfbWorldClassId and g_fogScale_602ed0, keeps the game's distance
// GLOBAL: 0x602ecc DAT_00602ecc
int DAT_00602ecc = 5;

// GLOBAL: 0x602ed0 g_pixelFogBlendB
float g_pixelFogBlendB = 1.5f;

// GLOBAL: 0x602ed4 g_pixelFogScaleA
float g_pixelFogScaleA = 0.75f;
// GLOBAL: 0x602ed8 g_pixelFogScaleA_tail
__declspec(align(4)) TagsIds8 g_pixelFogScaleA_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x602ef8 g_worldObjectClassId
const char *g_worldObjectClassId = "WorldObject";
// GLOBAL: 0x602efc g_worldObjectClassId_tail
__declspec(align(4)) TagsIds8 g_worldObjectClassId_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x602f1c g_cameraDataClassId
const char *g_cameraDataClassId = "CameraData";
// GLOBAL: 0x602f20 g_cameraDataClassId_tail
__declspec(align(4)) TagsIds8 g_cameraDataClassId_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x602f40 g_gameCameraClassId
const char * g_gameCameraClassId = "GameCamera";

// Node flags of the camera animation hierarchy (CameraRebuildAnimHierarchy, two nodes).
// GLOBAL: 0x602f44 g_camNodeFlags
unsigned g_camNodeFlags[2] = { 0, 1 };

// GLOBAL: 0x602f4c g_tfbShadowCameraClassId
const char *g_tfbShadowCameraClassId = "TFBShadowCamera";
// GLOBAL: 0x602f50 g_tfbShadowCameraClassId_tail
__declspec(align(4)) TagsIds8 g_tfbShadowCameraClassId_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x602f70 g_prevCutsceneFacing
float g_prevCutsceneFacing = 3.402823466e+38f;
// GLOBAL: 0x602f74 g_prevCutsceneFacing_tail
__declspec(align(4)) TagsIds8 g_prevCutsceneFacing_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x602f94 g_protoActorClassId
const char * g_protoActorClassId = "CProtoActor";

// ALONE: g_compareOpNames NULL-terminated list within its 7 entries (ScriptEnumReader stops at the 0 in slot 6, ops_value.cpp); an unnamed word follows before g_subsetTableSize
// GLOBAL: 0x602f98 g_compareOpNames
const char * g_compareOpNames[7] = { "<=", "=", ">=", "<", ">", "<>", 0 };

// 25 name hashes compared with ScriptReference::GetName in RegisterSubset (not string pointers, though the tree types them as
// const char *). Entries 0 and 14 are equal. 100 B; a type_info object (g_tdOutOfRange) follows.
// Count (25) and a zero word; g_subsetNames follows at +8 as in the game.
// ALONE: g_subsetTableSize scalar int count (big_render_463640.cpp, w2d_f.cpp loop `i < g_subsetTableSize`); the zero word behind it is never read
// GLOBAL: 0x602fb8 Script::g_subsetTableSize
namespace Script { int g_subsetTableSize = 25; }
// ALONE: g_subsetTableSize_pad the zero word behind the count, never read; g_subsetNames after it is reached by its own name
// GLOBAL: 0x602fbc g_subsetTableSize_pad
int g_subsetTableSize_pad = 0;

typedef char sz_g_subsetNames[sizeof(void *) != 4 || sizeof(unsigned[25]) == 100 ? 1 : -1];
// ALONE: g_subsetNames 25 entries indexed 0..g_subsetTableSize-1; the std::out_of_range type descriptor (g_tdOutOfRange) follows, then g_forEachOrderNames
// GLOBAL: 0x602fc0 g_subsetNames
__declspec(align(4)) unsigned g_subsetNames[25] = {
	
		0x9CEE1D71, 0x65700CDD, 0x1DC0CD73, 0x42A3902A, 0xEE67CC85,
		0x63976181, 0x43EEFE7A, 0x298B08AD, 0xD87C0C69, 0xF6DBC518,
		0xE0758062, 0x25F56E76, 0xD4848C8C, 0x8EB1B3DC, 0x9CEE1D71,
		0x6E351916, 0xD0E4FBBF, 0xA40C3C28, 0xBF14528A, 0x112801DF,
		0x20CA105A, 0x727F08EA, 0x094F8AB5, 0x64EE407F, 0x8D48678D,
};

// GLOBAL: 0x603024 g_tdOutOfRange
RttiTypeDesc<24> g_tdOutOfRange = { g_typeInfoVftable, 0, ".?AVout_of_range@std@@" };

// Names of the for-each order enum (ops_set.cpp ScriptEnumReader), NULL-terminated.
// GLOBAL: 0x603044 g_forEachOrderNames
const char * g_forEachOrderNames[4] = { "forward", "backward", "randomly", 0 };

// GLOBAL: 0x603054 g_cameraTransitionNames
const char * g_cameraTransitionNames[4] = { "no transition", "fade out", "iris out", 0 };

// GLOBAL: 0x603064 g_cameraTypeNames
const char * g_cameraTypeNames[4] = { "pan", "fade in", "iris in", 0 };

// GLOBAL: 0x603074 g_activeControllers
int g_activeControllers = 1;


// Standard reflected CRC-32 table (polynomial 0xEDB88320), 256 entries. 32 B of unnamed data follow.
// GLOBAL: 0x603078 g_crcTable
__declspec(align(4)) unsigned g_crcTable[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
	};
// GLOBAL: 0x603478 g_crcTable_tail
__declspec(align(4)) TagsIds8 g_crcTable_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x603498 g_valueKindNames
const char *g_valueKindNames[3] = { "constant", "random", 0 };
// GLOBAL: 0x6034a4 g_valueKindNames_tail
__declspec(align(4)) TagsIds8 g_valueKindNames_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x6034c4 g_radarSize
float g_radarSize = 0.15f;
// ALONE: g_radarSize_tail the float behind it (0.4, maybe a second radar size; read by no code), never read through this object; g_radarRange is reached by its own name
// GLOBAL: 0x6034c8 g_radarSize_tail
float g_radarSize_tail = 0.4f;

// GLOBAL: 0x6034cc g_radarRange
float g_radarRange = 3600.0f;
// GLOBAL: 0x6034d0 g_radarRange_tail
__declspec(align(4)) RadarTail g_radarRange_tail = { 0xff19ff19, 0xff1919ff, 0x80808080 };

// GLOBAL: 0x6034dc g_radarRenderClassId
const char *g_radarRenderClassId = "RadarRender";
// GLOBAL: 0x6034e0 g_radarRenderClassId_tail
__declspec(align(4)) TagsIds8 g_radarRenderClassId_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// ShadowRender class name, 1200 (shadow range?) and two rows of stream type ids; only the name pointer is read (handlers_x_shadow.cpp).
struct ShadowClassBlock { const char *m_className; float m_f1200; unsigned m_streamTypeIdsA[8]; unsigned m_streamTypeIdsB[8]; };
// ALONE: g_shadowRenderClassId only read as the class name pointer (handlers_x_shadow.cpp CAttributeCommandIterator(&attr, g_shadowRenderClassId)); the block is the 72 B up to g_ctfbModelClassId
// GLOBAL: 0x603500 g_shadowRenderClassId
__declspec(align(4)) const char *g_shadowRenderClassId = "ShadowRender";
// GLOBAL: 0x603504 g_shadowRenderClassId_tail
__declspec(align(4)) ShadowTail g_shadowRenderClassId_tail = { 1200.0f, { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// CTFBModel class name; the name pointer of the game ("Madagascar") follows it. Only the first is read (handlers_p_ctfbmodel.cpp).
struct CtfbModelIds { const char *m_className; const char *m_gameName; };
// ALONE: g_ctfbModelClassId only read as the class name pointer (CAttributeCommandIterator(&attr, g_ctfbModelClassId))
// GLOBAL: 0x603548 g_ctfbModelClassId
const char *g_ctfbModelClassId = "CTFBModel";
// GLOBAL: 0x60354c g_ctfbModelClassId_tail
__declspec(align(4)) const char *g_ctfbModelClassId_tail = "Madagascar";

// GLOBAL: 0x603550 g_saveName
const char * g_saveName = "SaveGames.mem";

// GLOBAL: 0x603554 g_saveHandle
int g_saveHandle = -1;

// GLOBAL: 0x603558 g_memCardStatus
__declspec(align(4)) int g_memCardStatus = -1;
// GLOBAL: 0x60355c g_memCardStatus_tail
__declspec(align(4)) const char *g_memCardStatus_tail[21] = { "SaveGames", "Sauvegardes", "Spielstand", "PartidasGuardadas", "SparadeSpel", "Salvataggi", "SaveGames", "SaveGames", "JogosGuardados", "SaveGames", "Saved Games", "Sauvegardes", "Spielst\344nde", "Partidas guardadas", "Sparade spel", "Salvataggi", "Saved Games", "Opgeslagen games", "Jogos Guardados", "Saved Games", "MemCard" };

// Language names indexed by g_langIndex (IsCurrentLanguageName, f/f_misc.cpp); g_langCodes follows directly.
// GLOBAL: 0x6035b0 g_langNames
__declspec(align(4)) const char *g_langNames[18] = {
	"English", "French", "German", "Spanish", "Swedish", "Italian",   // 0
	"Japanese", "Dutch", "Portuguese", "British", "Danish", "Finnish",   // 6
	"Norwegian", "Norse", "Russian", "Latin", "Esperanto", "Greek",   // 12
};

// Two-letter language codes for the same indices (GetLanguageCode, f/f_misc.cpp).
// GLOBAL: 0x6035f8 g_langCodes
__declspec(align(4)) const char *g_langCodes[18] = {
		(char *)"US", (char *)"FR", (char *)"DE", (char *)"SP", (char *)"SE", (char *)"IT",   
		(char *)"JP", (char *)"NL", (char *)"PT", (char *)"UK", (char *)"DK", (char *)"FI",   
		(char *)"NO", (char *)"SK", (char *)"RS", (char *)"LT", (char *)"ES", (char *)"GK",   
	};
// GLOBAL: 0x603640 g_langCodes_tail
__declspec(align(4)) TagsIds8 g_langCodes_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// The bounds sweep works on g_boundsItems[3][1001]: three rows A/B/C (the tree also names B and C), each starting with a pointer
// to g_boundsMaxX (the sentinel corner, read at +0/+4/+8: MaxX, Y, Z must stay adjacent and in order). Layout as in the game: count
// word + pad (8 B), rows A, B, C (4004 B each), g_poolHead, MaxX, MaxY, MaxZ.
// cl 13.10 emits a variable named by an earlier initialiser ahead of it, so `&g_boundsMaxX` in the rows would move g_boundsMaxX in
// front of them. The rows reach it as g_boundsCount + offset instead.
extern int g_boundsCount;   // the count word (0x603660) and the pad word behind it (0x603664) are two objects; the declaration is the one of tfb/collision/broadphase.h
#define BOUNDS_MAX_X ((float *)((char *)&g_boundsCount + 0x2ef8))   /* &g_boundsMaxX (0x606558) as g_boundsCount (0x603660) + 0x2ef8 */
// GLOBAL: 0x603660 g_boundsCount
int g_boundsCount = 1;
// GLOBAL: 0x603664 g_boundsCount_pad
int g_boundsCount_pad = 0;


// Head of the free list of the bounds pool (bounds.cpp: PoolNode free list); points at the pair pool g_pairPool (0x626560).
// GLOBAL: 0x606554 g_pairFreeList
namespace RWS { namespace TFB { struct CollisionPair; } }   // the pair type of tfb/collision/broadphase.h (including it here moved the layout: it declares g_boundsItems[3][1001])
extern RWS::TFB::CollisionPair g_pairPool[];
RWS::TFB::CollisionPair *g_pairFreeList = g_pairPool;


// The audio environment built by SetEnvironment from a g_envPresets row scaled by the level; GetEnvironment (0x44b0d0) returns its address.
// The first 0x30 bytes are an I3DL2 preset (EnvPresetRow layout); the next 0x34 are the font texture dictionary platform names
// "FontTxd_" + DX / PS2 / NGC / XBox, unread.
struct AudioEnvTail {
	char m_fontTxdDX[12]; char m_fontTxdPS2[12]; char m_fontTxdNGC[12]; char m_fontTxdXBox[16];
};
// GLOBAL: 0x606598 g_audioEnvironment
__declspec(align(4)) RwaEnvironment g_audioEnvironment = { -10000, -10000, 0.0f, 1.49f, 0.83f, -2602, 0.007f, 200, 0.011f, 100.0f, 100.0f, 5000.0f };
// ALONE: g_audioEnvironment_tail the platform names of the font texture dictionary behind the preset, never read through this object; g_fontTxdPrefix after it is reached by its own name
// GLOBAL: 0x6065c8 g_audioEnvironment_tail
__declspec(align(4)) AudioEnvTail g_audioEnvironment_tail = { "FontTxd_DX", "FontTxd_PS2", "FontTxd_NGC", "FontTxd_XBox" };

// GLOBAL: 0x6065fc g_fontTxdPrefix
char g_fontTxdPrefix[12] = "FontTxd_";

// GLOBAL: 0x606608 g_langPrefix
char g_langPrefix[8] = "LANG_";
// ALONE: g_langPrefix_tail the unnamed words behind the prefix up to g_alphaTag, never read through this object; g_alphaTag is reached by its own name
// GLOBAL: 0x606610 g_langPrefix_tail
__declspec(align(4)) LangPrefixTail g_langPrefix_tail = {
	{ 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 },
	{ 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 },
	"FACE_LAYER_FLAGS",
	{ 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 },
	"FACE_LAYER_FLAGS",
};

// Marks alpha-blended texture names (CompareMeshAlphaLast, r44c/misc.cpp: strstr(tex->m_name, g_alphaTag)). Points at "_alpha".
// GLOBAL: 0x606678 g_alphaTag
__declspec(align(4)) const char *g_alphaTag = "_alpha";
// GLOBAL: 0x60667c g_alphaTag_tail
__declspec(align(4)) AlphaTail g_alphaTag_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS", 0x0f0f0001 };

// GLOBAL: 0x6066e4 g_animDefaultNL
int g_animDefaultNL = 0xf0f0005;
// GLOBAL: 0x6066e8 g_animDefaultNL_tail
__declspec(align(4)) unsigned g_animDefaultNL_tail = 0x0f0f0001;

// GLOBAL: 0x6066ec g_animDefault
int g_animDefault = 0xf0f0001;
// GLOBAL: 0x6066f0 g_animDefault_tail
__declspec(align(4)) unsigned g_animDefault_tail = 0x0f0f0001;

// GLOBAL: 0x6066f4 g_animDefaultB
int g_animDefaultB = 0xf0f0002;
// GLOBAL: 0x6066f8 g_animDefaultB_tail
__declspec(align(4)) unsigned g_animDefaultB_tail = 0x0f0f0001;

// Default for AnimClipCore::m_4c (h/anim.h); 0x0f0f0002 is a tag like the rows around it.
// ALONE: g_animDefaultC scalar int (AnimClipCore ctor copies it into m_4c, anim.h); unnamed words follow before g_tfbFdDefault0
// GLOBAL: 0x6066fc g_animDefaultC
int g_animDefaultC = 0x0f0f0002;

// GLOBAL: 0x606798 g_tfbFdDefault0
int g_tfbFdDefault0 = 0x0f0f0004;

// GLOBAL: 0x60679c g_tfbFdEngOfs
int g_tfbFdEngOfs = -1;

// GLOBAL: 0x6067a0 g_tfbFdOfs
int g_tfbFdOfs = -1;

// GLOBAL: 0x6067a4 g_tfbFdHalf
float g_tfbFdHalf = 0.5f;

// ALONE: g_tfbFdMin one byte, read as unsigned char (tfbatomic_plugin.cpp IsVisible); the 3 zero bytes behind it are the alignment of g_tfbFdMin_tail
// GLOBAL: 0x6067a8 g_tfbFdMin
unsigned char g_tfbFdMin = 128;
// GLOBAL: 0x6067ac g_tfbFdMin_tail
__declspec(align(4)) FdMinTail g_tfbFdMin_tail = {
	{ 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 },
	"FACE_LAYER_FLAGS",
	0x0f0f0001
};

// GLOBAL: 0x6067d4 g_tfbB1Magic
int g_tfbB1Magic = 0xf0f0001;

// GLOBAL: 0x6067d8 g_tfbB1EngOfs
int g_tfbB1EngOfs = -1;

// ALONE: g_tfbB1Ofs scalar int (plugin offset, h_b1.cpp B1(o)); unnamed words follow before g_tfbMeshMagic
// GLOBAL: 0x6067dc g_tfbB1Ofs
int g_tfbB1Ofs = -1;

// GLOBAL: 0x606830 g_tfbMeshMagic
int g_tfbMeshMagic = 0x0f0f0003;
// GLOBAL: 0x606834 g_tfbMeshMagic_tail
__declspec(align(4)) TagsFace g_tfbMeshMagic_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS" };

// GLOBAL: 0x606858 g_tfbGeomMagicLo
unsigned g_tfbGeomMagicLo = 0xf0f0002;

// GLOBAL: 0x60685c g_tfbGeomMagic
unsigned g_tfbGeomMagic = 0xf0f0008;

// GLOBAL: 0x606860 g_tfbGeomEngOfs
int g_tfbGeomEngOfs = -1;

// GLOBAL: 0x606864 g_tfbGeomOfs
int g_tfbGeomOfs = -1;
// GLOBAL: 0x606868 g_tfbGeomOfs_tail
__declspec(align(4)) TagsFaceId g_tfbGeomOfs_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS", 0x0f0f0002 };

// GLOBAL: 0x606890 g_tfbLightMagic
int g_tfbLightMagic = 0xf0f0002;

// GLOBAL: 0x606894 g_tfbLightEngOfs
int g_tfbLightEngOfs = -1;

// GLOBAL: 0x606898 g_tfbLightOfs
int g_tfbLightOfs = -1;
// GLOBAL: 0x60689c g_tfbLightOfs_tail
__declspec(align(4)) TagsFace g_tfbLightOfs_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS" };

// GLOBAL: 0x6068c0 g_tfbMatMagicLo
unsigned g_tfbMatMagicLo = 0xf0f0002;

// GLOBAL: 0x6068c4 g_tfbMatMagic
unsigned g_tfbMatMagic = 0xf0f000d;

// GLOBAL: 0x6068c8 g_tfbMatEngOfs
int g_tfbMatEngOfs = -1;

// GLOBAL: 0x6068cc g_tfbMatOfs
int g_tfbMatOfs = -1;

// GLOBAL: 0x6068d0 g_secFadeStart
float g_secFadeStart = 3600.0f;

// GLOBAL: 0x6068d4 g_secFadeRange
float g_secFadeRange = 200.0f;

// Default env-map coefficient; TFBMaterial::UpdateAnimation (tfb_plugins2.cpp) overwrites it per material (0.8 or 0.3).
// ALONE: g_tfbEnvScale scalar float (tfb_plugins2.cpp); unnamed words follow before g_tfbWorldMagic
// GLOBAL: 0x6068d8 g_tfbEnvScale
float g_tfbEnvScale = 0.8f;

// GLOBAL: 0x6069b4 g_tfbWorldMagic
int g_tfbWorldMagic = 0x0f0f0003;

// GLOBAL: 0x6069b8 g_tfbWorldEngOfs
int g_tfbWorldEngOfs = -1;

// GLOBAL: 0x6069bc g_tfbWorldOfs
__declspec(align(4)) int g_tfbWorldOfs = -1;
// GLOBAL: 0x6069c0 g_tfbWorldOfs_tail
__declspec(align(4)) WorldTail g_tfbWorldOfs_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS", { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS", 0x0f0f0002 };

// GLOBAL: 0x606a0c g_tfbSectorMagic
int g_tfbSectorMagic = 0xf0f0003;

// GLOBAL: 0x606a10 g_tfbSectorEngOfs
int g_tfbSectorEngOfs = -1;

// GLOBAL: 0x606a14 g_tfbSectorOfs
int g_tfbSectorOfs = -1;
// GLOBAL: 0x606a18 g_tfbSectorOfs_tail
__declspec(align(4)) TagsIds8 g_tfbSectorOfs_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// GLOBAL: 0x606a38 g_currentLevel
int g_currentLevel = -1;
// GLOBAL: 0x606a3c g_currentLevel_tail
__declspec(align(4)) unsigned g_currentLevel_tail = 0;

// GLOBAL: 0x606a40 g_LevelTable
// Normal build: the title row (index 15, LEVEL_COUNT = 16) is part of the table, since a separate object behind it only stays adjacent
// with 4-byte alignment (x64 aligns rows to 8 and row 15 read garbage).
LevelTableRow g_LevelTable[16] = {
	{ "kingofny.stream", "kingofny.png", 9.0f,  56.0f, 397.0f },
	{ "escape.stream",   "escape.png",   8.0f,  54.0f, 403.0f },
	{ "street.stream",   "street.png",   9.0f,  68.0f,  95.0f },
	{ "mutiny.stream",   "mutiny.png",   7.0f, 235.0f, 405.0f },
	{ "jungle.stream",   "jungle.png",   8.0f, 289.0f, 178.0f },
	{ "rave.stream",     "rave.png",     3.0f,  90.0f,  92.0f },
	{ "lemurs.stream",   "lemurs.png",   8.0f, 107.0f, 157.0f },
	{ "banquet.stream",  "banquet.png",  9.0f, 323.0f, 339.0f },
	{ "coming.stream",   "coming.png",   9.0f, 312.0f, 395.0f },
	{ "beach.stream",    "beach.png",    8.0f, 341.0f, 410.0f },
	{ "rescue.stream",   "rescue.png",   7.0f,  91.0f,  94.0f },
	{ "battle.stream",   "battle.png",   8.0f, 196.0f, 389.0f },
	{ "shuffle.stream",  "shuffle.png",  4.0f, 206.0f, 335.0f },
	{ "golf.stream",     "golf.png",     5.0f, 198.0f, 182.0f },
	{ "map.stream",      "map.png",      6.0f, 228.0f, 386.0f },
	{ "Title.stream", 0, 0.0f, 0.0f, 0.0f },
};

// Row 15 (the title) right behind: the tree declares g_LevelTable[16], reads row 15 (big_level_lm2.cpp) and its first field as
// `const char *g_levelFileName` (big_level_lm1.cpp). Defined after the table to stay adjacent; the other spelling links to it by
// address (link_skeleton /ALTERNATENAME). Without it the port never left the title.
// GLOBAL: 0x606b6c g_levelFileName


// GLOBAL: 0x606b80 g_loadTexA
__declspec(align(4)) LoadScreenTex g_loadTexA[8] = {
		{ "Images\\high_leaf", 0 },
		{ "Images\\high_leaf", 0 },
		{ "Images\\high_leaf", 0 },
		{ "Images\\high_leaf", 0 },
		{ "Images\\flower1", 0 },
		{ "Images\\flower2", 0 },
		{ "Images\\flower2", 0 },
		{ "Images\\flower3", 0 },
	};

// GLOBAL: 0x606bc0 g_loadTexB
__declspec(align(4)) LoadScreenTex g_loadTexB[8] = {
		{ "Images\\low_leaf", 0 },
		{ "Images\\low_leaf", 0 },
		{ "Images\\low_leaf", 0 },
		{ "Images\\low_leaf", 0 },
		{ "Images\\flower1", 0 },
		{ "Images\\flower2", 0 },
		{ "Images\\flower3", 0 },
		{ "Images\\flower3", 0 },
	};

// GLOBAL: 0x606c00 g_loadSpriteDy
float g_loadSpriteDy[15] = { -2.0f, -3.0f, -3.0f, -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, 0.0f };

// GLOBAL: 0x606c3c RWS::MainLoop::Logic::Rate
namespace RWS { namespace MainLoop { namespace Logic { unsigned Rate = 60; } } }


// GLOBAL: 0x606c44 g_errTimer
int g_errTimer = -1;
// GLOBAL: 0x606c48 g_errTimer_tail
__declspec(align(4)) TagsIds8 g_errTimer_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 } };

// ALONE: DAT_00606c68 read as one int only (g_misc: `return DAT_00606c68`); g_defMemFuncs after it is reached by its own name
// ALONE: DAT_00606c68_tail unnamed tag words behind the int, never read through this object; g_defMemFuncs after it is reached by its own name
// GLOBAL: 0x606c6c DAT_00606c68_tail
__declspec(align(4)) TagsFace DAT_00606c68_tail = { { 0x0f0f0001, 0x0f0f0005, 0x0f0f0001, 0x0f0f0001, 0x0f0f0001, 0x0f0f0002, 0x0f0f0001, 0x0f0f0002 }, "FACE_LAYER_FLAGS" };

// ALONE: g_detailedShadows one byte flag (config.cpp: `!= 0`; read as bool); the 3 bytes behind it are the alignment of TFBStreamPump
// GLOBAL: 0x606cc4 g_detailedShadows
bool g_detailedShadows = true;

// TFBStreamPump 0x606cc8: called as (*TFBStreamPump)(0) by CStreamHandler; the retail value is the empty ret-4 stub. The 1368 B behind
// it (floats, string pointers, the stream/file format string pool) are left in the image.
// GLOBAL: 0x606cc8 TFBStreamPump
void (__stdcall *TFBStreamPump)(int) = FUN_00410380;

