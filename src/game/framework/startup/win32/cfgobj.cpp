// A value is addressed by a path such as "Settings\Display\Resolution": the part before the last backslash is the sub key, the
// rest the value name. Handed to zoosters::settings, whose backend is the registry or a settings.ini.
// CFLAGS: /GS
#include <stdio.h>
#include <string.h>
#include "zoosters/base.h"
#include "framework/startup/win32/cfgobj.h"

#include "zoosters/settings.h"

ZOOSTERS_ASSERT_SIZE(CfgObj, 0x78);
ZOOSTERS_ASSERT_OFFSET(CfgObj, m_product, 0x28);

enum {
	kRegPathSize = 0x104   // MAX_PATH
};

// The last byte of each buffer is never written: a name of 39 / 79 characters or more stays unterminated.
// 0x519980
CfgObj::CfgObj(const char *company, const char *product)
{
	strncpy(m_company, company, sizeof(m_company) - 1);
	strncpy(m_product, product, sizeof(m_product) - 1);
	zoosters::settings::SetApp(company, product);
}

// Splits at the last backslash. False when there is none behind the first character or a part does not fit.
// 0x5199b0
bool CfgObj::SplitPath(const char *path, char *dir, unsigned dirSize, char *file, unsigned fileSize)
{
	int split = (int)strlen(path);
	while (split > 0 && path[split] != '\\')
		split--;
	if (split == 0)
		return false;
	if (dirSize <= (unsigned)split)
		return false;
	if (fileSize <= strlen(path) - split - 1)
		return false;
	strncpy(dir, path, split);
	dir[split] = 0;
	strcpy(file, path + split + 1);
	return true;
}

// Script option setter.
// 0x519c90
void CfgObj::SetOption(const char *name, int value)
{
	char section[kRegPathSize];
	char file[kRegPathSize];
	if (SplitPath(name, section, kRegPathSize, file, kRegPathSize))
		zoosters::settings::SetInt(section, file, value);
}

// `def` is copied when the value is missing and fits. True if a string value was read.
// 0x519cb0
bool CfgObj::GetString(const char *key, const char *def, char *out, unsigned long size)
{
	char section[kRegPathSize];
	char file[kRegPathSize];
	if (SplitPath(key, section, kRegPathSize, file, kRegPathSize)) {
		zoosters::settings::Found found = zoosters::settings::GetString(section, file, out, size);
		if (found != zoosters::settings::Found::No)
			return found == zoosters::settings::Found::Yes;   // a value of another kind keeps the buffer, as in the game
	}
	if (strlen(def) < size)
		strcpy(out, def);
	return false;
}

// 0x519d20
int CfgObj::GetInt(const char *key, int def)
{
	char section[kRegPathSize];
	char file[kRegPathSize];
	int value;
	if (SplitPath(key, section, kRegPathSize, file, kRegPathSize) && zoosters::settings::GetInt(section, file, &value) == zoosters::settings::Found::Yes)
		return value;
	return def;
}
