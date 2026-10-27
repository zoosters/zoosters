// The language selection: tables of language codes and names and the current index.
#include <mbstring.h>
#include "tfb/save/language.h"

// GLOBAL: 0x626554 g_langIndex
// GLOBAL: 0x6035f8 g_langCodes
// GLOBAL: 0x6035b0 g_langNames

// 0x447630
const char *GetLanguageCode(int index)
{
	if (index >= 0 && index < 18)
		return g_langCodes[index];
	return "_XX";
}

// 0x447650
int IsCurrentLanguage(int index)
{
	return g_langIndex == index;
}

// 0x447670
int IsCurrentLanguageName(const char *name)
{
	const char *own = (g_langIndex >= 0 && g_langIndex < 18) ? g_langNames[g_langIndex] : "UNKNOWN";
	return _mbscmp((const unsigned char *)own, (const unsigned char *)name) == 0;
}

// 0x4476d0
__declspec(noinline) void SetLanguage(int index)   // the game's IsLanguage7Selectable is in another object and calls it
{
	if (index >= 0 && index < 18)
		g_langIndex = index;
}

// Probes whether language 7 can be selected: selects it, checks the index took, puts the old language back.
// 0x4456c0
bool IsLanguage7Selectable()
{
	int saved = GetLanguage();
	SetLanguage(7);
	if (GetLanguage() == 7) {
		SetLanguage(saved);
		return true;
	}
	SetLanguage(saved);
	return false;
}
