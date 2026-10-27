// Maths helpers of the particle code. Includers rely on it for <stdlib.h> and rw/rwcore.h.
#ifndef ZOOSTERS_RWS_MATHS_H
#define ZOOSTERS_RWS_MATHS_H
#include <stdlib.h>
#include "rw/rwcore.h"

namespace RWS {

const RwReal TWO_PI = 6.2831853f;

// The divisor is the game's generator maximum (0x7fff), not the host's RAND_MAX.
inline RwReal RealRand()
{
	return (RwReal)rand() / 32767.0f;
}

// A particle spray's emitter transform. No constructor on purpose: CFXPartSpray's constructor leaves it alone, and every spray
// sets the offset and angles before use.
class FrameTrans {
public:
	void SetOffset(const RwV3d &offset)
	{
		m_offset = offset;
		dirty = true;
	}
	void SetFromAngles(RwReal deflect, RwReal heading);
	void Recalc();

	RwMatrix m_transMtx; // must stay first: CFXPartSpray::HandleEvents reads the object as this matrix
	bool dirty;
	RwV3d m_offset;
	RwV3d m_upVect;
	RwV3d m_rightVect;
	RwV3d m_forwardVect;
};

} // namespace RWS

#endif
