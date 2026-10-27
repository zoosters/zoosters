// Occluder planes of the far-visibility test: quads with their own plane and four side planes through the eye, built each frame and queried for boxes and spheres.
#pragma once

struct OccluderPlane {
	float x, y, z, d;   // unit normal and distance: dot(normal, p) - d > 0 is in front
};

struct OccluderNode {
	OccluderNode *m_next;
	float x, y, z, d;   // the quad's plane
	OccluderPlane s[4]; // the four planes through the eye and the quad's edges
	float *m_box;       // the quad's 4 corners (Vec3 x 4)
};

extern OccluderNode *g_planeList413;

#include "rw/rwcore.h"
struct CamB;
void BuildOccluderPlanes(CamB *camera, float *records, unsigned count4, OccluderNode *buffer);
struct SphereP413;
int OccluderTestSphere(RwCamera *camera, SphereP413 *sphere, char enable);  // (cdecl)
int OccluderTestBox(RwCamera *camera, const RwBBox *box, char enable);  // (cdecl)
