#include "modules/fx/cfxmotionblur.h"

// GLOBAL: 0x617cb0 g_motionBlur (declared in the header)

// The game copies the attribute's dword into the float as an integer move; a float assignment goes through the x87 stack.
static void CopyRawBits(float &dest, int bits)
{
	*reinterpret_cast<int *>(&dest) = bits;
}

// GLOBAL: 0x602954 g_cfxMotionBlurClassId
extern const char *g_cfxMotionBlurClassId;

namespace RWS { namespace FX {

// GLOBAL: 0x5cb5a4 RWS::FX::CFXMotionBlur::vftable_for_CAttributeHandler
// GLOBAL: 0x5cb59c RWS::FX::CFXMotionBlur::vftable_for_CEventHandler
// 0x4110b0
// 0x410e00
// GLOBAL: 0x550250 ??0CEventId@RWS@@QAE@XZ
// GLOBAL: 0x464ef0 ??1CEventId@RWS@@QAE@XZ
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x5506d0 ?UnRegisterMsg@CEventHandler@RWS@@SAXAAVCEventId@2@@Z

// 0x411040
CAttributeHandler *(*g_keepMakeNew_CFXMotionBlur)(const CAttributePacket &) = &CFXMotionBlur::MakeNew;

// 0x410e10
CFXMotionBlur::CFXMotionBlur(const CAttributePacket &)
{
	g_motionBlur = this;
}

// 0x410ba0
CFXMotionBlur::~CFXMotionBlur()
{
	g_motionBlur = 0;
	UnLinkMsg(m_renderEvent);
	UnRegisterMsg(m_renderEvent);
}

// 0x410ea0
void CFXMotionBlur::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	for (CAttributeCommandIterator it(attr, g_cfxMotionBlurClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0:
			UnLinkMsg(m_renderEvent);
			UnRegisterMsg(m_renderEvent);
			RegisterMsg(m_renderEvent, it->GetAs_RwChar_ptr(), "RwCamera*");
			LinkMsg(m_renderEvent, "RwCamera*");
			break;
		case 1:
			m_priority = it->GetAs_RwInt32();
			UnLinkMsg(m_renderEvent);
			LinkMsg(m_renderEvent, "RwCamera*", (unsigned short)m_priority);
			break;
		case 2:
			CopyRawBits(m_state.m_blurAmount, it->GetAs_RwInt32());
			break;
		case 3:
			CopyRawBits(m_state.m_offsetX, it->GetAs_RwInt32());
			break;
		case 4:
			CopyRawBits(m_state.m_offsetY, it->GetAs_RwInt32());
			break;
		case 5:
			CopyRawBits(m_state.m_scaleX, it->GetAs_RwInt32());
			break;
		case 6:
			CopyRawBits(m_state.m_scaleY, it->GetAs_RwInt32());
			break;
		case 7:
			CopyRawBits(m_state.m_scale, it->GetAs_RwInt32());
			break;
		case 8:
			CopyRawBits(m_state.m_angle, it->GetAs_RwInt32());
			break;
		}
	}
	m_state.m_updateBlendPoly = 1;
}

}}


// Forwarding thunks: ecx (this) passes through the jmp.
struct GnThis_rest {
	ZOOSTERS_NOINLINE void FUN_00411100();
	void MotionBlurState_Dtor_thunk();
};

void GnThis_rest::FUN_00411100() { ((RWS::FX::MotionBlurState *)this)->~MotionBlurState(); }
// 0x410b90
void GnThis_rest::MotionBlurState_Dtor_thunk()
{
	FUN_00411100();
}
