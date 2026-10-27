#ifndef ZOOSTERS_RWS_TFBSHADOWCAMERA_H
#define ZOOSTERS_RWS_TFBSHADOWCAMERA_H
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "modules/fx/cfxbaselight.h"   // RWS::CAtomicPtr
// The game moves float attribute values with integer instructions (a float local would add x87 loads and stores): the value is an
// int word until it is stored through an int view of the float member.
static inline void SetBits(float &dst, int bits)
{
	*(int *)&dst = bits;
}

namespace RWS { namespace TFB {
// PS2 base order: EA
class TFBShadowCamera : public CSystemCommands, public CEventHandler, public CAttributeHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(TFBShadowCamera);
	TFBShadowCamera(const CAttributePacket &attr);
	~TFBShadowCamera();
	virtual void HandleEvents(CMsg &msg);
	virtual void HandleAttributes(const CAttributePacket &attr);
	void HandleSystemCommands(const CAttributePacket &attr);   // (system commands of the packet, called first by HandleAttributes)
	// the members are public: the shadow code and the script accessors (Script::ScriptShadow::Expose*) read and write the settings
	bool m_enabled;                     // +0x25 set to 1 by the constructor and by m_evtEnable, cleared by m_evtDisable
	CEventId m_evtLookAt;               // +0x28 "look_at_point" ("RwFrame*"): turns the camera atomic's frame by m_pitch and m_heading
	CEventId m_evtEnable;               // +0x30 (name set by attribute command 1)
	CEventId m_evtDisable;              // +0x38 (name set by attribute command 2)
	CEventId m_evtCamera;               // +0x40 "3rd_person_cam" ("RwCamera*"), linked with priority m_cameraPriority
	unsigned short m_cameraPriority;    // +0x48 attribute command 5
	char m_4a[2];
	CEventId m_unk4c;  // +0x4c constructed (name 0) by the constructor; never used
	float m_areaRadius;                 // +0x54 attribute 6, script "area radius" (feet * 12); 400.0 by default
	float m_pitch;                      // +0x58 attribute 7: rotation about X in degrees (HandleEvents, ShadowRender); 75.0 by default
	float m_heading;                    // +0x5c attribute 8: rotation about Y in degrees; 0.0 by default
	float m_verticalOffset;             // +0x60 attribute 9, script "vertical offset" (feet * 12); 500.0 by default
	RWS::CAtomicPtr m_atomic;           // +0x64 the camera's atomic (CreateAtomicInWorldFromResource); its flags 0x20 set by the constructor
	char m_68[4];
};
ZOOSTERS_ASSERT_SIZE(TFBShadowCamera, 0x6c);
ZOOSTERS_ASSERT_OFFSET(TFBShadowCamera, m_areaRadius, 0x54);
ZOOSTERS_ASSERT_OFFSET(TFBShadowCamera, m_verticalOffset, 0x60);
ZOOSTERS_ASSERT_OFFSET(TFBShadowCamera, m_atomic, 0x64);
}}

extern RWS::TFB::TFBShadowCamera *g_shadowCamera;   // the shadow camera behaviour (0 while there is none)
#endif
