// RpCollision queries on the port's worlds and atomics (RpCollisionWorldForAllIntersections 0x4a74d0, RpAtomicForAllIntersections
// 0x4aeb40) and the triangle tests they use (the box test is also exported, game 0x49efd0). They must report the same triangles as
// the game, in its order (line queries front to back), with the same distances and normals, and stop when the callback returns null.
// World queries walk the BSP (PortBspPlane nodes) and each reached sector's collision tree (plugin 0x11d, collTreeRead); sectors
// without a tree are skipped, as in the game. Atomic queries use the geometry's tree (geomCollRead) for single morph target
// geometries, a plain triangle walk without a tree, and interpolated triangles with several morph targets.
//   PORT_COLLCMP=1        run the tree walk and brute force per world query and compare the hit sets ("collcmp:" log lines)
//   PORT_ATOMCMP=1        compare tree and plain walk per call
//   PORT_ATOMLOG=1        log calls and callback sequences
//   PORT_ATOMSELFTEST=1   random queries against every geometry with a tree
// Included by rwshim.cpp (after ShimWorld / ShimSector).
#include <math.h>
#include <chrono>
#include "zoosters/os.h"
extern "C" float _rwInvSqrt(float x);
#define CLOG(...) do { if (port_log) { fprintf(port_log, __VA_ARGS__); } } while (0)

struct CcV3 { float x, y, z; };
struct RwV3d;   // the game's vector type (only pointers pass through here)
int RayTriangleIntersect(const RwV3d *, const RwV3d *, const RwV3d *, const RwV3d *, const RwV3d *, RwV3d *, float *, float *);   // 0x448d10 (src/game tfb/collision/raytriangle.h)
#define FUN_00448d10(a, b, c, d, e, f, g, h) RayTriangleIntersect((const RwV3d *)(a), (const RwV3d *)(b), (const RwV3d *)(c), (const RwV3d *)(d), (const RwV3d *)(e), (RwV3d *)(f), g, h)
namespace {

struct V3 { float x, y, z; };
struct Line { V3 start, end; };
struct Sphere { V3 center; float radius; };
struct BBox { V3 sup, inf; };
struct Intersection {                        // RpIntersection: union t (24 B) + type at +0x18
	union { Line line; V3 point; Sphere sphere; BBox box; void *object; } t;
	int type;                                // 1 line, 2 point, 3 sphere, 4 box, 5 atomic
};
struct CollTri { V3 normal, point; int index; V3 *vertices[3]; };   // RpCollisionTriangle, 0x28 B
struct RpTri { unsigned short v[3], mat; };

typedef void *(*WorldTriCB)(Intersection *, void *sector, CollTri *, float, void *);
typedef void *(*GeomTriCB)(Intersection *, CollTri *, float, void *);

inline V3 sub(const V3 &a, const V3 &b) { V3 r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
inline V3 cross(const V3 &a, const V3 &b) { V3 r = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; return r; }
inline float dot(const V3 &a, const V3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline V3 scale(const V3 &a, float s) { V3 r = { a.x * s, a.y * s, a.z * s }; return r; }
inline float cmin(float a, float b) { return a < b ? a : b; }
inline float cmax(float a, float b) { return a > b ? a : b; }
inline int bits(float f) { union { float f; int i; } u; u.f = f; return u.i; }

static int g_cSectors, g_cTris, g_cHits;
static bool port_readable_range(const void *p, unsigned n) { return p && zoosters::os::MemoryReadable(p, n); }
const float kCubeInSphere = 0.57f;

// One-sided segment / triangle test: only a triangle facing the segment is hit. *fraction is the plane crossing along delta (0 at
// start, 1 at start + delta), with a small relative tolerance outside [0, 1]. Every comparison is written so that a NaN fails it.
bool lineTri(const V3 &start, const V3 &delta, const V3 &v0, const V3 &v1, const V3 &v2, float *fraction)
{
	const V3 edge1 = sub(v1, v0), edge2 = sub(v2, v0);
	const V3 pvec = cross(delta, edge2);
	const float det = dot(edge1, pvec);
	if (!(det > 1e-8f))
		return false;
	const float lo = -det * 1e-5f;
	const float hi = det - lo;

	const V3 tvec = sub(start, v0);
	const float u = dot(tvec, pvec);
	if (!(u >= lo && u <= hi))
		return false;
	const V3 qvec = cross(tvec, edge1);
	const float v = dot(delta, qvec);
	if (!(v >= lo && u + v <= hi))
		return false;
	const float t = dot(edge2, qvec);
	if (!(t >= lo && t <= hi))
		return false;
	*fraction = t / det;
	return true;
}

// Sphere / triangle test: *normal is the unit normal (table inverse square root, as the game), *distance the signed distance of the
// centre from the plane.
bool sphereTri(const Sphere *sphere, const V3 &v0, const V3 &v1, const V3 &v2, V3 *normal, float *distance)
{
	const float radius = sphere->radius;
	const V3 a[3] = { sub(v0, sphere->center), sub(v1, sphere->center), sub(v2, sphere->center) };

	// Entirely on the far side of one of the six planes bounding the sphere's box: touching the upper plane counts as outside,
	// touching the lower one does not.
	for (int k = 0; k < 3; k++) {
		const float c0 = (&a[0].x)[k], c1 = (&a[1].x)[k], c2 = (&a[2].x)[k];
		if (c0 - radius >= 0.0f && c1 - radius >= 0.0f && c2 - radius >= 0.0f)
			return false;
		if (c0 + radius < 0.0f && c1 + radius < 0.0f && c2 + radius < 0.0f)
			return false;
	}

	const V3 n = cross(sub(v1, v0), sub(v2, v0));
	const float lengthSq = dot(n, n);
	if (lengthSq <= 0.0f)
		return false;
	const V3 unit = scale(n, _rwInvSqrt(lengthSq));
	const float plane = dot(a[0], unit);
	if (plane < -radius || plane > radius)
		return false;

	const float radiusSq = radius * radius;
	const float distSq[3] = { dot(a[0], a[0]), dot(a[1], a[1]), dot(a[2], a[2]) };
	if (!(distSq[0] <= radiusSq || distSq[1] <= radiusSq || distSq[2] <= radiusSq)) {
		// Nearest vertex; ties go to vertex 1, then 0, then 2.
		int m = 1;
		if (distSq[0] < distSq[m])
			m = 0;
		if (distSq[2] < distSq[m])
			m = 2;
		const int j = m == 0 ? 1 : 0, k = m == 2 ? 1 : 2;
		if (dot(a[m], a[j]) > distSq[m] && dot(a[m], a[k]) > distSq[m])
			return false;

		// Nearest point of each edge's line; a zero-length edge gives NaN or infinity and never rejects.
		for (int i = 0; i < 3; i++) {
			const V3 &p = a[i], &q = a[(i + 1) % 3], &opposite = a[(i + 2) % 3];
			const V3 e = sub(q, p);
			const float lambda = dot(e, p) / dot(e, e);
			const V3 w = { p.x - e.x * lambda, p.y - e.y * lambda, p.z - e.z * lambda };
			const float wSq = dot(w, w);
			if (wSq > radiusSq && wSq < dot(w, opposite))
				return false;
		}
	}

	*normal = unit;
	*distance = -plane;
	return true;
}

// Which faces of the box a point lies beyond: bit k for the upper face of axis k, bit k + 3 for the lower one (only when not
// beyond the upper). The box is closed, and a NaN coordinate is never beyond.
inline unsigned boxOutcode(const BBox &box, const V3 &p)
{
	unsigned code = 0;
	for (int k = 0; k < 3; k++) {
		const float c = (&p.x)[k];
		if (c > (&box.sup.x)[k])
			code |= 1u << k;
		else if (c < (&box.inf.x)[k])
			code |= 8u << k;
	}
	return code;
}

// Does the edge from base along dir cross one of the faces in `faces` strictly inside the face rectangle? Products only, no division.
inline bool edgeCrossesFaces(const BBox &box, const V3 &base, const V3 &dir, unsigned faces)
{
	const V3 up = sub(base, box.sup), low = sub(base, box.inf);
	const float *u = &up.x, *l = &low.x, *g = &dir.x;
	for (int a = 0; a < 3; a++) {
		const bool upper = faces & (1u << a), lower = faces & (8u << a);
		if (!upper && !lower)
			continue;
		const int b = (a + 1) % 3, c = (a + 2) % 3;
		const float ub = u[b] * g[a], lb = l[b] * g[a], uc = u[c] * g[a], lc = l[c] * g[a];
		const bool negative = g[a] < 0.0f;
		for (int side = 0; side < 2; side++) {
			if (!(side == 0 ? upper : lower))
				continue;
			const float t = side == 0 ? u[a] : l[a];
			const float tb = t * g[b], tc = t * g[c];
			const bool hit = negative ? (lb < tb && tb < ub && lc < tc && tc < uc) : (ub < tb && tb < lb && uc < tc && tc < lc);
			if (hit)
				return true;
		}
	}
	return false;
}

// Box / triangle test (closed box). Hot: about two million calls over the scenario set.
bool boxTri(const BBox *bbox, const V3 &v0, const V3 &v1, const V3 &v2)
{
	const BBox &box = *bbox;
	const unsigned c0 = boxOutcode(box, v0), c1 = boxOutcode(box, v1), c2 = boxOutcode(box, v2);
	if (!c0 || !c1 || !c2)
		return true;
	if (c0 & c1 & c2)
		return false;

	const V3 e1 = sub(v1, v0), e2 = sub(v2, v0);
	if (!(c0 & c1) && edgeCrossesFaces(box, v0, e1, c0 ^ c1))
		return true;
	if (!(c0 & c2) && edgeCrossesFaces(box, v0, e2, c0 ^ c2))
		return true;
	if (!(c1 & c2) && edgeCrossesFaces(box, v1, sub(v2, v1), c1 ^ c2))
		return true;

	// The box diagonal chosen from the normal's sign bits (so +0 and -0 differ) must pierce the triangle strictly inside.
	const V3 n = cross(e1, e2);
	V3 corner, diag;
	corner.x = box.inf.x;
	diag.x = box.sup.x - box.inf.x;
	if ((bits(n.x) ^ bits(n.y)) < 0) {
		corner.y = box.sup.y;
		diag.y = box.inf.y - box.sup.y;
	} else {
		corner.y = box.inf.y;
		diag.y = box.sup.y - box.inf.y;
	}
	if ((bits(n.x) ^ bits(n.z)) < 0) {
		corner.z = box.sup.z;
		diag.z = box.inf.z - box.sup.z;
	} else {
		corner.z = box.inf.z;
		diag.z = box.sup.z - box.inf.z;
	}
	const V3 t = sub(corner, v0);
	const float span = -dot(diag, n);
	const float s = dot(t, n);
	const V3 w = cross(t, diag);
	const float alpha = dot(e2, w);
	const float beta = -dot(e1, w);
	if (span < 0.0f)
		return span < s && s < 0.0f && span < alpha && alpha < 0.0f && beta < 0.0f && span < alpha + beta;
	return 0.0f < s && s < span && 0.0f < alpha && alpha < span && 0.0f < beta && alpha + beta < span;
}

inline void triNormal(V3 *n, const V3 &v0, const V3 &v1, const V3 &v2)
{
	V3 a = sub(v1, v0), b = sub(v2, v0);
	*n = cross(a, b);
	*n = scale(*n, _rwInvSqrt(dot(*n, *n)));
}
inline void initTri(CollTri *ct, int index, V3 *v0, V3 *v1, V3 *v2)
{
	ct->point = *v0; ct->index = index; ct->vertices[0] = v0; ct->vertices[1] = v1; ct->vertices[2] = v2;
	triNormal(&ct->normal, *v0, *v1, *v2);
}

// sector bounding box (sup/inf as RW boundingBox) vs query volume, with a margin (the BSP cell and the triangles' extent may differ
// slightly)
const float kMargin = 2.0f;
bool lineHitsBox(const Line &l, const float *sup, const float *inf)
{
	float t0 = 0.0f, t1 = 1.0f;
	const float s[3] = { l.start.x, l.start.y, l.start.z }, d[3] = { l.end.x - l.start.x, l.end.y - l.start.y, l.end.z - l.start.z };
	for (int i = 0; i < 3; i++) {
		float lo = cmin(inf[i], sup[i]) - kMargin, hi = cmax(inf[i], sup[i]) + kMargin;
		if (fabsf(d[i]) < 1e-12f) { if (s[i] < lo || s[i] > hi) return false; continue; }
		float a = (lo - s[i]) / d[i], b = (hi - s[i]) / d[i];
		if (a > b) { float t = a; a = b; b = t; }
		if (a > t0) t0 = a;
		if (b < t1) t1 = b;
		if (t0 > t1) return false;
	}
	return true;
}
bool boxHitsBox(const BBox &q, const float *sup, const float *inf)
{
	const float qlo[3] = { cmin(q.inf.x, q.sup.x), cmin(q.inf.y, q.sup.y), cmin(q.inf.z, q.sup.z) }, qhi[3] = { cmax(q.inf.x, q.sup.x), cmax(q.inf.y, q.sup.y), cmax(q.inf.z, q.sup.z) };
	for (int i = 0; i < 3; i++)
		if (qhi[i] < cmin(inf[i], sup[i]) - kMargin || qlo[i] > cmax(inf[i], sup[i]) + kMargin) return false;
	return true;
}

void worldLine(ShimWorld *w, Intersection *is, WorldTriCB cb, void *data)
{
	const Line &line = is->t.line;
	V3 delta = sub(line.end, line.start);
	for (ShimSector *sec = w->sectors; sec; sec = sec->next) {
		if (!sec->triangles || !sec->vertices || !lineHitsBox(line, sec->sup, sec->inf)) continue;
		const RpTri *tris = (const RpTri *)sec->triangles; V3 *verts = (V3 *)sec->vertices; g_cSectors++; g_cTris += sec->numTriangles;
		for (int i = 0; i < sec->numTriangles; i++) {
			V3 *v0 = verts + tris[i].v[0], *v1 = verts + tris[i].v[1], *v2 = verts + tris[i].v[2];
			float dist; CollTri ct;
			if (lineTri(line.start, delta, *v0, *v1, *v2, &dist)) {
				initTri(&ct, i, v0, v1, v2);
				g_cHits++; if (!cb(is, sec, &ct, dist, data)) return;
			}
		}
	}
}
// diagnostics (PORT_COLLLOG2): what the game's ground callback gets and does for box queries
void *boxCb(WorldTriCB cb, Intersection *is, void *sec, CollTri *ct, void *data, const RpTri *tris, int i)
{
	static int nlog; static const bool lg = getenv("PORT_COLLLOG2") != 0;
	if (!(lg && port_log && nlog < 600 && port_readable_range(data, 0xd8))) return cb(is, sec, ct, 0.0f, data);
	const float *q = (const float *)data; char *cr = *(char **)((char *)data + 0x70); char *ow = cr ? *(char **)(cr + 0x160) : 0;
	unsigned cf = cr ? *(unsigned *)(cr + 0x100) : 0, af = ow ? *(unsigned *)(ow + 0x1b0) : 0;
	static int nother; if (!(af & 4) && nother++ >= 10) return cb(is, sec, ct, 0.0f, data);
	fprintf(port_log, "gp: tri %d flag %02x n %.3f %.3f %.3f y %.4f %.4f %.4f | pre t %.3f rem %.3f d1 %d d2 %d cflags %08x af %08x pos %.4f %.4f %.4f disp %.4f %.4f %.4f up %.2f %.2f %.2f x88 %.4f a0 %.4f %.4f %.4f d2c %.4f %.4f %.4f hitA %p h74 %p\n", i, ((const unsigned char *)tris)[i * 8 + 6],
		ct->normal.x, ct->normal.y, ct->normal.z, ct->vertices[0]->y, ct->vertices[1]->y, ct->vertices[2]->y, q[0], q[1], ((unsigned char *)data)[0xd1], ((unsigned char *)data)[0xd2], cf, af, q[0x11], q[0x12], q[0x13], q[0xe], q[0xf], q[0x10], q[8], q[9], q[10], q[0x22], q[0x28], q[0x29], q[0x2a], q[0xb], q[0xc], q[0xd], *(void **)((char *)data + 0x78), *(void **)((char *)data + 0x74));
	fprintf(port_log, "gp:   verts %.4f %.4f %.4f / %.4f %.4f %.4f / %.4f %.4f %.4f\n", ct->vertices[0]->x, ct->vertices[0]->y, ct->vertices[0]->z, ct->vertices[1]->x, ct->vertices[1]->y, ct->vertices[1]->z, ct->vertices[2]->x, ct->vertices[2]->y, ct->vertices[2]->z);
	{	V3 Vp = { q[0x11] + q[1] * q[0xe], q[0x12] + q[1] * q[0xf], q[0x13] + q[1] * q[0x10] }, hit = { 0, 0, 0 }; float uv[2] = { 0, 0 }, dd = 0; const V3 *dir = (const V3 *)&q[0xb];
		int rr = FUN_00448d10((CcV3 *)&Vp, (const CcV3 *)dir, (CcV3 *)ct->vertices[0], (CcV3 *)ct->vertices[1], (CcV3 *)ct->vertices[2], (CcV3 *)&hit, uv, &dd);
		fprintf(port_log, "gp:   ray from %.4f %.4f %.4f dir %.2f %.2f %.2f -> %d dist %.4f uv %.4f %.4f hit %.3f %.3f %.3f\n", Vp.x, Vp.y, Vp.z, dir->x, dir->y, dir->z, rr, dd, uv[0], uv[1], hit.x, hit.y, hit.z); }
	void *r = cb(is, sec, ct, 0.0f, data);
	fprintf(port_log, "gp:   -> t %.3f rem %.3f nrm %.2f %.2f %.2f cflags %08x\n", q[0], q[1], q[5], q[6], q[7], cr ? *(unsigned *)(cr + 0x100) : 0);
	nlog++; return r;
}
void worldBox(ShimWorld *w, Intersection *is, WorldTriCB cb, void *data)
{
	const BBox *bbox = &is->t.box;
	for (ShimSector *sec = w->sectors; sec; sec = sec->next) {
		if (!sec->triangles || !sec->vertices || !boxHitsBox(*bbox, sec->sup, sec->inf)) continue;
		const RpTri *tris = (const RpTri *)sec->triangles; V3 *verts = (V3 *)sec->vertices; g_cSectors++; g_cTris += sec->numTriangles;
		for (int i = 0; i < sec->numTriangles; i++) {
			V3 *v0 = verts + tris[i].v[0], *v1 = verts + tris[i].v[1], *v2 = verts + tris[i].v[2];
			if (boxTri(bbox, *v0, *v1, *v2)) {
				CollTri ct; initTri(&ct, i, v0, v1, v2);
				g_cHits++; if (!boxCb(cb, is, sec, &ct, data, tris, i)) return;        // distance not defined
			}
		}
	}
}
void worldSphere(ShimWorld *w, const Sphere *sphere, Intersection *is, WorldTriCB cb, void *data)
{
	float recipRadius = 1.0f / sphere->radius, inner = kCubeInSphere * sphere->radius;
	BBox bb = { { sphere->center.x + sphere->radius, sphere->center.y + sphere->radius, sphere->center.z + sphere->radius },
	            { sphere->center.x - sphere->radius, sphere->center.y - sphere->radius, sphere->center.z - sphere->radius } };
	BBox ib = { { sphere->center.x + inner, sphere->center.y + inner, sphere->center.z + inner }, { sphere->center.x - inner, sphere->center.y - inner, sphere->center.z - inner } };
	for (ShimSector *sec = w->sectors; sec; sec = sec->next) {
		if (!sec->triangles || !sec->vertices || !boxHitsBox(bb, sec->sup, sec->inf)) continue;
		const RpTri *tris = (const RpTri *)sec->triangles; V3 *verts = (V3 *)sec->vertices; g_cSectors++; g_cTris += sec->numTriangles;
		for (int i = 0; i < sec->numTriangles; i++) {
			V3 *v0 = verts + tris[i].v[0], *v1 = verts + tris[i].v[1], *v2 = verts + tris[i].v[2];
			CollTri ct; float dist;
			bool outside = false;                                 // the tree reports `clip` = 0 for leaves entirely inside the inner cube
			const V3 *p[3] = { v0, v1, v2 };
			for (int k = 0; k < 3; k++)
				if (p[k]->x < ib.inf.x || p[k]->x > ib.sup.x || p[k]->y < ib.inf.y || p[k]->y > ib.sup.y || p[k]->z < ib.inf.z || p[k]->z > ib.sup.z) outside = true;
			if (!outside) {
				triNormal(&ct.normal, *v0, *v1, *v2);
				dist = dot(sub(sphere->center, *v0), ct.normal);
			} else if (!sphereTri(sphere, *v0, *v1, *v2, &ct.normal, &dist)) continue;
			dist *= recipRadius;
			ct.point = *v0; ct.index = i; ct.vertices[0] = v0; ct.vertices[1] = v1; ct.vertices[2] = v2;
			g_cHits++; if (!cb(is, sec, &ct, dist, data)) return;
		}
	}
}

// ---------------------------------------------------------------- collision trees
struct LNode { void *sector; float pa, pb; };
struct LineCtx { float start[3], delta[3], recip[3], padding[3]; };
struct LineIter { LineCtx c; LNode stack[80]; LNode *curr; };
struct BNode { void *sector; unsigned clip; };
struct BBoxIter { BBox bb, clipBB; BNode stack[80]; BNode *curr; };
const unsigned char kSplit = 0xff;

inline void lineCtxInit(LineCtx *c, const Line &l)
{
	const float s[3] = { l.start.x, l.start.y, l.start.z }, e[3] = { l.end.x, l.end.y, l.end.z };
	for (int i = 0; i < 3; i++) {
		c->start[i] = s[i]; c->delta[i] = e[i] - s[i];
		c->recip[i] = c->delta[i] != 0 ? 1 / c->delta[i] : 0;
		c->padding[i] = cmax(fabsf(s[i]), fabsf(e[i])) * 1e-6f;                  // rpCOLLISION_LINE_FEPS
	}
}
// line iterator step: axis = byte offset 0 / 4 / 8; curr->sector is replaced by the near child (the far one stays below it on the stack)
inline void lineStep(const LineCtx &c, LNode *&curr, int axis, void *left, void *right, float leftHi, float rightLo)
{
	int a = axis >> 2;
	float del = c.delta[a], sta = c.start[a], pad = c.padding[a], rec;
	float end = sta + curr->pb * del;
	sta = sta + curr->pa * del;
	float padLo = rightLo - pad, padHi = leftHi + pad;
	if (del > 0) {
		if (padLo > end) curr->sector = left;
		else if (padHi < sta) curr->sector = right;
		else {
			curr++; rec = c.recip[a];
			curr->sector = left; curr->pa = (curr - 1)->pa; curr->pb = (curr - 1)->pb;
			if (padHi < end) curr->pb += rec * (padHi - end);
			(curr - 1)->sector = right;
			if (padLo > sta) (curr - 1)->pa += rec * (padLo - sta);
		}
	} else {
		if (padHi < end) curr->sector = right;
		else if (padLo > sta) curr->sector = left;
		else {
			curr++; rec = c.recip[a];
			curr->sector = right; curr->pa = (curr - 1)->pa; curr->pb = (curr - 1)->pb;
			if (padLo > end) curr->pb += rec * (padLo - end);
			(curr - 1)->sector = left;
			if (padHi < sta) (curr - 1)->pa += rec * (padHi - sta);
		}
	}
}
// RpSectorLineItFindNextWorldSector: pops the next BSP leaf (stack bottom = root)
inline ShimSector *bspNextLine(LineIter *it)
{
	it->curr--;
	while (*(int *)it->curr->sector >= 0) {
		PortBspPlane *pl = (PortBspPlane *)it->curr->sector;
		lineStep(it->c, it->curr, pl->type, pl->left, pl->right, pl->leftValue, pl->rightValue);
	}
	return (ShimSector *)it->curr->sector;
}
inline void bspInitLine(LineIter *it, void *root, const Line &l)
{
	lineCtxInit(&it->c, l);
	it->stack[0].sector = root; it->stack[0].pa = 0; it->stack[0].pb = 1; it->curr = it->stack + 1;
}
// collision tree line iterator, started from the BSP iterator's current segment
struct CollLineIter { PortCollTree *tree; PortCollSector dummy; LineCtx c; LNode stack[80]; LNode *curr; };
inline void collLineInit(CollLineIter *it, PortCollTree *t, const LineIter *sit)
{
	it->tree = t; it->dummy.index = 0; it->dummy.type = 0;
	it->dummy.contents = t->numSplits == 0 ? (unsigned char)t->numEntries : kSplit;
	it->c = sit->c;
	it->stack[0].sector = &it->dummy; it->stack[0].pa = sit->curr->pa; it->stack[0].pb = sit->curr->pb; it->curr = it->stack + 1;
}
inline void collLineNext(CollLineIter *it, unsigned *start, unsigned *count)
{
	it->curr--;
	for (;;) {
		PortCollSector *sc = (PortCollSector *)it->curr->sector;
		if (sc->contents != kSplit) break;
		PortCollSplit *sp = it->tree->splits + sc->index;
		lineStep(it->c, it->curr, sp->left.type & 0x0c, &sp->left, &sp->right, sp->left.value, sp->right.value);
	}
	PortCollSector *sc = (PortCollSector *)it->curr->sector;
	*start = sc->index; *count = sc->contents;
}

// bbox iterators: BSP, and collision tree with clip flags
inline void bspInitBox(BBoxIter *it, void *root, const BBox &b) { it->bb = b; it->stack[0].sector = root; it->curr = it->stack + 1; }
inline ShimSector *bspNextBox(BBoxIter *it)
{
	it->curr--;
	while (*(int *)it->curr->sector >= 0) {
		PortBspPlane *pl = (PortBspPlane *)it->curr->sector;
		int a = pl->type >> 2;
		if (pl->rightValue > (&it->bb.sup.x)[a]) it->curr->sector = pl->left;
		else {
			it->curr->sector = pl->right;
			if (pl->leftValue >= (&it->bb.inf.x)[a]) { it->curr++; it->curr->sector = pl->left; }
		}
	}
	return (ShimSector *)it->curr->sector;
}
struct CollBoxIter { PortCollTree *tree; PortCollSector dummy; BBox bb, clipBB; BNode stack[80]; BNode *curr; };
inline unsigned clipFlagsOf(const float *inf, const float *sup, const BBox &c)
{
	return (inf[0] < c.inf.x ? 1u : 0) | (inf[1] < c.inf.y ? 4u : 0) | (inf[2] < c.inf.z ? 0x10u : 0) | (sup[0] > c.sup.x ? 2u : 0) | (sup[1] > c.sup.y ? 8u : 0) | (sup[2] > c.sup.z ? 0x20u : 0);
}
inline void collBoxInit(CollBoxIter *it, PortCollTree *t, const BBox &test, const BBox &clip)
{
	const float *ti = t->inf, *ts = t->sup;
	it->tree = t;
	if (ti[0] <= test.sup.x && ti[1] <= test.sup.y && ti[2] <= test.sup.z && ts[0] >= test.inf.x && ts[1] >= test.inf.y && ts[2] >= test.inf.z) {
		it->dummy.index = 0; it->dummy.type = 0;
		it->dummy.contents = t->numSplits == 0 ? (unsigned char)t->numEntries : kSplit;
		it->bb = test; it->clipBB = clip;
		it->stack[0].sector = &it->dummy; it->stack[0].clip = clipFlagsOf(ti, ts, clip); it->curr = it->stack + 1;
	} else it->curr = it->stack;
}
inline void collBoxNext(CollBoxIter *it, unsigned *start, unsigned *count, unsigned *clip)
{
	PortCollTree *t = it->tree;
	it->curr--;
	for (;;) {
		PortCollSector *sc = (PortCollSector *)it->curr->sector;
		if (sc->contents != kSplit) { *start = sc->index; *count = sc->contents; *clip = it->curr->clip; return; }
		PortCollSplit *sp = t->splits + sc->index;
		if (it->curr->clip == 0) {                                  // the slice below a branch is contiguous
			PortCollSector *ll = &sp->left, *rl = &sp->right;
			while (ll->contents == kSplit) ll = &t->splits[ll->index].left;
			while (rl->contents == kSplit) rl = &t->splits[rl->index].right;
			*start = ll->index; *count = (unsigned short)(rl->index + rl->contents - ll->index); *clip = 0; return;
		}
		int axis = sp->left.type & 0x0c, a = axis >> 2; float leftHi = sp->left.value, rightLo = sp->right.value;
		if (rightLo > (&it->bb.sup.x)[a]) it->curr->sector = &sp->left;
		else {
			it->curr->sector = &sp->right;
			if (leftHi >= (&it->bb.inf.x)[a]) {
				it->curr++;
				it->curr->sector = &sp->left; it->curr->clip = (it->curr - 1)->clip;
				if (leftHi <= (&it->clipBB.sup.x)[a]) it->curr->clip &= ~(2u << (axis >> 1));
				if (rightLo >= (&it->clipBB.inf.x)[a]) (it->curr - 1)->clip &= ~(1u << (axis >> 1));
			}
		}
	}
}

// ---------------------------------------------------------------- atomic queries
// The query is moved into the atomic's frame. More than one morph target: walk triangle by triangle through the interpolator
// (TriLerp); otherwise the geometry's tree (plugin 0x11d, geomTree) drives the walk, and without one every triangle of morph target 0
// is tested in index order. PORT_ATOMCMP=1 compares the tree and simple walks per call, PORT_ATOMLOG=1 logs each
// call and callback sequence.
int g_geomCollOffset = -1;                  // plugin offset of the geometry's PortCollTree* (RpCollisionPluginAttach)
inline PortCollTree *geomTree(rw::Geometry *g) { return g_geomCollOffset < 0 ? 0 : *(PortCollTree **)((char *)g + g_geomCollOffset); }
inline float intervalDist(float v, float a, float b) { return v < a ? a - v : (v > b ? v - b : 0.0f); }

inline bool intervalTest(float start, float recip, float inf, float sup, float *pa, float *pb)
{
	if (recip == 0.0f) return start >= inf && start <= sup;
	float p0 = recip * (inf - start), p1 = recip * (sup - start);
	if (p1 < p0) { float t = p0; p0 = p1; p1 = t; }
	bool result = (*pa <= p1) && (*pb >= p0);
	if (result) { *pa = cmax(*pa, p0); *pb = cmin(*pb, p1); }
	return result;
}
// line iterator at the tree root (no parent iterator): segment clipped to the padded tree bbox
inline void collLineInitRoot(CollLineIter *it, PortCollTree *t, const Line &l)
{
	it->tree = t; it->dummy.index = 0; it->dummy.type = 0;
	it->dummy.contents = t->numSplits == 0 ? (unsigned char)t->numEntries : kSplit;
	lineCtxInit(&it->c, l);
	it->stack[0].sector = &it->dummy; it->stack[0].pa = 0; it->stack[0].pb = 1; it->curr = it->stack + 1;
	float *pa = &it->stack[0].pa, *pb = &it->stack[0].pb;
	bool r = intervalTest(it->c.start[0], it->c.recip[0], t->inf[0] - it->c.padding[0], t->sup[0] + it->c.padding[0], pa, pb);
	if (r) r = intervalTest(it->c.start[1], it->c.recip[1], t->inf[1] - it->c.padding[1], t->sup[1] + it->c.padding[1], pa, pb);
	if (r) r = intervalTest(it->c.start[2], it->c.recip[2], t->inf[2] - it->c.padding[2], t->sup[2] + it->c.padding[2], pa, pb);
	if (!r) it->curr = it->stack;
}

struct TriLerp { const V3 *vertsA, *vertsB; float fac; V3 tri[3]; };
struct PortInterp { int flags; short start, end; float time, recipTime, position; };   // RpInterpolator (baclump.h)
inline void lerpUpdate(TriLerp *l, const RpTri *tri, V3 *&v0, V3 *&v1, V3 *&v2)
{
	if (l->vertsA == l->vertsB) { v0 = (V3 *)l->vertsA + tri->v[0]; v1 = (V3 *)l->vertsA + tri->v[1]; v2 = (V3 *)l->vertsA + tri->v[2]; return; }
	for (int k = 0; k < 3; k++) {
		const V3 &va = l->vertsA[tri->v[k]], &vb = l->vertsB[tri->v[k]];
		V3 d = sub(vb, va); d = scale(d, l->fac);
		l->tri[k].x = d.x + va.x; l->tri[k].y = d.y + va.y; l->tri[k].z = d.z + va.z;
	}
	v0 = &l->tri[0]; v1 = &l->tri[1]; v2 = &l->tri[2];
}
inline void lerpInit(TriLerp *l, rw::Geometry *g, const PortInterp *ip)                // indices outside the geometry: use target 0
{
	int s = ip->start, e = ip->end;
	if (s < 0 || s >= g->numMorphTargets) s = 0;
	if (e < 0 || e >= g->numMorphTargets) e = 0;
	l->vertsA = (const V3 *)g->morphTargets[s].vertices; l->vertsB = (const V3 *)g->morphTargets[e].vertices;
	l->fac = ip->recipTime * ip->position;
}

enum { PATH_TREE, PATH_SIMPLE, PATH_MORPH };
static const bool g_atomCmp = getenv("PORT_ATOMCMP") != 0, g_atomLog = getenv("PORT_ATOMLOG") != 0;
inline int geomPath(rw::Geometry *g, int mode) { return g->numMorphTargets > 1 ? PATH_MORPH : ((mode == 0 && geomTree(g)) ? PATH_TREE : PATH_SIMPLE); }
static int g_atomTris;                      // triangles tested by the last atomic query (diagnostics)

// line, in the frame of the atomic
void geomLine(rw::Geometry *g, int path, const rw::Atomic *atom, const Line &line, Intersection *is, GeomTriCB cb, void *data)
{
	V3 delta = sub(line.end, line.start);
	const RpTri *tris = (const RpTri *)g->triangles;
	if (path == PATH_TREE) {
		PortCollTree *t = geomTree(g); V3 *verts = (V3 *)g->morphTargets[0].vertices;
		CollLineIter it; collLineInitRoot(&it, t, line);
		while (it.curr > it.stack) {
			unsigned start, count; collLineNext(&it, &start, &count);
			for (unsigned i = start; i < start + count; i++) {
				unsigned index = t->map ? t->map[i] : i; g_atomTris++;
				const RpTri *tri = tris + index;
				V3 *v0 = verts + tri->v[0], *v1 = verts + tri->v[1], *v2 = verts + tri->v[2];
				float dist; CollTri ct;
				if (lineTri(line.start, delta, *v0, *v1, *v2, &dist)) {
					initTri(&ct, index, v0, v1, v2);
					if (!cb(is, &ct, dist, data)) return;
				}
			}
		}
		return;
	}
	TriLerp lerp; V3 *v0 = lerp.tri, *v1 = lerp.tri + 1, *v2 = lerp.tri + 2;
	if (path == PATH_MORPH) lerpInit(&lerp, g, (const PortInterp *)atom->rwInterpolator);
	else { lerp.vertsA = lerp.vertsB = (const V3 *)g->morphTargets[0].vertices; lerp.fac = 0; }
	for (int i = 0; i < g->numTriangles; i++) {                 // SimpleGeomForAllTriIntsecLine / MorphGeomForAllTriIntsecLine
		g_atomTris++;
		lerpUpdate(&lerp, tris + i, v0, v1, v2);
		float dist; CollTri ct;
		if (lineTri(line.start, delta, *v0, *v1, *v2, &dist)) {
			initTri(&ct, i, v0, v1, v2);
			if (!cb(is, &ct, dist, data)) return;
		}
	}
}
// sphere, in the frame of the atomic
void geomSphere(rw::Geometry *g, int path, const rw::Atomic *atom, const Sphere *sphere, Intersection *is, GeomTriCB cb, void *data)
{
	const RpTri *tris = (const RpTri *)g->triangles;
	float recipRadius = 1.0f / sphere->radius;
	if (path == PATH_TREE) {
		PortCollTree *t = geomTree(g); V3 *verts = (V3 *)g->morphTargets[0].vertices;
		float dx = intervalDist(sphere->center.x, t->inf[0], t->sup[0]), dy = intervalDist(sphere->center.y, t->inf[1], t->sup[1]), dz = intervalDist(sphere->center.z, t->inf[2], t->sup[2]);
		float distSq = dx * dx; distSq += dy * dy; distSq += dz * dz;
		if (!(distSq <= sphere->radius * sphere->radius)) return;     // sphere against the tree bbox
		float inner = kCubeInSphere * sphere->radius;
		BBox bb = { { sphere->center.x + sphere->radius, sphere->center.y + sphere->radius, sphere->center.z + sphere->radius },
		            { sphere->center.x - sphere->radius, sphere->center.y - sphere->radius, sphere->center.z - sphere->radius } };
		BBox ib = { { sphere->center.x + inner, sphere->center.y + inner, sphere->center.z + inner }, { sphere->center.x - inner, sphere->center.y - inner, sphere->center.z - inner } };
		CollBoxIter cit; collBoxInit(&cit, t, bb, ib);
		while (cit.curr > cit.stack) {
			unsigned start, count, clip; collBoxNext(&cit, &start, &count, &clip);
			for (unsigned i = start; i < start + count; i++) {
				unsigned index = t->map ? t->map[i] : i; g_atomTris++;
				const RpTri *tri = tris + index;
				V3 *v0 = verts + tri->v[0], *v1 = verts + tri->v[1], *v2 = verts + tri->v[2];
				CollTri ct; float dist;
				if (!clip) { triNormal(&ct.normal, *v0, *v1, *v2); dist = dot(sub(sphere->center, *v0), ct.normal); }
				else if (!sphereTri(sphere, *v0, *v1, *v2, &ct.normal, &dist)) continue;
				dist *= recipRadius;
				ct.point = *v0; ct.index = index; ct.vertices[0] = v0; ct.vertices[1] = v1; ct.vertices[2] = v2;
				if (!cb(is, &ct, dist, data)) return;
			}
		}
		return;
	}
	TriLerp lerp; V3 *v0 = lerp.tri, *v1 = lerp.tri + 1, *v2 = lerp.tri + 2;
	if (path == PATH_MORPH) lerpInit(&lerp, g, (const PortInterp *)atom->rwInterpolator);
	else { lerp.vertsA = lerp.vertsB = (const V3 *)g->morphTargets[0].vertices; lerp.fac = 0; }
	for (int i = 0; i < g->numTriangles; i++) {                 // SimpleGeomForAllTriIntsecSphere / MorphGeomForAllTriIntsecSphere
		g_atomTris++;
		lerpUpdate(&lerp, tris + i, v0, v1, v2);
		CollTri ct; float dist;
		if (sphereTri(sphere, *v0, *v1, *v2, &ct.normal, &dist)) {
			dist *= recipRadius;
			ct.point = *v0; ct.index = i; ct.vertices[0] = v0; ct.vertices[1] = v1; ct.vertices[2] = v2;
			if (!cb(is, &ct, dist, data)) return;
		}
	}
}
// AtomicForAllTriIntsecLine / Sphere: world-space query -> frame of the atomic -> geometry (wline set: line query, else wsphere)
void atomicQuery(rw::Atomic *a, Intersection *is, const Line *wline, const Sphere *wsphere, int mode, GeomTriCB cb, void *data)
{
	rw::Geometry *g = a->geometry; if (!g || !g->morphTargets) return;
	rw::Matrix inv; rw::Matrix::invert(&inv, a->getFrame()->getLTM());
	int path = geomPath(g, mode);
	if (wline) {
		Line local; rw::V3d pts[2] = { *(const rw::V3d *)&wline->start, *(const rw::V3d *)&wline->end }, out[2];
		rw::V3d::transformPoints(out, pts, 2, &inv);
		memcpy(&local.start, &out[0], 12); memcpy(&local.end, &out[1], 12);
		geomLine(g, path, a, local, is, cb, data);
	} else {
		Sphere local; rw::V3d c = *(const rw::V3d *)&wsphere->center, lc;
		rw::V3d::transformPoints(&lc, &c, 1, &inv);
		memcpy(&local.center, &lc, 12);
		float scaleSq = inv.at.x * inv.at.x + inv.at.y * inv.at.y + inv.at.z * inv.at.z;   // uniform scaling assumed (as RW)
		local.radius = sqrtf(scaleSq) * wsphere->radius;
		geomSphere(g, path, a, &local, is, cb, data);
	}
}

// ---- PORT_ATOMLOG / PORT_ATOMCMP
struct AtomHit { int index; float dist; };
struct AtomCollector { AtomHit h[4096]; int n; };
void *atomCollect(Intersection *, CollTri *ct, float dist, void *data)
{
	AtomCollector *c = (AtomCollector *)data;
	if (c->n < 4096) { c->h[c->n].index = ct->index; c->h[c->n].dist = dist; c->n++; }
	return data;
}
struct AtomLog { GeomTriCB cb; void *data; int call; };
void *atomLogCb(Intersection *is, CollTri *ct, float dist, void *data)
{
	AtomLog *l = (AtomLog *)data;
	if (port_log && l->call <= 200000) fprintf(port_log, "atomlog: #%d hit tri %d dist %.6f\n", l->call, ct->index, dist);
	return l->cb(is, ct, dist, l->data);
}
int atomHitCmp(const void *a, const void *b) { return ((const AtomHit *)a)->index - ((const AtomHit *)b)->index; }
void atomCompare(rw::Atomic *a, Intersection *is, const Line *wl, const Sphere *ws)
{
	static AtomCollector tree, brute; static AtomHit ta[4096], ba[4096];
	static int calls, bad, orderDiff, nPath[3], nType[8], withHits, treeTris, bruteTris, nonMono, lineHits; static unsigned totalHits;
	rw::Geometry *g = a->geometry; if (!g || !g->morphTargets) return;
	int path = geomPath(g, 0); nPath[path]++; nType[is->type & 7]++;
	tree.n = brute.n = 0;
	g_atomTris = 0; atomicQuery(a, is, wl, ws, 0, atomCollect, &tree); treeTris += g_atomTris;
	g_atomTris = 0; atomicQuery(a, is, wl, ws, 1, atomCollect, &brute); bruteTris += g_atomTris;
	calls++; totalHits += brute.n; if (brute.n) withHits++;
	bool sameOrder = tree.n == brute.n;
	for (int i = 0; sameOrder && i < tree.n; i++) if (tree.h[i].index != brute.h[i].index) sameOrder = false;
	if (tree.n > 1 && is->type == 1) { lineHits++; for (int i = 1; i < tree.n; i++) if (tree.h[i].dist < tree.h[i - 1].dist) { nonMono++; break; } }
	memcpy(ta, tree.h, tree.n * sizeof(AtomHit)); memcpy(ba, brute.h, brute.n * sizeof(AtomHit));
	qsort(ta, tree.n, sizeof(AtomHit), atomHitCmp); qsort(ba, brute.n, sizeof(AtomHit), atomHitCmp);
	bool same = tree.n == brute.n;
	for (int i = 0; same && i < tree.n; i++) if (ta[i].index != ba[i].index || fabsf(ta[i].dist - ba[i].dist) > 1e-4f * (1.0f + fabsf(ba[i].dist))) same = false;
	if (!same) { bad++; if (bad <= 20 && port_log) fprintf(port_log, "atomcmp: MISMATCH atomic %p type %d path %d tree %d hits, brute %d hits\n", (void *)a, is->type, path, tree.n, brute.n); }
	else if (!sameOrder) orderDiff++;
	if (port_log && (calls <= 5 || calls % 500 == 0)) { fprintf(port_log, "atomcmp: %d calls (line %d sphere %d atomic %d; tree %d simple %d morph %d), %d mismatches, %d with another order, hits %u in %d calls, line calls with >1 hits %d (dist not monotone in %d), triangles tested tree %d vs brute %d\n", calls, nType[1], nType[3], nType[5], nPath[PATH_TREE], nPath[PATH_SIMPLE], nPath[PATH_MORPH], bad, orderDiff, totalHits, withHits, lineHits, nonMono, treeTris, bruteTris); fflush(port_log); }
}

// ---- PORT_ATOMSELFTEST=1: random line / sphere queries against every geometry with a tree, tree walk vs simple walk
rw::Geometry *g_treeGeoms[64]; bool g_treeGeomTested[64]; int g_nTreeGeoms;
static const bool g_atomSelfTest = getenv("PORT_ATOMSELFTEST") != 0;
unsigned g_stSeed = 12345u;
inline float stRand() { g_stSeed = g_stSeed * 1664525u + 1013904223u; return (float)(g_stSeed >> 8) / 16777216.0f; }
void atomSelfTest(rw::Geometry *g)
{
	PortCollTree *t = geomTree(g); if (!t || g->numMorphTargets > 1 || !g->triangles) return;
	static AtomCollector tree, brute; static AtomHit ta[4096], ba[4096];
	float ext[3], mid[3];
	for (int i = 0; i < 3; i++) { ext[i] = (t->sup[i] - t->inf[i]) * 0.5f; mid[i] = (t->sup[i] + t->inf[i]) * 0.5f; }
	float diag = sqrtf(ext[0] * ext[0] + ext[1] * ext[1] + ext[2] * ext[2]);
	int nq[2] = { 0, 0 }, bad[2] = { 0, 0 }, withHits[2] = { 0, 0 }, orderDiff[2] = { 0, 0 }, multi = 0, treeMono = 0, idxMono = 0; unsigned treeTris[2] = { 0, 0 }, bruteTris[2] = { 0, 0 }, hits[2] = { 0, 0 };
	Intersection is; memset(&is, 0, sizeof is);
	for (int q = 0; q < 6000; q++) {
		int kind = q & 1;                                       // 0 line, 1 sphere
		V3 p[2];
		for (int k = 0; k < 2; k++) { p[k].x = mid[0] + ext[0] * (stRand() * 3.0f - 1.5f); p[k].y = mid[1] + ext[1] * (stRand() * 3.0f - 1.5f); p[k].z = mid[2] + ext[2] * (stRand() * 3.0f - 1.5f); }
		if ((q & 6) == 0) { p[1].x = p[0].x; }                  // axis-parallel lines too
		if ((q & 6) == 2) { p[1].y = p[0].y; p[1].z = p[0].z; }
		tree.n = brute.n = 0;
		Line ql = { p[0], p[1] }; if (ql.start.x == ql.end.x && ql.start.y == ql.end.y && ql.start.z == ql.end.z) ql.end.x += 1.0f;
		Sphere qs = { p[0], diag * (0.05f + 0.55f * stRand()) };
		for (int pass = 0; pass < 2; pass++) {
			AtomCollector *c = pass ? &brute : &tree; int path = pass ? PATH_SIMPLE : PATH_TREE; g_atomTris = 0;
			if (kind == 0) { is.type = 1; is.t.line = ql; geomLine(g, path, 0, ql, &is, atomCollect, c); }
			else { is.type = 3; is.t.sphere = qs; geomSphere(g, path, 0, &qs, &is, atomCollect, c); }
			(pass ? bruteTris : treeTris)[kind] += g_atomTris;
		}
		nq[kind]++; hits[kind] += brute.n; if (brute.n) withHits[kind]++;
		memcpy(ta, tree.h, tree.n * sizeof(AtomHit)); memcpy(ba, brute.h, brute.n * sizeof(AtomHit));
		bool sameOrder = tree.n == brute.n;
		for (int i = 0; sameOrder && i < tree.n; i++) if (tree.h[i].index != brute.h[i].index) sameOrder = false;
		qsort(ta, tree.n, sizeof(AtomHit), atomHitCmp); qsort(ba, brute.n, sizeof(AtomHit), atomHitCmp);
		bool same = tree.n == brute.n;
		for (int i = 0; same && i < tree.n; i++) if (ta[i].index != ba[i].index || fabsf(ta[i].dist - ba[i].dist) > 1e-4f * (1.0f + fabsf(ba[i].dist))) same = false;
		if (!same) { bad[kind]++; if (bad[kind] <= 3 && port_log) fprintf(port_log, "atomselftest: MISMATCH %s tree %d hits brute %d hits\n", kind ? "sphere" : "line", tree.n, brute.n); }
		else if (!sameOrder) orderDiff[kind]++;
		if (kind == 0 && tree.n > 1) {
			multi++; bool tm = true, im = true;
			for (int i = 1; i < tree.n; i++) { if (tree.h[i].dist < tree.h[i - 1].dist) tm = false; if (brute.h[i].dist < brute.h[i - 1].dist) im = false; }
			treeMono += tm; idxMono += im;
		}
	}
	if (port_log) {
		fprintf(port_log, "atomselftest: geometry %p (%d triangles, tree %d entries %d splits): lines %d (%d with hits, %u hits, %d mismatches, %d in another order, triangles tested tree %u vs simple %u); spheres %d (%d with hits, %u hits, %d mismatches, %d in another order, triangles tested tree %u vs simple %u); line queries with >1 hit %d: dist non-decreasing in tree order %d, in index order %d\n",
			(void *)g, g->numTriangles, t->numEntries, t->numSplits, nq[0], withHits[0], hits[0], bad[0], orderDiff[0], treeTris[0], bruteTris[0], nq[1], withHits[1], hits[1], bad[1], orderDiff[1], treeTris[1], bruteTris[1], multi, treeMono, idxMono);
		fflush(port_log);
	}
}
void atomSelfTestAll()
{
	for (int i = 0; i < g_nTreeGeoms; i++) if (!g_treeGeomTested[i]) { g_treeGeomTested[i] = true; atomSelfTest(g_treeGeoms[i]); }
}

// tree-driven world queries; false = the callback stopped the walk
bool treeLine(ShimWorld *w, Intersection *is, WorldTriCB cb, void *data)
{
	LineIter sit; bspInitLine(&sit, w->bspRoot, is->t.line);
	V3 start = { sit.c.start[0], sit.c.start[1], sit.c.start[2] }, delta = { sit.c.delta[0], sit.c.delta[1], sit.c.delta[2] };
	while (sit.curr > sit.stack) {
		ShimSector *sec = bspNextLine(&sit); PortCollTree *t = sec->collTree;
		if (!t || !sec->triangles || !sec->vertices) continue;
		const RpTri *tris = (const RpTri *)sec->triangles; V3 *verts = (V3 *)sec->vertices; g_cSectors++;
		CollLineIter cit; collLineInit(&cit, t, &sit);
		while (cit.curr > cit.stack) {
			unsigned s0, cnt; collLineNext(&cit, &s0, &cnt);
			for (unsigned i = s0; i < s0 + cnt; i++) {
				unsigned index = t->map ? t->map[i] : i; g_cTris++;
				V3 *v0 = verts + tris[index].v[0], *v1 = verts + tris[index].v[1], *v2 = verts + tris[index].v[2];
				float dist; CollTri ct;
				if (lineTri(start, delta, *v0, *v1, *v2, &dist)) {
					initTri(&ct, index, v0, v1, v2);
					g_cHits++; if (!cb(is, sec, &ct, dist, data)) return false;
				}
			}
		}
	}
	return true;
}
bool treeBox(ShimWorld *w, Intersection *is, WorldTriCB cb, void *data)
{
	const BBox *bbox = &is->t.box;
	BBoxIter sit; bspInitBox(&sit, w->bspRoot, *bbox);
	while (sit.curr > sit.stack) {
		ShimSector *sec = bspNextBox(&sit); PortCollTree *t = sec->collTree;
		if (!t || !sec->triangles || !sec->vertices) continue;
		const RpTri *tris = (const RpTri *)sec->triangles; V3 *verts = (V3 *)sec->vertices; g_cSectors++;
		CollBoxIter cit; collBoxInit(&cit, t, *bbox, *bbox);
		while (cit.curr > cit.stack) {
			unsigned s0, cnt, clip; collBoxNext(&cit, &s0, &cnt, &clip);
			for (unsigned i = s0; i < s0 + cnt; i++) {
				unsigned index = t->map ? t->map[i] : i; g_cTris++;
				V3 *v0 = verts + tris[index].v[0], *v1 = verts + tris[index].v[1], *v2 = verts + tris[index].v[2];
				if (!clip || boxTri(bbox, *v0, *v1, *v2)) {
					CollTri ct; initTri(&ct, index, v0, v1, v2);
					g_cHits++; if (!boxCb(cb, is, sec, &ct, data, tris, index)) return false;      // distance not defined
				}
			}
		}
	}
	return true;
}
bool treeSphere(ShimWorld *w, const Sphere *sphere, Intersection *is, WorldTriCB cb, void *data)
{
	float recipRadius = 1.0f / sphere->radius, inner = kCubeInSphere * sphere->radius;
	BBox bb = { { sphere->center.x + sphere->radius, sphere->center.y + sphere->radius, sphere->center.z + sphere->radius },
	            { sphere->center.x - sphere->radius, sphere->center.y - sphere->radius, sphere->center.z - sphere->radius } };
	BBox ib = { { sphere->center.x + inner, sphere->center.y + inner, sphere->center.z + inner }, { sphere->center.x - inner, sphere->center.y - inner, sphere->center.z - inner } };
	BBoxIter sit; bspInitBox(&sit, w->bspRoot, bb);
	while (sit.curr > sit.stack) {
		ShimSector *sec = bspNextBox(&sit); PortCollTree *t = sec->collTree;
		if (!t || !sec->triangles || !sec->vertices) continue;
		float dx = intervalDist(sphere->center.x, t->inf[0], t->sup[0]), dy = intervalDist(sphere->center.y, t->inf[1], t->sup[1]), dz = intervalDist(sphere->center.z, t->inf[2], t->sup[2]);
		float distSq = dx * dx; distSq += dy * dy; distSq += dz * dz;
		if (!(distSq <= sphere->radius * sphere->radius)) continue;
		const RpTri *tris = (const RpTri *)sec->triangles; V3 *verts = (V3 *)sec->vertices; g_cSectors++;
		CollBoxIter cit; collBoxInit(&cit, t, bb, ib);
		while (cit.curr > cit.stack) {
			unsigned s0, cnt, clip; collBoxNext(&cit, &s0, &cnt, &clip);
			for (unsigned i = s0; i < s0 + cnt; i++) {
				unsigned iPoly = t->map ? t->map[i] : i; g_cTris++;
				V3 *v0 = verts + tris[iPoly].v[0], *v1 = verts + tris[iPoly].v[1], *v2 = verts + tris[iPoly].v[2];
				CollTri ct; float dist;
				if (!clip) { triNormal(&ct.normal, *v0, *v1, *v2); dist = dot(sub(sphere->center, *v0), ct.normal); }
				else if (!sphereTri(sphere, *v0, *v1, *v2, &ct.normal, &dist)) continue;
				dist *= recipRadius;
				ct.point = *v0; ct.index = iPoly; ct.vertices[0] = v0; ct.vertices[1] = v1; ct.vertices[2] = v2;
				g_cHits++; if (!cb(is, sec, &ct, dist, data)) return false;
			}
		}
	}
	return true;
}

// ---- PORT_COLLCMP: the complete hit sets of the tree walk and the brute force must be equal
struct Hit { void *sec; int index; float dist; };
struct Collector { Hit *h; int n, cap; };
void *collect(Intersection *, void *sec, CollTri *ct, float dist, void *data)
{
	Collector *c = (Collector *)data;
	if (c->n == c->cap) { c->cap = c->cap ? c->cap * 2 : 256; c->h = (Hit *)realloc(c->h, c->cap * sizeof(Hit)); }
	c->h[c->n].sec = sec; c->h[c->n].index = ct->index; c->h[c->n].dist = dist; c->n++;
	return data;
}
int hitCmp(const void *a, const void *b)
{
	const Hit *x = (const Hit *)a, *y = (const Hit *)b;
	if (x->sec != y->sec) return x->sec < y->sec ? -1 : 1;
	return x->index < y->index ? -1 : (x->index > y->index ? 1 : 0);
}
inline bool distDiffers(int type, float a, float b) { return type != 4 && fabsf(a - b) > 1e-4f * (1.0f + fabsf(b)); }
void compareQueries(ShimWorld *w, Intersection *is, const Sphere *sph)
{
	static Collector a, b; static int queries, bad, orderDiff, badType[8], nType[8]; static unsigned treeTris, bruteTris, totalHits;
	a.n = b.n = 0;
	int c0 = g_cTris;
	switch (is->type) {
	case 1: treeLine(w, is, collect, &a); break;
	case 4: treeBox(w, is, collect, &a); break;
	default: treeSphere(w, sph, is, collect, &a); break;
	}
	treeTris += g_cTris - c0; c0 = g_cTris;
	switch (is->type) {
	case 1: worldLine(w, is, collect, &b); break;
	case 4: worldBox(w, is, collect, &b); break;
	default: worldSphere(w, sph, is, collect, &b); break;
	}
	bruteTris += g_cTris - c0; totalHits += b.n;
	bool sameOrder = a.n == b.n;
	for (int i = 0; sameOrder && i < a.n; i++) if (a.h[i].sec != b.h[i].sec || a.h[i].index != b.h[i].index) sameOrder = false;
	qsort(a.h, a.n, sizeof(Hit), hitCmp); qsort(b.h, b.n, sizeof(Hit), hitCmp);
	bool same = a.n == b.n;
	for (int i = 0; same && i < a.n; i++)
		if (hitCmp(&a.h[i], &b.h[i]) || distDiffers(is->type, a.h[i].dist, b.h[i].dist)) same = false;
	queries++; nType[is->type & 7]++; if (!sameOrder && same) orderDiff++;
	if (!same) {
		bad++; badType[is->type & 7]++;
		if (bad <= 25 && port_log) {
			fprintf(port_log, "collcmp: MISMATCH type %d tree %d hits, brute %d hits\n", is->type, a.n, b.n);
			int shown = 0, i = 0, j = 0;
			while ((i < a.n || j < b.n) && shown < 6) {
				int c = i >= a.n ? 1 : (j >= b.n ? -1 : hitCmp(&a.h[i], &b.h[j]));
				if (c == 0) { if (distDiffers(is->type, a.h[i].dist, b.h[j].dist)) { fprintf(port_log, "collcmp:   dist differs sec %p tri %d: tree %.6f brute %.6f\n", a.h[i].sec, a.h[i].index, a.h[i].dist, b.h[j].dist); shown++; } i++; j++; }
				else if (c < 0) { fprintf(port_log, "collcmp:   tree only: sec %p tri %d dist %.5f\n", a.h[i].sec, a.h[i].index, a.h[i].dist); i++; shown++; }
				else { fprintf(port_log, "collcmp:   brute only: sec %p (collTree %p, %d tris) tri %d dist %.5f\n", b.h[j].sec, (void *)((ShimSector *)b.h[j].sec)->collTree, ((ShimSector *)b.h[j].sec)->numTriangles, b.h[j].index, b.h[j].dist); j++; shown++; }
			}
			if (is->type == 1) fprintf(port_log, "collcmp:   line %.3f %.3f %.3f -> %.3f %.3f %.3f\n", is->t.line.start.x, is->t.line.start.y, is->t.line.start.z, is->t.line.end.x, is->t.line.end.y, is->t.line.end.z);
			else if (is->type == 4) fprintf(port_log, "collcmp:   box %.3f %.3f %.3f / %.3f %.3f %.3f\n", is->t.box.inf.x, is->t.box.inf.y, is->t.box.inf.z, is->t.box.sup.x, is->t.box.sup.y, is->t.box.sup.z);
			else fprintf(port_log, "collcmp:   sphere %.3f %.3f %.3f r %.3f\n", sph->center.x, sph->center.y, sph->center.z, sph->radius);
		}
	}
	if (port_log && (queries <= 5 || queries % 2000 == 0)) { fprintf(port_log, "collcmp: %d queries (line %d sphere %d box %d atomic %d), %d mismatches (line %d sphere %d box %d atomic %d), %d with a different order, hits %u, triangles tested tree %u vs brute %u\n", queries, nType[1], nType[3], nType[4], nType[5], bad, badType[1], badType[3], badType[4], badType[5], orderDiff, totalHits, treeTris, bruteTris); fflush(port_log); }
}

} // namespace

// RpCollision plugin of the world sectors, extension chunk 0x11d: int32 version (>= 0x36001; the game's streams are all 0x37002), then
// rwID_COLLTREE (0x2C) { STRUCT { flags, inf[3], sup[3], numEntries, numSplits }, splits[16 B each], uint16 map[numEntries] when flags
// & 1 }.
static PortCollTree *collTreeParse(rw::Stream *s, rw::int32 length)
{
	rw::uint32 len; int version = 0; PortCollTree *t = 0;
	s->read8(&version, 4);
	if (version >= 0x36001 && rw::findChunk(s, 0x2C, &len, 0) && rw::findChunk(s, rwID_STRUCT, &len, 0)) {
		struct { unsigned flags; float inf[3], sup[3]; unsigned numEntries, numSplits; } hd;
		s->read8(&hd, sizeof hd);
		if (hd.numEntries > 0 && hd.numEntries < 65536 && hd.numSplits < 65536) {
			t = (PortCollTree *)calloc(1, sizeof(PortCollTree));
			t->flags = hd.flags; memcpy(t->inf, hd.inf, 12); memcpy(t->sup, hd.sup, 12);
			t->numEntries = (unsigned short)hd.numEntries; t->numSplits = (unsigned short)hd.numSplits;
			if (hd.numSplits) { t->splits = (PortCollSplit *)malloc(hd.numSplits * sizeof(PortCollSplit)); s->read8(t->splits, hd.numSplits * sizeof(PortCollSplit)); }
			if (hd.flags & 1) { t->map = (unsigned short *)malloc(hd.numEntries * 2); s->read8(t->map, hd.numEntries * 2); }
		}
	}
	return t;
}
static rw::Stream *collTreeRead(rw::Stream *s, rw::int32 length, void *object, rw::int32, rw::int32)
{
	rw::uint32 pos = s->tell();
	ShimSector *sec = (ShimSector *)object;
	PortCollTree *t = collTreeParse(s, length);
	if (t) sec->collTree = t;
	rw::int32 left = (rw::int32)(pos + length) - (rw::int32)s->tell();
	if (left) s->seek(left, 1);                                   // RW trusts the chunk length
	return s;
}
// the same plugin id on geometries: the geometry owns its tree
// 0x62a7b0 _rpCollisionGeometryDataOffset: game code (CollisionGeometryBoxSweep 0x447a00) reads it to find a geometry's tree.
// The normal build defines the game's variable here (C linkage, as declared in rw/rpcollis.h) and sets it at startup below.
extern "C" { int _rpCollisionGeometryDataOffset = 0; int _rxPipelineGlobalsOffset = 0; }   // _rxPipelineGlobalsOffset: read by tfbatomic / tfbsector (rw/rpworld.h)
#define g_gameCollGeomOffset _rpCollisionGeometryDataOffset
static void *geomCollCtor(void *obj, rw::int32 offset, rw::int32) { *(PortCollTree **)((char *)obj + offset) = 0; return obj; }
static void *geomCollDtor(void *obj, rw::int32 offset, rw::int32)
{
	PortCollTree *t = *(PortCollTree **)((char *)obj + offset);
	for (int i = 0; i < g_nTreeGeoms; i++) if (g_treeGeoms[i] == obj) { g_treeGeoms[i] = g_treeGeoms[--g_nTreeGeoms]; g_treeGeomTested[i] = g_treeGeomTested[g_nTreeGeoms]; break; }
	if (t) { free(t->splits); free(t->map); free(t); *(PortCollTree **)((char *)obj + offset) = 0; }
	return obj;
}
static rw::Stream *geomCollRead(rw::Stream *s, rw::int32 length, void *object, rw::int32 offset, rw::int32)
{
	static int nTrees, nGeoms; static const bool lg = getenv("PORT_COLLLOG") != 0;
	rw::uint32 pos = s->tell();
	PortCollTree *t = collTreeParse(s, length);
	if (t) *(PortCollTree **)((char *)object + offset) = t;
	if (t && g_nTreeGeoms < 64) { g_treeGeoms[g_nTreeGeoms] = (rw::Geometry *)object; g_treeGeomTested[g_nTreeGeoms++] = false; }
	nGeoms++; if (t) nTrees++;
	if (lg && port_log && nGeoms <= 40) { rw::Geometry *g = (rw::Geometry *)object; fprintf(port_log, "collis: geometry collision tree %s (geometry %p, %d triangles, %d morph targets%s)\n", t ? "read" : "NOT read", object, g->numTriangles, g->numMorphTargets, t ? "" : ", chunk ignored"); }
	rw::int32 left = (rw::int32)(pos + length) - (rw::int32)s->tell();
	if (left) s->seek(left, 1);
	return s;
}
extern "C" int RpCollisionPluginAttach(void)
{
	static int done;
	if (!done) {
		done = 1;
		g_sectorPlugins.registerPlugin(4, 0x11d, 0, 0, 0);
		g_sectorPlugins.registerStream(0x11d, collTreeRead, 0, 0);
		g_geomCollOffset = rw::Geometry::registerPlugin(sizeof(PortCollTree *), 0x11d, geomCollCtor, geomCollDtor, 0);
		if (g_geomCollOffset >= 0) rw::Geometry::registerPluginStream(0x11d, geomCollRead, 0, 0);
		g_gameCollGeomOffset = g_geomCollOffset;   // the game's copy (RpCollisionPluginAttach sets it in RW)
		CLOG("rwcore: geometry collision plugin offset %d -> _rpCollisionGeometryDataOffset %p\n", g_geomCollOffset, (void *)&g_gameCollGeomOffset);
		CLOG("rwcore: RpCollisionPluginAttach (sector collision trees, plugin 0x11d)\n");
	}
	return 1;
}

#if defined(__aarch64__)
static inline unsigned long long port_cycles() { unsigned long long t; __asm__ volatile("mrs %0, cntvct_el0" : "=r"(t)); return t; }   // the generic timer: only ratios of it are logged
#else
extern "C" unsigned __int64 __rdtsc(void);
#pragma intrinsic(__rdtsc)
static inline unsigned long long port_cycles() { return __rdtsc(); }
#endif
static void GetSystemTimeAsFileTime(unsigned long long *t)   // 100 ns units of a real clock (the port's own counters are virtual)
{
	*t = (unsigned long long)std::chrono::duration_cast<std::chrono::duration<long long, std::ratio<1, 10000000> > >(std::chrono::steady_clock::now().time_since_epoch()).count();
}
static const bool g_collTimeOn = getenv("PORT_COLLTIME") != 0;
extern "C" void port_atom_selftest(void) { if (g_atomSelfTest && g_nTreeGeoms) atomSelfTestAll(); }   // PORT_ATOMSELFTEST=1: once per presented frame (rwcore.cpp RwCameraShowRaster)
extern "C" void port_semtrace_query(int type, const float *six, int tree, int sectors, int tris, int hits);   // provisional/semtrace.cpp (normal builds; PORT_SEMQ)
extern "C" { unsigned port_collq_world[8], port_collq_world_hits[8], port_collq_atomic[8]; }   // semtrace.cpp: queries / callback hits per intersection type (0..7)
extern "C" ShimWorld *RpCollisionWorldForAllIntersections(ShimWorld *w, void *intersection, void *cb, void *data)
{
	Intersection *is = (Intersection *)intersection;
	if ((unsigned)is->type < 8) port_collq_world[is->type]++;
	static int logged;
	struct Timer {                               // PORT_COLLTIME=1: time in world queries (callbacks included) vs real wall time (QPC / tick counters are virtual), logged every 2000 queries
		unsigned long long t0; Timer() { if (g_collTimeOn) t0 = port_cycles(); }
		~Timer() {
			if (!g_collTimeOn) return;
			static unsigned long long acc, lastTsc, lastFt; static int n; unsigned long long t1 = port_cycles(), ft;
			if (!lastTsc) { lastTsc = t0; GetSystemTimeAsFileTime(&lastFt); }
			acc += t1 - t0; n++;
			if (n % 2000 == 0 && port_log) {
				GetSystemTimeAsFileTime(&ft);
				double wallMs = (double)(ft - lastFt) / 1e4, frac = (double)acc / (double)(t1 - lastTsc);
				fprintf(port_log, "colltime: %d world queries: %.1f ms real wall since the last line, %.2f%% of it in world queries (%.1f us per query)\n", n, wallMs, 100.0 * frac, wallMs * frac * 1000.0 / 2000.0); fflush(port_log);
				acc = 0; lastTsc = t1; lastFt = ft;
			}
		}
	} timer;
	if (logged < 3 && port_log) { logged++; fprintf(port_log, "collis: RpCollisionWorldForAllIntersections type %d (brute force over %d sectors)\n", is->type, w->numSectors); }
	g_cSectors = g_cTris = g_cHits = 0;
	struct Logger { Intersection *is; void *dp; ~Logger() { static int n; if (port_log && getenv("PORT_COLLLOG") && is->t.line.start.x > 3000.0f && is->t.line.start.x < 4500.0f && is->t.line.start.z > 6000.0f && n++ < 300) { fprintf(port_log, "collis: world query type %d: %.1f %.1f %.1f / %.1f %.1f %.1f -> sectors %d tris %d hits %d\n", is->type, is->t.line.start.x, is->t.line.start.y, is->t.line.start.z, is->t.line.end.x, is->t.line.end.y, is->t.line.end.z, g_cSectors, g_cTris, g_cHits); if (dp) { const float *q = (const float *)dp; fprintf(port_log, "collis:   probe data %p: t %.4f rem %.4f hit %.1f %.1f %.1f nrm %.3f %.3f %.3f corpus %p\n", dp, q[0], q[1], q[2], q[3], q[4], q[5], q[6], q[7], *(void **)((const char *)dp + 0x70)); } } } } logger = { is, (is->type == 4 && port_readable_range(data, 0x80)) ? data : 0 };
	static const bool cmp = getenv("PORT_COLLCMP") != 0;
	const bool tree = w->bspRoot != 0;
	Sphere s = { { 0, 0, 0 }, 0 };      // type 5: port_atomic_world_sphere = RpAtomicGetWorldBoundingSphere (radius scaled by the LTM, unlike librw's)
	if (is->type == 3) s = is->t.sphere;
	else if (is->type == 5) { const rw::Sphere *bs = port_atomic_world_sphere((rw::Atomic *)is->t.object); s.center.x = bs->center.x; s.center.y = bs->center.y; s.center.z = bs->center.z; s.radius = bs->radius; }
	if (tree && cmp && (is->type == 1 || is->type == 3 || is->type == 4 || is->type == 5)) compareQueries(w, is, &s);
	if (tree && cmp && is->type == 4) {          // the game's own queries are lines and boxes: also exercise the sphere walk with spheres around the box queries (compare only)
		static int n; const BBox &b = is->t.box; n++;
		Intersection si; memset(&si, 0, sizeof si); si.type = 3;
		si.t.sphere.center.x = (b.sup.x + b.inf.x) * 0.5f; si.t.sphere.center.y = (b.sup.y + b.inf.y) * 0.5f; si.t.sphere.center.z = (b.sup.z + b.inf.z) * 0.5f;
		float hx = (b.sup.x - b.inf.x) * 0.5f, hy = (b.sup.y - b.inf.y) * 0.5f, hz = (b.sup.z - b.inf.z) * 0.5f;
		si.t.sphere.radius = sqrtf(hx * hx + hy * hy + hz * hz) * (0.25f + 0.75f * (float)(n % 5));
		if (si.t.sphere.radius > 0.0f) compareQueries(w, &si, &si.t.sphere);
	}
	g_cSectors = g_cTris = g_cHits = 0;
	switch (is->type) {
	case 1: if (tree) treeLine(w, is, (WorldTriCB)cb, data); else worldLine(w, is, (WorldTriCB)cb, data); break;
	case 3: case 5: if (tree) treeSphere(w, &s, is, (WorldTriCB)cb, data); else worldSphere(w, &s, is, (WorldTriCB)cb, data); break;
	case 4: if (tree) treeBox(w, is, (WorldTriCB)cb, data); else worldBox(w, is, (WorldTriCB)cb, data); break;
	default: CLOG("collis: RpCollisionWorldForAllIntersections: unsupported type %d\n", is->type); break;
	}
	if ((unsigned)is->type < 8) port_collq_world_hits[is->type] += g_cHits;
	port_semtrace_query(is->type, &is->t.line.start.x, tree ? 1 : 0, g_cSectors, g_cTris, g_cHits);
	return w;
}

extern "C" rw::Sphere *port_atomic_world_sphere(rw::Atomic *a);   // rwcore.cpp
extern "C" rw::Atomic *RpAtomicForAllIntersections(rw::Atomic *a, void *intersection, void *cb, void *data)
{
	Intersection *is = (Intersection *)intersection;
	if ((unsigned)is->type < 8) port_collq_atomic[is->type]++;
	const Line *wl = 0; Sphere s = { { 0, 0, 0 }, 0 };
	switch (is->type) {
	case 1: wl = &is->t.line; break;
	case 3: s = is->t.sphere; break;
	case 5: {
		const rw::Sphere *bs = port_atomic_world_sphere((rw::Atomic *)is->t.object);   // RpAtomicGetWorldBoundingSphere: radius scaled by the LTM
		s.center.x = bs->center.x; s.center.y = bs->center.y; s.center.z = bs->center.z; s.radius = bs->radius;
		break; }
	default: CLOG("collis: RpAtomicForAllIntersections: unsupported type %d\n", is->type); return a;
	}
	if (g_atomCmp) atomCompare(a, is, wl, &s);
	GeomTriCB useCb = (GeomTriCB)cb; void *useData = data; AtomLog lg;
	if (g_atomLog && port_log && a->geometry && a->geometry->morphTargets) {
		static int call; lg.cb = (GeomTriCB)cb; lg.data = data; lg.call = ++call; useCb = atomLogCb; useData = &lg;
		if (call <= 200000) fprintf(port_log, "atomlog: #%d atomic %p type %d path %d geometry %p tris %d morph %d\n", call, (void *)a, is->type, geomPath(a->geometry, 0), (void *)a->geometry, a->geometry->numTriangles, a->geometry->numMorphTargets);
	}
	atomicQuery(a, is, wl, &s, 0, useCb, useData);
	return a;
}

extern "C" int RtIntersectionBBoxTriangle(void *box, void *p0, void *p1, void *p2) { return boxTri((const BBox *)box, *(const V3 *)p0, *(const V3 *)p1, *(const V3 *)p2) ? 1 : 0; }
