// ZOOSTERS_OPTIONS string table (see strings.h).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strings.h"
#include "bridge/options_bridge.h"

char *GetLocalizedString(int id);   // 0x44f580 (src/game/tfb/level/resourcehandlers.cpp; hooked for ids >= 0x10000)

namespace opt_strings {
namespace {
struct Entry { char name[40]; char *text; };
const int kMax = 160;
Entry g_e[kMax];
int g_n;
bool g_loaded;

int find(const char *name)
{
	for (int i = 0; i < g_n; i++)
		if (_stricmp(g_e[i].name, name) == 0) return i;
	return -1;
}

int add(const char *name, const char *text)
{
	if (g_n >= kMax) return -1;
	Entry &e = g_e[g_n];
	strncpy(e.name, name, sizeof(e.name) - 1); e.name[sizeof(e.name) - 1] = 0;
	size_t n = strlen(text);
	e.text = (char *)malloc(n + 1);
	size_t k = 0;
	for (size_t i = 0; i < n; i++)
		if (text[i] != '%') e.text[k++] = text[i];     // text sprites take the string as a format: no '%'
	e.text[k] = 0;
	return g_n++;
}

void load()
{
	g_loaded = true;
	char path[520];
	if (!opt_bridge::ModFile("Levels\\options_en.txt", path, sizeof(path))) { opt_bridge::Log("strings: no options_en.txt, built-in English"); return; }
	FILE *f = fopen(path, "rb");
	if (!f) return;
	char line[400];
	int n = 0;
	while (fgets(line, sizeof(line), f)) {
		size_t len = strlen(line);
		while (len && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = 0;
		if (!len || line[0] == '#' || line[0] == ';') continue;
		char *eq = strchr(line, '=');
		if (!eq || eq == line) continue;
		*eq = 0;
		if (find(line) < 0 && add(line, eq + 1) >= 0) n++;
	}
	fclose(f);
	opt_bridge::Log("strings: %d from %s", n, path);
}
}  // namespace

int Id(const char *name, const char *fallback)
{
	if (!g_loaded) load();
	int i = find(name);
	if (i < 0) i = add(name, fallback);
	return i < 0 ? 0 : kFirstId + i;
}

const char *Text(int id)
{
	int i = id - kFirstId;
	return (i >= 0 && i < g_n) ? g_e[i].text : 0;
}

const char *Str(const char *name, const char *fallback)
{
	int id = Id(name, fallback);
	if (!id) return fallback;
	const char *t = GetLocalizedString(id);
	return t ? t : fallback;
}

}  // namespace opt_strings

extern "C" const char *port_options_string(int id) { return opt_strings::Text(id); }   // hook of GetLocalizedString
