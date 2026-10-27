// The game's language selection: a global index into the language tables (18 entries). The saved game stores the language id.
#ifndef ZOOSTERS_TFB_SAVE_LANGUAGE_H
#define ZOOSTERS_TFB_SAVE_LANGUAGE_H

extern const char *g_langCodes[18];  // "US", "FR", ...
extern const char *g_langNames[18];  // "English", ...
extern int g_langIndex;

const char *GetLanguageCode(int index);  // "_XX" for an index outside the table
int IsCurrentLanguage(int index);
int IsCurrentLanguageName(const char *name);
int GetLanguage();                             // 0x4476c0
void SetLanguage(int index);
bool IsLanguage7Selectable();  // sets language 7 and checks that it took (restores the old one)

#endif
