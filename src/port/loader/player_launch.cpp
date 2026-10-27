// Player launch: the exe started without arguments (Finder, Explorer, a shortcut, the .app bundle). Finds the game's data folder:
// PORT_GAME_DIR, else the one remembered in settings.ini ([Settings\Port] GameDir), else a `Game` folder (or the folder itself) next
// to the exe / .app, else a folder dialog; the choice is stored. A folder counts when it holds Levels/Title.stream. port_run.cpp then
// runs with no frame limit and the OS user folder (saves, settings.ini, port.log). The dialogs are SDL3's with the sdl OS layer, the
// shell's with win32.
#include <stdlib.h>
#include <string.h>
#include <string>
#ifdef ZOOSTERS_OS_SDL
#include <SDL3/SDL.h>
#else
#include <windows.h>
#include <shobjidl.h>
#endif
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

#ifdef ZOOSTERS_OS_SDL
// The exe's folder with a trailing separator; Contents/Resources/ inside a bundle.
static std::string ExeFolder()
{
	const char *base = SDL_GetBasePath();
	return base ? base : "";
}

static void Message(int icon, const char *text)
{
	SDL_ShowSimpleMessageBox(icon == 2 ? SDL_MESSAGEBOX_ERROR : icon == 1 ? SDL_MESSAGEBOX_WARNING : SDL_MESSAGEBOX_INFORMATION,
	                         "Madagascar", text, nullptr);
}

struct DialogState { bool done = false; std::string path; };
static void SDLCALL OnFolder(void *user, const char *const *files, int)
{
	DialogState *st = (DialogState *)user;
	if (files && files[0])
		st->path = files[0];
	st->done = true;
}

static std::string PickFolder(const std::string &start)
{
	DialogState st;
	SDL_ShowOpenFolderDialog(OnFolder, &st, nullptr, start.empty() ? nullptr : start.c_str(), false);
	while (!st.done) {
		SDL_Event e;
		if (SDL_WaitEventTimeout(&e, 100) && e.type == SDL_EVENT_QUIT)
			break;
	}
	return st.path;
}

static bool DialogsBegin() { return SDL_InitSubSystem(SDL_INIT_VIDEO); }
static void DialogsEnd() { SDL_QuitSubSystem(SDL_INIT_VIDEO); }
#else
static std::string ExeFolder()
{
	char path[MAX_PATH];
	DWORD n = GetModuleFileNameA(0, path, sizeof path);
	if (!n || n >= sizeof path)
		return "";
	char *sl = strrchr(path, '\\');
	if (!sl)
		return "";
	sl[1] = 0;
	return path;
}

static void Message(int icon, const char *text)
{
	MessageBoxA(0, text, "Madagascar", MB_OK | MB_SETFOREGROUND | (icon == 2 ? MB_ICONERROR : icon == 1 ? MB_ICONWARNING : MB_ICONINFORMATION));
}

// The shell's folder picker (IFileOpenDialog, FOS_PICKFOLDERS); the path in the ANSI code page, as the rest of the win32 OS layer.
static std::string PickFolder(const std::string &start)
{
	std::string out;
	IFileOpenDialog *dlg = 0;
	if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, 0, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (void **)&dlg)))
		return out;
	DWORD opts = 0;
	dlg->GetOptions(&opts);
	dlg->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
	dlg->SetTitle(L"Madagascar: choose the game folder");
	wchar_t wstart[MAX_PATH];
	IShellItem *folder = 0;
	if (!start.empty() && MultiByteToWideChar(CP_ACP, 0, start.c_str(), -1, wstart, MAX_PATH) &&
	    SUCCEEDED(SHCreateItemFromParsingName(wstart, 0, IID_IShellItem, (void **)&folder))) {
		dlg->SetFolder(folder);
		folder->Release();
	}
	IShellItem *item = 0;
	if (SUCCEEDED(dlg->Show(0)) && SUCCEEDED(dlg->GetResult(&item))) {
		wchar_t *wpath = 0;
		if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &wpath))) {
			char path[MAX_PATH * 2];
			if (WideCharToMultiByte(CP_ACP, 0, wpath, -1, path, sizeof path, 0, 0))
				out = path;
			CoTaskMemFree(wpath);
		}
		item->Release();
	}
	dlg->Release();
	return out;
}

static bool g_comInit;
static bool DialogsBegin() { g_comInit = SUCCEEDED(CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)); return true; }
static void DialogsEnd() { if (g_comInit) CoUninitialize(); g_comInit = false; }
#endif

// The folder that holds the exe, or the .app bundle when the exe is inside one (<x>.app/Contents/MacOS/<exe>).
static std::string LaunchFolder()
{
	std::string dir = ExeFolder();
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

static std::string AskForFolder(const std::string &start)
{
	if (!DialogsBegin())
		return "";
	Message(0, "Choose the folder of your Madagascar installation (the one with the Levels folder, e.g. \"Game\").");
	std::string path = PickFolder(start);
	DialogsEnd();
	return path;
}

// -> 1 and the game folder in `out`; 0 when none was found or chosen (a message box has said so).
extern "C" int port_player_game_dir(char *out, int size)
{
	zoosters::settings::SetApp("Activision", "Madagascar");   // as CfgObj: the first read may import the original game's registry settings
	std::string dir;
	const char *env = getenv("PORT_GAME_DIR");
	if (env && IsGameDir(env))
		dir = env;
	char stored[1024];
	if (dir.empty() && zoosters::settings::GetString(kSection, kName, stored, sizeof stored) == zoosters::settings::Found::Yes && IsGameDir(stored))
		dir = stored;
	std::string launch = LaunchFolder();
	if (dir.empty() && IsGameDir(launch + "/Game"))
		dir = launch + "/Game";
	if (dir.empty() && IsGameDir(launch))
		dir = launch;
	for (int tries = 0; dir.empty() && tries < 3; tries++) {
		std::string picked = AskForFolder(launch);
		if (picked.empty())
			break;
		if (IsGameDir(picked))
			dir = picked;
		else if (IsGameDir(picked + "/Game"))
			dir = picked + "/Game";
		else
			Message(1, "That folder has no Levels/Title.stream. Choose the game's folder.");
	}
	if (dir.empty()) {
		Message(2, "No game folder: the game cannot start.");
		return 0;
	}
	if (dir.size() + 1 > (size_t)size)
		return 0;
	if (!(zoosters::settings::GetString(kSection, kName, stored, sizeof stored) == zoosters::settings::Found::Yes && dir == stored))
		zoosters::settings::SetString(kSection, kName, dir.c_str());
	strcpy(out, dir.c_str());
	return 1;
}

// Where the bundled mod files are: Contents/Resources/Mod in the .app, else <exe folder>/Mod. -> 1 when found.
extern "C" int port_player_mod_dir(char *out, int size)
{
	std::string mod = ExeFolder();
	if (mod.empty())
		return 0;
	mod += "Mod";
	bool isDir = false;
	uint64_t bytes = 0;
	if (!zoosters::os::FileInfo(mod.c_str(), &isDir, &bytes) || !isDir || mod.size() + 1 > (size_t)size)
		return 0;
	strcpy(out, mod.c_str());
	return 1;
}
