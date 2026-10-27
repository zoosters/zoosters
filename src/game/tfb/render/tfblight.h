// TFB light plugin 0x800000C2 ("TFBLIGHT"): the world level a light belongs to.
#ifndef ZOOSTERS_TFB_RENDER_TFBLIGHT_H
#define ZOOSTERS_TFB_RENDER_TFBLIGHT_H

#include "rw/rpworld.h"
#include "tfb/render/tfbplugins.h"

struct TFBLightData {
	int m_version; // g_tfbLightMagic
	int m_unk04;   // zeroed by the constructor, copied by the copy callback; nobody reads it
	int m_level;   // world level: 3 = the active world, else the opaque world (CLevel::AddLight)
};
ZOOSTERS_ASSERT_SIZE(TFBLightData, 0xc);

extern int g_tfbLightEngOfs;  // engine plugin offset (holds "TFBLIGHT")
extern int g_tfbLightOfs;  // object plugin offset inside an RpLight (<= 0: not registered)
extern int g_tfbLightMagic;

static inline TFBLightData *TFBLightGetData(RpLight *light)
{
	return (TFBLightData *)((char *)light + g_tfbLightOfs);
}

void TFBPlugin_C2_SetLevel(RpLight *light, int level);
int TFBPlugin_C2_GetLevel(RpLight *light);
RwBool TFBPlugin_C2_Attach();

#endif
