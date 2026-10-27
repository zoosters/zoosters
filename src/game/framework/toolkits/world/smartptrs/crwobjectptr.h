// Base of the owning RenderWare object pointers. One pointer wide, no vtable: behaviour layouts rely on it.
#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CRWOBJECTPTR_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_SMARTPTRS_CRWOBJECTPTR_H

namespace RWS {

template <class T> class CRwObjectPtr {
public:
	CRwObjectPtr() : m_ptr(0) {}
	CRwObjectPtr(T *ptr) : m_ptr(ptr) {}

	operator T *() const { return m_ptr; }
	T *operator->() const { return m_ptr; }
	T &operator*() const { return *m_ptr; }

	T *ptr() const { return m_ptr; }
	// the old object is left alone
	void setptr(T *ptr) { m_ptr = ptr; }

protected:
	T *m_ptr;
};

} // namespace RWS

#endif
