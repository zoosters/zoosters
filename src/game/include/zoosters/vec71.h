// Normal build only: a std::vector look-alike with the MSVC 7.1 object layout {allocator word, first, last, end} (16 bytes).
// The game reads some vectors through layout structs ({int alloc; T *first; T *last;}: LdRegVec, Vec8, SymbolRegistry, ...) written against
// the 7.1 library; the modern std::vector has no allocator word, so a std::vector owner and such a view disagree. The owner uses this type
// instead (typedef under #ifndef ZOOSTERS_GAME_ABI). Only what the game's call sites use: size, [], push_back, reserve, insert(pos, n, v),
// erase(first, last), begin/end.
#ifndef ZOOSTERS_VEC71_H
#define ZOOSTERS_VEC71_H

#include <stdlib.h>
#include <string.h>
#include <new>

namespace zoosters {

template <class T>
struct Vec71 {
	int m_alloc;  // (the 7.1 allocator object, empty)
	T *m_first;
	T *m_last;
	T *m_end;

	Vec71() : m_alloc(0), m_first(0), m_last(0), m_end(0) {}
	~Vec71() { free(m_first); }
	Vec71(const Vec71 &) = delete;
	Vec71 &operator=(const Vec71 &) = delete;

	void reset() { free(m_first); m_first = m_last = m_end = 0; }   // the game's `vector t(0, e); v.swap(t)` idiom
	unsigned size() const { return m_first == 0 ? 0 : (unsigned)(m_last - m_first); }
	unsigned capacity() const { return m_first == 0 ? 0 : (unsigned)(m_end - m_first); }
	T *begin() { return m_first; }
	T *end() { return m_last; }
	T &operator[](unsigned i) { return m_first[i]; }

	void reserve(unsigned n)
	{
		if (capacity() >= n)
			return;
		unsigned sz = size();
		T *p = (T *)malloc(n * sizeof(T));
		if (!p)
			throw std::bad_alloc();
		if (sz)
			memcpy(p, m_first, sz * sizeof(T));
		free(m_first);
		m_first = p;
		m_last = p + sz;
		m_end = p + n;
	}
	void push_back(const T &v)
	{
		if (m_last == m_end)
			reserve(size() ? size() + size() / 2 + 1 : 1);
		*m_last++ = v;
	}
	void resize(unsigned n, T v)   // grow with copies of v / shrink (7.1 resize(n, c))
	{
		if (n < size())
			erase(m_first + n, m_last);
		else if (n > size())
			insert(m_last, n - size(), v);
	}
	void insert(T *pos, unsigned n, const T &v)   // only used at the end: `insert(end, n, v)`
	{
		unsigned at = (unsigned)(pos - m_first);
		unsigned sz = size();
		if (sz + n > capacity())
			reserve(sz + n);
		T *p = m_first + at;
		memmove(p + n, p, (sz - at) * sizeof(T));
		for (unsigned i = 0; i < n; i++)
			p[i] = v;
		m_last = m_first + sz + n;
	}
	void erase(T *first, T *last)
	{
		if (first == last)
			return;
		memmove(first, last, (size_t)((char *)m_last - (char *)last));
		m_last -= (last - first);
	}
};

}  // namespace zoosters

#endif
