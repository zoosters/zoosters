// vtable TU: the constructor makes cl emit the vtables, the scalar deleting destructor and the thunk
#include <stddef.h>
#include "modules/tfb/cprotoactor.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
namespace RWS { namespace TFB {
// GLOBAL: 0x5cdbbc RWS::TFB::CProtoActor::vftable
// 0x42f230
// GLOBAL: 0x42e5c0 ??1CProtoActor@TFB@RWS@@UAE@XZ
// 0x42dfa0
CProtoActor::CProtoActor(const CAttributePacket &)
{
	m_frame.at.z = 1.0f;
	m_frame.up.y = 1.0f;
	m_frame.right.x = 1.0f;
	m_scriptAsset = -1;
	m_linkedHandler = 0;
	m_wayPoints = 0;
	m_numWayPoints = 0;
	m_spline = 0;
	m_frame.up.x = 0;
	m_frame.right.z = 0;
	m_frame.right.y = 0;
	m_frame.at.y = 0;
	m_frame.at.x = 0;
	m_frame.up.z = 0;
	m_frame.pos.z = 0;
	m_frame.pos.y = 0;
	m_frame.pos.x = 0;
	m_frame.flags = (m_frame.flags & 0xfffdffff) | 3;
	m_actor = new GameActor(this);
	m_actor->AdjustRefCount(1);
	m_scriptGuid[0] = 0;
	m_scriptGuid[1] = 0;
	m_scriptGuid[2] = 0;
	m_scriptGuid[3] = 0;
}
}}
