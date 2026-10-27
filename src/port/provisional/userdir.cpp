// The port's user folder: saves, screen shots, settings.ini. Replaces the game's GetUserDataPath (0x519d60, "<My Documents>\<section>"; PORT_REPLACES leaves out userdata.cpp)
// and keeps its section names, so every platform gets the same layout below its own root:
//   PORT_USER_DIR (environment)   tests and portable installs; port_run.cpp sets it to <exe dir>\user unless started with `u`
//   Windows                       My Documents, as the game (CSIDL_PERSONAL): saves shared with the original game
//   macOS                         ~/Library/Application Support (the app-private place; Documents would need a privacy prompt)
//   Linux                         $XDG_DATA_HOME, else ~/.local/share
//   Android (Quest)               not done: SDL_GetAndroidInternalStoragePath once that build exists
// e.g. the save folder "Activision\Madagascar\Save" -> ~/Library/Application Support/Activision/Madagascar/Save on macOS.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

#ifdef _WIN32
#define snprintf _snprintf   // the cl 13.10 lane (build_port.py) builds against the MSVC 7.1 CRT; _snprintf may leave the text unterminated
#endif

#include "zoosters/os.h"   // UserDataRoot (My Documents on Windows; the platform's per-user data folder with the SDL backend), MakeDir

extern "C" FILE *port_log;

static void native_separators(char *p)
{
	for (; *p; p++)
		if (*p == '\\' || *p == '/')
			*p = PORT_PATH_SEP;
}

extern "C" int port_user_root(char *buf, int size)
{
	char root[512] = "";
	const char *env = getenv("PORT_USER_DIR");
	if (env && *env)
		snprintf(root, sizeof(root), "%s", env);
	else {
		zoosters::os::UserDataRoot(root, sizeof(root));
	}
	root[sizeof(root) - 1] = 0;
	if (!*root || (int)strlen(root) >= size)
		return 0;
	native_separators(root);
	strcpy(buf, root);
	return 1;
}

extern "C" int port_user_file(const char *name, char *buf, int size)
{
	char root[512];
	if (!port_user_root(root, sizeof(root)))
		return 0;
	int n = snprintf(buf, size, "%s%c%s%c%s", root, PORT_PATH_SEP, PORT_USER_SUBDIR, PORT_PATH_SEP, name);
	if (n < 0 || n >= size) {
		*buf = 0;
		return 0;
	}
	native_separators(buf);
	return 1;
}

extern "C" int port_make_dirs(const char *path)
{
	char p[768];
	if (strlen(path) >= sizeof(p))
		return 0;
	strcpy(p, path);
	native_separators(p);
	for (char *s = p + 1; ; s++) {   // + 1: keep a leading separator (POSIX root)
		if (*s != PORT_PATH_SEP && *s)
			continue;
		char c = *s;
		*s = 0;
		if (!(s - p == 2 && p[1] == ':')) {   // "C:" is no directory to create
			zoosters::os::MakeDir(p);
		}
		if (!c)
			break;
		*s = c;
	}
	return 1;
}

// 0x519d60: "<user root>\<section>" (native separators) into buf; `fallback` when no root is known. Returns 0 (buf untouched) when
// the text needs `size` bytes or more, else 1 (the game's contract).
char GetUserDataPath(const char *section, const char *fallback, char *buf, int size)
{
	char path[768];
	if (port_user_root(path, sizeof(path))) {
		size_t n = strlen(path);
		snprintf(path + n, sizeof(path) - n, "%c%s", PORT_PATH_SEP, section);
	} else
		snprintf(path, sizeof(path), "%s", fallback);
	path[sizeof(path) - 1] = 0;
	native_separators(path);
	if (strlen(path) >= (unsigned)size)
		return 0;
	strcpy(buf, path);
	if (port_log)
		fprintf(port_log, "userdir: GetUserDataPath(%s) -> %s\n", section, buf);
	return 1;
}
