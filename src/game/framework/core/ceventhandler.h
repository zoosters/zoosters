// Messages: named records with priority-sorted handler lists, and the event handler base class.
#ifndef ZOOSTERS_FRAMEWORK_CORE_CEVENTHANDLER_H
#define ZOOSTERS_FRAMEWORK_CORE_CEVENTHANDLER_H

#include "zoosters/base.h"

struct RwFreeList;
struct RpAtomic;

namespace RWS {

class CEventHandler;

// A node of a message's handler list; the head node is embedded in the record.
class CLinkedMsg {
public:
	constexpr CLinkedMsg() : p_next(0), m_prev(0), m_handler(0), m_linkCount(1), m_priority(0xFFFF) {}
	constexpr CLinkedMsg(CEventHandler *handler, unsigned short priority)
	    : p_next(0), m_prev(0), m_handler(handler), m_linkCount(1), m_priority(priority)
	{}
	static void *operator new(size_t size) noexcept;
	static void operator delete(void *p);

	static RwFreeList *pFreeList;

	CLinkedMsg *p_next;
	CLinkedMsg *m_prev;
	CEventHandler *m_handler; // null in the head node and in a node whose unlink is pending
	int m_linkCount;
	unsigned short m_priority;
};
ZOOSTERS_ASSERT_SIZE(CLinkedMsg, 0x14);
ZOOSTERS_ASSERT_OFFSET(CLinkedMsg, m_handler, 0x8);
ZOOSTERS_ASSERT_OFFSET(CLinkedMsg, m_linkCount, 0xc);
ZOOSTERS_ASSERT_OFFSET(CLinkedMsg, m_priority, 0x10);

class CRegisteredMsgs {
public:
	constexpr CRegisteredMsgs()
	    : m_name(0), m_format(0), m_nameHash(0), m_formatHash(0), m_next(0), m_prev(0), m_registeredCount(0), m_pendingUnlink(0),
	      m_dispatchDepth(0)
	{}
	static void *operator new(size_t size) noexcept;
	static void operator delete(void *p);

	static RwFreeList *pFreeList;

	CLinkedMsg Erm;
	char *m_name;
	char *m_format;
	unsigned m_nameHash;
	unsigned m_formatHash; // not set when a format is added through an id's shortcut (RegisterMsg)
	CRegisteredMsgs *m_next;
	CRegisteredMsgs *m_prev;
	int m_registeredCount;
	int m_pendingUnlink; // a node was unlinked during a dispatch; swept when it ends
	int m_dispatchDepth; // non-zero while being sent (nested sends count)
};
ZOOSTERS_ASSERT_SIZE(CRegisteredMsgs, 0x38);
ZOOSTERS_ASSERT_OFFSET(CRegisteredMsgs, m_name, 0x14);
ZOOSTERS_ASSERT_OFFSET(CRegisteredMsgs, m_nameHash, 0x1c);
ZOOSTERS_ASSERT_OFFSET(CRegisteredMsgs, m_next, 0x24);
ZOOSTERS_ASSERT_OFFSET(CRegisteredMsgs, m_registeredCount, 0x2c);
ZOOSTERS_ASSERT_OFFSET(CRegisteredMsgs, m_dispatchDepth, 0x34);

// Handle to a registered message, null if invalid. Counts the registrations and links made through it.
class CEventId {
public:
	CEventId();
	~CEventId();
	CEventId &operator=(const CEventId &rhs);
	CEventId &operator=(const CRegisteredMsgs *rhs)
	{
		pEventId = const_cast<CRegisteredMsgs *>(rhs);
		return *this;
	}
	operator CRegisteredMsgs *() const { return pEventId; }
	bool operator==(const CEventId &rhs) const { return pEventId == rhs.pEventId; }
	bool operator!=(const CEventId &rhs) const { return pEventId != rhs.pEventId; }
	const CRegisteredMsgs *operator->() const { return pEventId; }

	void IncRegisteredCount()
	{
		pEventId->m_registeredCount++;
		m_registered_count++;
	}
	void DecRegisteredCount()
	{
		pEventId->m_registeredCount--;
		if (--m_registered_count == 0)
			pEventId = 0;
	}

	CRegisteredMsgs *pEventId;
	short m_link_count;
	short m_registered_count;
};
ZOOSTERS_ASSERT_SIZE(CEventId, 8);
ZOOSTERS_ASSERT_OFFSET(CEventId, m_link_count, 4);
ZOOSTERS_ASSERT_OFFSET(CEventId, m_registered_count, 6);

// pData is described by the message's format.
class CMsg {
public:
	CMsg() {}
	CMsg(const CEventId &id) : Id(id.pEventId), pData(0) {}
	CMsg(const CEventId &id, void *data) : Id(id.pEventId), pData(data) {}

	CRegisteredMsgs *Id;
	void *pData;
};
ZOOSTERS_ASSERT_SIZE(CMsg, 8);

int _SendMsg(CMsg &msg);
void SendMsgToEventHandler(CMsg &msg, CEventHandler *handler);
void SendMsgToEventHandler(const CEventId &id, CEventHandler *handler);

extern CEventId iMsgRunningTick;
extern CEventId iMsgRunningPreTick;
extern CEventId iMsgRunningPostTick;
extern CEventId iMsgStartSystem;
extern CEventId iMsgStopSystem;
extern CEventId iMsgDeleteEventHandler;
extern CEventId iMsgDeleteEntity;
extern CEventId iMsgOnDeleteEntity;

class CEventHandler {
public:
	CEventHandler() : m_Active(1) {}
	virtual ~CEventHandler();
	virtual void HandleEvents(CMsg &msg) = 0;

	int m_Active;

	int GetLinkedPriority(CEventId &id);
	void LinkMsg(CEventId &id, const char *format = 0, unsigned short priority = 0x8000);
	void UnLinkMsg(CEventId &id);
	void ReplaceLinkedMsg(CEventId &id, const char *name, const char *format);
	static void RegisterMsg(CEventId &id, const char *name, const char *format);
	static void UnRegisterMsg(CEventId &id);
	static void ReplaceRegisteredMsg(CEventId &id, const char *name, const char *format);
	static int LinkMsgToEventHandler(CEventHandler *handler, CEventId &id, const char *format = 0,
	                                 unsigned short priority = 0x8000);
	static void SetLinkPriority(CEventHandler &handler, CEventId &id, const char *format, unsigned short priority = 0x8000);
	static void ProcessPendingUnlinks(CLinkedMsg *first);
	static void DeleteEventHandlers(const CMsg *msg);
	static void DeleteEventHandlers(const CEventId &id);
	static void Open(unsigned linkBlockSize = 0, unsigned regBlockSize = 0);
	static void Close();
	static void RegisterStreamChunkHandlers();
	static void UnRegisterStreamChunkHandlers();
};
ZOOSTERS_ASSERT_OFFSET(CEventHandler, m_Active, 4);
ZOOSTERS_ASSERT_SIZE(CEventHandler, 8);

// RpAtomic plugin: the behaviour that owns an atomic.
namespace RpAtomicToEventHandler {
int PluginAttach(void);
void SetEventHandler(RpAtomic &atomic, CEventHandler *handler);
} // namespace RpAtomicToEventHandler

#ifdef ZOOSTERS_RWS_SENDMSG
// Per-unit copy, as in the game.
static const CEventId ERROR_NOT_A_VALID_MESSAGE_ID;

inline int SendMsg(CMsg &msg)
{
	if (msg.Id != ERROR_NOT_A_VALID_MESSAGE_ID.pEventId && msg.Id->Erm.p_next)
		return _SendMsg(msg);
	return 0;
}
#endif

} // namespace RWS

#endif
