#include <stdio.h>
#include <string.h>
#include "framework/startup/win32/textdict.h"
#include "zoosters/os.h"

// A missing file leaves the object empty (IsLoaded is false).
// 0x51a250
TextDict::TextDict(const char *path)
{
	m_text = 0;
	m_lines = 0;
	m_textSize = 0;
	m_lineCount = 0;
	char native[260];   // as the file system spells it (POSIX: `/`, case matched)
	FILE *file = fopen(zoosters::os::NativePath(path, native, sizeof native), "rb");
	if (!file)
		return;
	fseek(file, 0, SEEK_END);
	m_textSize = ftell(file);
	m_text = (char *)MemAlloc(m_textSize + 1);
	fseek(file, 0, SEEK_SET);
	fread(m_text, 1, m_textSize, file);
	m_text[m_textSize] = 0;
	fclose(file);

	// every LF ends a line and is counted; every CR and LF becomes a NUL
	for (int i = 0; i < m_textSize; i++) {
		if (m_text[i] == '\n') {
			m_lineCount++;
			m_text[i] = 0;
		}
		if (m_text[i] == '\r')
			m_text[i] = 0;
	}
	m_lineCount++;   // the last line has no line end
	m_lines = (char **)MemAlloc(m_lineCount * sizeof(char *));

	// Runs of NULs (CR LF pairs, empty lines) count as one line end; the last character of the file always ends a line.
	// A backslash followed by 'n' becomes a real CR LF pair.
	bool afterLineEnd = false;
	int lineCount = 0;
	int lineStart = 0;
	for (int i = 0; i < m_textSize; i++) {
		char c = m_text[i];
		if (c == 0 || i == m_textSize - 1) {
			if (!afterLineEnd) {
				m_lines[lineCount++] = m_text + lineStart;
				afterLineEnd = true;
			}
			lineStart = i + 1;
		} else {
			afterLineEnd = false;
			if (c == '\\' && i < m_textSize - 1 && m_text[i + 1] == 'n') {
				m_text[i] = '\r';
				m_text[i + 1] = '\n';
			}
		}
	}
	m_lineCount = lineCount;
}

bool TextDict::IsLoaded() const
{
	return m_text && m_lines && m_textSize > 0 && m_lineCount > 0;
}

// The id must match completely up to the '=': "ID_A" doesn't find "ID_AB=".
// The game never frees the "name=" search key (a small leak per lookup); kept.
// 0x51a390
char *TextDict::Find(const char *name) const
{
	int nameLen = (int)strlen(name);
	char *key = (char *)MemAlloc(nameLen + 2);
	strcpy(key, name);
	key[nameLen] = '=';
	key[nameLen + 1] = 0;
	if (IsLoaded()) {
		for (int i = 0; i < m_lineCount; i++) {
			if (strncmp(m_lines[i], key, nameLen + 1) == 0)
				return m_lines[i] + nameLen + 1;
		}
	}
	return 0;
}

// 0x51a430
char *TextDict_Find(TextDict *dict, const char *name)
{
	if (!dict)
		return 0;
	return dict->Find(name);
}

// 0x51a450
void TextDict_Free(TextDict *dict)
{
	if (dict) {
		operator delete(dict->m_text);
		operator delete(dict->m_lines);
		operator delete(dict);
	}
}

// 0x51a480
TextDict *TextDict_Load(const char *path)
{
	TextDict *dict = new TextDict(path);
	if (dict->IsLoaded())
		return dict;
	TextDict_Free(dict);
	return 0;
}
