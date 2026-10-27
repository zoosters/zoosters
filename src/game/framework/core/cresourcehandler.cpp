#include "framework/core/cresourcehandler.h"

namespace RWS {

// GLOBAL: 0x62ff20 RWS::CResourceHandler::g_pHead
CResourceHandler *CResourceHandler::g_pHead = 0;

// GLOBAL: 0x5dc098 RWS::CResourceHandler::vftable
// 0x552470
CResourceHandler::CResourceHandler(void) : m_pNext(g_pHead)
{
	g_pHead = this;
}

// 0x552530
// 0x552490
CResourceHandler::~CResourceHandler(void)
{
	for (CResourceHandler **link = &g_pHead; *link; link = &(*link)->m_pNext) {
		if (*link == this) {
			*link = m_pNext;
			break;
		}
	}
}

// 0x5524c0
CResourceHandler *CResourceHandler::FindHandler(const char *const type)
{
	for (CResourceHandler *handler = g_pHead; handler; handler = handler->m_pNext) {
		if (handler->IsHandled(type))
			return handler;
	}
	return 0;
}

// 0x5524f0
void CResourceHandler::OpenAll(void)
{
	for (CResourceHandler *handler = g_pHead; handler; handler = handler->m_pNext)
		handler->Open();
}

// 0x552510
void CResourceHandler::CloseAll(void)
{
	for (CResourceHandler *handler = g_pHead; handler; handler = handler->m_pNext)
		handler->Close();
}

}  // namespace RWS
