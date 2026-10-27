// A loaded resource record. All records form a list, newest first, walked by CResourceIterator.
#ifndef ZOOSTERS_FRAMEWORK_CORE_CRESOURCE_H
#define ZOOSTERS_FRAMEWORK_CORE_CRESOURCE_H

#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "framework/core/rwsguid.h"
#include "framework/stringpool.h"

namespace RWS {

class CResource {
public:
	enum { TypeBufferSize = 250 };

	CResource();
	~CResource();

	const char *GetName(void) const { return m_name; }
	void SetName(const char *name);
	const char *GetType(void) const { return m_type; }
	void SetType(const char *type);
	const void *GetData(void) const { return m_data; }
	void SetData(const void *data) { m_data = data; }
	unsigned GetSize(void) const { return m_size; }
	void SetSize(unsigned size) { m_size = size; }
	unsigned GetLockRefCount(void) const { return m_lockCount; }
	void SetLockRefCount(unsigned count) { m_lockCount = count; }
	const RWSGUID &GetId(void) const { return m_id; }
	void SetId(const RWSGUID &id) { m_id = id; }

	CResource *GetNext(void) const { return m_next; }
	static CResource *GetHead(void) { return sm_ResourceHead; }

private:
	CResource(const CResource &);
	CResource &operator=(const CResource &);

	CResource *m_next;
	CResource *m_prev;
	const char *m_name;   // pooled (InternString) or null
	char m_type[TypeBufferSize];
	const void *m_data;
	unsigned m_size;
	unsigned m_lockCount;
	RWSGUID m_id;

	static CResource *sm_ResourceHead;
};
ZOOSTERS_ASSERT_SIZE(CResource, 0x124);

class CResourceIterator {
public:
	CResourceIterator() : m_current(CResource::GetHead()) {}
	bool IsDone(void) const { return m_current == 0; }
	CResource &operator*() const { return *m_current; }
	CResource *operator->() const { return m_current; }
	CResourceIterator &operator++()
	{
		if (m_current)
			m_current = m_current->GetNext();
		return *this;
	}
	void Remove(void);   // deletes the current record, steps to the next

private:
	CResource *m_current;
};

}  // namespace RWS

#endif
