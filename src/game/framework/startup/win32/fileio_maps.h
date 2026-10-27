// The file I/O layer's tables: open handles and queued overlapped requests. The game's are stdext::hash_map; other C++ libraries
// get std::unordered_map (the walk order doesn't matter). Defined in src/data/normal/p4b_static_init.cpp.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_FILEIO_MAPS_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_FILEIO_MAPS_H
#if defined(_MSC_VER)
#include <stddef.h>   // uintptr_t (MSVC 7.1 has no stdint.h)
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#include <hash_map>
#define ZOOSTERS_FILEIO_HASH_MAP stdext::hash_map
#else
#include <stdint.h>
#include <unordered_map>
#define ZOOSTERS_FILEIO_HASH_MAP std::unordered_map
#endif

struct ReqRec;
typedef ZOOSTERS_FILEIO_HASH_MAP<uintptr_t, bool> FileMap;       // handle -> opened with FILE_FLAG_OVERLAPPED
typedef ZOOSTERS_FILEIO_HASH_MAP<unsigned, ReqRec *> ReqMap;    // request id -> queued request

#endif
