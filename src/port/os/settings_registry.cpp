// zoosters::settings on the Windows registry (--settings registry), shared with the original game and its launcher:
// HKCU\Software\<company>\<product>\<section>, then HKLM (installer defaults). Unlike the game, keys are closed again (the game
// leaks a handle per access).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "zoosters/settings.h"

namespace zoosters {
namespace settings {

namespace {

char g_company[0x40];
char g_product[0x60];

bool KeyName(const char *section, char *key, size_t size)
{
	int n = _snprintf(key, size, "Software\\%s\\%s\\%s", g_company, g_product, section);
	return n > 0 && (size_t)n < size;
}

// One read from one hive. `size` is the buffer size in, the value size out; false for anything but ERROR_SUCCESS (ERROR_MORE_DATA
// included).
bool Query(HKEY root, const char *key, const char *name, DWORD *type, void *data, DWORD *size)
{
	HKEY h;
	if (RegOpenKeyA(root, key, &h) != ERROR_SUCCESS)
		return false;
	bool ok = RegQueryValueExA(h, name, 0, type, (LPBYTE)data, size) == ERROR_SUCCESS;
	RegCloseKey(h);
	return ok;
}

// The game's GetValue: HKEY_CURRENT_USER, else HKEY_LOCAL_MACHINE (a value found in the first hive ends the search whatever its type).
bool Read(const char *section, const char *name, DWORD *type, void *data, DWORD *size)
{
	char key[0x104];
	if (!KeyName(section, key, sizeof(key)))
		return false;
	DWORD capacity = *size;
	if (Query(HKEY_CURRENT_USER, key, name, type, data, size))
		return true;
	*size = capacity;
	return Query(HKEY_LOCAL_MACHINE, key, name, type, data, size);
}

bool Write(const char *section, const char *name, DWORD type, const void *data, DWORD size)
{
	char key[0x104];
	HKEY h;
	if (!KeyName(section, key, sizeof(key)) || RegCreateKeyA(HKEY_CURRENT_USER, key, &h) != ERROR_SUCCESS)
		return false;
	bool ok = RegSetValueExA(h, name, 0, type, (const BYTE *)data, size) == ERROR_SUCCESS;
	RegCloseKey(h);
	return ok;
}

}

void SetApp(const char *company, const char *product)
{
	_snprintf(g_company, sizeof(g_company) - 1, "%s", company);
	_snprintf(g_product, sizeof(g_product) - 1, "%s", product);
}

Found GetInt(const char *section, const char *name, int *out)
{
	DWORD type, value, size = sizeof(value);
	if (!Read(section, name, &type, &value, &size))
		return Found::No;
	if (type != REG_DWORD)
		return Found::OtherType;
	*out = (int)value;
	return Found::Yes;
}

Found GetString(const char *section, const char *name, char *out, size_t cap)
{
	DWORD type, size = (DWORD)cap;
	if (!Read(section, name, &type, out, &size))
		return Found::No;
	return type == REG_SZ ? Found::Yes : Found::OtherType;
}

bool SetInt(const char *section, const char *name, int value)
{
	return Write(section, name, REG_DWORD, &value, sizeof(value));
}

// The game never writes a string setting (CfgObj::SetOption stores ints); this is plain REG_SZ with its NUL.
bool SetString(const char *section, const char *name, const char *value)
{
	return Write(section, name, REG_SZ, value, (DWORD)strlen(value) + 1);
}

}
}
