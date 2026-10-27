// The game's out-of-line RwV3d helpers of the Corpus unit (thiscall on the vector).
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"

struct RwV3dOps : public RwV3d {
	void Scale(float s);
	void AddScaled(float s, const RwV3d *v);
	void CrossProduct(const RwV3d *a, const RwV3d *b);
	void IncrementScaled(const RwV3d *a, float s, const RwV3d *v);
};

// 0x4227f0
void RwV3dOps::Scale(float s)
{
	x *= s;
	y *= s;
	z *= s;
}

// 0x422880
void RwV3dOps::AddScaled(float s, const RwV3d *v)
{
	x += s * v->x;
	y += s * v->y;
	z += s * v->z;
}

// 0x422810
void RwV3dOps::CrossProduct(const RwV3d *a, const RwV3d *b)
{
	x = (b->z * a->y) - (b->y * a->z);
	y = (a->z * b->x) - (a->x * b->z);
	z = (b->y * a->x) - (b->x * a->y);
}

// 0x422850
void RwV3dOps::IncrementScaled(const RwV3d *a, float s, const RwV3d *v)
{
	x = s * v->x + a->x;
	y = s * v->y + a->y;
	z = s * v->z + a->z;
}

