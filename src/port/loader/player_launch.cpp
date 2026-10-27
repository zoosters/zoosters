// Player launch of the portable build (--os sdl): the exe started without arguments (Finder, shortcut, .app bundle). Finds the
// game's data folder: PORT_GAME_DIR, else the one remembered in settings.ini ([Settings\Port] GameDir), else a `Game` folder (or
// the folder itself) next to the exe / .app, else a folder dialog; the choice is stored. A folder counts when it holds
// Levels/Title.stream. port_run.cpp then runs with no frame limit and the OS user folder (saves, settings.ini, port.log).
#include <SDL3/SDL.h>
#include <string>
#include "zoosters/os.h"
#include "zoosters/settings.h"

static const char kSection[] = "Settings\\Port", kName[] = "GameDir";

static bool IsGameDir(const std::string &dir)
{
	if (dir.empty())
		return false;
	std::string probe = dir + "/Levels/Title.stream";
	char native[1024];
	bool isDir = false;
	uint64_t size = 0;
	return zoosters::os::FileInfo(zoosters::os::NativePath(probe.c_str(), native, sizeof native), &isDir, &size) && !isDir && size > 0;
}

// The folder that holds the exe, or the .app bundle when the exe is inside one (<x>.app/Contents/MacOS/<exe>).
static std::string LaunchFolder()
{
	const char *base = SDL_GetBasePath();   // the exe's folder; Contents/Resources/ inside a bundle
	if (!base)
		return "";
	std::string dir(base);
	while (!dir.empty() && (dir.back() == '/' || dir.back() == '\\'))
		dir.pop_back();
	size_t app = dir.rfind(".app/Contents");
	if (app != std::string::npos) {
		dir.resize(app + 4);                // .../<x>.app
		size_t cut = dir.find_last_of('/');
		dir.resize(cut == std::string::npos ? 0 : cut);
	}
	return dir;
}

struct DialogState { bool done = false; std::string path; };
static void SDLCALL OnFolder(void *user, const char *const *files, int)
{
	DialogState *st = (DialogState *)user;
	if (files && files[0])
		st->path = files[0];
	st->done = true;
}

static std::string AskForFolder(const std::string &start)
{
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
		return "";
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Madagascar",
	                         "Choose the folder of your Madagascar installation (the one with the Levels folder, e.g. \"Game\").", nullptr);
	DialogState st;
	SDL_ShowOpenFolderDialog(OnFolder, &st, nullptr, start.empty() ? nullptr : start.c_str(), false);
	while (!st.done) {
		SDL_Event e;
		if (SDL_WaitEventTimeout(&e, 100) && e.type == SDL_EVENT_QUIT)
			break;
	}
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return st.path;
}

// -> 1 and the game folder in `out`; 0 when none was found or chosen (a message box has said so).
extern "C" int port_player_game_dir(char *out, int size)
{
	std::string dir;
	const char *env = SDL_getenv("PORT_GAME_DIR");
	if (env && IsGameDir(env))
		dir = env;
	char stored[1024];
	if (dir.empty() && zoosters::settings::GetString(kSection, kName, stored, sizeof stored) == zoosters::settings::Found::Yes && IsGameDir(stored))
		dir = stored;
	std::string near = LaunchFolder();
	if (dir.empty() && IsGameDir(near + "/Game"))
		dir = near + "/Game";
	if (dir.empty() && IsGameDir(near))
		dir = near;
	for (int tries = 0; dir.empty() && tries < 3; tries++) {
		std::string picked = AskForFolder(near);
		if (picked.empty())
			break;
		if (IsGameDir(picked))
			dir = picked;
		else if (IsGameDir(picked + "/Game"))
			dir = picked + "/Game";
		else
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Madagascar", "That folder has no Levels/Title.stream. Choose the game's folder.", nullptr);
	}
	if (dir.empty()) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Madagascar", "No game folder: the game cannot start.", nullptr);
		return 0;
	}
	if (dir.size() + 1 > (size_t)size)
		return 0;
	if (!(zoosters::settings::GetString(kSection, kName, stored, sizeof stored) == zoosters::settings::Found::Yes && dir == stored))
		zoosters::settings::SetString(kSection, kName, dir.c_str());
	SDL_strlcpy(out, dir.c_str(), size);
	return 1;
}

// Where the bundled mod files are: Contents/Resources/Mod in the .app (SDL_GetBasePath), else <exe folder>/Mod. -> 1 when found.
extern "C" int port_player_mod_dir(char *out, int size)
{
	const char *base = SDL_GetBasePath();
	if (!base)
		return 0;
	std::string mod = std::string(base) + "Mod";
	bool isDir = false;
	uint64_t bytes = 0;
	if (!zoosters::os::FileInfo(mod.c_str(), &isDir, &bytes) || !isDir || mod.size() + 1 > (size_t)size)
		return 0;
	SDL_strlcpy(out, mod.c_str(), size);
	return 1;
}
