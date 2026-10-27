// TFB atomic plugin 0x800000FD ("TFBATOMIC"): per-atomic colour, fade, draw flags and billboard frame, with the atomic render callback.
// The plugin API does nothing and reports "not applied" (0, true for IsVisible) while the plugin is not registered (g_tfbFdOfs <= 0).
#ifndef ZOOSTERS_TFB_RENDER_TFBATOMIC_H
#define ZOOSTERS_TFB_RENDER_TFBATOMIC_H

#include "rw/rpworld.h"
#include "tfb/render/tfbplugins.h"

// TFBAtomicData::m_flags; the names are ours.
enum TFBAtomicFlag {
	TFB_ATOMIC_LIT = 0x4,              // RenderCallback: rpGEOMETRYLIGHT on the geometry; set by the constructor
	TFB_ATOMIC_FLAG_08 = 0x8,          // Update: an atomic without it that is opaque and unfaded needs no draw (returns 0)
	TFB_ATOMIC_HAS_COLOR = 0x80,       // the stream carries a colour (format 3 and 4 readers)
	TFB_ATOMIC_FADE_RANGE = 0x200,     // Update: fades out between m_fadeFrom and m_fadeTo
	TFB_ATOMIC_NO_DISTANCE_DIM = 0x1000, // Update: without it the atomic is dimmed (x0.3) in the far zone
	TFB_ATOMIC_SORT_TRIANGLES = 0x2000,  // set after the stream read: RenderCallback re-sorts the triangles by time
	TFB_ATOMIC_HIDDEN = 0x8000,        // IsVisible: not drawn
	TFB_ATOMIC_NO_ZTEST = 0x10000      // RenderCallback: drawn without the z test
};

// TFBAtomicData::m_billboardFlags (stream formats 3 and 4).
enum TFBBillboardFlag {
	TFB_BILLBOARD_ON = 0x1,            // the atomic's frame is rebuilt each frame to face the camera (keeps m_matrix as its base)
	TFB_BILLBOARD_RESET_ROTATION = 0x2, // the stream reader replaces the rotation of m_matrix by the identity
	TFB_BILLBOARD_FIXED_UP = 0x4,      // keep one up axis, turn around it
	TFB_BILLBOARD_FACE_POSITION = 0x8, // face the camera's position (else the camera's viewing direction)
	TFB_BILLBOARD_UP_FROM_PARENT = 0x10 // the fixed up axis is the up vector of the parent frame's rotation
};

struct TFBAtomicData {
	int m_version;               // stream format id 0x0F0F0002..4; g_tfbFdDefault0 after the constructor
	union {
		RwRGBA m_color;          // RenderCallback takes the alpha (the top byte) and fades it
		RwUInt32 m_colorBits;
	};
	unsigned m_flags;            // TFBAtomicFlag
	int m_transformFlags;        // the frame's user data TRANSFORM_FLAGS
	int m_billboardFlags;        // TFBBillboardFlag
	float m_fadeFrom;            // view distances of the fade range (stream format 4; TFBPlugin_FD_SetRange)
	float m_fadeTo;
	unsigned m_createTime;       // game clock (TFBClock_GetRawTime, ms) when the data was constructed
	RwMatrix *m_matrix;          // the billboard's base matrix (a copy of the frame's modelling matrix), 0 without one
	union {
		float m_fade;            // smoothed fade factor (1.0 = opaque; FLT_MAX = not set yet)
		int m_fadeBits;          // the game compares and stores the two special values as integers
	};
	float m_viewDistance;        // the view distance the last Update was called with
};
ZOOSTERS_ASSERT_SIZE(TFBAtomicData, 0x2c);
ZOOSTERS_ASSERT_OFFSET(TFBAtomicData, m_color, 0x04);
ZOOSTERS_ASSERT_OFFSET(TFBAtomicData, m_flags, 0x08);
ZOOSTERS_ASSERT_OFFSET(TFBAtomicData, m_billboardFlags, 0x10);
ZOOSTERS_ASSERT_OFFSET(TFBAtomicData, m_createTime, 0x1c);
ZOOSTERS_ASSERT_OFFSET(TFBAtomicData, m_matrix, 0x20);
ZOOSTERS_ASSERT_OFFSET(TFBAtomicData, m_viewDistance, 0x28);

extern int g_tfbFdEngOfs;  // engine plugin offset (holds "TFBATOMIC")
extern int g_tfbFdOfs;  // object plugin offset inside an RpAtomic (<= 0: not registered)

static inline TFBAtomicData *TFBAtomicGetData(RpAtomic *atomic)
{
	return (TFBAtomicData *)((char *)atomic + g_tfbFdOfs);
}
static inline const TFBAtomicData *TFBAtomicGetData(const RpAtomic *atomic)
{
	return (const TFBAtomicData *)((const char *)atomic + g_tfbFdOfs);
}

// Plugin API (cdecl).
unsigned TFBPlugin_FD_GetFlags(RpAtomic *atomic);
int TFBPlugin_FD_SetFlagBits(RpAtomic *atomic, unsigned bits);  // flags |= bits; 1 when applied
int TFBPlugin_FD_ClearFlagBits(RpAtomic *atomic, unsigned bits);  // flags &= ~bits
int TFBPlugin_FD_SetFlags(RpAtomic *atomic, unsigned bits);  // flags = bits
unsigned TFBPlugin_FD_TestFlags(RpAtomic *atomic, unsigned bits);  // flags & bits
int TFBPlugin_FD_SetTransformFlags(RpAtomic *atomic, int transformFlags);
void TFBPlugin_FD_SetRange(RpAtomic *atomic, float from, float to);
bool TFBPlugin_FD_IsVisible(RpAtomic *atomic);  // not hidden, alpha and fade above the minimum
RwBool TFBPlugin_FD_Attach();

// The atomic renderer: the render callback every atomic gets, the per-frame update of the visibility pass, and the billboard frame.
class TFBAtomic {
public:
	// Draws the atomic through its pipeline with the plugin's colour and fade, updating its materials first.
	static RpAtomic *RenderCallback(RpAtomic *atomic);
	// Per-frame fade update at view distance `viewDistance` from `camera`; returns 0 when the atomic needs no draw.
	static int Update(RpAtomic *atomic, RwCamera *camera, float viewDistance);
	// Rebuilds the atomic's frame so that it faces `camera`.
	static void UpdateBillboardFrame(RpAtomic *atomic, RwCamera *camera);
};

#endif
