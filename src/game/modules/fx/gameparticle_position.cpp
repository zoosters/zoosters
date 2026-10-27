#include "modules/fx/gameparticle.h"

// Codegen: the second and third copy of *pos go into pre-declared locals, the products are parenthesised and the frame pointer is
// reloaded into a local for RwFrameTranslate.
namespace RWS { namespace FX {

// 0x40a950
void GameParticle::SetVector(const Vec3 *v, Script::ScriptReference *ownerRef)
{
	GameParticle *owner = static_cast<GameParticle *>(ownerRef);
	const RwV3d *pos = (const RwV3d *)v;   // Vec3 = RwV3d layout
	RwV3d q;
	RwV3d delta;
	RwV3d p = *pos;
	delta.x = (p.x - owner->m_frame->modelling.pos.x);
	q = *pos;
	RwV3d r;
	delta.y = (q.y - owner->m_frame->modelling.pos.y);
	r = *pos;
	delta.z = (r.z - owner->m_frame->modelling.pos.z);
	RwFrame *frame = owner->m_frame;
	RwFrameTranslate(frame, &delta, rwCOMBINEPOSTCONCAT);
}
}}
