// Common definitions: fixed-width integer types (cl 13.10 has no <stdint.h>) and the compile-time checks that pin the game's layouts.
#ifndef ZOOSTERS_BASE_H
#define ZOOSTERS_BASE_H

#include <stddef.h>

#include <stdint.h>

// ZOOSTERS_PTR64: pointers are 8 bytes (Windows x64, and every LP64 target: macOS / Linux / Android arm64 and x86_64). Code that means
// "64-bit layout" tests this, never _WIN64 (false on macOS arm64, so such a site silently took the 32-bit path).
#if defined(_WIN64) || defined(__LP64__) || defined(_LP64)
#define ZOOSTERS_PTR64 1
#endif

// ZOOSTERS_ASSERT_SIZE(T, n) / ZOOSTERS_ASSERT_OFFSET(T, m, off): namespace-scope checks of the game's 32-bit struct layouts.
// Active only where pointers are 4 bytes and the ABI is MSVC-like: the game build (ZOOSTERS_GAME_ABI, cl 13.10; MSVC 7.1 library
// types, std::vector/string/map are 16/28/12 bytes) and the normal 32-bit build (ZOOSTERS_NORMAL_BUILD, which reads game data by
// these layouts; zoosters::Vec71 mirrors the old containers). Other builds check nothing, nor does 32-bit on the Itanium ABI
// (i686 MinGW), where empty bases and vptrs of multiply-inherited classes sit differently.
#if defined(_MSC_VER)
#define ZOOSTERS_LAYOUT_CHECK(cond) (sizeof(void *) != 4 || (cond))
#else
#define ZOOSTERS_LAYOUT_CHECK(cond) 1
#endif
#ifndef ZOOSTERS_ASSERT_SIZE
#define ZOOSTERS_ASSERT_SIZE(T, n) \
	typedef char zoosters_size_check_##T[ZOOSTERS_LAYOUT_CHECK(sizeof(T) == (n)) ? 1 : -1]
#endif
#define ZOOSTERS_ASSERT_OFFSET(T, m, off) \
	typedef char zoosters_offset_check_##T##_##m[ZOOSTERS_LAYOUT_CHECK(offsetof(T, m) == (off)) ? 1 : -1]
// ZOOSTERS_ASSERT_OFFSET_AT(tag, expr, off): the same for an offset expression that is no plain member (nested designator
// `offsetof(T, a.b)`, a member of a nested class, a base offset plus a member offset); `tag` makes the typedef name unique.
#define ZOOSTERS_ASSERT_OFFSET_AT(tag, expr, off) \
	typedef char zoosters_offset_check_##tag[ZOOSTERS_LAYOUT_CHECK((expr) == (off)) ? 1 : -1]
#if defined(__clang__) || defined(__GNUC__)
// offsetof on a class with bases or virtuals (GameActor, Corpus) is "conditionally supported"; every compiler we use supports it
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

// ZOOSTERS_NOINLINE: keeps a function out of line (the attribute form outside MSVC).
#if defined(_MSC_VER)
#define ZOOSTERS_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define ZOOSTERS_NOINLINE __attribute__((noinline))
#else
#define ZOOSTERS_NOINLINE
#endif

// ZOOSTERS_PROTO_REGISTER_AFTER(stmt): for a script type prototype whose constructor registers ops / members that call the TFBType_*
// accessor again while its function-local static is still being built. MSVC 7.1 marks the static initialised before running the
// constructor, so the inner call sees the half-built object; standard C++ runs the initialiser again and recurses forever.
// So registration runs once, in the accessor, right after construction.
#define ZOOSTERS_PROTO_REGISTER_AFTER(stmt) do { static bool zoosters_registered_; if (!zoosters_registered_) { zoosters_registered_ = true; stmt; } } while (0)

// zoosters::Opaque(p): p unchanged, but the optimiser may assume nothing about the object it points to. For the game's
// ScriptReference-shaped members whose vtable pointer is overwritten with another class's (UserReference::m_type,
// ScriptSubset::m_type): calls through them must dispatch through that vtable pointer, and clang binds them statically from the
// declared type. MSVC needs no barrier; the cl 13.10 view keeps the game's direct expressions.
#ifdef __cplusplus
namespace zoosters {
template <class T> inline T *Opaque(T *p)
{
#if defined(__GNUC__) || defined(__clang__)
	__asm__("" : "+r"(p));
#endif
	return p;
}
}
#endif

#endif
