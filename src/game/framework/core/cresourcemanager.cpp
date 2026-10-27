#include <stdlib.h>
#include <string.h>
#include "framework/core/cresourcemanager.h"
#include "framework/core/cresource.h"
#include "framework/core/cresourcehandler.h"
#include "framework/core/loadhooks.h"
#include "framework/core/memoryhandler.h"
#include "rw/rttoolkits.h"

namespace RWS {

// GLOBAL: 0x62fee4 RWS::CResource::sm_ResourceHead
CResource *CResource::sm_ResourceHead = 0;

// 0x550eb0
CResource::CResource() : m_next(sm_ResourceHead), m_prev(0), m_name(0), m_data(0), m_size(0), m_lockCount(0)
{
	m_type[0] = 0;
	m_id.Clear();
	if (sm_ResourceHead)
		sm_ResourceHead->m_prev = this;
	sm_ResourceHead = this;
}

CResource::~CResource()
{
	m_data = 0;
	if (m_prev)
		m_prev->m_next = m_next;
	else if (sm_ResourceHead == this)
		sm_ResourceHead = m_next;
	if (m_next)
		m_next->m_prev = m_prev;
}

void CResource::SetName(const char *name)
{
	const char *pooled = InternString(name);
	m_name = (pooled && *pooled) ? pooled : 0;
}

void CResource::SetType(const char *type)
{
	const char *pooled = InternString(type);
	if (pooled && *pooled) {
		// longer than the buffer is cut
		strncpy(m_type, pooled, TypeBufferSize - 1);
		m_type[TypeBufferSize - 1] = 0;
	} else {
		m_type[0] = 0;
	}
}

// 0x5512e0
void CResourceIterator::Remove(void)
{
	if (!m_current)
		return;
	CResource *following = m_current->GetNext();
	delete m_current;
	m_current = following;
}

namespace anon {

// Returns the handler's result: null (failed or no handler), (void *)-1 (payload consumed, not loaded, not recorded) or the object.
// 0x551160
void *LoadResource(const char *name, const RWSGUID *id, const char *type, const char *path, RwStream *stream, unsigned streamSize)
{
	CResourceHandler *handler = CResourceHandler::FindHandler(type);
	if (!handler)
		return 0;
	unsigned resourceSize = 0;
	void *data = handler->Load(name, type, path, stream, streamSize, resourceSize);
	if (data && data != reinterpret_cast<void *>(-1))
		CResourceManager::Add(name, id, type, data, resourceSize);
	return data;
}

}  // namespace anon

// 0x550f10
void CResourceManager::Add(const char *name, const RWSGUID *id, const char *type, const void *data, unsigned size)
{
	CResource *resource = RWS_NEW CResource;
	resource->SetName(name);
	if (id)
		resource->SetId(*id);
	resource->SetType(type);
	resource->SetData(data);
	resource->SetSize(size);
}

// 0x551000
const void *CResourceManager::FindById(const RWSGUID *id, const char **type, unsigned *size, const char **name)
{
	if (!id)
		return 0;
	for (CResourceIterator it; !it.IsDone(); ++it) {
		if (it->GetId() == *id) {
			if (type)
				*type = it->GetType();
			if (size)
				*size = it->GetSize();
			if (name)
				*name = it->GetName();
			return it->GetData();
		}
	}
	return 0;
}

// 0x551080
const void *CResourceManager::FindByName(const char *const name, RWSGUID *id, const char **type, unsigned *size)
{
	if (!name)
		return 0;
	for (CResourceIterator it; !it.IsDone(); ++it) {
		if (it->GetName() && rwstrcmp(it->GetName(), name) == 0) {
			if (id)
				*id = it->GetId();
			if (type)
				*type = it->GetType();
			if (size)
				*size = it->GetSize();
			return it->GetData();
		}
	}
	return 0;
}

// 0x551110
void CResourceManager::Lock(const void *ptr)
{
	for (CResourceIterator it; !it.IsDone(); ++it) {
		if (it->GetData() == ptr) {
			it->SetLockRefCount(it->GetLockRefCount() + 1);
			return;
		}
	}
}

// 0x551330
bool CResourceManager::PurgeItem(CResourceIterator &it)
{
	CResource &resource = *it;
	if (resource.GetLockRefCount() != 0)
		return false;
	CResourceHandler *handler = CResourceHandler::FindHandler(resource.GetType());
	if (!handler || !handler->UnLoad(resource.GetType(), const_cast<void *>(resource.GetData())))
		return false;
	it.Remove();
	return true;
}

// 0x5514c0
const void *CResourceManager::Remove(const void *data)
{
	for (CResourceIterator it; !it.IsDone(); ++it) {
		if (it->GetData() == data)
			return PurgeItem(it) ? data : 0;
	}
	return 0;
}

// 0x551380
bool CResourceManager::Purge(void)
{
	bool purgedAll = true;
	CResourceIterator it;
	while (!it.IsDone()) {
		if (!PurgeItem(it)) {
			purgedAll = false;
			++it;
		}
	}
	return purgedAll;
}

// 0x551420
bool CResourceManager::PurgeAll(void)
{
	bool purgedAll = true;
	CResourceIterator it;
	while (!it.IsDone()) {
		it->SetLockRefCount(0);
		if (!PurgeItem(it)) {
			purgedAll = false;
			++it;
		}
	}
	return purgedAll;
}

// 0x551140
void CResourceManager::Open(void)
{
	CResourceHandler::OpenAll();
}

// 0x551150
void CResourceManager::Close(void)
{
	CResourceHandler::CloseAll();
}

// 0x5512a0
void *CResourceManager::UpLoad(const char *name, const RWSGUID *id, const char *type, const char *location, RwStream *stream,
                               unsigned size)
{
	(void)location;   // embedded resources get an empty path
	void *data = anon::LoadResource(name, id, type, "", stream, size);
	TFBLoadingProgress();
	return data;
}

// 0x5511d0
void *CResourceManager::Load(const char *name, const RWSGUID *id, const char *type, const char *fileName, const char *location)
{
	char path[512];
	char file[512];
	strncpy(path, location ? location : "", sizeof(path) - 1);
	path[sizeof(path) - 1] = 0;
	strncpy(file, fileName ? fileName : "", sizeof(file) - 1);
	file[sizeof(file) - 1] = 0;

	void *handle = RwFopen(file, "rb");
	if (!handle)
		return 0;
	RwFseek(handle, 0, 2);   // SEEK_END
	unsigned fileSize = static_cast<unsigned>(RwFtell(handle));
	RwFclose(handle);

	RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, file);
	if (!stream)
		return 0;
	void *data = anon::LoadResource(name, id, type, path, stream, fileSize);
	RwStreamClose(stream, file);
	return data;
}

// Port: Level_ReleaseGameObjects frees the string pool while the resources that stay loaded keep their names in it. The game
// reads the freed bytes, which its heap leaves in place; here the names are copied before and interned again after.
char *SaveResourceNames()
{
	size_t n = 1;
	for (CResourceIterator it; !it.IsDone(); ++it)
		n += (it->GetName() ? strlen(it->GetName()) : 0) + 1;
	char *buf = (char *)malloc(n), *p = buf;
	if (!buf)
		return 0;
	for (CResourceIterator it; !it.IsDone(); ++it) {
		const char *s = it->GetName();
		size_t l = s ? strlen(s) : 0;
		memcpy(p, s ? s : "", l + 1);
		p += l + 1;
	}
	return buf;
}

void RestoreResourceNames(char *names)
{
	if (!names)
		return;
	const char *p = names;
	for (CResourceIterator it; !it.IsDone(); ++it) {
		it->SetName(p);   // "" (no name) stays 0
		p += strlen(p) + 1;
	}
	free(names);
}

}  // namespace RWS
