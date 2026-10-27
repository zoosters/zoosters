#ifndef ZOOSTERS_TFB_RENDER_ATOMICFLAGS_VIEWS_H
#define ZOOSTERS_TFB_RENDER_ATOMICFLAGS_VIEWS_H

#include "rw/rwviewbase.h"

struct RwRaster;

// An RwTexture seen as the game's code reaches its name: the base-class conversion reproduces the null check on the name address that the game's comparator makes. The pad is the texture's raster, dictionary and dictionary link.
struct TexturePadBase {
	RwRaster *m_raster;
	void *m_dict;
	RwLLLink m_inDict;
};
struct TextureNameBase {
	char m_name[1];
};
struct TextureNameView : TexturePadBase, TextureNameBase {};

// RwCamera's raster members (frameBuffer, zBuffer).
struct RwCameraBuffersView {
	RwObjectHasFrameView m_object;
	RwInt32 m_projection;
	void *m_beginUpdate;
	void *m_endUpdate;
	RwMatrix m_viewMatrix;
	RwRaster *m_frameBuffer;
	RwRaster *m_zBuffer;
};

#endif
