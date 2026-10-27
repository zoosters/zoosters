// zoosters::settings: the settings store behind the normal build's CfgObj. Backend picked at link time: port/os/settings_registry.cpp
// (HKCU\Software\<company>\<product>, reads fall back to HKLM like the game) or port/os/settings_file.cpp
// (<user folder>/<company>/<product>/settings.ini; qol tier, --os sdl, every non-Windows platform).
// The cl 13.10 view (ZOOSTERS_GAME_ABI) never uses it: CfgObj keeps the game's registry calls there.
// A value is addressed by section (backslash-separated path below the root, e.g. "Settings\\Display") and name ("Resolution"); int or string.
#ifndef ZOOSTERS_SETTINGS_H
#define ZOOSTERS_SETTINGS_H

#include <stddef.h>

namespace zoosters {
namespace settings {

enum class Found { No, Yes, OtherType };   // OtherType: the value exists but holds the other kind (the game's REG_SZ / REG_DWORD type checks)

void SetApp(const char *company, const char *product);   // CfgObj's constructor
Found GetInt(const char *section, const char *name, int *out);
Found GetString(const char *section, const char *name, char *out, size_t size);   // size includes the NUL; a value that does not fit = No (the registry answers ERROR_MORE_DATA, the game then treats it as missing)
bool SetInt(const char *section, const char *name, int value);
bool SetString(const char *section, const char *name, const char *value);

}
}

#endif
