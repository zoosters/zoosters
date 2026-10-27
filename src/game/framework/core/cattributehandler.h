// Attribute packets (the records describing a level-stream entity), their iterators, and CAttributeHandler: the base of every
// behaviour, with the registry of live entities by instance id.
#ifndef ZOOSTERS_FRAMEWORK_CORE_CATTRIBUTEHANDLER_H
#define ZOOSTERS_FRAMEWORK_CORE_CATTRIBUTEHANDLER_H

#include <string.h>

#include "zoosters/base.h"
#include "framework/core/rwsguid.h"

namespace RWS {

class CAttributeHandler;

// Record types of a packet; any other type word is the id of an attribute command.
enum {
	ATTRIBUTE_CHUNK_CREATE_CLASS = 0x20000000,
	ATTRIBUTE_CHUNK_INSTANCE_ID = 0x40000000,
	ATTRIBUTE_CHUNK_CLASS_SECTION = 0x80000000u,
};

// A record viewed in place: size (including this 8-byte header), type, payload. Size 0 ends the packet.
class CAttributeDataChunk {
public:
	bool IsEndChunk() const { return size_ == 0; }
	const CAttributeDataChunk *pNextChunk() const
	{
		return reinterpret_cast<const CAttributeDataChunk *>(reinterpret_cast<const char *>(this) + size_);
	}
	const void *Payload() const { return &data_; }

	const unsigned int size_;
	const unsigned int type_;
	const unsigned int data_; // first payload word; the payload continues past it
};
ZOOSTERS_ASSERT_SIZE(CAttributeDataChunk, 12);
ZOOSTERS_ASSERT_OFFSET(CAttributeDataChunk, size_, 0);
ZOOSTERS_ASSERT_OFFSET(CAttributeDataChunk, type_, 4);
ZOOSTERS_ASSERT_OFFSET(CAttributeDataChunk, data_, 8);

class CAttributeDataChunkIterator {
public:
	CAttributeDataChunkIterator(const CAttributeDataChunk &first);
	bool IsFinished() const { return m_current->IsEndChunk(); }
	CAttributeDataChunkIterator &operator++();
	const CAttributeDataChunk *operator->() const { return m_current; }

private:
	const CAttributeDataChunk *m_current;
};

// A reference to a packet is a reference to its first record (the create-class record).
class CAttributePacket {
public:
	RWSGUID GetInstanceId() const;
	const char *GetNameOfClassToCreate() const { return static_cast<const char *>(m_first.Payload()); }
	CAttributeHandler *CreateEntity(void) const;
	const CAttributeDataChunk &FirstChunk() const { return m_first; }

private:
	CAttributeDataChunk m_first;
};

// The type word is the command id, the payload its value.
class CAttributeCommand {
public:
	unsigned int GetCommandId() const { return record.type_; }
	template <class T> void GetCommandData(T &dest) const { memcpy(&dest, record.Payload(), sizeof(T)); }
	void GetCommandData(unsigned int &dest) const { dest = record.data_; }
	void GetCommandData(bool &dest) const { dest = record.data_ != 0; }
	void GetCommandData(const char *&dest) const { dest = GetAs_RwChar_ptr(); }
	template <class T> void GetCommandData(T *dest) const { *dest = (T)record.Payload(); }
	const void *GetAs_void_ptr() const { return record.Payload(); }
	const char *GetAs_RwChar_ptr() const { return static_cast<const char *>(record.Payload()); }
	const float *GetAs_RwReal_ptr() const { return static_cast<const float *>(record.Payload()); }
	float GetAs_RwReal() const
	{
		float value;
		memcpy(&value, record.Payload(), sizeof(value));
		return value;
	}
	unsigned short GetAs_RwUInt16() const
	{
		unsigned short value;
		memcpy(&value, record.Payload(), sizeof(value));
		return value;
	}
	unsigned int GetAs_RwUInt32() const { return record.data_; }
	int GetAs_RwInt32() const { return static_cast<int>(record.data_); }

	CAttributeDataChunk record;
};
ZOOSTERS_ASSERT_OFFSET(CAttributeCommand, record, 0);

// The commands of every section whose class name is the target class.
class CAttributeCommandIterator {
public:
	CAttributeCommandIterator(const CAttributePacket &packet, const char *targetClass);
	bool IsFinished() const { return m_current->IsEndChunk(); }
	CAttributeCommandIterator &operator++();
	const CAttributeCommand *operator->() const { return reinterpret_cast<const CAttributeCommand *>(m_current); }

private:
	void SkipToNextAttributeCommand(bool insideTarget);

	const CAttributeDataChunk *m_current;
	const char *m_targetClass;
};
ZOOSTERS_ASSERT_SIZE(CAttributeCommandIterator, 8);

class CAttributeHandler {
public:
	typedef void (*NewAttributeHandlerCallBack)();

	CAttributeHandler() : m_flags(0) { m_instanceId.Clear(); }
	virtual ~CAttributeHandler();
	virtual void HandleAttributes(const CAttributePacket &attr);
	virtual void *GetAttributeTable(const char *classId, void *rNumEntries, void *rpThis, void *ignoreTopLevel);
	virtual void *GetAttrPostUpdateFunc(void *&pThis);

	static void Remove(const RWSGUID &instanceId);
	static CAttributeHandler *Find(const RWSGUID &instanceId);
	static void Purge();
	static void PurgeAll();
	int IsLocked() { return (m_flags & FLAG_LOCKED) ? 1 : 0; }
	unsigned GetFlags() const { return m_flags; }
	void Lock() { m_flags |= FLAG_LOCKED; }
	static void SetNewAttributeHandlerCallBack(NewAttributeHandlerCallBack callBack)
	{
		sm_pNewAttributeHandlerCallBack = callBack;
	}
	static void RegisterStreamChunkHandlers();
	static void UnRegisterStreamChunkHandlers();

private:
	friend class CAttributePacket;
	enum { FLAG_LOCKED = 2 };

	void AddToRegistry(const RWSGUID &instanceId);

	static CAttributeHandler *g_pHead;
	static NewAttributeHandlerCallBack sm_pNewAttributeHandlerCallBack;

	unsigned m_flags;
	RWSGUID m_instanceId;
	CAttributeHandler *m_nextHandler; // set only while in the registry
};
ZOOSTERS_ASSERT_SIZE(CAttributeHandler, 0x1c);

} // namespace RWS

#endif
