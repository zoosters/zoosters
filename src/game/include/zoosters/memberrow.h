// zoosters::MemberRow<A, B, ...>: the member table of a script type in the normal build. The game's GetMembers returns an array of
// one-pointer member objects of DIFFERENT classes (each is only its vptr; callers index it as ScriptReference[n]). The normal build
// constructs the real member objects in order in one block of n pointer-sized slots, so each has its own class's compiler vtable.
#ifndef ZOOSTERS_MEMBERROW_H
#define ZOOSTERS_MEMBERROW_H

#include <stddef.h>
#include <new>

namespace zoosters {

// A member slot whose class is bigger than its vptr (the set-valued members: the object is 0x18 bytes). The game builds a
// temporary and keeps only its vptr in the one-pointer slot; the member's functions only read their `owner` argument, never
// `this`'s fields. Same here: the slot's dynamic type is T with T's own compiler vtable (primary vptr at offset 0: single
// inheritance, MSVC and Itanium ABIs). The vptr a T gets from its compiler vtable: the game stores it into static objects it
// never constructs (`*(void **)&g_proto = vtable`).
template<class T>
void *VptrOf()
{
	alignas(T) unsigned char tmp[sizeof(T)];
	T *t = new (tmp) T();
	void *vptr = *reinterpret_cast<void **>(t);
	t->~T();
	return vptr;
}

template<class T>
class VptrOnly {
	void *m_vptr;

public:
	VptrOnly() : m_vptr(VptrOf<T>()) {}
};

template<class... Ts>
class MemberRow {
	static_assert(((sizeof(Ts) == sizeof(void *)) && ...), "a member object is exactly its vptr (the callers index the row by pointer size)");
	alignas(void *) unsigned char m_slots[sizeof...(Ts) * sizeof(void *)];

public:
	MemberRow()
	{
		size_t i = 0;
		((new (m_slots + sizeof(void *) * i++) Ts()), ...);
	}
	~MemberRow()
	{
		size_t i = 0;
		((reinterpret_cast<Ts *>(m_slots + sizeof(void *) * i++)->~Ts()), ...);   // VptrOnly<T>: trivial (the slot is not a whole T)
	}
	MemberRow(const MemberRow &) = delete;
	MemberRow &operator=(const MemberRow &) = delete;
	void *data() { return m_slots; }
	static constexpr int count = (int)sizeof...(Ts);
};

}  // namespace zoosters

#endif
