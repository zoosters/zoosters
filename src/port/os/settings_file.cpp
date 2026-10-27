// zoosters::settings backed by <user folder>/<company>/<product>/settings.ini (--settings file; the only choice off Windows):
//   [Settings\Display]
//   Resolution=1024x768
// A section is a registry sub key path; names match case-insensitively. A whole-number value (decimal or 0x hex) reads back as an int, anything else as a string.
// The file is read once and rewritten whole (via settings.ini.tmp) only when a value changes; comments are not kept. No installer defaults: a missing value reads as not found.
// On Windows builds without ZOOSTERS_OS_SDL a missing file is first filled from the original game's HKCU\Software\<company>\<product> (read only).
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>
#include "zoosters/settings.h"
#include "../provisional/platform.h"
#if defined(_WIN32) && !defined(ZOOSTERS_OS_SDL)
#define ZOOSTERS_SETTINGS_IMPORT 1
#include <windows.h>
#endif

extern "C" FILE *port_log;
#define CLOG(...) do { if (port_log) fprintf(port_log, __VA_ARGS__); } while (0)

namespace zoosters {
namespace settings {

namespace {

struct Value { std::string name, text; };
struct Section { std::string name; std::vector<Value> values; };

std::vector<Section> g_sections;
std::string g_product = "Madagascar";
std::string g_company;   // set by SetApp (CfgObj's constructor runs before the first read); empty = unknown, no registry import
bool g_loaded;
char g_path[600];

bool same(const std::string &a, const char *b)
{
	size_t n = strlen(b);
	if (a.size() != n)
		return false;
	for (size_t i = 0; i < n; i++) {
		char x = a[i], y = b[i];
		if (x >= 'A' && x <= 'Z') x += 32;
		if (y >= 'A' && y <= 'Z') y += 32;
		if (x != y)
			return false;
	}
	return true;
}

std::string trim(const std::string &s)
{
	size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
	return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}

Section *find_section(const char *name)
{
	for (Section &s : g_sections)
		if (same(s.name, name))
			return &s;
	return 0;
}

Value *find_value(Section *s, const char *name)
{
	if (s)
		for (Value &v : s->values)
			if (same(v.name, name))
				return &v;
	return 0;
}

// One line per value: line breaks become blanks, at most 511 characters (SetString and the import share it).
std::string one_line(const char *p, size_t n)
{
	std::string text;
	for (size_t i = 0; i < n && p[i] && text.size() < 511; i++)
		text += (p[i] == '\n' || p[i] == '\r') ? ' ' : p[i];
	return text;
}

#ifdef ZOOSTERS_SETTINGS_IMPORT
struct Import { int imported, skipped; };

// Copies the values of `key` (section = `path`, "" for the root) and then its sub keys. REG_DWORD becomes decimal text, REG_SZ its text;
// other types and names that cannot round-trip through the file ("=" or a line break) are skipped.
void import_key(HKEY key, const std::string &path, int depth, Import *r)
{
	DWORD nSub, maxSub, nVal, maxName, maxData;
	if (RegQueryInfoKeyA(key, 0, 0, 0, &nSub, &maxSub, 0, &nVal, &maxName, &maxData, 0, 0) != ERROR_SUCCESS)
		return;
	std::vector<char> name(maxName + 2), data(maxData + 2);
	for (DWORD i = 0; i < nVal; i++) {
		DWORD nameLen = (DWORD)name.size(), type, size = (DWORD)data.size() - 1;
		if (RegEnumValueA(key, i, name.data(), &nameLen, 0, &type, (BYTE *)data.data(), &size) != ERROR_SUCCESS)
			continue;
		std::string text;
		if (type == REG_DWORD && size == 4) {
			char num[16];
			snprintf(num, sizeof(num), "%d", *(const int *)data.data());
			text = num;
		} else if (type == REG_SZ) {
			data[size] = 0;   // stored strings need not end in a NUL
			text = trim(one_line(data.data(), size));
		} else {
			r->skipped++;
			continue;
		}
		std::string vname(name.data(), nameLen);
		if (vname.empty() || vname.find_first_of("=\r\n") != std::string::npos || vname[0] == ';' || vname[0] == '#' || vname[0] == '[') {
			r->skipped++;
			continue;
		}
		Section *s = find_section(path.c_str());
		if (!s) {
			g_sections.push_back(Section{path, {}});
			s = &g_sections.back();
		}
		if (find_value(s, vname.c_str()))
			continue;   // same name differing only in case: the registry would not have both either
		s->values.push_back(Value{vname, text});
		r->imported++;
	}
	if (depth >= 8)
		return;
	std::vector<char> sub(maxSub + 2);
	for (DWORD i = 0; i < nSub; i++) {
		DWORD subLen = (DWORD)sub.size();
		if (RegEnumKeyExA(key, i, sub.data(), &subLen, 0, 0, 0, 0) != ERROR_SUCCESS)
			continue;
		std::string subName(sub.data(), subLen);
		HKEY child;
		if (RegOpenKeyExA(key, subName.c_str(), 0, KEY_READ, &child) != ERROR_SUCCESS)
			continue;
		import_key(child, path.empty() ? subName : path + "\\" + subName, depth + 1, r);
		RegCloseKey(child);
	}
}

// settings.ini does not exist: fills it once from HKCU\Software\<company>\<product> (read only) and writes it. No key = no file (defaults).
void import_registry();
#endif

void load()
{
	if (g_loaded)
		return;
	g_loaded = true;
	if (!port_user_file("settings.ini", g_path, sizeof(g_path))) {
		CLOG("settings: no user folder, settings are not kept\n");
		return;
	}
	FILE *f = fopen(g_path, "rb");
	if (!f) {
#ifdef ZOOSTERS_SETTINGS_IMPORT
		if (!g_company.empty()) {   // CfgObj's constructor (SetApp) runs before the first read
			import_registry();
			return;
		}
#endif
		CLOG("settings: %s not found (defaults)\n", g_path);
		return;
	}
	char line[1024];
	Section *cur = 0;
	int count = 0;
	while (fgets(line, sizeof(line), f)) {
		std::string l = trim(line);
		if (l.empty() || l[0] == ';' || l[0] == '#')
			continue;
		if (l[0] == '[' && l[l.size() - 1] == ']') {
			std::string name = trim(l.substr(1, l.size() - 2));
			cur = find_section(name.c_str());
			if (!cur) {
				g_sections.push_back(Section{name, {}});
				cur = &g_sections.back();
			}
			continue;
		}
		size_t eq = l.find('=');
		if (!cur || eq == std::string::npos)
			continue;
		cur->values.push_back(Value{trim(l.substr(0, eq)), trim(l.substr(eq + 1))});
		count++;
	}
	fclose(f);
	CLOG("settings: %s: %d values\n", g_path, count);
}

bool save()
{
	if (!*g_path)
		return false;
	char dir[600];
	strcpy(dir, g_path);
	char *sl = strrchr(dir, PORT_PATH_SEP);
	if (sl) {
		*sl = 0;
		port_make_dirs(dir);
	}
	char tmp[620];
	snprintf(tmp, sizeof(tmp), "%s.tmp", g_path);
	FILE *f = fopen(tmp, "wb");
	if (!f) {
		CLOG("settings: cannot write %s (errno %d)\n", tmp, errno);
		return false;
	}
	fprintf(f, "; %s settings (one [section] per group of settings)\n", g_product.c_str());
	for (const Section &s : g_sections) {
		fprintf(f, "\n[%s]\n", s.name.c_str());
		for (const Value &v : s.values)
			fprintf(f, "%s=%s\n", v.name.c_str(), v.text.c_str());
	}
	bool ok = fclose(f) == 0;
	{   // replaces an existing file (std::filesystem::rename: MoveFileEx(REPLACE_EXISTING) on Windows, rename(2) elsewhere)
		std::error_code ec;
		std::filesystem::rename(tmp, g_path, ec);
		ok = ok && !ec;
	}
	if (!ok)
		CLOG("settings: cannot replace %s\n", g_path);
	return ok;
}

#ifdef ZOOSTERS_SETTINGS_IMPORT
void import_registry()
{
	std::string root = "Software\\" + g_company + "\\" + g_product;
	HKEY key;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, root.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
		CLOG("settings: %s not found, no registry values in HKCU\\%s (defaults)\n", g_path, root.c_str());
		return;
	}
	Import r = {0, 0};
	import_key(key, std::string(), 0, &r);
	RegCloseKey(key);
	if (r.imported && save())
		CLOG("settings: imported %d values from HKCU\\%s (%d skipped)\n", r.imported, root.c_str(), r.skipped);
	else
		CLOG("settings: %s not found, no registry values in HKCU\\%s (%d skipped)\n", g_path, root.c_str(), r.skipped);
}
#endif

// Whole number -> its low 32 bits; false for anything else (then the value is a string).
bool parse_dword(const std::string &t, unsigned *out)
{
	if (t.empty())
		return false;
	const char *s = t.c_str();
	char *end;
	errno = 0;
	long long v = (t.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) ? strtoll(s + 2, &end, 16) : strtoll(s, &end, 10);
	if (*end || errno || end == s || v < -2147483648LL || v > 4294967295LL)
		return false;
	*out = (unsigned)v;
	return true;
}

// Stores `text` under section / name and rewrites the file when that changed something.
bool store(const char *section, const char *name, const char *text)
{
	load();
	if (!section || !name)
		return false;
	Section *s = find_section(section);
	if (!s) {
		g_sections.push_back(Section{section, {}});
		s = &g_sections.back();
	}
	Value *v = find_value(s, name);
	if (v && v->text == text)
		return true;   // unchanged: no rewrite
	if (v)
		v->text = text;
	else
		s->values.push_back(Value{name, text});
	CLOG("settings: [%s] %s=%s\n", section, name, text);
	return save();
}

}

void SetApp(const char *company, const char *product)
{
	g_company = company;   // the file's place is the port's user folder (userdir.cpp); the names only locate the registry key of the import
	g_product = product;
}

Found GetInt(const char *section, const char *name, int *out)
{
	load();
	const Value *v = find_value(find_section(section ? section : ""), name ? name : "");
	if (!v)
		return Found::No;
	unsigned dword;
	if (!parse_dword(v->text, &dword))
		return Found::OtherType;
	*out = (int)dword;
	return Found::Yes;
}

Found GetString(const char *section, const char *name, char *out, size_t size)
{
	load();
	const Value *v = find_value(find_section(section ? section : ""), name ? name : "");
	if (!v)
		return Found::No;
	unsigned dword;
	bool isInt = parse_dword(v->text, &dword);
	size_t need = isInt ? 4 : v->text.size() + 1;   // a too small buffer reads as missing (ERROR_MORE_DATA), as in the registry
	if (size < need)
		return Found::No;
	if (isInt)
		return Found::OtherType;
	memcpy(out, v->text.c_str(), need);
	return Found::Yes;
}

bool SetInt(const char *section, const char *name, int value)
{
	char text[16];
	snprintf(text, sizeof(text), "%d", value);
	return store(section, name, text);
}

bool SetString(const char *section, const char *name, const char *value)
{
	std::string text;
	for (const char *p = value ? value : ""; *p && text.size() < 511; p++)
		text += (*p == '\n' || *p == '\r') ? ' ' : *p;   // one line per value
	return store(section, name, text.c_str());
}

}
}
