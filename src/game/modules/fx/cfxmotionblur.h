#ifndef ZOOSTERS_MODULES_FX_CFXMOTIONBLUR_H
#define ZOOSTERS_MODULES_FX_CFXMOTIONBLUR_H
#include "rw/rwcore.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
namespace RWS { namespace FX {

struct MotionBlurState {
	MotionBlurState();
	~MotionBlurState();
	float m_blurAmount;
	float m_offsetX;          // +0x04
	float m_offsetY;          // +0x08
	float m_scaleX;           // +0x0c
	float m_scaleY;           // +0x10
	float m_scale;            // +0x14
	float m_angle;            // +0x18
	int m_updateBlendPoly;    // dirty flag, set by every setter
	RwMatrix *m_matrix;       // +0x20
};
ZOOSTERS_ASSERT_SIZE(MotionBlurState, 0x24);

// PS2 base order: AE
class CFXMotionBlur : public CSystemCommands, public CAttributeHandler, public CEventHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(CFXMotionBlur);
	CFXMotionBlur(const CAttributePacket &attr);
	~CFXMotionBlur();
	virtual void HandleEvents(CMsg &msg) {}     // shared empty function
	virtual void HandleAttributes(const CAttributePacket &attr);

	MotionBlurState m_state;                            // +0x28 (public: the script members Expose* of "MOTION BLUR" use it)
protected:
	CEventId m_renderEvent;                             // the camera message the effect listens to
	int m_priority;                                     // link order of that message
};
ZOOSTERS_ASSERT_SIZE(CFXMotionBlur, 0x58);
ZOOSTERS_ASSERT_OFFSET(CFXMotionBlur, m_state, 0x28);

}}

// GLOBAL: 0x617cb0 g_motionBlur
extern RWS::FX::CFXMotionBlur *g_motionBlur;   // the live effect, 0 when there is none
#endif
