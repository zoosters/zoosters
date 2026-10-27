// CollisionBox::FindBoxContact: first contact of two moving oriented boxes. TFB's port of Eberly's FindObbObb (bx3bx3fi.cpp,
// "Dynamic Collision Detection using Oriented Bounding Boxes", section 2) with RenderWare types.
#include "tfb/collision/collisionbox_contact.h"
#include <math.h>

// The latest start of overlap found so far over the candidate axes (Eberly: T, type, side).
struct ObbSweep {
	RwReal time; // fraction of the step at which the projections start to overlap on every axis tested so far
	int axis;    // the axis that gave `time` (OBB_AXIS_*)
	int side;    // +1: B approaches from the positive side of that axis, -1: from the negative side
};

// out = p + t * q (Eberly's COMBO)
static void Combo(RwV3d *out, const RwV3d *p, RwReal t, const RwV3d *q)
{
	out->x = p->x + t * q->x;
	out->y = p->y + t * q->y;
	out->z = p->z + t * q->z;
}

// One candidate axis over the step (Eberly's FIND0 / FIND1, times as fractions of the step). p0 / p1: projected distance of the
// centres at the start / end of the step; r: sum of the projected radii. Returns true if the axis separates the boxes during the
// whole step; else, when they are apart on it at the start, records the time at which they meet.
static bool SeparatedDuringStep(ObbSweep *sweep, RwReal p0, RwReal p1, RwReal r, int axis)
{
	RwReal gap = r - p0;
	if (gap < 0.0f) { // apart on the positive side
		if (p1 - r > 0.0f)
			return true;
		RwReal time = gap / (p1 - p0);
		if (time > sweep->time) {
			sweep->time = time;
			sweep->axis = axis;
			sweep->side = 1;
		}
		return false;
	}
	gap = -r - p0;
	if (gap > 0.0f) { // apart on the negative side
		if (p1 + r < 0.0f)
			return true;
		RwReal time = gap / (p1 - p0);
		if (time > sweep->time) {
			sweep->time = time;
			sweep->axis = axis;
			sweep->side = -1;
		}
	}
	return false;
}

// Swept test of box a (velocity va) against box b (velocity vb) over the step [0, t].
// Returns the separating axis (OBB_AXIS_*) if the boxes stay apart during the step. Else returns OBB_AXIS_NONE; *tOut (optional)
// receives the fraction of the step at which they first touch (0 if they overlap at the start) and, when `point` is given, *point
// the contact point and *normal (optional) the contact normal, oriented against the relative velocity.
// FUNCTION: 0x451480 CollisionBox::FindBoxContact
int CollisionBox::FindBoxContact(RwReal t, CollisionBox *a, const RwV3d *va, CollisionBox *b, const RwV3d *vb, RwReal *tOut,
                                 RwV3d *point, RwV3d *normal)
{
	const RwV3d *A[3] = { a->GetAxis(0), a->GetAxis(1), a->GetAxis(2) };
	const RwV3d *B[3] = { b->GetAxis(0), b->GetAxis(1), b->GetAxis(2) };
	const RwReal *extA = a->m_extent;
	const RwReal *extB = b->m_extent;

	// Box a stays put: W = velocity of b relative to a, D0 / D1 = centre difference at the start / end of the step.
	RwV3d W, D0, D1;
	RwV3dSub(&W, vb, va);
	RwV3dSub(&D0, b->GetCentre(), a->GetCentre());
	Combo(&D1, &D0, t, &W);

	RwReal AB[3][3]; // AB[i][j] = A_i . B_j
	RwReal absAB[3][3];
	RwReal AD0[3], AD1[3]; // A_i . D0, A_i . D1
	RwReal BD0[3], BD1[3]; // B_j . D0, B_j . D1
	ObbSweep sweep = { 0.0f, OBB_AXIS_NONE, 0 };

	// The face normals of a.
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			AB[i][j] = RwV3dDotProduct(A[i], B[j]);
			absAB[i][j] = fabsf(AB[i][j]);
		}
		AD0[i] = RwV3dDotProduct(A[i], &D0);
		AD1[i] = RwV3dDotProduct(A[i], &D1);
		RwReal r = extA[i] + ProjectedExtent(extB, absAB[i][0], absAB[i][1], absAB[i][2]);
		if (SeparatedDuringStep(&sweep, AD0[i], AD1[i], r, OBB_AXIS_A0 + i))
			return OBB_AXIS_A0 + i;
	}

	// The face normals of b.
	for (int j = 0; j < 3; ++j) {
		BD0[j] = RwV3dDotProduct(B[j], &D0);
		BD1[j] = RwV3dDotProduct(B[j], &D1);
		RwReal r = ProjectedExtent(extA, absAB[0][j], absAB[1][j], absAB[2][j]) + extB[j];
		if (SeparatedDuringStep(&sweep, BD0[j], BD1[j], r, OBB_AXIS_B0 + j))
			return OBB_AXIS_B0 + j;
	}

	// The edge directions A_i x B_j.
	for (int i = 0; i < 3; ++i) {
		int i1 = (i + 1) % 3, i2 = (i + 2) % 3;
		for (int j = 0; j < 3; ++j) {
			int j1 = (j + 1) % 3, j2 = (j + 2) % 3;
			RwReal rB = extB[j1] * absAB[i][j2] + extB[j2] * absAB[i][j1];
			RwReal rA = extA[i1] * absAB[i2][j] + extA[i2] * absAB[i1][j];
			RwReal p0 = AD0[i2] * AB[i1][j] - AB[i2][j] * AD0[i1];
			RwReal p1 = AD1[i2] * AB[i1][j] - AB[i2][j] * AD1[i1];
			if (SeparatedDuringStep(&sweep, p0, p1, rB + rA, OBB_AXIS_A0xB0 + 3 * i + j))
				return OBB_AXIS_A0xB0 + 3 * i + j;
		}
	}

	// Seen along the relative velocity the boxes move on a line: the axes W x A_k and W x B_k separate them for the whole step
	// when the projected centre distance exceeds the projected radii by more than |W| / 10 (TFB's tolerance; Eberly tests > 0).
	if (W.x != 0.0f || W.y != 0.0f || W.z != 0.0f) {
		RwV3d WxD0;
		RwV3dCrossProduct(&WxD0, &W, &D0);
		RwReal threshold = (RwReal)sqrt(RwV3dDotProduct(&W, &W)) * 0.1f;

		RwReal WA[3], absWA[3];
		for (int k = 0; k < 3; ++k) {
			WA[k] = RwV3dDotProduct(&W, A[k]);
			absWA[k] = fabsf(WA[k]);
		}
		for (int k = 0; k < 3; ++k) {
			int k1 = (k + 1) % 3, k2 = (k + 2) % 3;
			RwReal distance = fabsf(RwV3dDotProduct(&WxD0, A[k]));
			RwReal rB =
			    ProjectedExtent(extB, fabsf(WA[k2] * AB[k1][0] - WA[k1] * AB[k2][0]),
				                fabsf(WA[k2] * AB[k1][1] - WA[k1] * AB[k2][1]), fabsf(WA[k2] * AB[k1][2] - WA[k1] * AB[k2][2]));
			RwReal rA = extA[k1] * absWA[k2] + extA[k2] * absWA[k1];
			if (distance - (rB + rA) > threshold)
				return OBB_AXIS_WxA0 + k;
		}

		RwReal WB[3], absWB[3];
		for (int k = 0; k < 3; ++k) {
			WB[k] = RwV3dDotProduct(&W, B[k]);
			absWB[k] = fabsf(WB[k]);
		}
		for (int k = 0; k < 3; ++k) {
			int k1 = (k + 1) % 3, k2 = (k + 2) % 3;
			RwReal distance = fabsf(RwV3dDotProduct(&WxD0, B[k]));
			RwReal rA =
			    ProjectedExtent(extA, fabsf(WB[k2] * AB[0][k1] - WB[k1] * AB[0][k2]),
				                fabsf(WB[k2] * AB[1][k1] - WB[k1] * AB[1][k2]), fabsf(WB[k2] * AB[2][k1] - WB[k1] * AB[2][k2]));
			RwReal rB = extB[k1] * absWB[k2] + extB[k2] * absWB[k1];
			if (distance - (rA + rB) > threshold)
				return OBB_AXIS_WxB0 + k;
		}
	}

	// They touch. The contact point comes from the last axis to start overlapping (Eberly section 2.3.2, Table 2).
	if (point) {
		RwReal x[3], y[3]; // contact coordinates along the axes of a / of b
		int side = sweep.side;
		if (sweep.axis >= OBB_AXIS_A0 && sweep.axis <= OBB_AXIS_A2) {
			int i = sweep.axis - OBB_AXIS_A0;
			for (int j = 0; j < 3; ++j)
				y[j] = ContactCoordinate(AB[i][j], extB[j], -side);
			ContactPoint(b, vb, y, sweep.time, point);
			if (normal)
				*normal = *A[i];
		} else if (sweep.axis >= OBB_AXIS_B0 && sweep.axis <= OBB_AXIS_B2) {
			int j = sweep.axis - OBB_AXIS_B0;
			for (int i = 0; i < 3; ++i)
				x[i] = ContactCoordinate(AB[i][j], extA[i], side);
			ContactPoint(a, va, x, sweep.time, point);
			if (normal)
				*normal = *B[j];
		} else if (sweep.axis >= OBB_AXIS_A0xB0 && sweep.axis <= OBB_AXIS_A2xB2) {
			int i = (sweep.axis - OBB_AXIS_A0xB0) / 3, j = (sweep.axis - OBB_AXIS_A0xB0) % 3;
			int i1 = (i + 1) % 3, i2 = (i + 2) % 3, j1 = (j + 1) % 3, j2 = (j + 2) % 3;
			x[i1] = ContactCoordinate(AB[i2][j], extA[i1], -side);
			x[i2] = ContactCoordinate(AB[i1][j], extA[i2], side);
			y[j1] = ContactCoordinate(AB[i][j2], extB[j1], -side);
			y[j2] = ContactCoordinate(AB[i][j1], extB[j2], side);
			// the closest points of the two edge lines: solve for the coordinate along A_i
			RwReal div = 1.0f - AB[i][j] * AB[i][j];
			if (fabsf(div) > 1e-6f) {
				RwV3d D;
				Combo(&D, &D0, sweep.time, &W);
				RwReal ad = RwV3dDotProduct(&D, A[i]);
				RwReal bd = RwV3dDotProduct(&D, B[j]);
				RwReal tmp = AB[i1][j] * x[i1] + AB[i2][j] * x[i2] - bd;
				x[i] = (ad + AB[i][j] * tmp + AB[i][j1] * y[j1] + AB[i][j2] * y[j2]) / div;
			} else {
				x[i] = 0.0f;
			}
			ContactPoint(a, va, x, sweep.time, point);
			if (normal)
				RwV3dCrossProduct(normal, A[i], B[j]);
		} else if (normal) {
			RwV3dNegate(normal, &D0); // overlapping at the start: from b towards a
		}
		if (normal && sweep.axis != OBB_AXIS_NONE) {
			if (RwV3dDotProduct(&W, normal) > 0.0f)
				RwV3dNegate(normal, normal);
		}
	}
	if (tOut)
		*tOut = sweep.time;
	return OBB_AXIS_NONE;
}
