// PS2 RTTI: bases CEventHandler@0, CAttributeHandler@8, CSystemCommands@36 (0x24), LinearAllocationPolicy@40.
#ifndef ZOOSTERS_TFB_CTFBWORLD_H
#define ZOOSTERS_TFB_CTFBWORLD_H

#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "rw/rpworld.h"

struct YonBox;   // g/yonbox.h

namespace RWS { namespace TFB {

// Vtables 0x5ccb5c (the CEventHandler part: ~ 0x41c3e0, HandleEvents 0x41c310) and 0x5ccb4c (the CAttributeHandler part at +8:
// thunk 0x41b140, HandleAttributes 0x41b640, base, base). Registered size 0x58 (RegisterFactory 0x5c5b60).
class CTFBWorld : public CEventHandler, public CAttributeHandler, public CSystemCommands, public LinearAllocationPolicy {
public:
	// The walk of a world's sectors in a world pass: RenderSectorsPVS when the opaque world has PVS data, else RenderSectors.
	typedef void (CTFBWorld::*SectorWalk)(RwCamera *camera, RpWorld *world);

	RWS_MAKENEWCLASS(CTFBWorld);
	CTFBWorld(const CAttributePacket &attr);
	~CTFBWorld();
	virtual void HandleEvents(CMsg &msg);
	virtual void HandleAttributes(const CAttributePacket &attr);

	void RenderFrame(RwCamera *camera);
	// Resource type 4 of command 0: a clump resource gets the TFB atomic flags, a frame if it has none, and goes into the level.
	void AddClumpResource(RpClump *clump, const char *resourceType);
	// Sector walks: render every sector of the camera's frustum list; the PVS walk first sets the PVS view position.
	void RenderSectorsPVS(RwCamera *camera, RpWorld *world);
	void RenderSectors(RwCamera *camera, RpWorld *world);
	// Queues the atomics of a sector for the render queue (the body does not read `this`; the caller passes the world in ecx).
	void QueueSectorAtomics(RpWorldSector *sector);
	void RenderTransparentPass(RwCamera *camera);   // the translucent world, back to front, after the opaque pass
	void RenderSkyPass(RwCamera *camera);
	void RenderFogPass(RwCamera *camera);
	void RenderWorldPass(RwCamera *camera, RpWorld *world);   // one world front to back; the opaque one also draws the yon box and shadows
	void FlushRenderQueue();

	// +0x28 (the two empty bases CSystemCommands + LinearAllocationPolicy: members start at sizeof(bases) + 4)
	SectorWalk m_renderSectors;       // +0x28 (8 bytes: member function pointer set by command 0 of type 0)
	RwRGBA m_clearColor;              // +0x30 camera clear colour of the sky pass (the constructor clears r, g and b)
	CEventId m_renderEvent;           // +0x34 the camera event it renders for (command 4: name, command 7: priority)
	CEventId m_afterOpaqueEvent;      // +0x3c sent by RenderFrame after the opaque world passes (command 5)
	CEventId m_afterRenderEvent;      // +0x44 sent after the frame with the same camera (command 6)
	int m_unk4c;  // +0x4c attribute command 12 (1 after the constructor); never read
	YonBox *m_yonBox;                 // +0x50 the distant geometry (built by FileAtomicInLevelWorld), freed by the destructor
	unsigned char m_yonIgnore;        // +0x54 script ScriptSystem.RenderYonIgnore (through g_yonObject); 1 after the constructor; gates RenderFogPass
	char m_pad55[3];
};
ZOOSTERS_ASSERT_SIZE(CTFBWorld, 0x58);
ZOOSTERS_ASSERT_OFFSET(CTFBWorld, m_renderSectors, 0x28);
ZOOSTERS_ASSERT_OFFSET(CTFBWorld, m_clearColor, 0x30);
ZOOSTERS_ASSERT_OFFSET(CTFBWorld, m_renderEvent, 0x34);
ZOOSTERS_ASSERT_OFFSET(CTFBWorld, m_yonBox, 0x50);
ZOOSTERS_ASSERT_OFFSET(CTFBWorld, m_yonIgnore, 0x54);

// This module's copy of ERROR_NOT_A_VALID_MESSAGE_ID (the id that is never sent).
extern const CEventId g_noMessageId;

static inline void SendLinkedMessage(const CEventId &id, void *data)
{
	CMsg msg(id, data);
	if (msg.Id != g_noMessageId.pEventId && msg.Id->Erm.p_next)
		_SendMsg(msg);
}

}}

// The behaviour the script member RenderYonIgnore reads (the one CTFBWorld; set by its constructor).
extern RWS::TFB::CTFBWorld *g_yonObject;

// The frame of a camera as a direct field read: RwCameraGetFrame (an inline function in rw/rwcore.h) changes the register allocation of
// these passes (RenderTransparentPass, RenderSkyPass, ... read eax/edx in a different order).
#define CameraFrame(camera) RwCameraGetFrame(camera)   // the RwObject parent: +4 with 4-byte pointers, +8 with 8-byte ones

#endif
