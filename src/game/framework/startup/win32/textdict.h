// The localised UI strings (data\mdgdict.bnx): "ID_NAME=text" lines, loaded once by Settings_Create.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_TEXTDICT_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_TEXTDICT_H

#include "framework/core/memoryhandler.h"

// Line ends become NULs, so each m_lines entry is a C string "NAME=text" inside the buffer. A literal backslash-n becomes CR LF
// when loading.
struct TextDict {
	char *m_text;     // the whole file plus a terminating NUL
	char **m_lines;   // every non-empty line
	int m_textSize;   // without the terminator
	int m_lineCount;

	TextDict(const char *path);
	char *Find(const char *name) const;   // text after "name=" or 0
	bool IsLoaded() const;                   

	static void *operator new(size_t size) { return MemAlloc(size); }
	static void operator delete(void *p) { ::operator delete(p); }
};

TextDict *TextDict_Load(const char *path);   // 0 when the file is missing or empty
char *TextDict_Find(TextDict *dict, const char *name);   // null-safe
void TextDict_Free(TextDict *dict);

#endif
