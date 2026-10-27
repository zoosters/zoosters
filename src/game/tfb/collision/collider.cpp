#include "tfb/collision/collider.h"
#include "tfb/collision/broadphase.h"
#include "script/script.h"
#include <float.h>
#include <math.h>

namespace RWS {
namespace TFB {

// -FLT_MAX, the empty-bounds marker. The game tests it as an integer.
static const int32_t kBitsMinusFltMax = (int32_t)0xFF7FFFFF;

void Collider::AssignSerial()
{
	static unsigned next;
	unsigned v = ++next & 0xffffff;
	m_padCD[0] = (uint8_t)v;
	m_padCD[1] = (uint8_t)(v >> 8);
	m_padCD[2] = (uint8_t)(v >> 16);
}

// Out of the broad phase. The list entries stay until the next sort.
// 0x447790
void Collider::ClearLinks()
{
	m_max.m_collider = 0;
	m_min.m_collider = 0;
}

// Back into the broad phase. The corners park at FLT_MAX until the next UpdateBounds, unless there are no bounds yet.
// 0x4477a0
void Collider::Reset()
{
	m_max.m_collider = this;
	m_min.m_collider = this;
	if (*(const int32_t *)&m_min.m_point.x != kBitsMinusFltMax) {
		m_max.m_point.x = m_min.m_point.x = FLT_MAX;
		m_max.m_point.y = m_min.m_point.y = FLT_MAX;
		m_max.m_point.z = m_min.m_point.z = FLT_MAX;
	}
}

// Row 0..3 (right, up, at, pos), column 0..2 (x, y, z).
static inline RwReal MatrixElement(const RwMatrix &m, int row, int col)
{
	return (&m.right.x)[row * 4 + col];
}

// Places the world box with `frame` and recomputes the bounds: the box's reach on each axis plus 0.5, grown to include the
// owner's position and the motion over `dt`. A collider with no bounds yet joins the broad phase.
// 0x4477d0
void Collider::UpdateBounds(const RwMatrix *frame, RwReal dt)
{
	bool hadBounds = m_min.m_point.x != -FLT_MAX;
	if (&m_worldBox.m_frame != frame)
		RwMatrixMultiply(&m_worldBox.m_frame, &m_localBox.m_frame, frame);

	const RwMatrix &box = m_worldBox.m_frame;
	const RwReal *ext = m_worldBox.m_extent;
	const RwReal *pos = &frame->pos.x;
	const RwReal *vel = &m_velocity.x;
	RwReal *lo = &m_min.m_point.x;
	RwReal *hi = &m_max.m_point.x;
	for (int i = 0; i < 3; i++) {
		// summed in the game's order
		RwReal reach = fabsf(MatrixElement(box, 0, i) * ext[0]) + fabsf(MatrixElement(box, 2, i) * ext[2]);
		reach = reach + fabsf(MatrixElement(box, 1, i) * ext[1]) + 0.5f;
		RwReal centre = MatrixElement(box, 3, i);
		lo[i] = centre - reach;
		hi[i] = centre + reach;
		if (pos[i] < lo[i])
			lo[i] = pos[i];
		else if (pos[i] > hi[i])
			hi[i] = pos[i];
		RwReal motion = dt * vel[i];
		if (motion <= 0.0f)
			lo[i] += motion;
		else
			hi[i] += motion;
	}

	if (m_min.m_collider == 0) {
		m_max.m_collider = this;
		m_min.m_collider = this;
		if (!hadBounds) {
			int n = g_boundsCount;
			for (int axis = 0; axis < 3; axis++) {
				g_boundsItems[axis][n] = &m_min;
				g_boundsItems[axis][n + 1] = &m_max;
			}
			m_owner->AdjustRefCount(1); // one reference per corner
			m_owner->AdjustRefCount(1);
			g_boundsCount += 2;
		}
	}
}

} // namespace TFB
} // namespace RWS
