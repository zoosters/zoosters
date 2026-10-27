// .rdata constants: floats, axes, format strings, TFB default tables. Neighbours share a file in address order; unnamed
// literals between them are tail members (m_tN) of the object before.
#include "rw/rwcore.h"   // RwV3d (src/game)
#include "tfb/render/tfbrenderstate.h"   // TFBRenderState (src/game)

// GLOBAL: 0x5ca5a8 g_floatZero
extern const float g_floatZero = 0.0f;

// GLOBAL: 0x5ca668 g_floatOne
extern const float g_floatOne = 1.0f;

// GLOBAL: 0x5ca758 g_twelveW3a
extern const float g_twelveW3a = 12.0f;

// GLOBAL: 0x5caf58 g_xAxis
extern const RwV3d g_xAxis = { 1.0f, 0.0f, 0.0f };

// ALONE: g_yAxis RwV3d passed by pointer to RwFrameRotate (12 B read); followed by CFXColorLight::vftable
// GLOBAL: 0x5caf64 g_yAxis
extern const RwV3d g_yAxis = { 0.0f, 1.0f, 0.0f };

// GLOBAL: 0x5cb310 g_particleXAxis
extern const RwV3d g_particleXAxis = { 1.0f, 0.0f, 0.0f };

// GLOBAL: 0x5cb31c g_particleYAxis
extern const RwV3d g_particleYAxis = { 0.0f, 1.0f, 0.0f };

// GLOBAL: 0x5cb328 g_particleZAxis
extern const RwV3d g_particleZAxis = { 0.0f, 0.0f, 1.0f };

// ALONE: g_fmtD format string, only read through a pointer up to its NUL
// GLOBAL: 0x5cc994 g_fmtD
extern const char g_fmtD[4] = "%d";

// ALONE: g_randScale scalar float (1/32767, scales rand() to [0, 1]); followed by unnamed literals
// GLOBAL: 0x5cc9ac g_randScale
extern const float g_randScale = 3.0518509e-05f;

// ALONE: g_millisToSeconds scalar float (script time in ms -> s); followed by unnamed literals
// GLOBAL: 0x5cc9c8 g_millisToSeconds
extern const float g_millisToSeconds = 0.001f;

// ALONE: g_propRotateAxis float[3] copied as one RwV3d (prop_ctor_418); followed by a vtable
// GLOBAL: 0x5ccb80 g_propRotateAxis
extern const RwV3d g_propRotateAxis = { 0.0f, 1.0f, 0.0f };

// GLOBAL: 0x5cd030 g_camAxisX
extern const RwV3d g_camAxisX = { 1.0f, 0.0f, 0.0f };

// GLOBAL: 0x5cd03c g_camAxisY
extern const RwV3d g_camAxisY = { 0.0f, 1.0f, 0.0f };

// ALONE: g_camAxisZ RwV3d constant, read as 3 floats
// GLOBAL: 0x5cd048 g_camAxisZ
extern const RwV3d g_camAxisZ = { 0.0f, 0.0f, 1.0f };

// GLOBAL: 0x5cd114 g_shadowCamAxisX
extern const RwV3d g_shadowCamAxisX = { 1.0f, 0.0f, 0.0f };

// ALONE: g_shadowCamAxisY RwV3dS passed to RwFrameRotate (12 B read); followed by a vtable
// GLOBAL: 0x5cd120 g_shadowCamAxisY
extern const RwV3d g_shadowCamAxisY = { 0.0f, 1.0f, 0.0f };

// ALONE: g_corpusYAxis RwV3d passed to RwFrameRotate / RotationFromTo (12 B read); followed by a vtable
// GLOBAL: 0x5cd160 g_corpusYAxis
extern const RwV3d g_corpusYAxis = { 0.0f, 1.0f, 0.0f };

// GLOBAL: 0x5cd2a4 g_aimAxisB
extern const RwV3d g_aimAxisB = { 1.0f, 0.0f, 0.0f };

// ALONE: g_aimAxisA RwV3d passed to RwMatrixRotate (12 B read); followed by a vtable
// GLOBAL: 0x5cd2b0 g_aimAxisA
extern const RwV3d g_aimAxisA = { 0.0f, 1.0f, 0.0f };

// ALONE: g_negEpsilon scalar float
// GLOBAL: 0x5d4d0c g_negEpsilon
extern const float g_negEpsilon = -0.0001f;

// GLOBAL: 0x5d4d70 DAT_005d4d70
extern const RwV3d DAT_005d4d70 = { 1.0f, 0.0f, 0.0f };

// GLOBAL: 0x5d4d7c DAT_005d4d7c
extern const RwV3d DAT_005d4d7c = { 0.0f, 1.0f, 0.0f };

// ALONE: g_tfbFdDefault1 scalar int (h_fd.cpp d->m_color = g_tfbFdDefault1); unnamed pooled literals (floats, strings, g_strTfbScript pointer) follow
// GLOBAL: 0x5d579c g_tfbFdDefault1
extern const int g_tfbFdDefault1 = -1;

// Default render state of a TFB material, copied whole into materials without their own. The "TFB Material Plugin" name follows.
// ALONE: g_tfbMatDefault 20 B block copied whole (TFBFive2); the "TFB Material Plugin" literal behind it is never read through it
// GLOBAL: 0x5d57f4 g_tfbMatDefault
extern const TFBRenderState g_tfbMatDefault = { 5, 6, 2, 5, 0 };
