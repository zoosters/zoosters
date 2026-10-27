#ifndef ZOOSTERS_TFB_RENDER_CAMERA_STUB_H
#define ZOOSTERS_TFB_RENDER_CAMERA_STUB_H

#include "rw/rwviewbase.h"

// The frame buffer raster and the camera members the full-screen quads read.
struct RasterCA { void *m_parent; void *m_pixels; void *m_palette; int m_width; int m_height; };
struct RwCameraStub {
	RwObjectHasFrameView m_object;
	RwInt32 m_projection;
	void *m_beginUpdate;
	void *m_endUpdate;
	RwMatrix m_viewMatrix;
	RasterCA *m_frameBuffer;
	void *m_zBuffer;
	RwV2d m_viewWindow;
	RwV2d m_recipViewWindow;
	RwV2d m_viewOffset;
	float m_nearPlane;
};

#endif
