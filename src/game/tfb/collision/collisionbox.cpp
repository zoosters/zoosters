// Point and segment against a box: Eberly's line-box separating axes, then slab clipping for the entry point.
#include "tfb/collision/collisionbox.h"
#include <math.h>

// Point (dir == 0) or segment start .. start + dir. entryTime (optional): the fraction where the segment enters, 0 if it
// starts inside. Returns a CollisionBoxSegmentResult.
// 0x458e70
int CollisionBox::TestSegment(const RwV3d *start, const RwV3d *dir, RwReal *entryTime) const
{
	RwV3d diff;
	RwV3dSub(&diff, start, GetCentre());

	if (dir == 0) {
		for (int i = 0; i < 3; ++i) {
			if (fabsf(RwV3dDotProduct(&diff, GetAxis(i))) > m_extent[i])
				return COLLISIONBOX_MISS;
		}
		if (entryTime)
			*entryTime = 0.0f;
		return COLLISIONBOX_INSIDE;
	}

	// separating axes dir x A_i
	RwReal dirDot[3];
	RwReal absDirDot[3];
	for (int i = 0; i < 3; ++i) {
		dirDot[i] = RwV3dDotProduct(dir, GetAxis(i));
		absDirDot[i] = fabsf(dirDot[i]);
	}
	RwV3d dirCrossDiff;
	RwV3dCrossProduct(&dirCrossDiff, dir, &diff);
	if (fabsf(RwV3dDotProduct(&dirCrossDiff, GetAxis(0))) > m_extent[1] * absDirDot[2] + m_extent[2] * absDirDot[1])
		return COLLISIONBOX_MISS;
	if (fabsf(RwV3dDotProduct(&dirCrossDiff, GetAxis(1))) > m_extent[0] * absDirDot[2] + m_extent[2] * absDirDot[0])
		return COLLISIONBOX_MISS;
	if (fabsf(RwV3dDotProduct(&dirCrossDiff, GetAxis(2))) > m_extent[0] * absDirDot[1] + m_extent[1] * absDirDot[0])
		return COLLISIONBOX_MISS;

	// Clip against the three slabs. startDist / endDist: signed distance to the face to cross; startDist 0 inside the slab.
	RwReal startDist[3];
	RwReal endDist[3];
	for (int i = 0; i < 3; ++i) {
		RwReal s = RwV3dDotProduct(&diff, GetAxis(i));
		if (fabsf(s) <= m_extent[i]) {
			startDist[i] = 0.0f;
			endDist[i] = dirDot[i];
			continue;
		}
		if (s * dirDot[i] >= 0.0f)
			return COLLISIONBOX_MOVING_AWAY;
		RwReal end = s + dirDot[i];
		if (s < 0.0f) {
			end = end + m_extent[i];
			if (end < 0.0f)
				return COLLISIONBOX_FALLS_SHORT;
			s = s + m_extent[i];
		} else {
			end = end - m_extent[i];
			if (end > 0.0f)
				return COLLISIONBOX_FALLS_SHORT;
			s = s - m_extent[i];
		}
		startDist[i] = s;
		endDist[i] = end;
	}
	if (entryTime == 0)
		return COLLISIONBOX_INSIDE;

	// entry = the last face crossed
	int result = COLLISIONBOX_INSIDE;
	*entryTime = 0.0f;
	for (int i = 0; i < 3; ++i) {
		if (startDist[i] != 0.0f) {
			RwReal t = startDist[i] / (startDist[i] - endDist[i]);
			if (t > *entryTime) {
				*entryTime = t;
				result = COLLISIONBOX_ENTERS + (startDist[i] < 0.0f ? 3 + i : i);
			}
		}
	}
	return result;
}
