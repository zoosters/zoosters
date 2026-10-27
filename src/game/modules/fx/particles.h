// Everything is public: the out-of-line copies take member addresses and GameParticle copies and patches members.
#ifndef ZOOSTERS_MODULES_FX_PARTICLES_H
#define ZOOSTERS_MODULES_FX_PARTICLES_H
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rwmacros.h"
#include "rw/rpplugins.h"
#include "framework/core/memoryhandler.h"
#include "framework/toolkits/math/maths.h"
#include "modules/fx/ptankext.h"

extern int g_ptankPluginOffset; // GLOBAL: 0x6308e8 g_ptankPluginOffset (RpPTankAtomicDataOffset)
static inline RpPTankExtView *RpPTankExtOf(RpAtomic *atomic)
{
	return *(RpPTankExtView **)((char *)atomic + g_ptankPluginOffset);
}

namespace RWS { namespace Particle {

// Trait bits of a stage and of a particle's motion record: which values a "from" stage sets, and which a "to" stage animates.
const RwUInt32 kTraitSpeed = 0x01; // speed along the direction of motion
const RwUInt32 kTraitGravity = 0x02;
const RwUInt32 kTraitDrag = 0x04;
const RwUInt32 kTraitColour = 0x08;
const RwUInt32 kTraitSize = 0x10;
const RwUInt32 kTraitCell = 0x20; // texture atlas cell (CPB_TexPart)

const RwUInt32 kTankSlots = 100;

const int kParticleMemType = 2;

struct TankView {
	void *m_tank;         // the pool's Tank
	void *m_nextTank;     // the Tank that followed it when it was opened: a walk goes on from there even if this one moves lists
	bool m_inBusyList;    // which list an all-tank walk is on
	RwUInt32 m_ordinal;   // the tank's place in the particle range
	RwUInt32 m_capacity;  // slots
	RwUInt32 m_liveCount; // live particles (this view's copy)
	RwUInt32 m_slot;      // the slot the next MarkLive / MarkDead acts on
	RwUInt32 *m_ages;
	bool *m_live;
	RpAtomic *m_atomic;
	RpPTankLockStruct m_sizes; // the size column, write-locked while the view is open
};

template <class T> class ParticlePool;

// The slot-level steps ParticlePool drives are deliberately non-virtual: the pool is a template over the concrete class, and CPB_TexPart
// replaces the ones it extends by hiding them.
class CPB_BasePart {
public:
	enum Stage { STAGE_FROM = 0, STAGE_TO = 1, STAGE_SPLIT_FROM = 2, STAGE_SPLIT_TO = 3, STAGE_COUNT = 4 };

	struct Motion {
		RwV3d m_move;     // units per frame
		RwUInt32 m_traits;    // the "to" stage's traits: which animations run
		RwReal m_speedGain;   // per frame
		RwReal m_fall;        // downward velocity change per frame
		RwReal m_fallStep;
		RwReal m_damping;     // velocity factor per frame
		RwReal m_dampingStep;
		RwInt16 m_redLevel, m_redStep; // colour channels in 8.8 fixed point and their per-frame change
		RwInt16 m_greenLevel, m_greenStep;
		RwInt16 m_blueLevel, m_blueStep;
		RwInt16 m_alphaLevel, m_alphaStep;
		RwReal m_sizeStep;
	};

	struct StageParams {
		RwUInt32 m_traits;
		RwUInt32 m_redBase, m_redJitter;
		RwUInt32 m_greenBase, m_greenJitter;
		RwUInt32 m_blueBase, m_blueJitter;
		RwUInt32 m_alphaBase, m_alphaJitter;
		RwReal m_speedBase, m_speedJitter;
		RwReal m_fall;
		RwReal m_damping; // stored as the per-frame velocity factor (1 - drag)
		RwReal m_extent, m_extentJitter;
		RwRGBA m_colour;
	};

	struct SlotCursor {
		SlotCursor() : m_free(0) {}

		void Open(TankView view, Motion **blocks)
		{
			m_free = view.m_capacity - view.m_liveCount;
			m_live = view.m_live;
			RpPTankAtomicLock(view.m_atomic, &m_positionLock, rpPTANKLFLAGPOSITION, rpPTANKLOCKWRITE);
			RpPTankAtomicLock(view.m_atomic, &m_colourLock, rpPTANKLFLAGCOLOR, rpPTANKLOCKWRITE);
			m_motion = blocks[view.m_ordinal];
			m_motions = blocks[view.m_ordinal];
			m_pos = reinterpret_cast<RwV3d *>(m_positionLock.data);
			m_dims = reinterpret_cast<RwV2d *>(view.m_sizes.data);
			m_colour = reinterpret_cast<RwRGBA *>(m_colourLock.data);
			m_age = view.m_ages;
			m_slot = 0;
		}
		void Advance(const TankView &view)
		{
			m_pos = reinterpret_cast<RwV3d *>(reinterpret_cast<RwUInt8 *>(m_pos) + m_positionLock.stride);
			m_dims = reinterpret_cast<RwV2d *>(reinterpret_cast<RwUInt8 *>(m_dims) + view.m_sizes.stride);
			m_colour = reinterpret_cast<RwRGBA *>(reinterpret_cast<RwUInt8 *>(m_colour) + m_colourLock.stride);
			++m_age;
			++m_motion;
			++m_slot;
		}
		void Seek(const TankView &view, RwUInt32 slot)
		{
			m_pos = reinterpret_cast<RwV3d *>(m_positionLock.data + m_positionLock.stride * slot);
			m_dims = reinterpret_cast<RwV2d *>(view.m_sizes.data + view.m_sizes.stride * slot);
			m_colour = reinterpret_cast<RwRGBA *>(m_colourLock.data + m_colourLock.stride * slot);
			m_motion = m_motions + slot;
		}
		// Moves to the next slot that is not live (one exists whenever this is called) and claims it.
		RwUInt32 Claim(const TankView &view)
		{
			while (*m_live)
				++m_live;
			m_slot = (RwUInt32)(m_live - view.m_live);
			--m_free;
			return m_slot;
		}

		bool *m_live;
		RwUInt32 m_free;
		RwUInt32 m_slot;
		RpPTankLockStruct m_positionLock;
		RpPTankLockStruct m_colourLock;
		RwV3d *m_pos;
		RwV2d *m_dims;
		RwRGBA *m_colour;
		RwUInt32 *m_age;
		Motion *m_motions; // the tank's block
		Motion *m_motion;  // the current slot's record
	};

	struct SplitFrame {
		bool Build(RwV3d *velocity, RwReal coneMin, RwReal coneMax)
		{
			m_coneMin = coneMin;
			m_coneWidth = coneMax - coneMin;
			m_length = RwV3dLength(velocity);
			if (m_length > 0.0f) {
				RwV3dScale(&m_heading, velocity, 1.0f / m_length);
				RwV3d helper;
				if (m_heading.y >= 0.5f || m_heading.y <= -0.5f) {
					helper.x = 1.0f;
					helper.y = 0.0f;
				} else {
					helper.x = 0.0f;
					helper.y = 1.0f;
				}
				helper.z = 0.0f;
				RwV3dCrossProduct(&m_side, &m_heading, &helper);
				RwV3dNormalize(&m_side, &m_side);
				RwV3dCrossProduct(&m_normal, &m_heading, &m_side);
				return true;
			}
			return false;
		}
		void Deflect(RwV3d *out) const; // particles.cpp

		RwV3d m_side;
		RwV3d m_heading;
		RwV3d m_normal;
		RwReal m_length;
		RwReal m_coneMin;
		RwReal m_coneWidth;
	};

	CPB_BasePart(RwUInt32 blockCount, RwUInt32 recordsPerBlock);

	// vtable order: slots 0-7. Slots 0-4 resize the per-tank record tables (Resize drives them).
	virtual bool NewTable(RwUInt32 blockCount);
	virtual void KeepBlock(RwUInt32 i);
	virtual void DropBlock(RwUInt32 i);
	virtual bool AddBlock(RwUInt32 i);
	virtual void CommitTable();
	virtual ~CPB_BasePart();
	virtual void ConfigureTank(RpAtomic *atomic, int srcBlend, int dstBlend);
	virtual RwUInt32 TankDataFlags() { return rpPTANKDFLAGPOSITION | rpPTANKDFLAGCOLOR; }

	void Resize(RwUInt32 particleCount);
	// After copying another processor: the copy must not share its record tables or open cursors.
	void ForgetTables();

	// configuration
	static RwReal ToRadians(RwReal degrees) { return degrees * 0.0174532924f; }   // pi / 180 rounded to float (0x3C8EFA35)
	RwUInt32 Scaled(RwUInt32 n) { return (RwUInt32)((RwReal)n * m_rateScale); }
	void SetRate(RwUInt32 n)
	{
		m_ratePerFrame = n;
		m_scaledRate = Scaled(m_ratePerFrame);
	}
	void SetRateScale(RwReal f)
	{
		m_rateScale = f;
		m_scaledRate = Scaled(m_ratePerFrame);
		m_scaledRateJitter = Scaled(m_rateJitter);
		m_scaledChildCount = Scaled(m_childCount);
		m_scaledChildCountJitter = Scaled(m_childCountJitter);
	}
	void Enable(bool on) { m_emitting = on; }
	bool IsEnabled() { return m_emitting; }
	// The game's addition (the pool calls it while the crane prompt shows); the rate jitter is deliberately left alone.
	void StopOutput()
	{
		m_scaledRate = 0;
		m_ratePerFrame = 0;
	}
	void SetRateJitter(RwUInt32 n)
	{
		m_rateJitter = n;
		m_scaledRateJitter = Scaled(n);
	}
	void SetLife(RwUInt32 n)
	{
		m_life = n;
		m_maxAge = m_life + m_lifeJitter;
	}
	void SetLifeJitter(RwUInt32 n)
	{
		m_lifeJitter = n;
		m_maxAge = m_life + m_lifeJitter;
	}
	void SetOrigin(RwV3d &p) { m_origin = p; }
	void SetUpAxis(RwV3d &v) { RwV3dNormalize(&m_upAxis, &v); }
	void SetRightAxis(RwV3d &v) { RwV3dNormalize(&m_rightAxis, &v); }
	void SetAtAxis(RwV3d &v) { RwV3dNormalize(&m_atAxis, &v); }
	void SetTiltX(RwReal degrees) { m_tiltX = ToRadians(degrees); }
	void SetTiltZ(RwReal degrees) { m_tiltZ = ToRadians(degrees); }
	void SetBoxX(RwReal f) { m_boxX = f; }
	void SetBoxY(RwReal f) { m_boxY = f; }
	void SetBoxZ(RwReal f) { m_boxZ = f; }
	void SetSpeed(Stage s, RwReal f) { m_stages[s].m_speedBase = f; }
	void SetSpeedJitter(Stage s, RwReal f) { m_stages[s].m_speedJitter = f; }
	void SetColor(Stage s, RwRGBA c)
	{
		m_stages[s].m_colour = c;
		m_stages[s].m_redBase = c.red;
		m_stages[s].m_greenBase = c.green;
		m_stages[s].m_blueBase = c.blue;
		m_stages[s].m_alphaBase = c.alpha;
	}
	void SetRedJitter(Stage s, RwUInt32 n) { m_stages[s].m_redJitter = n; }
	void SetGreenJitter(Stage s, RwUInt32 n) { m_stages[s].m_greenJitter = n; }
	void SetBlueJitter(Stage s, RwUInt32 n) { m_stages[s].m_blueJitter = n; }
	void SetAlphaJitter(Stage s, RwUInt32 n) { m_stages[s].m_alphaJitter = n; }
	void SetFall(Stage s, RwReal f) { m_stages[s].m_fall = f; }
	void SetResistance(Stage s, RwReal f) { m_stages[s].m_damping = 1.0f - f; }
	void SetExtent(Stage s, RwReal f) { m_stages[s].m_extent = f; }
	void SetExtentJitter(Stage s, RwReal f) { m_stages[s].m_extentJitter = f; }
	void AddTraits(Stage s, RwUInt32 bits) { m_stages[s].m_traits |= bits; }
	void RemoveTraits(Stage s, RwUInt32 bits) { m_stages[s].m_traits &= ~bits; }
	void EnableSplit(bool on) { m_splitOn = on; }
	void SetSplitAge(RwUInt32 n) { m_splitAge = n; }
	void SetChildAgeJitter(RwUInt32 n) { m_childAgeJitter = n; }
	void SetChildCount(RwUInt32 n)
	{
		m_childCount = n;
		m_scaledChildCount = Scaled(n);
	}
	void SetChildCountJitter(RwUInt32 n)
	{
		m_childCountJitter = n;
		m_scaledChildCountJitter = Scaled(n);
	}
	void SetConeMin(RwReal degrees) { m_coneMin = ToRadians(degrees); }
	void SetConeMax(RwReal degrees) { m_coneMax = ToRadians(degrees); }

	// per-frame motion
	// The 8.8 channels wrap at 16 bits on purpose (as the original's 16-bit arithmetic does).
	static RwInt16 Wrap16(int value) { return static_cast<RwInt16>(static_cast<RwUInt16>(value & 0xffff)); }
	static RwUInt8 ChannelByte(RwInt16 fixed) { return static_cast<RwUInt8>(static_cast<RwUInt16>(fixed) >> 8); }
	static RwInt16 StepChannel(RwInt16 level, RwInt16 step) { return Wrap16((int)level + (int)step); }
	void GainSpeed(RwV3d *velocity, Motion *motion)
	{
		if (motion->m_traits & kTraitSpeed) {
			RwReal length = RwV3dLength(velocity);
			if (length > 0.0f) {
				RwReal factor = (1.0f / length) * motion->m_speedGain;
				RwV3d gain;
				RwV3dScale(&gain, velocity, factor);
				RwV3dAdd(velocity, velocity, &gain);
			}
		}
	}
	// One frame of the particle under the walk cursor. Drag and gravity apply every frame whatever the traits; the traits only
	// gate the animations.
	void AnimateCurrent()
	{
		Motion *motion = m_walk.m_motion;
		RwV3dAdd(m_walk.m_pos, m_walk.m_pos, &motion->m_move);
		RwV3dScale(&motion->m_move, &motion->m_move, motion->m_damping);
		motion->m_move.y = motion->m_move.y - motion->m_fall;
		GainSpeed(&motion->m_move, motion);
		if (motion->m_traits & kTraitGravity)
			motion->m_fall = motion->m_fall + motion->m_fallStep;
		if (motion->m_traits & kTraitDrag)
			motion->m_damping = motion->m_damping + motion->m_dampingStep;
		if (motion->m_traits & kTraitColour) {
			RwRGBA *colour = m_walk.m_colour;
			motion->m_redLevel = StepChannel(motion->m_redLevel, motion->m_redStep);
			motion->m_greenLevel = StepChannel(motion->m_greenLevel, motion->m_greenStep);
			motion->m_blueLevel = StepChannel(motion->m_blueLevel, motion->m_blueStep);
			motion->m_alphaLevel = StepChannel(motion->m_alphaLevel, motion->m_alphaStep);
			colour->red = ChannelByte(motion->m_redLevel);
			colour->green = ChannelByte(motion->m_greenLevel);
			colour->blue = ChannelByte(motion->m_blueLevel);
			colour->alpha = ChannelByte(motion->m_alphaLevel);
		}
		if (motion->m_traits & kTraitSize) {
			RwV2d *size = m_walk.m_dims;
			size->x = size->x + motion->m_sizeStep;
			size->y = size->y + motion->m_sizeStep;
		}
	}

	// particle set-up (particles.cpp)
	void ApplyFrom(const StageParams &stage, RwRGBA *colour, RwV2d *size, Motion *motion, bool unitDirection, bool replaceSpeed);
	void PlanTo(const StageParams &stage, const RwRGBA *colour, const RwV2d *size, Motion *motion, RwReal frames);
	// A stage pair on the particle under `cursor`: `from` applied now, `from + 1` reached in `frames` frames.
	void EnterStages(Stage from, SlotCursor &cursor, RwReal frames, bool fresh)
	{
		ApplyFrom(m_stages[from], cursor.m_colour, cursor.m_dims, cursor.m_motion, fresh, fresh);
		PlanTo(m_stages[from + 1], cursor.m_colour, cursor.m_dims, cursor.m_motion, frames);
	}
	RwReal PlaceNew(TankView &view, SlotCursor &cursor);
	void CloneParent(SlotCursor &child, SlotCursor &parent, SplitFrame &frame, TankView &childView);
	void TurnForSplit(SlotCursor &cursor, SplitFrame &frame);

	// ---- the steps ParticlePool drives ----
	static RwUInt32 Jittered(RwUInt32 base, RwUInt32 jitter)
	{
		if (jitter != 0)
			return base + (RwUInt32)rand() % (jitter + 1);
		return base;
	}

	// Walking a tank. The walk reads the live flags from a snapshot, so particles added to the tank during the walk wait for the
	// next frame.
	void BeginTank(TankView &view)
	{
		m_walk.Open(view, m_motionBlocks);
		memcpy(m_liveSnapshot, view.m_live, view.m_capacity);
		m_walkLive = m_liveSnapshot;
	}
	bool CurrentLive() const { return *m_walkLive; }
	RwUInt32 CurrentSlot() const { return m_walk.m_slot; }
	// One frame older; true when the particle has just reached its split age.
	bool AgeCurrent()
	{
		++*m_walk.m_age;
		return m_splitOn && *m_walk.m_age == m_splitAge;
	}
	RwReal FramesToLive() const
	{
		RwUInt32 age = *m_walk.m_age;
		if (age >= m_maxAge)
			return 1.0f;
		return (RwReal)(m_maxAge - age);
	}
	void EnterSplit(RwReal frames)
	{
		if (m_coneMin != 0.0f || m_coneMax != 0.0f) {
			if (m_splitFrame.Build(&m_walk.m_motion->m_move, m_coneMin, m_coneMax))
				TurnForSplit(m_walk, m_splitFrame);
		}
		EnterStages(STAGE_SPLIT_FROM, m_walk, frames, false);
	}
	// The children the current particle spawns at its split (0: none); readies the fill cursor when there are any.
	RwUInt32 ChildrenToSpawn()
	{
		if (!m_emitting || (m_scaledChildCount == 0 && m_scaledChildCountJitter == 0))
			return 0;
		m_fill.m_free = 0;
		return Jittered(m_scaledChildCount, m_scaledChildCountJitter);
	}
	// True when the current particle has outlived its life; it is then dead in the snapshot and named in the view's slot.
	bool RetireIfExpired(TankView &view)
	{
		if (*m_walk.m_age < m_maxAge)
			return false;
		view.m_slot = m_walk.m_slot;
		*m_walkLive = false;
		return true;
	}
	void NextSlot(TankView &view)
	{
		++m_walkLive;
		m_walk.Advance(view);
	}

	// Filling a tank with new particles (each frame) and with children (at a split).
	RwUInt32 NewParticlesToSpawn()
	{
		if (!m_emitting || (m_scaledRate == 0 && m_scaledRateJitter == 0))
			return 0;
		m_fill.m_free = 0;
		return Jittered(m_scaledRate, m_scaledRateJitter);
	}
	void BeginFill(TankView &view) { m_fill.Open(view, m_motionBlocks); }
	RwUInt32 FillRoom() const { return m_fill.m_free; }
	void SpawnChild(TankView &view, RwReal frames)
	{
		view.m_slot = m_fill.Claim(view);
		CloneParent(m_fill, m_walk, m_splitFrame, view);
		EnterStages(STAGE_SPLIT_FROM, m_fill, frames, false);
	}
	RwReal SpawnParticle(TankView &view)
	{
		view.m_slot = m_fill.Claim(view);
		return PlaceNew(view, m_fill);
	}

	Motion **m_motionBlocks; // one Motion[m_recordsPerBlock] per tank
	StageParams m_stages[STAGE_COUNT];
	bool m_splitOn;
	bool m_emitting;
	RwUInt32 m_ratePerFrame; // new particles per frame (configured)
	RwUInt32 m_rateJitter;
	RwUInt32 m_scaledRate; // after the rate scale
	RwUInt32 m_scaledRateJitter;
	RwUInt32 m_maxAge;         // frames: life + life jitter
	RwUInt32 m_life;           // not set by the constructor (as in the original)
	RwUInt32 m_lifeJitter;
	RwUInt32 m_childAgeJitter; // random extra age of children
	RwUInt32 m_splitAge;       // age at which the split happens
	RwUInt32 m_childCount;     // children per particle (configured)
	RwUInt32 m_scaledChildCount;
	RwUInt32 m_childCountJitter;
	RwUInt32 m_scaledChildCountJitter;
	RwUInt32 m_blockCount;
	RwUInt32 m_recordsPerBlock;
	RwV3d m_origin; // emitter position and (unit) axes
	RwV3d m_upAxis;
	RwV3d m_rightAxis;
	RwV3d m_atAxis;
	RwReal m_tiltX; // radians
	RwReal m_tiltZ;
	RwReal m_boxX; // emitter box extent along each axis
	RwReal m_boxY;
	RwReal m_boxZ;
	RwReal m_coneMin; // radians
	RwReal m_coneMax;
	RwReal m_rateScale;
	Motion **m_pendingMotionBlocks; // scratch of the table resize
	SlotCursor m_walk;              // walk cursor (Tick)
	SlotCursor m_fill;              // fill cursor (new particles and children)
	bool *m_walkLive;               // the walk's place in the snapshot
	SplitFrame m_splitFrame;        // persistent and never initialised, as in the original (children may reuse it)
	bool m_liveSnapshot[kTankSlots];
};

class CPB_TexPart : public CPB_BasePart {
public:
	struct CellTrack {
		bool m_ascending;
		RwReal m_cell; // float cell number; the cell shown is its integer part
		RwReal m_step;
		RwReal m_endCell;
		RwReal m_startCell;
	};

	struct CellStage {
		RwUInt32 m_firstCell;
		RwUInt32 m_lastCell;
		RwReal m_loops;
		RwReal m_loopJitter;
	};

	struct CellCursor {
		void Open(TankView view, CellTrack **blocks)
		{
			RpPTankAtomicLock(view.m_atomic, &m_uvLock, rpPTANKLFLAGVTX2TEXCOORDS, rpPTANKLOCKWRITE);
			m_uv = reinterpret_cast<RwV2d *>(m_uvLock.data);
			m_track = blocks[view.m_ordinal];
			m_tracks = blocks[view.m_ordinal];
		}
		void Advance()
		{
			m_uv = reinterpret_cast<RwV2d *>(reinterpret_cast<RwUInt8 *>(m_uv) + m_uvLock.stride);
			++m_track;
		}
		void Seek(RwUInt32 slot)
		{
			m_uv = reinterpret_cast<RwV2d *>(m_uvLock.data + m_uvLock.stride * slot);
			m_track = m_tracks + slot;
		}

		RpPTankLockStruct m_uvLock;
		RwV2d *m_uv; // two per particle: top-left, bottom-right
		CellTrack *m_track;
		CellTrack *m_tracks;
	};

	CPB_TexPart(RwUInt32 blockCount, RwUInt32 recordsPerBlock)
	    : CPB_BasePart(blockCount, recordsPerBlock), m_cellBlocks(nullptr), m_cellCount(1), m_columns(1), m_cellWidth(1.0f),
	      m_cellHeight(1.0f), m_texture(nullptr)
	{
		memset(m_cellStages, 0, sizeof(m_cellStages));
		m_stages[STAGE_FROM].m_traits |= kTraitCell;
	}
	~CPB_TexPart();

	bool NewTable(RwUInt32 blockCount);
	void KeepBlock(RwUInt32 i);
	void DropBlock(RwUInt32 i);
	bool AddBlock(RwUInt32 i);
	void CommitTable();
	RwUInt32 TankDataFlags() { return rpPTANKDFLAGPOSITION | rpPTANKDFLAGCOLOR | rpPTANKDFLAGVTX2TEXCOORDS; }
	void ConfigureTank(RpAtomic *atomic, int srcBlend, int dstBlend, bool unused);
	void SetTexture(RwTexture *texture, ParticlePool<CPB_TexPart> &pool);
	void ForgetTables();

	void CloneParentCell(CellCursor &child, CellCursor &parent, TankView &childView);
	void PickStartCell(StageParams &stage, CellStage &cells, RwV2d *uvs, CellTrack *track);
	void PlanCells(StageParams &stage, CellStage &cells, CellTrack *track, RwReal frames);
	void EnterCellStages(Stage from, RwV2d *uvs, CellTrack *track, RwReal frames)
	{
		PickStartCell(m_stages[from], m_cellStages[from], uvs, track);
		PlanCells(m_stages[from + 1], m_cellStages[from + 1], track, frames);
	}

	void SetFirstCell(Stage s, RwUInt32 n) { m_cellStages[s].m_firstCell = n; }
	void SetLastCell(Stage s, RwUInt32 n) { m_cellStages[s].m_lastCell = n; }
	void SetCellLoops(Stage s, RwReal f) { m_cellStages[s].m_loops = f; }
	void SetCellLoopJitter(Stage s, RwReal f) { m_cellStages[s].m_loopJitter = f; }
	void SetAtlasGrid(RwUInt32 columns, RwUInt32 rows)
	{
		m_cellCount = columns * rows;
		m_columns = columns;
		m_cellWidth = 1.0f / (RwReal)columns;
		m_cellHeight = 1.0f / (RwReal)rows;
	}

	void ShowCell(RwV2d *uvs, RwUInt32 cell)
	{
		if (cell >= m_cellCount)
			cell = m_cellCount - 1;
		RwUInt32 row = cell / m_columns;
		RwUInt32 column = cell % m_columns;
		uvs[0].x = m_cellWidth * (RwReal)column;
		uvs[0].y = m_cellHeight * (RwReal)row;
		uvs[1].x = m_cellWidth * (RwReal)(column + 1);
		uvs[1].y = m_cellHeight * (RwReal)(row + 1);
	}
	void AnimateCell(RwV2d *uvs, Motion *motion, CellTrack *track)
	{
		if (motion->m_traits & kTraitCell) {
			RwUInt32 oldCell = (RwUInt32)track->m_cell;
			track->m_cell = track->m_cell + track->m_step;
			if (track->m_ascending) {
				if (track->m_cell >= track->m_endCell + 1.0f)
					track->m_cell = track->m_cell - ((track->m_endCell - track->m_startCell) + 1.0f);
			} else if (track->m_cell <= track->m_endCell - 1.0f) {
				track->m_cell = track->m_cell + ((track->m_startCell - track->m_endCell) + 1.999f);
			}
			RwUInt32 newCell = (RwUInt32)track->m_cell;
			if (newCell != oldCell)
				ShowCell(uvs, newCell);
		}
	}

	// the pool's steps: the base behaviour, then the cell part
	void BeginTank(TankView &view)
	{
		CPB_BasePart::BeginTank(view);
		m_walkCells.Open(view, m_cellBlocks);
	}
	void EnterSplit(RwReal frames)
	{
		CPB_BasePart::EnterSplit(frames);
		EnterCellStages(STAGE_SPLIT_FROM, m_walkCells.m_uv, m_walkCells.m_track, frames);
	}
	void BeginFill(TankView &view)
	{
		CPB_BasePart::BeginFill(view);
		m_fillCells.Open(view, m_cellBlocks);
	}
	void SpawnChild(TankView &view, RwReal frames)
	{
		CPB_BasePart::SpawnChild(view, frames);
		CloneParentCell(m_fillCells, m_walkCells, view);
		EnterCellStages(STAGE_SPLIT_FROM, m_fillCells.m_uv, m_fillCells.m_track, frames);
	}
	void AnimateCurrent()
	{
		CPB_BasePart::AnimateCurrent();
		AnimateCell(m_walkCells.m_uv, m_walk.m_motion, m_walkCells.m_track);
	}
	void NextSlot(TankView &view)
	{
		CPB_BasePart::NextSlot(view);
		m_walkCells.Advance();
	}
	void SpawnParticle(TankView &view)
	{
		RwReal frames = CPB_BasePart::SpawnParticle(view);
		m_fillCells.Seek(view.m_slot);
		EnterCellStages(STAGE_FROM, m_fillCells.m_uv, m_fillCells.m_track, frames);
	}

	CellStage m_cellStages[STAGE_COUNT];
	CellTrack **m_cellBlocks;
	CellTrack **m_pendingCellBlocks;
	RwUInt32 m_cellCount;
	RwUInt32 m_columns;
	RwReal m_cellWidth;
	RwReal m_cellHeight;
	RwTexture *m_texture;
	CellCursor m_walkCells; // walk cursor
	CellCursor m_fillCells; // fill cursor
};

}} // namespace RWS::Particle

// MarkDead reads the crane prompt flag; the Tank destructor uses FrameHelper::FrameDestroy.
extern unsigned char g_cranePromptShown; // GLOBAL: 0x617e7c g_cranePromptShown (set while the "Prompt_Crane_Button" sprite shows, big_sprite_415b.cpp)
namespace RWS { namespace FrameHelper { RwBool FrameDestroy(RwFrame *frame); } }

namespace RWS { namespace Particle {

// Tanks with live particles are on the busy list (the renderer draws it), empty ones on the idle list; a tank always enters a list at its head,
// which decides where new particles land and the walk order.
template <class T> class ParticlePool {
public:
	enum DrawMode { DRAW_NO_DEPTH_WRITE = 0, DRAW_DEPTH_WRITE = 1, DRAW_TWO_PASS = 2 };

	struct Tank {
		Tank(RwUInt32 capacity, RwUInt32 dataFlags, RwUInt32 ordinal);
		~Tank()
		{
			delete[] m_live;
			delete[] m_ages;
			RwFrame *frame = RpAtomicGetFrame(m_atomic);
			if (frame) {
				RpAtomicSetFrame(m_atomic, nullptr);
				RWS::FrameHelper::FrameDestroy(frame);
			}
			RpPTankAtomicDestroy(m_atomic);
		}

		// Takes the tank out of its list; its own links are left as they were.
		void Detach(Tank *&head)
		{
			if (head == this)
				head = m_next;
			if (m_next)
				m_next->m_prev = m_prev;
			if (m_prev)
				m_prev->m_next = m_next;
		}
		void PushFront(Tank *&head)
		{
			if (head)
				head->m_prev = this;
			m_prev = nullptr;
			m_next = head;
			head = this;
		}
		void Pin() { ++m_pins; }
		void Unpin()
		{
			if (--m_pins == 0)
				RpPTankAtomicUnlock(m_atomic);
		}

		RpAtomic *m_atomic;
		bool *m_live;     // one per slot
		RwUInt32 *m_ages; // in frames, one per slot
		Tank *m_next;
		Tank *m_prev;
		RwUInt32 m_capacity;  // slots
		RwUInt32 m_liveCount; // live particles
		RwUInt32 m_ordinal;   // tank k holds particles k * kTankSlots ...
		RwUInt32 m_pins;      // open views; the PTank is unlocked when the last closes
	};

	ParticlePool(RwUInt32 capacity, void *unused)
	    : m_idle(nullptr), m_busy(nullptr), m_drawMode(DRAW_NO_DEPTH_WRITE), m_blendSrc(rwBLENDSRCALPHA),
	      m_blendDst(rwBLENDINVSRCALPHA), m_stencilWrite(false)
	{
		// The tank count goes where T expects a particle count (as in the original); the game only builds empty pools.
		RwUInt32 tankCount = TankCountFor(capacity);
		m_proc = new (kParticleMemType) T(tankCount, kTankSlots);
		m_capacity = 0;
		Resize(capacity, unused);
	}
	~ParticlePool()
	{
		delete m_proc;
		DestroyList(m_idle);
		DestroyList(m_busy);
	}

	void Resize(RwUInt32 capacity, void *unused);
	T *Processor() { return m_proc; }
	void Tick();
	void Emit();

	// Every open view pins its tank; CloseTank (or moving on to the next tank) unpins it.
	bool OpenFirstBusy(TankView &view)
	{
		if (!m_busy)
			return false;
		OpenTank(m_busy, view);
		view.m_nextTank = m_busy->m_next;
		view.m_inBusyList = true;
		return true;
	}
	bool OpenNextBusy(TankView &view)
	{
		CloseTank(view);
		Tank *next = static_cast<Tank *>(view.m_nextTank);
		if (!next)
			return false;
		OpenTank(next, view);
		view.m_nextTank = next->m_next;
		return true;
	}
	bool OpenFirstAny(TankView &view)
	{
		if (m_busy)
			return OpenFirstBusy(view);
		if (!m_idle)
			return false;
		OpenTank(m_idle, view);
		view.m_nextTank = m_idle->m_next;
		view.m_inBusyList = false;
		return true;
	}
	bool OpenNextAny(TankView &view)
	{
		CloseTank(view);
		Tank *next = static_cast<Tank *>(view.m_nextTank);
		if (!next && view.m_inBusyList) {
			next = m_idle;
			view.m_inBusyList = false;
		}
		if (!next)
			return false;
		OpenTank(next, view);
		view.m_nextTank = next->m_next;
		return true;
	}
	template <class Visit> void ForEachBusyTank(Visit visit)
	{
		TankView view;
		if (!OpenFirstBusy(view))
			return;
		do {
			visit(view);
		} while (OpenNextBusy(view));
	}
	template <class Visit> void ForEachTank(Visit visit)
	{
		TankView view;
		if (!OpenFirstAny(view))
			return;
		do {
			visit(view);
		} while (OpenNextAny(view));
	}
	// Opens a tank with room: the first busy one that has some, else the idle head (moved to the busy list).
	bool OpenTankWithRoom(TankView &view)
	{
		for (Tank *tank = m_busy; tank; tank = tank->m_next) {
			if (tank->m_liveCount < tank->m_capacity) {
				OpenTank(tank, view);
				return true;
			}
		}
		if (!m_idle)
			return false;
		Tank *tank = m_idle;
		tank->Detach(m_idle);
		tank->PushFront(m_busy);
		OpenTank(tank, view);
		return true;
	}
	void CloseTank(TankView &view) { TankOf(view)->Unpin(); }

	void MarkLive(TankView &view)
	{
		Tank *tank = TankOf(view);
		tank->m_live[view.m_slot] = true;
		++view.m_liveCount;
		++tank->m_liveCount;
	}
	void MarkDead(TankView &view)
	{
		Tank *tank = TankOf(view);
		RwV2d *size = reinterpret_cast<RwV2d *>(view.m_sizes.data + view.m_sizes.stride * view.m_slot);
		size->x = 0.0f;
		size->y = 0.0f;
		tank->m_live[view.m_slot] = false;
		--view.m_liveCount;
		--tank->m_liveCount;
		if (tank->m_liveCount == 0) {
			tank->Detach(m_busy);
			tank->PushFront(m_idle);
			// The game's own addition: while the crane prompt shows, a system whose tank empties stops emitting.
			if (g_cranePromptShown)
				m_proc->StopOutput();
		}
	}

	void SetDrawMode(DrawMode mode) { m_drawMode = mode; }
	void SetBlendSrc(RwBlendFunction blend) { m_blendSrc = blend; }
	void SetBlendDst(RwBlendFunction blend) { m_blendDst = blend; }
	void SetStencilWrite(bool on) { m_stencilWrite = on; }

	static RwUInt32 TankCountFor(RwUInt32 particles) { return particles / kTankSlots + (particles % kTankSlots != 0 ? 1 : 0); }
	static Tank *TankOf(const TankView &view) { return static_cast<Tank *>(view.m_tank); }
	static void OpenTank(Tank *tank, TankView &view)
	{
		tank->Pin();
		RpPTankAtomicLock(tank->m_atomic, &view.m_sizes, rpPTANKLFLAGSIZE, rpPTANKLOCKWRITE);
		view.m_ordinal = tank->m_ordinal;
		view.m_tank = tank;
		view.m_liveCount = tank->m_liveCount;
		view.m_capacity = tank->m_capacity;
		view.m_live = tank->m_live;
		view.m_ages = tank->m_ages;
		view.m_atomic = tank->m_atomic;
	}
	static Tank *FindTank(Tank *head, RwUInt32 ordinal)
	{
		for (Tank *tank = head; tank; tank = tank->m_next) {
			if (tank->m_ordinal == ordinal)
				return tank;
		}
		return nullptr;
	}
	static void DestroyList(Tank *head)
	{
		while (head) {
			Tank *next = head->m_next;
			delete head;
			head = next;
		}
	}
	static void DestroyTanksFrom(Tank *&head, RwUInt32 firstGone)
	{
		for (Tank *tank = head; tank;) {
			Tank *next = tank->m_next;
			if (tank->m_ordinal >= firstGone) {
				tank->Detach(head);
				delete tank;
			}
			tank = next;
		}
	}
	void ConfigureTank(Tank *tank) { m_proc->ConfigureTank(tank->m_atomic, m_blendSrc, m_blendDst, m_stencilWrite); }
	void WalkTank(TankView &view);
	// Opens a tank with room whenever the current one is full and fills `count` slots through spawn(view); stops early when every tank is full.
	template <class Spawn> void Fill(RwUInt32 count, Spawn spawn);

	Tank *m_idle;
	Tank *m_busy; // the renderer draws this list
	T *m_proc;    // the processor (owned)
	RwUInt32 m_capacity;
	DrawMode m_drawMode;
	RwBlendFunction m_blendSrc;
	RwBlendFunction m_blendDst;
	bool m_stencilWrite; // the particle pass writes the stencil buffer
};

template <class T> void ParticlePool<T>::Resize(RwUInt32 capacity, void *)
{
	if (capacity == m_capacity)
		return;
	const RwUInt32 slots = kTankSlots;
	const RwUInt32 oldCapacity = m_capacity;
	const RwUInt32 dataFlags = m_proc->TankDataFlags();

	if (capacity < oldCapacity) {
		RwUInt32 kills = oldCapacity - capacity;
		RwUInt32 remainder = oldCapacity % slots;
		RwUInt32 wholeTanks = 0;
		if (kills > remainder && remainder != 0) {
			// the partly filled last tank goes entirely
			wholeTanks = 1;
			kills -= remainder;
		}
		wholeTanks += kills / slots;
		kills %= slots;
		const RwUInt32 firstGone = TankCountFor(oldCapacity) - wholeTanks;
		if (wholeTanks != 0) {
			DestroyTanksFrom(m_busy, firstGone);
			DestroyTanksFrom(m_idle, firstGone);
		}
		if (kills != 0) {
			// The cut falls inside tank firstGone - 1: replace it by a smaller, empty one (its particles are lost).
			Tank **list = &m_busy;
			Tank *cut = FindTank(m_busy, firstGone - 1);
			if (!cut) {
				list = &m_idle;
				cut = FindTank(m_idle, firstGone - 1);
			}
			if (cut) {
				cut->Detach(*list);
				RwUInt32 keep = cut->m_capacity - kills;
				if (keep != 0) {
					Tank *tank = new (kParticleMemType) Tank(keep, dataFlags, cut->m_ordinal);
					tank->PushFront(m_idle);
					ConfigureTank(tank);
				}
				delete cut;
			}
		}
	} else {
		const RwUInt32 room = slots - oldCapacity % slots;
		RwUInt32 extra = capacity - oldCapacity;
		RwUInt32 next = oldCapacity / slots;
		if (oldCapacity % slots != 0) {
			// The partly filled last tank is replaced by a larger, empty one (its particles are lost). Unlike shrinking, the
			// idle list is searched first.
			Tank **list = &m_idle;
			Tank *partial = FindTank(m_idle, next);
			if (!partial) {
				list = &m_busy;
				partial = FindTank(m_busy, next);
			}
			if (partial) {
				partial->Detach(*list);
				delete partial;
			}
			RwUInt32 size;
			if (extra <= room) {
				size = slots - (room - extra);
				extra = 0;
			} else {
				size = slots;
				extra -= room;
			}
			Tank *tank = new (kParticleMemType) Tank(size, dataFlags, next);
			tank->PushFront(m_idle);
			ConfigureTank(tank);
			++next;
		}
		while (extra != 0) {
			RwUInt32 size;
			if (extra > slots) {
				size = slots;
				extra -= slots;
			} else {
				size = extra;
				extra = 0;
			}
			// configured before linking here, the other way round above (as the original)
			Tank *tank = new (kParticleMemType) Tank(size, dataFlags, next);
			ConfigureTank(tank);
			tank->PushFront(m_idle);
			++next;
		}
	}
	m_proc->Resize(capacity);
	m_capacity = capacity;
}

template <class T> template <class Spawn> void ParticlePool<T>::Fill(RwUInt32 count, Spawn spawn)
{
	TankView view;
	bool open = false;
	for (RwUInt32 i = 0; i < count; ++i) {
		if (m_proc->FillRoom() == 0) {
			if (open)
				CloseTank(view);
			open = OpenTankWithRoom(view);
			if (!open)
				break;
			m_proc->BeginFill(view);
		}
		spawn(view);
		MarkLive(view);
	}
	if (open)
		CloseTank(view);
}

template <class T> void ParticlePool<T>::Emit()
{
	// the count is drawn before any tank is looked at, even when no tank has room
	Fill(m_proc->NewParticlesToSpawn(), [this](TankView &view) { m_proc->SpawnParticle(view); });
}

template <class T> void ParticlePool<T>::WalkTank(TankView &view)
{
	m_proc->BeginTank(view);
	RwUInt32 remaining = view.m_liveCount;
	RwUInt32 done = 0;
	while (done < remaining && m_proc->CurrentSlot() < view.m_capacity) {
		if (m_proc->CurrentLive()) {
			if (m_proc->AgeCurrent()) {
				RwReal frames = m_proc->FramesToLive();
				m_proc->EnterSplit(frames);
				Fill(m_proc->ChildrenToSpawn(), [this, frames](TankView &child) { m_proc->SpawnChild(child, frames); });
			}
			if (m_proc->RetireIfExpired(view)) {
				MarkDead(view);
				--remaining;
			}
		}
		if (m_proc->CurrentLive()) {
			m_proc->AnimateCurrent();
			++done;
		}
		m_proc->NextSlot(view);
	}
}

template <class T> void ParticlePool<T>::Tick()
{
	ForEachBusyTank([this](TankView &view) { WalkTank(view); });
}

}} // namespace RWS::Particle

#endif
