// TFB clump plugin 0x800000B1 ("TFBCLUMP"): flags and render frame of a clump, and the clump render.
#ifndef ZOOSTERS_TFB_RENDER_TFBCLUMP_H
#define ZOOSTERS_TFB_RENDER_TFBCLUMP_H

#include "rw/rpworld.h"
#include "tfb/render/tfbplugins.h"

struct TFBClumpData {
	int m_version;       // g_tfbB1Magic (the stream format id)
	unsigned m_flags;    // set by TFBPlugin_B1_SetFlagBits, copied by the copy callback
	int m_renderFrame;   // render frame stamp (callers store RWS::MainLoop::Render::GetFrameCount)
};
ZOOSTERS_ASSERT_SIZE(TFBClumpData, 0xc);

extern int g_tfbB1EngOfs;  // engine plugin offset (holds "TFBCLUMP")
extern int g_tfbB1Ofs;  // object plugin offset inside an RpClump (<= 0: not registered)
extern int g_tfbB1Magic;

// The plugin data of a clump (the plugin must be registered).
static inline TFBClumpData *TFBClumpGetData(RpClump *clump)
{
	return (TFBClumpData *)((char *)clump + g_tfbB1Ofs);
}

int TFBPlugin_B1_SetFlagBits(RpClump *clump, unsigned bits);  // flags |= bits; 1 when applied
unsigned TFBPlugin_B1_TestFlags(RpClump *clump, unsigned bits);  // flags & bits
int TFBPlugin_B1_GetRenderFrame(RpClump *clump);
int TFBPlugin_B1_SetRenderFrame(RpClump *clump, int renderFrame);
RwBool TFBPlugin_B1_Attach();

// Renders every atomic of the clump that has the render flag; returns 0 when one callback returned 0, else the clump.
RpClump *TFBClump_Render(RpClump *clump);

#endif
