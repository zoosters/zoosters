#include "framework/core/cattributehandler.h"
#include "framework/core/cclassfactory.h"
#include "rw/rwcore.h"

namespace RWS {

// Address anchors for accessors that are inline here but out of line in the game.

// 0x54fdf0
unsigned int (CAttributeCommand::*g_anchorGetCommandId)() const = &CAttributeCommand::GetCommandId;
// 0x401f10
const char *(CAttributeCommand::*g_anchorGetAsRwCharPtr)() const = &CAttributeCommand::GetAs_RwChar_ptr;
// 0x40d930
void (CAttributeCommand::*g_anchorGetCommandDataString)(const char *&) const = &CAttributeCommand::GetCommandData;
// 0x406840
void (CAttributeCommand::*g_anchorGetCommandDataUInt)(unsigned int &) const = &CAttributeCommand::GetCommandData;
// 0x406f00
float (CAttributeCommand::*g_anchorGetAsRwReal)() const = &CAttributeCommand::GetAs_RwReal;
// 0x4087a0
void (CAttributeCommand::*g_anchorGetCommandDataV3d)(RwV3d &) const = &CAttributeCommand::GetCommandData<RwV3d>;
// 0x54fe20
bool (CAttributeCommandIterator::*g_anchorCommandIteratorIsFinished)() const = &CAttributeCommandIterator::IsFinished;
// 0x54fec0
const CAttributeCommand *(CAttributeCommandIterator::*g_anchorCommandIteratorArrow)() const =
    &CAttributeCommandIterator::operator->;

// 0x54fe00
CAttributeDataChunkIterator::CAttributeDataChunkIterator(const CAttributeDataChunk &first) : m_current(&first) {}

// 0x54fe10
CAttributeDataChunkIterator &CAttributeDataChunkIterator::operator++()
{
	m_current = m_current->pNextChunk();
	return *this;
}

// 0x54fe30
void CAttributeCommandIterator::SkipToNextAttributeCommand(bool insideTarget)
{
	const CAttributeDataChunk *chunk = m_current;
	for (; !chunk->IsEndChunk(); chunk = chunk->pNextChunk()) {
		unsigned int type = chunk->type_;
		if (type == ATTRIBUTE_CHUNK_CREATE_CLASS || type == ATTRIBUTE_CHUNK_INSTANCE_ID)
			continue;
		if (type == ATTRIBUTE_CHUNK_CLASS_SECTION) {
			insideTarget = rwstrcmp(static_cast<const char *>(chunk->Payload()), m_targetClass) == 0;
			continue;
		}
		if (insideTarget)
			break;
	}
	m_current = chunk;
}

// 0x550020
CAttributeCommandIterator::CAttributeCommandIterator(const CAttributePacket &packet, const char *targetClass)
    : m_current(&packet.FirstChunk()), m_targetClass(targetClass)
{
	SkipToNextAttributeCommand(false);
}

// 0x54fea0
CAttributeCommandIterator &CAttributeCommandIterator::operator++()
{
	m_current = m_current->pNextChunk();
	SkipToNextAttributeCommand(true);
	return *this;
}

// 0x54ffc0
RWSGUID CAttributePacket::GetInstanceId() const
{
	RWSGUID id;
	for (const CAttributeDataChunk *chunk = &m_first; !chunk->IsEndChunk(); chunk = chunk->pNextChunk()) {
		if (chunk->type_ == ATTRIBUTE_CHUNK_INSTANCE_ID) {
			memcpy(&id, chunk->Payload(), sizeof(id));
			return id;
		}
	}
	id.Clear();
	return id;
}

// GLOBAL: 0x62fea8 RWS::CAttributeHandler::g_pHead
CAttributeHandler *CAttributeHandler::g_pHead = 0;
// GLOBAL: 0x62ff50 RWS::CAttributeHandler::sm_pNewAttributeHandlerCallBack
CAttributeHandler::NewAttributeHandlerCallBack CAttributeHandler::sm_pNewAttributeHandlerCallBack = 0;

// 0x550040
CAttributeHandler::~CAttributeHandler()
{
	Remove(m_instanceId);
}

// 0x410380
void CAttributeHandler::HandleAttributes(const CAttributePacket &attr) {}

// 0x401000
void *CAttributeHandler::GetAttributeTable(const char *classId, void *rNumEntries, void *rpThis, void *ignoreTopLevel)
{
	*static_cast<unsigned *>(rNumEntries) = 0;
	*static_cast<void **>(rpThis) = 0;
	return 0;
}

void CAttributeHandler::AddToRegistry(const RWSGUID &instanceId)
{
	m_instanceId = instanceId;
	m_nextHandler = g_pHead;
	g_pHead = this;
}

// 0x54fed0
void CAttributeHandler::Remove(const RWSGUID &instanceId)
{
	for (CAttributeHandler **link = &g_pHead; *link; link = &(*link)->m_nextHandler) {
		if ((*link)->m_instanceId == instanceId) {
			*link = (*link)->m_nextHandler;
			return;
		}
	}
}

// 0x54ff20
CAttributeHandler *CAttributeHandler::Find(const RWSGUID &instanceId)
{
	for (CAttributeHandler *handler = g_pHead; handler; handler = handler->m_nextHandler) {
		if (handler->m_instanceId == instanceId)
			return handler;
	}
	return 0;
}

// 0x54ff60
void CAttributeHandler::Purge()
{
	CAttributeHandler **link = &g_pHead;
	while (CAttributeHandler *handler = *link) {
		if (handler->IsLocked()) {
			link = &handler->m_nextHandler;
		} else {
			// the destructor may unlink further handlers: carry on from *link
			*link = handler->m_nextHandler;
			delete handler;
		}
	}
}

// 0x54ff90
void CAttributeHandler::PurgeAll()
{
	while (CAttributeHandler *handler = g_pHead) {
		g_pHead = handler->m_nextHandler;
		delete handler;
	}
}

// 0x550060
CAttributeHandler *CAttributePacket::CreateEntity(void) const
{
	RWSGUID instanceId = GetInstanceId();
	if (CAttributeHandler *old = CAttributeHandler::Find(instanceId))
		delete old;
	CAttributeHandler *handler = CClassFactory::MakeNew(GetNameOfClassToCreate(), *this);
	if (handler) {
		handler->AddToRegistry(instanceId);
		if (CAttributeHandler::sm_pNewAttributeHandlerCallBack)
			CAttributeHandler::sm_pNewAttributeHandlerCallBack();
	}
	return handler;
}

} // namespace RWS
