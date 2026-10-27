// vtable TU: the constructor makes cl emit the vtables, the scalar deleting destructor and the thunk (the destructor stays in this TU like
// the light's)
#include "script/script.h"
#include "tfb/render/tfbatomic.h"
#include "modules/fx/partspray.h"
#include "framework/toolkits/world/framehelper.h"
namespace RWS { namespace FX {
// GLOBAL: 0x5cb514 RWS::FX::CFXPartSpray::vftable_for_CAttributeHandler
// GLOBAL: 0x5cb50c RWS::FX::CFXPartSpray::vftable_for_CEventHandler
// 0x410220
// 0x40daf0
// GLOBAL: 0x413360 ?HandleAttributes@CTFBCommand@TFB@RWS@@QAEXABVCAttributePacket@3@@Z
// GLOBAL: 0x44d240 RWS::Particle::CPB_TexPart::SetTexture
// GLOBAL: 0x43e6f0 ?FUN_0043e6f0@@YAHPBD@Z
// GLOBAL: 0x44eff0 ?RwFrameCreate_thunk@@YAPAURwFrame@@XZ

// 0x40d940
CFXPartSpray::CFXPartSpray(const CAttributePacket &packet) : CTFBCommand(packet), m_offsetHeading(0), m_offsetDeflection(0), m_drawOrder(0)
{
	const void *resource = CSystemCommands::FindFirstResource(packet);
	if (m_model.ptr())
		TFBPlugin_FD_SetFlagBits(m_model.ptr(), 0x1000);
	m_visualRadius = 20.0f;
	m_options = 0;
	m_host = new GameParticle(this);
	m_host->AdjustRefCount(1);
	const char *cmd = m_0;
	GameParticle *host = m_host;
	((::Script::ScriptReference *)host)->ResolveSymbol(host->GetType(), cmd, 0);
	if (resource)
		FUN_0043e6f0(">>>>>>>>> SPUD - particle has a DFF attached!!!\n");
	if (m_model.ptr())
		m_host->m_frame = RpAtomicGetFrame(m_model.ptr());
	else {
		m_host->m_ownsFrame = 1;
		m_host->m_frame = RwFrameCreate_thunk();
		RwFrameSetIdentity(m_host->m_frame);
	}
	TFB_GetInactiveParticleSet()->Insert(m_host, 3.4028235e38f);
}

// 0x40db50
CFXPartSpray::~CFXPartSpray()
{
	m_host->m_buf->Processor()->SetTexture(nullptr, *m_host->m_buf); // releases the emitter's texture
	m_host->m_owner = nullptr;
	m_host->AdjustRefCount(-1);
	UnLinkMsg(m_frameEvent);
	UnRegisterMsg(m_frameEvent);
	UnLinkMsg(m_enableEvent);
	UnRegisterMsg(m_enableEvent);
	UnLinkMsg(m_rateEvent);
	UnRegisterMsg(m_rateEvent);
}
}}
