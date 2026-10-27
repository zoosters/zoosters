#include "modules/tfb/sprites/spritemanager.h"
#include "script/actor.h"

// GLOBAL: 0x6029e0 g_spriteManagerClassId
// GLOBAL: 0x617e88 g_evtNullSprite
extern const char *g_spriteManagerClassId;
extern RWS::CEventId g_evtNullSprite;

// Attribute command as the iterator hands it out: header (+4 = command id) then the data from +8.
struct AttrCmdData {
	int m_00;
	int m_commandId;
	int m_data;
};
#define CMD_DATA(it) ((AttrCmdData *)(it).operator->())

namespace RWS { namespace TFB {
// 0x414bb0
// command 0 = receive-render event name (m_Render_In), 1 = link priority (m_link_priority), 2 = transmit event name (m_Render_Out)
void SpriteManager::HandleAttributes(const CAttributePacket &attr)
{
	CAttributeHandler::HandleAttributes(attr);
	for (CAttributeCommandIterator it(attr, g_spriteManagerClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0:
			UnLinkMsg((CEventId &)m_Render_In);
			UnRegisterMsg((CEventId &)m_Render_In);
			{
				AttrCmdData *c = CMD_DATA(it);
				RegisterMsg((CEventId &)m_Render_In, (const char *)&c->m_data, "RwCamera*");
			}
			LinkMsg((CEventId &)m_Render_In, "RwCamera*", (unsigned short)m_link_priority);
			break;
		case 1:
			{
				int v = CMD_DATA(it)->m_data;
				if (v < 0)
					v = 0xFFFF;
				m_link_priority = v;
			}
			UnLinkMsg((CEventId &)m_Render_In);
			LinkMsg((CEventId &)m_Render_In, "RwCamera*", (unsigned short)m_link_priority);
			break;
		case 2:
			UnRegisterMsg((CEventId &)m_Render_Out);
			{
				AttrCmdData *c = CMD_DATA(it);
				RegisterMsg((CEventId &)m_Render_Out, (const char *)&c->m_data, "RwCamera*");
			}
			break;
		}
	}
}
}}

namespace RWS { namespace TFB {
// 0x415a80
void SpriteManager::HandleEvents(CMsg &cmsg)
{
	CMsg &msg = cmsg;
	if ((const int *)msg.Id == (const int *)m_Render_In.pEventId) {
		if (!RenderListDirty())
			Render();
		if (m_Render_Out.m_registered_count != 0) {
			CMsg out;
			out.Id = m_Render_Out.pEventId;
			out.pData = msg.pData;
			if ((const int *)out.Id != (const int *)g_evtNullSprite.pEventId && *(const int *)out.Id != 0)
				_SendMsg(out);
		}
	}
}
}}
