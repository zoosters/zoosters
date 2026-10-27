#include "modules/fx/cfxcolorlight.h"
const char *InternString(const char *pStr);   // 0x464d00

extern const char *g_ctfbCommandClassId;   // GLOBAL: 0x6029bc g_ctfbCommandClassId

namespace RWS { namespace TFB {

// 0x413360
void CTFBCommand::HandleAttributes(const CAttributePacket &attr)
{
	for (CAttributeCommandIterator it(attr, g_ctfbCommandClassId); !it.IsFinished(); ++it) {
		it->GetCommandId();
		m_0 = InternString(it->GetAs_RwChar_ptr());
	}
}

}}
