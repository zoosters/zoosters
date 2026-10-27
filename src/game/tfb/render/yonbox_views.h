#ifndef ZOOSTERS_TFB_RENDER_YONBOX_VIEWS_H
#define ZOOSTERS_TFB_RENDER_YONBOX_VIEWS_H

#include "rw/rwviewbase.h"

// the render-frame stamp of an atomic (+0x60 of the RpAtomic with 4-byte pointers; the queue marks an atomic once per frame)
struct AtomicStampView {
	RwObjectHasFrameView m_object;
	void *m_repEntry;
	void *m_geometry;
	RwSphereView m_boundingSphere;
	RwSphereView m_worldBoundingSphere;
	void *m_clump;
	RwLLLink m_inClump;
	void *m_renderCallBack;
	char m_interpolator[0x14];
	unsigned short m_renderFrame;
};

#endif
