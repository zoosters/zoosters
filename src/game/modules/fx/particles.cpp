#include "modules/fx/particles.h"

namespace RWS { namespace Particle {

ZOOSTERS_ASSERT_SIZE(TankView, 0x30);
ZOOSTERS_ASSERT_OFFSET_AT(Motion_size, sizeof(CPB_BasePart::Motion), 0x38);
ZOOSTERS_ASSERT_OFFSET_AT(StageParams_size, sizeof(CPB_BasePart::StageParams), 0x40);
ZOOSTERS_ASSERT_OFFSET_AT(SlotCursor_size, sizeof(CPB_BasePart::SlotCursor), 0x34);
ZOOSTERS_ASSERT_OFFSET_AT(SplitFrame_size, sizeof(CPB_BasePart::SplitFrame), 0x30);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_stages, 0x008);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_splitOn, 0x108);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_origin, 0x148);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_rateScale, 0x194);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_pendingMotionBlocks, 0x198);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_walk, 0x19c);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_fill, 0x1d0);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_walkLive, 0x204);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_splitFrame, 0x208);
ZOOSTERS_ASSERT_OFFSET(CPB_BasePart, m_liveSnapshot, 0x238);
ZOOSTERS_ASSERT_SIZE(CPB_BasePart, 0x29c);
ZOOSTERS_ASSERT_OFFSET_AT(CellTrack_size, sizeof(CPB_TexPart::CellTrack), 0x14);
ZOOSTERS_ASSERT_OFFSET_AT(CellStage_size, sizeof(CPB_TexPart::CellStage), 0x10);
ZOOSTERS_ASSERT_OFFSET_AT(CellCursor_size, sizeof(CPB_TexPart::CellCursor), 0x14);
ZOOSTERS_ASSERT_OFFSET(CPB_TexPart, m_cellStages, 0x29c);
ZOOSTERS_ASSERT_OFFSET(CPB_TexPart, m_texture, 0x2f4);
ZOOSTERS_ASSERT_OFFSET(CPB_TexPart, m_walkCells, 0x2f8);
ZOOSTERS_ASSERT_SIZE(CPB_TexPart, 0x320);

namespace {

// A full turn in radians as a float (bit pattern 0x40c90fdb).
const RwReal kFullTurn = 6.2831853f;

size_t BlockBytes(size_t recordSize, RwUInt32 count)
{
	return (recordSize * count + 15) & ~(size_t)15;
}

template <class Record> bool AllocTable(Record **&table, RwUInt32 count)
{
	if (count == 0) {
		table = nullptr;
		return true;
	}
	table = static_cast<Record **>(MemAllocHint(count * sizeof(Record *), kParticleMemType));
	if (!table)
		return false;
	memset(table, 0, count * sizeof(Record *));
	return true;
}

template <class Record> void SwapTable(Record **&current, Record **&pending)
{
	if (current)
		operator delete(current);
	current = pending;
	pending = nullptr;
}

template <class Record> void FreeTable(Record **table, RwUInt32 count)
{
	if (!table)
		return;
	for (RwUInt32 i = 0; i < count; ++i) {
		if (table[i])
			operator delete(table[i]);
	}
	operator delete(table);
}

RwUInt8 JitteredChannel(RwUInt32 base, RwUInt32 jitter, RwReal r)
{
	RwReal value = (RwReal)base + ((RwReal)jitter * ((r * 2.0f) - 1.0f));
	RwInt32 channel = (RwInt32)value;
	if (channel < 0)
		channel = 0;
	else if (channel > 255)
		channel = 255;
	return (RwUInt8)channel;
}

RwReal TargetChannel(RwUInt32 base, RwUInt32 jitter)
{
	RwReal value;
	if (jitter == 0)
		value = (RwReal)base;
	else
		value = (RwReal)base + (((RealRand() * 2.0f) - 1.0f) * (RwReal)jitter);
	if (value < 0.0f)
		value = 0.0f;
	else if (value > 255.0f)
		value = 255.0f;
	return value;
}

// The per-frame change of an 8.8 channel; the value is kept as its low 16 bits (a 0 -> 255 ramp over one frame wraps).
RwInt16 ChannelStep(RwReal target, RwUInt8 current, RwReal frames)
{
	return CPB_BasePart::Wrap16((RwInt32)(((target - (RwReal)current) / frames) * 256.0f));
}

RwReal FramesUntil(RwUInt32 age, RwUInt32 limit)
{
	return age >= limit ? 1.0f : (RwReal)(limit - age);
}

void Scatter(RwV3d *pos, const RwV3d &axis, RwReal extent)
{
	RwReal r = RealRand() - 0.5f;
	pos->x = pos->x + ((axis.x * extent) * r);
	pos->y = pos->y + ((axis.y * extent) * r);
	pos->z = pos->z + ((axis.z * extent) * r);
}

RwV3d Tilted(const RwV3d &from, const RwV3d &towards, RwReal range)
{
	RwReal angle = range * (RealRand() - 0.5f);
	RwReal c = RwCos(angle);
	RwReal s = RwSin(angle);
	RwV3d out;
	out.x = (from.x * c) + (towards.x * s);
	out.y = (from.y * c) + (towards.y * s);
	out.z = (from.z * c) + (towards.z * s);
	return out;
}

void SetTankBlend(RpAtomic *atomic, int srcBlend, int dstBlend)
{
	RpPTankExtOf(atomic)->vertexAlpha = 1;
	RpPTankExtOf(atomic)->srcBlend = srcBlend;
	RpPTankExtOf(atomic)->dstBlend = dstBlend;
	RpPTankExtOf(atomic)->instFlags |= 0x10000000;
}

} // namespace

void CPB_BasePart::SplitFrame::Deflect(RwV3d *out) const
{
	RwReal azimuth = RealRand() * kFullTurn;
	RwReal s = RwSin(azimuth);
	RwReal c = RwCos(azimuth);
	RwV3d around;
	around.x = (m_side.x * s) + (m_normal.x * c);
	around.y = (m_side.y * s) + (m_normal.y * c);
	around.z = (m_side.z * s) + (m_normal.z * c);
	RwReal away = (RealRand() * m_coneWidth) + m_coneMin;
	RwReal s2 = RwSin(away);
	RwReal c2 = RwCos(away);
	out->x = ((around.x * s2) + (m_heading.x * c2)) * m_length;
	out->y = ((around.y * s2) + (m_heading.y * c2)) * m_length;
	out->z = ((around.z * s2) + (m_heading.z * c2)) * m_length;
}

// m_life and the cursors are left unset, as in the original (SetLife always runs before a pool ticks).
// 0x44d0f0
CPB_BasePart::CPB_BasePart(RwUInt32 blockCount, RwUInt32 recordsPerBlock)
    : m_motionBlocks(nullptr), m_splitOn(false), m_emitting(true), m_ratePerFrame(0), m_rateJitter(0), m_scaledRate(0),
      m_scaledRateJitter(0), m_maxAge(0), m_lifeJitter(0), m_childAgeJitter(0), m_splitAge(1), m_childCount(0),
      m_scaledChildCount(0), m_childCountJitter(0), m_scaledChildCountJitter(0), m_blockCount(blockCount),
      m_recordsPerBlock(recordsPerBlock), m_tiltX(0.0f), m_tiltZ(0.0f), m_boxX(0.0f), m_boxY(0.0f), m_boxZ(0.0f), m_coneMin(0.0f),
      m_coneMax(0.0f), m_rateScale(1.0f)
{
	m_origin.x = 0.0f;
	m_origin.y = 0.0f;
	m_origin.z = 0.0f;
	m_rightAxis.x = 1.0f;
	m_rightAxis.y = 0.0f;
	m_rightAxis.z = 0.0f;
	m_upAxis.x = 0.0f;
	m_upAxis.y = 1.0f;
	m_upAxis.z = 0.0f;
	m_atAxis.x = 0.0f;
	m_atAxis.y = 0.0f;
	m_atAxis.z = 1.0f;
	memset(m_stages, 0, sizeof(m_stages));
	m_stages[STAGE_FROM].m_traits = kTraitSpeed | kTraitGravity | kTraitDrag | kTraitColour | kTraitSize;
	// Deliberately the block count where Resize expects a particle count (as the original; the game passes 0).
	Resize(blockCount);
}

// 0x44d220
// 0x44baa0
CPB_BasePart::~CPB_BasePart()
{
	FreeTable(m_motionBlocks, m_blockCount);
}

// Grows or shrinks the record tables to cover particleCount particles (through the virtual slots, so a CPB_TexPart resizes both
// of its tables).
// 0x44baf0
void CPB_BasePart::Resize(RwUInt32 particleCount)
{
	RwUInt32 newBlocks = particleCount / m_recordsPerBlock + (particleCount % m_recordsPerBlock != 0 ? 1 : 0);
	if (newBlocks == m_blockCount)
		return;
	RwUInt32 oldBlocks = m_blockCount;
	NewTable(newBlocks);
	RwUInt32 kept = newBlocks < oldBlocks ? newBlocks : oldBlocks;
	for (RwUInt32 i = 0; i < kept; ++i)
		KeepBlock(i);
	for (RwUInt32 i = newBlocks; i < oldBlocks; ++i)
		DropBlock(i);
	for (RwUInt32 i = oldBlocks; i < newBlocks; ++i)
		AddBlock(i);
	CommitTable();
	m_blockCount = newBlocks;
}

// 0x44bb90
bool CPB_BasePart::NewTable(RwUInt32 blockCount)
{
	return AllocTable(m_pendingMotionBlocks, blockCount);
}

// 0x44bbf0
void CPB_BasePart::KeepBlock(RwUInt32 i)
{
	m_pendingMotionBlocks[i] = m_motionBlocks[i];
}

// 0x44bc10
void CPB_BasePart::DropBlock(RwUInt32 i)
{
	operator delete(m_motionBlocks[i]);
}

// 0x44bc30
bool CPB_BasePart::AddBlock(RwUInt32 i)
{
	m_pendingMotionBlocks[i] = static_cast<Motion *>(MemAllocHint(BlockBytes(sizeof(Motion), m_recordsPerBlock), kParticleMemType));
	return m_pendingMotionBlocks[i] != nullptr;
}

// 0x44bc70
void CPB_BasePart::CommitTable()
{
	SwapTable(m_motionBlocks, m_pendingMotionBlocks);
}

void CPB_BasePart::ForgetTables()
{
	m_blockCount = 0;
	m_pendingMotionBlocks = nullptr;
	m_motionBlocks = nullptr;
	m_fill.m_free = 0;
	m_walk.m_free = 0;
}

// A "from" stage's values on one particle. unitDirection: the velocity already is a unit direction; replaceSpeed: the speed
// replaces the velocity instead of adding to it.
// 0x44bca0
void CPB_BasePart::ApplyFrom(const StageParams &stage, RwRGBA *colour, RwV2d *size, Motion *motion, bool unitDirection,
                             bool replaceSpeed)
{
	if (stage.m_traits & kTraitSpeed) {
		RwV3d dir;
		if (unitDirection) {
			dir = motion->m_move;
		} else {
			RwReal length = RwV3dLength(&motion->m_move);
			if (length > 0.0f)
				RwV3dScale(&dir, &motion->m_move, 1.0f / length);
			else
				dir = motion->m_move;
		}
		RwReal speed;
		if (stage.m_speedJitter == 0.0f)
			speed = stage.m_speedBase;
		else
			speed = stage.m_speedBase + (stage.m_speedJitter * RealRand());
		if (replaceSpeed) {
			motion->m_move.x = dir.x * speed;
			motion->m_move.y = dir.y * speed;
			motion->m_move.z = dir.z * speed;
		} else {
			motion->m_move.x = motion->m_move.x + (dir.x * speed);
			motion->m_move.y = motion->m_move.y + (dir.y * speed);
			motion->m_move.z = motion->m_move.z + (dir.z * speed);
		}
	}
	if (stage.m_traits & kTraitGravity)
		motion->m_fall = stage.m_fall;
	if (stage.m_traits & kTraitDrag)
		motion->m_damping = stage.m_damping;
	if (stage.m_traits & kTraitColour) {
		if (stage.m_redJitter == 0 && stage.m_greenJitter == 0 && stage.m_blueJitter == 0 && stage.m_alphaJitter == 0) {
			*colour = stage.m_colour;
		} else {
			// all four channels draw as soon as one has a jitter
			colour->red = JitteredChannel(stage.m_redBase, stage.m_redJitter, RealRand());
			colour->green = JitteredChannel(stage.m_greenBase, stage.m_greenJitter, RealRand());
			colour->blue = JitteredChannel(stage.m_blueBase, stage.m_blueJitter, RealRand());
			colour->alpha = JitteredChannel(stage.m_alphaBase, stage.m_alphaJitter, RealRand());
		}
	}
	if (stage.m_traits & kTraitSize) {
		size->x = stage.m_extent;
		size->y = stage.m_extent;
		if (stage.m_extentJitter != 0.0f) {
			RwReal r = RealRand();
			size->x = size->x + (stage.m_extentJitter * r);
			size->y = size->y + (stage.m_extentJitter * r);
		}
	}
}

// The per-frame changes that take one particle from its current values to a "to" stage's values in `frames` frames.
// 0x44bfc0
void CPB_BasePart::PlanTo(const StageParams &stage, const RwRGBA *colour, const RwV2d *size, Motion *motion, RwReal frames)
{
	motion->m_traits = stage.m_traits;
	if (stage.m_traits & kTraitSpeed) {
		if (stage.m_speedJitter != 0.0f)
			motion->m_speedGain = stage.m_speedBase + (RealRand() * stage.m_speedJitter);
		else
			motion->m_speedGain = stage.m_speedBase;
	}
	if (stage.m_traits & kTraitGravity)
		motion->m_fallStep = (stage.m_fall - motion->m_fall) / frames;
	if (stage.m_traits & kTraitDrag) {
		// Deliberately the current gravity, not the damping: the original's slip, kept because the gate compares against it.
		motion->m_dampingStep = (stage.m_damping - motion->m_fall) / frames;
	} else {
		motion->m_dampingStep = 0.0f;
	}
	if (stage.m_traits & kTraitColour) {
		RwReal red = TargetChannel(stage.m_redBase, stage.m_redJitter);
		RwReal green = TargetChannel(stage.m_greenBase, stage.m_greenJitter);
		RwReal blue = TargetChannel(stage.m_blueBase, stage.m_blueJitter);
		RwReal alpha = TargetChannel(stage.m_alphaBase, stage.m_alphaJitter);
		motion->m_redLevel = Wrap16(colour->red * 256);
		motion->m_greenLevel = Wrap16(colour->green * 256);
		motion->m_blueLevel = Wrap16(colour->blue * 256);
		motion->m_alphaLevel = Wrap16(colour->alpha * 256);
		motion->m_redStep = ChannelStep(red, colour->red, frames);
		motion->m_greenStep = ChannelStep(green, colour->green, frames);
		motion->m_blueStep = ChannelStep(blue, colour->blue, frames);
		motion->m_alphaStep = ChannelStep(alpha, colour->alpha, frames);
	}
	if (stage.m_traits & kTraitSize) {
		RwReal target;
		if (stage.m_extentJitter != 0.0f)
			target = stage.m_extent + (RealRand() * stage.m_extentJitter);
		else
			target = stage.m_extent;
		motion->m_sizeStep = (target - size->x) / frames;
	}
}

// A child of the particle under `parent`, in the child cursor's slot of the tank `childView` names: the parent's state (after its
// own split set-up) with a new direction.
// 0x44c400
void CPB_BasePart::CloneParent(SlotCursor &child, SlotCursor &parent, SplitFrame &frame, TankView &childView)
{
	const RwUInt32 slot = child.m_slot;
	RwUInt32 extraAge = 0;
	if (m_childAgeJitter != 0)
		extraAge = (RwUInt32)rand() % m_childAgeJitter;
	childView.m_ages[slot] = *parent.m_age + extraAge;
	child.Seek(childView, slot);

	RwV3d dir;
	frame.Deflect(&dir);

	*child.m_pos = *parent.m_pos;
	*child.m_colour = *parent.m_colour;
	*child.m_dims = *parent.m_dims;
	*child.m_motion = *parent.m_motion;
	child.m_motion->m_move = dir;
}

// 0x44c590
void CPB_BasePart::TurnForSplit(SlotCursor &cursor, SplitFrame &frame)
{
	frame.Deflect(&cursor.m_motion->m_move);
}

// A new particle in the cursor's slot: position in the emitter box, direction within the tilt ranges, a random age (how the life
// jitter works), then the "from" and "to" stages. Returns the frames until the "to" stage is reached.
// 0x44c660
RwReal CPB_BasePart::PlaceNew(TankView &view, SlotCursor &cursor)
{
	const RwUInt32 slot = cursor.m_slot;
	cursor.Seek(view, slot);
	RwV3d *pos = cursor.m_pos;
	*pos = m_origin;
	if (m_boxX != 0.0f)
		Scatter(pos, m_rightAxis, m_boxX);
	if (m_boxY != 0.0f)
		Scatter(pos, m_upAxis, m_boxY);
	if (m_boxZ != 0.0f)
		Scatter(pos, m_atAxis, m_boxZ);

	// tilt towards the right axis, then towards the at axis
	RwV3d dir = m_upAxis;
	if (m_tiltX != 0.0f)
		dir = Tilted(dir, m_rightAxis, m_tiltX);
	if (m_tiltZ != 0.0f)
		dir = Tilted(dir, m_atAxis, m_tiltZ);
	cursor.m_motion->m_move = dir;

	RwUInt32 age = 0;
	if (m_lifeJitter != 0)
		age = (RwUInt32)rand() % m_lifeJitter;
	if (m_splitOn && age >= m_splitAge)
		age = m_splitAge - 1; // every particle still reaches its split
	view.m_ages[slot] = age;

	const RwReal frames = FramesUntil(age, m_splitOn && m_splitAge <= m_maxAge ? m_splitAge : m_maxAge);
	EnterStages(STAGE_FROM, cursor, frames, true);
	return frames;
}

// 0x44caa0
CPB_TexPart::~CPB_TexPart()
{
	FreeTable(m_cellBlocks, m_blockCount);
}

void CPB_TexPart::ForgetTables()
{
	CPB_BasePart::ForgetTables();
	m_pendingCellBlocks = nullptr;
	m_cellBlocks = nullptr;
}

// 0x44cb70
void CPB_TexPart::PickStartCell(StageParams &stage, CellStage &cells, RwV2d *uvs, CellTrack *track)
{
	if (stage.m_traits & kTraitCell) {
		if (cells.m_firstCell != cells.m_lastCell) {
			RwUInt32 low = cells.m_firstCell < cells.m_lastCell ? cells.m_firstCell : cells.m_lastCell;
			RwUInt32 high = cells.m_firstCell < cells.m_lastCell ? cells.m_lastCell : cells.m_firstCell;
			track->m_cell = (RwReal)(low + (RwUInt32)rand() % ((high - low) + 1));
			ShowCell(uvs, (RwUInt32)track->m_cell);
		} else {
			track->m_cell = (RwReal)cells.m_firstCell;
			ShowCell(uvs, cells.m_firstCell);
		}
		track->m_startCell = track->m_cell;
	} else {
		track->m_startCell = RwFloor(track->m_cell);
		track->m_cell = track->m_startCell;
	}
}

// The cell animation towards the "to" stage's cell, with its loops.
// 0x44cc30
void CPB_TexPart::PlanCells(StageParams &stage, CellStage &cells, CellTrack *track, RwReal frames)
{
	if (!(stage.m_traits & kTraitCell))
		return;
	RwReal end;
	if (cells.m_firstCell != cells.m_lastCell) {
		// Deliberately unsorted and without + 1, unlike PickStartCell: the last cell is never chosen, and first > last gives a
		// huge modulus. The original's range, kept for exactness.
		end = (RwReal)(cells.m_firstCell + (RwUInt32)rand() % (cells.m_lastCell - cells.m_firstCell));
	} else {
		end = (RwReal)cells.m_firstCell;
	}
	track->m_endCell = end;
	RwReal loops;
	if (cells.m_loopJitter != 0.0f)
		loops = cells.m_loops + (RealRand() * cells.m_loopJitter);
	else
		loops = cells.m_loops;
	if (end >= track->m_cell) {
		track->m_step = ((((end - track->m_cell) + 1.0f) * (1.0f + loops)) - 0.001f) / frames;
		track->m_ascending = true;
	} else {
		track->m_step = ((((end - track->m_cell) - 1.0f) * (1.0f + loops)) + 0.001f) / frames;
		track->m_ascending = false;
		track->m_cell = track->m_cell + 0.999f;
	}
}

// 0x44cd30
bool CPB_TexPart::NewTable(RwUInt32 blockCount)
{
	if (!CPB_BasePart::NewTable(blockCount))
		return false;
	return AllocTable(m_pendingCellBlocks, blockCount);
}

// 0x44cda0
void CPB_TexPart::KeepBlock(RwUInt32 i)
{
	CPB_BasePart::KeepBlock(i);
	m_pendingCellBlocks[i] = m_cellBlocks[i];
}

// 0x44cdd0
void CPB_TexPart::DropBlock(RwUInt32 i)
{
	CPB_BasePart::DropBlock(i);
	operator delete(m_cellBlocks[i]);
}

// 0x44ce00
bool CPB_TexPart::AddBlock(RwUInt32 i)
{
	if (!CPB_BasePart::AddBlock(i))
		return false;
	m_pendingCellBlocks[i] = static_cast<CellTrack *>(MemAllocHint(BlockBytes(sizeof(CellTrack), m_recordsPerBlock), kParticleMemType));
	return m_pendingCellBlocks[i] != nullptr;
}

// 0x44ce80
void CPB_TexPart::CommitTable()
{
	CPB_BasePart::CommitTable();
	SwapTable(m_cellBlocks, m_pendingCellBlocks);
}

// 0x44cee0
void CPB_TexPart::CloneParentCell(CellCursor &child, CellCursor &parent, TankView &childView)
{
	child.Seek(childView.m_slot);
	child.m_uv[0] = parent.m_uv[0];
	child.m_uv[1] = parent.m_uv[1];
	*child.m_track = *parent.m_track;
}

// Slot 6 of both processor classes (CPB_TexPart does not override it): the tank atomic's render states.
// 0x44c3b0
void CPB_BasePart::ConfigureTank(RpAtomic *atomic, int srcBlend, int dstBlend)
{
	SetTankBlend(atomic, srcBlend, dstBlend);
}

// The texture-particle version: the same render states, then the processor's texture onto the tank's material. The fourth
// argument is not used (the pool passes its stencil flag).
// 0x44cb00
void CPB_TexPart::ConfigureTank(RpAtomic *atomic, int srcBlend, int dstBlend, bool)
{
	SetTankBlend(atomic, srcBlend, dstBlend);
	RpMaterialSetTexture(RpGeometryGetMaterial(RpAtomicGetGeometry(atomic), 0), m_texture);
}

}}
