#ifndef ZOOSTERS_MODULES_TFB_CAMERADATA_H
#define ZOOSTERS_MODULES_TFB_CAMERADATA_H
#include <stddef.h>
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
namespace Script { class ScriptCamera; }
namespace RWS { namespace TFB {
// PC: the PS2 ScriptBuddy base is a separately allocated Script::ScriptCamera (vtable 0x5ccfec, 11 slots) behind a non-polymorphic base at +0x1c
// whose inline constructor allocates it (the game stores the CameraData vtable after the allocation).
class CameraLink {
public:
	__forceinline CameraLink();
protected:
	::Script::ScriptCamera *m_cam;                       // +0x1c the ScriptCamera, allocated and linked by the constructor
};
class CameraData : public CAttributeHandler, public CameraLink, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(CameraData);
	CameraData(const CAttributePacket &attr);
	~CameraData();
	virtual void HandleAttributes(const CAttributePacket &attr);
	virtual void HandleSystemCommands(const CAttributePacket &attr);      // slot 4
	// m_cam sits at 0x1c with 4-byte pointers, at 0x28 with 8-byte ones (the base CAttributeHandler grows)
	static CameraData *FromCamSlot(void *slot) { return (CameraData *)((char *)slot - offsetof(CameraData, m_cam)); }
	intptr_t GetAnimation() const { return m_animation; }
#ifdef ZOOSTERS_DEBUGUI   // port debug overlay only (port/debugui/debugui_camtour.cpp)
	::Script::ScriptCamera *DbgCamera() const { return m_cam; }
	const char *DbgLookFrom() const { return m_lookFromName; }
	const char *DbgLookAt() const { return m_lookAtName; }
#endif
protected:
	const char *m_lookFromName;                              // +0x20 interned look-from actor name of command 1 ("(Current)" clears it)
	const char *m_lookAtName;                                // +0x24 interned look-at actor name of command 4
	intptr_t m_animation;                                   // +0x28 the HANIMANIMATION resource of system command 0 (HandleSystemCommands)
};
ZOOSTERS_ASSERT_SIZE(CameraData, 0x2c);
}}
#ifdef ZOOSTERS_DEBUGUI
void port_dbg_camera_data(RWS::TFB::CameraData *data, int alive);   // port/debugui/debugui_camtour.cpp
#endif
#endif
