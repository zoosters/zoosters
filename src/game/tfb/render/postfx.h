// Post-processing state of the level (pixel fog, depth of field, bloom), set by the ScriptFog members and the world's attributes.
#pragma once

extern unsigned char g_pixelFogEnable;         // 0x617d5c
extern float g_pixelFogDist;
extern int g_pixelCutoff;
extern unsigned char g_pixelFogColor[4];  // r, g, b, a
extern int g_dofBluriness;
extern unsigned char g_bloomEnable;
extern float g_bloomAddFactor;
extern float g_bloomHaloSize;

void SetPixelFogDistance(float distance);
int SetDofBluriness(int bluriness);

#include "rw/rwcore.h"
char UpdatePixelFogCutoff(RwCamera *camera);
int UpdatePixelFogCutoffAndRange(RwCamera *camera, RwRaster *raster);
void DrawBloomIfEnabled(RwCamera *camera);
void DrawPixelFogQuad(RwCamera *camera);
