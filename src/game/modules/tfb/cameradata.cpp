// The constructor makes cl emit the vtable and the scalar deleting destructor. It inlines Script::ScriptCamera's constructor, so that
// class's vtable (0x5ccfec) and
// scalar deleting destructor (0x41d890) are emitted here too.
#include "modules/tfb/cameradata.h"
#include "script/actor.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptfog.h"
#include "script/types/scriptcamera.h"
// NOVTABLE: Script::ScriptCameraBase
namespace RWS { namespace TFB {
// GLOBAL: 0x5ccfd8 RWS::TFB::CameraData::vftable
// 0x41df50
__forceinline CameraLink::CameraLink() { m_cam = new ::Script::ScriptCamera; }

// Near-match of 0x41d7c0 (203 B, the game's size): the game addresses the pointer member through esi where cl keeps the adjusted base `this` in ebx
// (one extra push/pop, three loads reordered). Not annotated; acts as the scaffold.
CameraData::CameraData(const CAttributePacket &) : m_lookFromName(0), m_lookAtName(0), m_animation(0)
{
	m_cam->m_ownerSlot = &m_cam;
	m_cam->AdjustRefCount(1);
#ifdef ZOOSTERS_DEBUGUI   // port debug overlay (camera tour / cut log)
	port_dbg_camera_data(this, 1);
#endif
}
}}
// GLOBAL: 0x5ccfec Script::ScriptCamera::vftable
// 0x41d890
// GLOBAL: 0x41d8b0 ??1ScriptCamera@Script@@UAE@XZ
namespace RWS { namespace TFB {
// 0x41d430
CameraData::~CameraData()
{
#ifdef ZOOSTERS_DEBUGUI
	port_dbg_camera_data(this, 0);
#endif
	m_cam->m_ownerSlot = 0;
	m_cam->AdjustRefCount(-1);
}
}}
