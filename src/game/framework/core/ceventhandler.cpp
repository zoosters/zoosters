#include "framework/core/ceventhandler.h"
#include "framework/core/idfromstring.h"
#include "framework/core/memoryhandler.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"

#include <string.h>

namespace RWS {

namespace {

const RwUInt32 GLOBAL_DURATION_HINT = 0x40000;

inline bool SameText(unsigned hashA, const char *textA, unsigned hashB, const char *textB)
{
	return hashA == hashB && strcmp(textA, textB) == 0;
}

char *CopyText(const char *text)
{
	size_t size = strlen(text) + 1;
	char *copy = static_cast<char *>(MemAllocHint(size, 2));
	memcpy(copy, text, size);
	return copy;
}

inline void UnlinkNode(CLinkedMsg *node)
{
	node->m_prev->p_next = node->p_next;
	if (node->p_next)
		node->p_next->m_prev = node->m_prev;
}

// The outermost dispatch sweeps the deferred unlinks.
inline void EndDispatch(CRegisteredMsgs *record)
{
	if (--record->m_dispatchDepth == 0 && record->m_pendingUnlink) {
		CEventHandler::ProcessPendingUnlinks(record->Erm.p_next);
		record->m_pendingUnlink = 0;
	}
}

} // namespace

// Sentinel of the doubly linked list of live records, newest first.
// GLOBAL: 0x6115fc RWS::pRMsgs
static CRegisteredMsgs g_messageRegistry;

// GLOBAL: 0x62feb4 RWS::CLinkedMsg::pFreeList
RwFreeList *CLinkedMsg::pFreeList = 0;
// GLOBAL: 0x62feb8 RWS::CRegisteredMsgs::pFreeList
RwFreeList *CRegisteredMsgs::pFreeList = 0;

void *CLinkedMsg::operator new(size_t size) noexcept
{
	return RwFreeListAlloc(pFreeList, GLOBAL_DURATION_HINT);
}

// 0x550330
void CLinkedMsg::operator delete(void *p)
{
	if (p)
		RwFreeListFree(pFreeList, p);
}

void *CRegisteredMsgs::operator new(size_t size) noexcept
{
	return RwFreeListAlloc(pFreeList, GLOBAL_DURATION_HINT);
}

// 0x550350
void CRegisteredMsgs::operator delete(void *p)
{
	if (p)
		RwFreeListFree(pFreeList, p);
}

// 0x550250
CEventId::CEventId() : pEventId(0), m_link_count(0), m_registered_count(0) {}

// 0x464ef0
CEventId::~CEventId() {}

// Copies the pointer only, counting one registration.
// 0x550410
CEventId &CEventId::operator=(const CEventId &rhs)
{
	pEventId = rhs.pEventId;
	if (pEventId)
		IncRegisteredCount();
	return *this;
}

// 0x5509e0
void CEventHandler::Open(unsigned linkBlockSize, unsigned regBlockSize)
{
	CLinkedMsg::pFreeList = RwFreeListCreate(sizeof(CLinkedMsg), linkBlockSize ? linkBlockSize : 20, 16, GLOBAL_DURATION_HINT);
	CRegisteredMsgs::pFreeList =
	    RwFreeListCreate(sizeof(CRegisteredMsgs), regBlockSize ? regBlockSize : 10, 16, GLOBAL_DURATION_HINT);
	RegisterMsg(iMsgDeleteEventHandler, "iMsgDeleteEventHandler", 0);
	RegisterMsg(iMsgDeleteEntity, "iMsgDeleteEntity", 0);
	RegisterMsg(iMsgOnDeleteEntity, "iMsgOnDeleteEntity", "CEventHandler*");
}

// 0x550a60
void CEventHandler::Close()
{
	UnRegisterMsg(iMsgDeleteEventHandler);
	UnRegisterMsg(iMsgDeleteEntity);
	UnRegisterMsg(iMsgOnDeleteEntity);
	if (CRegisteredMsgs::pFreeList) {
		RwFreeListDestroy(CRegisteredMsgs::pFreeList);
		CRegisteredMsgs::pFreeList = 0;
	}
	if (CLinkedMsg::pFreeList) {
		RwFreeListDestroy(CLinkedMsg::pFreeList);
		CLinkedMsg::pFreeList = 0;
	}
}

// 0x550440
void CEventHandler::RegisterMsg(CEventId &id, const char *name, const char *format)
{
	if (!name || !*name) {
		id.pEventId = 0;
		return;
	}
	unsigned nameHash = GenerateUIntIdFromString(name);
	unsigned formatHash = GenerateUIntIdFromString(format);

	// Shortcut through the record the id holds. A format added here keeps the old format hash, as the game does: a later link
	// with that format then fails the format test.
	CRegisteredMsgs *record = id.pEventId;
	if (id.m_registered_count > 0 && record && SameText(record->m_nameHash, record->m_name, nameHash, name)) {
		if (record->m_format) {
			if (format && SameText(record->m_formatHash, record->m_format, formatHash, format)) {
				id.IncRegisteredCount();
				return;
			}
		} else if (format) {
			record->m_format = CopyText(format);
		}
	}

	for (record = g_messageRegistry.m_next; record; record = record->m_next) {
		if (SameText(record->m_nameHash, record->m_name, nameHash, name)) {
			if (!record->m_format && format) {
				record->m_format = CopyText(format);
				record->m_formatHash = formatHash;
			}
			id.pEventId = record;
			id.IncRegisteredCount();
			return;
		}
	}

	record = new CRegisteredMsgs;
	record->m_next = g_messageRegistry.m_next;
	record->m_prev = &g_messageRegistry;
	if (g_messageRegistry.m_next)
		g_messageRegistry.m_next->m_prev = record;
	g_messageRegistry.m_next = record;
	record->m_name = CopyText(name);
	record->m_format = format ? CopyText(format) : 0;
	record->m_nameHash = nameHash;
	record->m_formatHash = formatHash;
	id.pEventId = record;
	id.IncRegisteredCount();
}

// 0x5506d0
void CEventHandler::UnRegisterMsg(CEventId &id)
{
	CRegisteredMsgs *record = id.pEventId;
	if (!record)
		return;
	if (id.m_registered_count == 0) {
		id.pEventId = 0;
		return;
	}
	id.DecRegisteredCount();
	// a record that still has nodes (pending too) stays alive
	if (record->m_registeredCount == 0 && !record->Erm.p_next) {
		::operator delete(record->m_name);
		::operator delete(record->m_format);
		record->m_prev->m_next = record->m_next;
		if (record->m_next)
			record->m_next->m_prev = record->m_prev;
		id.pEventId = 0;
		delete record;
	}
}

// 0x5508e0
void CEventHandler::ReplaceRegisteredMsg(CEventId &id, const char *name, const char *format)
{
	UnRegisterMsg(id);
	RegisterMsg(id, name, format);
}

// 0x5507e0
int CEventHandler::LinkMsgToEventHandler(CEventHandler *handler, CEventId &id, const char *format, unsigned short priority)
{
	CRegisteredMsgs *record = id.pEventId;
	if (!record)
		return 1;
	id.m_link_count++;
	for (CLinkedMsg *node = record->Erm.p_next; node; node = node->p_next) {
		if (node->m_handler == handler) {
			node->m_linkCount++;
			return 1;
		}
	}
	if (format && record->m_format && !SameText(record->m_formatHash, record->m_format, GenerateUIntIdFromString(format), format))
		return 0;

	CLinkedMsg *node = new CLinkedMsg(handler, priority);
	if (!node)
		return 0;
	// newest first among equal priorities
	CLinkedMsg *prev = &record->Erm;
	while (prev->p_next && prev->p_next->m_priority > priority)
		prev = prev->p_next;
	node->p_next = prev->p_next;
	node->m_prev = prev;
	if (prev->p_next)
		prev->p_next->m_prev = node;
	prev->p_next = node;
	return 1;
}

// 0x550ac0
void CEventHandler::LinkMsg(CEventId &id, const char *format, unsigned short priority)
{
	if (id.pEventId)
		LinkMsgToEventHandler(this, id, format, priority);
}

// 0x550760
void CEventHandler::UnLinkMsg(CEventId &id)
{
	CRegisteredMsgs *record = id.pEventId;
	if (!record || id.m_link_count == 0 || !record->m_name)
		return;
	for (CLinkedMsg *node = record->Erm.p_next; node; node = node->p_next) {
		if (node->m_handler != this)
			continue;
		id.m_link_count--;
		if (--node->m_linkCount == 0) {
			if (record->m_dispatchDepth) {
				// the dispatch may still walk through it: flag it, swept when the dispatch ends
				node->m_handler = 0;
				record->m_pendingUnlink = 1;
			} else {
				UnlinkNode(node);
				delete node;
			}
		}
		return;
	}
}

// 0x5502f0
int CEventHandler::GetLinkedPriority(CEventId &id)
{
	if (!id.pEventId)
		return -1;
	for (CLinkedMsg *node = id.pEventId->Erm.p_next; node; node = node->p_next) {
		if (node->m_handler == this)
			return node->m_priority;
	}
	return -2;
}

// 0x550b70
void CEventHandler::SetLinkPriority(CEventHandler &handler, CEventId &id, const char *format, unsigned short priority)
{
	handler.UnLinkMsg(id);
	handler.LinkMsg(id, format, priority);
}

// 0x550b20
void CEventHandler::ReplaceLinkedMsg(CEventId &id, const char *name, const char *format)
{
	UnLinkMsg(id);
	UnRegisterMsg(id);
	RegisterMsg(id, name, format);
	LinkMsg(id, format);
}

// 0x550900
void CEventHandler::ProcessPendingUnlinks(CLinkedMsg *first)
{
	CLinkedMsg *node = first;
	while (node) {
		CLinkedMsg *next = node->p_next;
		if (!node->m_handler) {
			UnlinkNode(node);
			delete node;
		}
		node = next;
	}
}

// The caller checked that msg.Id is valid.
// 0x550bb0
int _SendMsg(CMsg &msg)
{
	CRegisteredMsgs *record = msg.Id;
	CLinkedMsg *node = record->Erm.p_next;
	if (!node)
		return 0;
	record->m_dispatchDepth++;
	do {
		CEventHandler *handler = node->m_handler;
		if (handler && handler->m_Active)
			handler->HandleEvents(msg);
		// read after the call: nodes linked during it are visited too
		node = node->p_next;
	} while (node);
	EndDispatch(record);
	return 1;
}

// 0x550950
void SendMsgToEventHandler(CMsg &msg, CEventHandler *handler)
{
	if (!handler || !msg.Id || !handler->m_Active)
		return;
	CRegisteredMsgs *record = msg.Id;
	record->m_dispatchDepth++;
	handler->HandleEvents(msg);
	EndDispatch(record);
}

// 0x5509b0
void SendMsgToEventHandler(const CEventId &id, CEventHandler *handler)
{
	if (handler) {
		CMsg msg(id);
		SendMsgToEventHandler(msg, handler);
	}
}

// 0x550c20
CEventHandler::~CEventHandler()
{
	if (iMsgOnDeleteEntity.pEventId && iMsgOnDeleteEntity.pEventId->Erm.p_next) {
		CMsg msg(iMsgOnDeleteEntity, static_cast<CEventHandler *>(this));
		_SendMsg(msg);
	}
	UnLinkMsg(iMsgDeleteEventHandler);
	UnLinkMsg(iMsgDeleteEntity);
}

// Head first; each destructor removes its node.
// 0x550290
void CEventHandler::DeleteEventHandlers(const CMsg *msg)
{
	CRegisteredMsgs *record = msg->Id;
	if (!record)
		return;
	while (CLinkedMsg *node = record->Erm.p_next) {
		if (!node->m_handler)
			break;
		delete node->m_handler;
	}
}

// 0x5502c0
void CEventHandler::DeleteEventHandlers(const CEventId &id)
{
	if (id.pEventId) {
		CMsg msg(id);
		DeleteEventHandlers(&msg);
	}
}

namespace RpAtomicToEventHandler {
namespace {

const RwUInt32 PLUGIN_ID = 0x7FF; // vendor 7, plugin 0xFF

// GLOBAL: 0x62febc RWS::RpAtomicToEventHandler::anon::iPlugin_Offset
RwInt32 g_pluginOffset;

inline CEventHandler *&Slot(void *atomic, RwInt32 offset)
{
	return *reinterpret_cast<CEventHandler **>(static_cast<char *>(atomic) + offset);
}

// 0x550370
void *Plugin_Constructor(void *object, RwInt32 offset, RwInt32 size)
{
	Slot(object, offset) = 0;
	return object;
}

// 0x550390
void *Plugin_Copy(void *dst, const void *src, RwInt32 offset, RwInt32 size)
{
	Slot(dst, offset) = Slot(const_cast<void *>(src), offset);
	return dst;
}

} // namespace

// 0x5503b0
int PluginAttach(void)
{
	g_pluginOffset = RpAtomicRegisterPlugin(sizeof(void *), PLUGIN_ID, Plugin_Constructor, 0, Plugin_Copy);
	return g_pluginOffset != 0;
}

// 0x5503e0
void SetEventHandler(RpAtomic &atomic, CEventHandler *handler)
{
	Slot(&atomic, g_pluginOffset) = handler;
}

} // namespace RpAtomicToEventHandler

} // namespace RWS
