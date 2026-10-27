// CollisionBox::FindTriangleContact: first contact of a moving oriented box and a triangle. TFB's port of Eberly's FindTriObb
// (tr3bx3fi.cpp of Magic Software, "Dynamic Collision Detection using Oriented Bounding Boxes", section 3), with its own
// sweep-step rules (1e-4 skips), six more separating axes (W x A_k, W x E_j) and the axis codes of the box-box test.
#include "tfb/collision/collisionbox_contact.h"
#include <math.h>

// The latest start of overlap found so far over the candidate axes (Eberly: T, type, extr, side).
struct TriSweep {
	RwReal time; // T: when the projections start to overlap on every axis so far (a time, not a fraction of the step)
	int axis;    // the axis that gave `time` (OBB_AXIS_*)
	int extr;    // the triangle point that first touches the slab: face axes 0 = p0, 1 = p0 + E0, 2 = p0 + E1; edge axes 0 / 1
	int side;    // +1: the triangle approaches from the positive side of the axis, -1: from the negative side
};

// The triangle in the terms of the box (Eberly's convenience variables of FindTriObb). The box is at rest, the triangle moves
// with W.
struct BoxTriangle {
	RwV3d E[3];      // the edges E0 = p1 - p0 and E1 = p2 - p0, and E2 = E1 - E0
	RwV3d N;         // E0 x E1 (not normalised)
	RwV3d D;         // p0 - box centre
	RwV3d W;         // velocity of the triangle relative to the box (-vel)
	RwReal AN[3];    // A_i . N
	RwReal AE[3][3]; // A_i . E_j; the E2 column is AE[i][1] - AE[i][0], not a dot product
};

// One candidate axis, triangle beyond the +R end of the box's slab (gap = R - p < 0, p: its nearest point, speed: its velocity
// along the axis). Returns true if it stays there during the whole step [0, t] (the axis separates); else records the time at
// which it reaches the slab.
// NOTE: the time is gap / speed; Eberly's source has `t * gap / speed`, the game dropped the factor t.
static bool SeparatedAbove(TriSweep *sweep, RwReal t, RwReal gap, RwReal speed, int axis, int extr)
{
	if (speed * t - gap > 0.0f)
		return true;
	RwReal time = gap / speed;
	if (time > sweep->time) {
		sweep->time = time;
		sweep->axis = axis;
		sweep->extr = extr;
		sweep->side = 1;
	}
	return false;
}

// The same beyond the -R end (gap = -R - p > 0).
static bool SeparatedBelow(TriSweep *sweep, RwReal t, RwReal gap, RwReal speed, int axis, int extr)
{
	if (speed * t - gap < 0.0f)
		return true;
	RwReal time = gap / speed;
	if (time > sweep->time) {
		sweep->time = time;
		sweep->axis = axis;
		sweep->extr = extr;
		sweep->side = -1;
	}
	return false;
}

// The triangle's normal as axis (Eberly's FIND0): its three points project onto one point p.
static bool SweepNormalAxis(TriSweep *sweep, RwReal t, RwReal p, RwReal speed, RwReal radius)
{
	RwReal gap = radius - p;
	if (gap < 0.0f)
		return SeparatedAbove(sweep, t, gap, speed, OBB_AXIS_N, 0);
	gap = -radius - p;
	return gap > 0.0f && SeparatedBelow(sweep, t, gap, speed, OBB_AXIS_N, 0);
}

// A face normal of the box as axis (Eberly's FIND1): the triangle projects onto [p + min(0, e0, e1), p + max(0, e0, e1)]; the
// nearest of its points to the slab is p (extr 0), p + e0 (1) or p + e1 (2). Skipped when both edges are (almost) parallel to
// the slab.
static bool SweepFaceAxis(TriSweep *sweep, RwReal t, RwReal p, RwReal e0, RwReal e1, RwReal speed, RwReal radius, int axis)
{
	if (!(fabsf(e0) > kAxisEpsilon || fabsf(e1) > kAxisEpsilon))
		return false;
	RwReal gap = radius - p;
	if (gap < 0.0f) {
		if (e1 <= e0) {
			if (e1 >= 0.0f)
				return SeparatedAbove(sweep, t, gap, speed, axis, 0);
			gap = gap - e1;
			return gap < 0.0f && SeparatedAbove(sweep, t, gap, speed, axis, 2);
		}
		gap = gap - e0;
		return gap < 0.0f && SeparatedAbove(sweep, t, gap, speed, axis, 1);
	}
	gap = -radius - p;
	if (gap > 0.0f) {
		if (e1 >= e0) {
			if (e1 <= 0.0f)
				return SeparatedBelow(sweep, t, gap, speed, axis, 0);
			gap = gap - e1;
			return gap > 0.0f && SeparatedBelow(sweep, t, gap, speed, axis, 2);
		}
		gap = gap - e0;
		return gap > 0.0f && SeparatedBelow(sweep, t, gap, speed, axis, 1);
	}
	return false;
}

// An edge axis A_i x E_j as axis (Eberly's FIND2): the triangle projects onto two points, p and p + m (m = +-N . A_i, the offset
// of the other edge). Skipped when |m| is within the epsilon.
static bool SweepEdgeAxis(TriSweep *sweep, RwReal t, RwReal p, RwReal m, RwReal speed, RwReal radius, int axis)
{
	RwReal gap = radius - p;
	if (gap < 0.0f) {
		if (m > kAxisEpsilon)
			return SeparatedAbove(sweep, t, gap, speed, axis, 0);
		if (m < -kAxisEpsilon) {
			gap = gap - m;
			return gap < 0.0f && SeparatedAbove(sweep, t, gap, speed, axis, 1);
		}
		return false;
	}
	gap = -radius - p;
	if (gap > 0.0f) {
		if (m < -kAxisEpsilon)
			return SeparatedBelow(sweep, t, gap, speed, axis, 0);
		if (m > kAxisEpsilon) {
			gap = gap - m;
			return gap > 0.0f && SeparatedBelow(sweep, t, gap, speed, axis, 1);
		}
	}
	return false;
}

// Fills `bt` for the box and the triangle `tri` moving with -vel relative to it.
static void MakeBoxTriangle(BoxTriangle *bt, const CollisionBox *box, const RwV3d *const *tri, const RwV3d *vel)
{
	RwV3dSub(&bt->E[0], tri[1], tri[0]);
	RwV3dSub(&bt->E[1], tri[2], tri[0]);
	RwV3dSub(&bt->E[2], &bt->E[1], &bt->E[0]);
	RwV3dCrossProduct(&bt->N, &bt->E[0], &bt->E[1]);
	RwV3dSub(&bt->D, tri[0], box->GetCentre());
	RwV3dNegate(&bt->W, vel);
	for (int i = 0; i < 3; ++i) {
		const RwV3d *axis = box->GetAxis(i);
		bt->AN[i] = RwV3dDotProduct(&bt->N, axis);
		bt->AE[i][0] = RwV3dDotProduct(&bt->E[0], axis);
		bt->AE[i][1] = RwV3dDotProduct(&bt->E[1], axis);
		bt->AE[i][2] = bt->AE[i][1] - bt->AE[i][0];
	}
}

// The swept axes in Eberly's order: the triangle's normal, the box's face normals, the edge pairs A_i x E_j. Returns the axis
// that separates box and triangle for the whole step, else OBB_AXIS_NONE (`sweep` then holds the latest start of overlap).
static int SweepAxes(const CollisionBox *box, RwReal t, const BoxTriangle *bt, TriSweep *sweep)
{
	const RwReal *ext = box->m_extent;
	const RwV3d *A[3] = { box->GetAxis(0), box->GetAxis(1), box->GetAxis(2) };

	RwReal radius = ProjectedExtent(ext, fabsf(bt->AN[0]), fabsf(bt->AN[1]), fabsf(bt->AN[2]));
	if (SweepNormalAxis(sweep, t, RwV3dDotProduct(&bt->N, &bt->D), RwV3dDotProduct(&bt->N, &bt->W), radius))
		return OBB_AXIS_N;

	for (int i = 0; i < 3; ++i) {
		if (SweepFaceAxis(sweep, t, RwV3dDotProduct(&bt->D, A[i]), bt->AE[i][0], bt->AE[i][1], RwV3dDotProduct(&bt->W, A[i]),
		                  ext[i], OBB_AXIS_A0 + i))
			return OBB_AXIS_A0 + i;
	}

	for (int i = 0; i < 3; ++i) {
		int i1 = (i + 1) % 3, i2 = (i + 2) % 3;
		RwReal pD[3], pW[3]; // (A_i x E_j) . D and . W
		for (int j = 0; j < 3; ++j) {
			if (j < 2) {
				RwV3d axis;
				RwV3dCrossProduct(&axis, A[i], &bt->E[j]);
				pD[j] = RwV3dDotProduct(&axis, &bt->D);
				pW[j] = RwV3dDotProduct(&bt->W, &axis);
			} else { // as Eberly: A_i x E2 is not formed, its projections are the difference of the E1 and E0 ones
				pD[2] = pD[1] - pD[0];
				pW[2] = pW[1] - pW[0];
			}
			radius = ext[i1] * fabsf(bt->AE[i2][j]) + ext[i2] * fabsf(bt->AE[i1][j]);
			RwReal m = j == 0 ? bt->AN[i] : -bt->AN[i];
			if (SweepEdgeAxis(sweep, t, pD[j], m, pW[j], radius, OBB_AXIS_A0xE0 + 3 * i + j))
				return OBB_AXIS_A0xE0 + 3 * i + j;
		}
	}
	return OBB_AXIS_NONE;
}

// An axis with no time term (W x ...): the triangle projects onto [d + min(0, e1, e2), d + max(0, e1, e2)], separated when that
// misses [-R, R]. d: projection of p0, e1 / e2: projections of the edges E0 / E1.
static bool SeparatedStatic(RwReal d, RwReal e1, RwReal e2, RwReal radius)
{
	RwReal g1 = d - radius;
	if (g1 > 0.0f)
		return (e2 <= e1 ? g1 + e2 : g1 + e1) > 0.0f;
	RwReal g2 = d + radius;
	if (g2 < 0.0f)
		return (e2 >= e1 ? g2 + e2 : g2 + e1) < 0.0f;
	return false;
}

// The axes W x A_k and W x E_j (TFB's additions: seen along the relative velocity the box and the triangle move on a line).
// Returns the separating axis or OBB_AXIS_NONE.
static int StaticAxes(const CollisionBox *box, const BoxTriangle *bt)
{
	const RwReal *ext = box->m_extent;
	const RwV3d *A[3] = { box->GetAxis(0), box->GetAxis(1), box->GetAxis(2) };

	for (int k = 0; k < 3; ++k) {
		int k1 = (k + 1) % 3, k2 = (k + 2) % 3;
		RwV3d axis;
		RwV3dCrossProduct(&axis, &bt->W, A[k]);
		RwReal radius = ext[k1] * fabsf(RwV3dDotProduct(&axis, A[k1])) + ext[k2] * fabsf(RwV3dDotProduct(&axis, A[k2]));
		if (SeparatedStatic(RwV3dDotProduct(&bt->D, &axis), RwV3dDotProduct(&bt->E[0], &axis), RwV3dDotProduct(&bt->E[1], &axis),
		                    radius))
			return OBB_AXIS_WxA0 + k;
	}

	for (int j = 0; j < 3; ++j) {
		RwV3d axis;
		RwV3dCrossProduct(&axis, &bt->W, &bt->E[j]);
		// the edge E_j itself projects to exactly 0 on W x E_j
		RwReal e1 = j == 0 ? 0.0f : RwV3dDotProduct(&bt->E[0], &axis);
		RwReal e2 = j == 1 ? 0.0f : RwV3dDotProduct(&bt->E[1], &axis);
		RwReal radius = ProjectedExtent(ext, fabsf(RwV3dDotProduct(&axis, A[0])), fabsf(RwV3dDotProduct(&axis, A[1])),
		                                fabsf(RwV3dDotProduct(&axis, A[2])));
		if (SeparatedStatic(RwV3dDotProduct(&bt->D, &axis), e1, e2, radius))
			return OBB_AXIS_WxE0 + j;
	}
	return OBB_AXIS_NONE;
}

// Contact on an edge axis A_i x E_j (Eberly's cases A_i x E_j of the point choice): the two box coordinates along the other axes
// follow from the signs of the projections, the third from the plane through the contact edge (numer / (N . A_i x E_j)).
static void EdgeContactPoint(const CollisionBox *box, const BoxTriangle *bt, const TriSweep *sweep, const RwV3d *vel,
                             RwV3d *point, RwV3d *normal)
{
	const RwReal *ext = box->m_extent;
	int i = (sweep->axis - OBB_AXIS_A0xE0) / 3, j = (sweep->axis - OBB_AXIS_A0xE0) % 3;
	int i1 = (i + 1) % 3, i2 = (i + 2) % 3;
	RwReal x[3];
	x[i1] = ContactCoordinate(bt->AE[i2][j], ext[i1], -sweep->side);
	x[i2] = ContactCoordinate(bt->AE[i1][j], ext[i2], sweep->side);

	RwV3d D = bt->D; // the triangle's offset at the contact time, then crossed with the edge
	RwV3dIncrementScaled(&D, &bt->W, sweep->time);
	RwV3d DxE, AxE[3];
	RwV3dCrossProduct(&DxE, &D, &bt->E[j]);
	RwReal NdD = RwV3dDotProduct(&bt->N, &DxE);
	RwReal NdAxE[3]; // N . (A_k x E_j)
	for (int k = 0; k < 3; ++k) {
		RwV3dCrossProduct(&AxE[k], box->GetAxis(k), &bt->E[j]);
		NdAxE[k] = RwV3dDotProduct(&bt->N, &AxE[k]);
	}
	RwReal numer = NdD - NdAxE[i1] * x[i1] - NdAxE[i2] * x[i2];
	if (sweep->extr != 0) { // the contact is at the far end of the edge pair: shifted by N . N
		RwReal NdN = RwV3dDotProduct(&bt->N, &bt->N);
		numer = j == 0 ? numer - NdN : numer + NdN;
	}
	x[i] = numer / NdAxE[i];
	ContactPoint(box, vel, x, sweep->time, point);
	if (normal)
		*normal = AxE[i];
}

// The contact point (and normal) of the first touch recorded in `sweep`, written to *point (and *normal if not null).
// NOTE: when no axis gave a time (the triangle overlaps the box at the start) only *normal is written (= N), *point stays
// untouched.
static void FillContact(const CollisionBox *box, const BoxTriangle *bt, const TriSweep *sweep, const RwV3d *const *tri,
                        const RwV3d *vel, RwV3d *point, RwV3d *normal)
{
	RwReal x[3];
	if (sweep->axis == OBB_AXIS_N) {
		for (int i = 0; i < 3; ++i)
			x[i] = ContactCoordinate(bt->AN[i], box->m_extent[i], sweep->side);
		ContactPoint(box, vel, x, sweep->time, point);
		if (normal)
			*normal = bt->N;
	} else if (sweep->axis >= OBB_AXIS_A0 && sweep->axis <= OBB_AXIS_A2) {
		// the triangle's vertex that touches the face: p0, p0 + E0 or p0 + E1, moved by W over the contact time
		*point = *tri[0];
		RwV3dIncrementScaled(point, &bt->W, sweep->time);
		if (sweep->extr != 0)
			RwV3dAdd(point, point, &bt->E[sweep->extr == 1 ? 0 : 1]);
		if (normal)
			*normal = *box->GetAxis(sweep->axis - OBB_AXIS_A0);
	} else if (sweep->axis >= OBB_AXIS_A0xE0 && sweep->axis <= OBB_AXIS_A2xE2) {
		EdgeContactPoint(box, bt, sweep, vel, point, normal);
	} else if (normal) {
		*normal = bt->N;
	}
	// NOTE: the normal is oriented along the triangle's velocity relative to the box (FindBoxContact orients its normal against
	// the relative velocity). It is not normalised for the axes N and A_i x E_j.
	if (normal && sweep->axis != OBB_AXIS_NONE) {
		if (RwV3dDotProduct(&bt->W, normal) < 0.0f)
			RwV3dNegate(normal, normal);
	}
}

// Swept test of this box (velocity vel) against the triangle tri[0..2] (at rest) over the step [0, t].
// Returns the separating axis (OBB_AXIS_*) if they stay apart, else OBB_AXIS_NONE. *tOut (optional) gets the first touch
// time (0 if overlapping at the start); *point / *normal (optional) get the contact point and normal. From the box the
// triangle moves with W = -vel.
// For t < 0 the step test always returns: the axis with the latest overlap start so far, or OBB_AXIS_NONE with nothing
// written (callers presumably never pass t < 0)
// FUNCTION: 0x454880 CollisionBox::FindTriangleContact
int CollisionBox::FindTriangleContact(RwReal t, const RwV3d *const *tri, const RwV3d *vel, RwReal *tOut, RwV3d *point,
                                      RwV3d *normal)
{
	BoxTriangle bt;
	MakeBoxTriangle(&bt, this, tri, vel);
	TriSweep sweep = { 0.0f, OBB_AXIS_NONE, 0, 0 };

	int axis = SweepAxes(this, t, &bt, &sweep);
	if (axis != OBB_AXIS_NONE)
		return axis;

	// The sweep is over: with a nonzero step the latest start of overlap must lie inside it.
	if (t != 0.0f && sweep.time >= t)
		return sweep.axis;

	if (bt.W.x != 0.0f || bt.W.y != 0.0f || bt.W.z != 0.0f) {
		axis = StaticAxes(this, &bt);
		if (axis != OBB_AXIS_NONE)
			return axis;
	}

	if (point)
		FillContact(this, &bt, &sweep, tri, vel, point, normal);
	if (tOut)
		*tOut = sweep.time;
	return OBB_AXIS_NONE;
}
