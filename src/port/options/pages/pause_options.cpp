// ZOOSTERS_OPTIONS: the pause menu's Options page, replaced by a C++ screen. Hooks in src/game (all `#ifdef ZOOSTERS_OPTIONS`):
// SpriteManager::Update calls port_options_frame once per frame; NoteControlSprite asks port_options_is_menu_sprite(name) so the
// game keeps the menu control buffer and the cursor while our sprites are up.
// Takeover: the pause script (PR_Pause.AI) goes to page 4 by showing its Options sprites (Set_Options_title is one). The frame in
// which that title is visible opens our screen; the original page-4 sprites are hidden every frame and the script's pad input is
// blocked (GameController::m_blockMask). Back commits the changed values to the registry once, unblocks the input and injects one
// triangle press: the script's own "pop one menu level" branch takes it back to page 1.
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../bridge/options_bridge.h"
#include "../settings.h"
#include "../display.h"
#include "../strings.h"
#include "../ui/ui.h"

using namespace opt_bridge;

namespace {
enum State { CLOSED, OPEN, CLOSING };
State g_state = CLOSED;
int g_closingFrames;
bool g_closeWhenReleased;                  // Back was chosen: close once no button is held (a held Cross / Enter / Esc then reached the
                                           // pause script as a new press: Back confirmed Goals)
bool g_title;                              // the screen was opened from the title menu (not the pause menu)
bool g_openRequest;                        // the title's Options row was accepted (ControlsManager::Poll)
int g_page;                                // 0 the options, 1 the Display sub-page
int g_snapshot[16];                        // values at open: only changed settings are written on close

const char *kDistLabels[10] = { "10", "20", "30", "40", "50", "60", "70", "80", "90", "Full" };   // the last one comes from the string table

// the page's text, through the string table (port/options/strings/en.txt via GetLocalizedString): OPT_<ID> for a setting
const char *settingLabel(const opt_settings::Setting &s)
{
	char name[40] = "OPT_";
	int n = 4;
	for (const char *p = s.id; *p && n < 38; p++) name[n++] = (char)toupper((unsigned char)*p);
	name[n] = 0;
	return opt_strings::Str(name, s.label);
}

int distIndex(int v) { return v <= 0 || v >= 95 ? 9 : (v + 5) / 10 - 1; }

// HUD scale: Auto, then 50 .. 100 in steps of 10 (the numbers are percent)
const char *kHudScaleLabels[7] = { "Auto", "50", "60", "70", "80", "90", "100" };   // the first comes from the string table
int hudScaleIndex(int p) { return p <= 0 ? 0 : p < 50 ? 1 : p > 100 ? 6 : (p - 40) / 10; }
const char *kModeLabels[3] = { "Windowed", "Borderless", "Full Screen" };   // from the string table
const char *kWidthLabels[2] = { "Full", "16:9" };

int pageRows(int page)
{
	int n = 0;
	for (int i = 0; i < opt_settings::g_settingCount; i++) n += opt_settings::g_settings[i].page == page;
	return n;
}

// one setting of the page as its control
void drawSetting(const opt_settings::Setting &s)
{
	int v = s.get();
	if (strcmp(s.id, "distance") == 0) {
		int idx = distIndex(v);
		if (opt_ui::Selector(settingLabel(s), &idx, kDistLabels, 10)) s.set(idx == 9 ? 0 : (idx + 1) * 10);
	} else if (strcmp(s.id, "resolution") == 0) {
		int idx = opt_display::ResolutionIndex();
		if (opt_ui::Selector(settingLabel(s), &idx, opt_display::ResolutionLabels(), opt_display::ResolutionCount())) s.set(opt_display::ResolutionCodeAt(idx));
	} else if (strcmp(s.id, "windowmode") == 0) {
		kModeLabels[0] = opt_strings::Str("OPT_WINDOWED", "Windowed");
		kModeLabels[1] = opt_strings::Str("OPT_BORDERLESS", "Borderless");
		kModeLabels[2] = opt_strings::Str("OPT_FULLSCREEN", "Full Screen");
		if (opt_ui::Selector(settingLabel(s), &v, kModeLabels, opt_display::ModeCount())) s.set(v);
	} else if (strcmp(s.id, "hudscale") == 0) {
		kHudScaleLabels[0] = opt_strings::Str("OPT_AUTO", "Auto");
		int idx = hudScaleIndex(v);
		if (opt_ui::Selector(settingLabel(s), &idx, kHudScaleLabels, 7)) s.set(idx == 0 ? 0 : 40 + idx * 10);
	} else if (strcmp(s.id, "hudwidth") == 0) {
		kWidthLabels[0] = opt_strings::Str("OPT_FULL", "Full");
		if (opt_ui::Selector(settingLabel(s), &v, kWidthLabels, 2)) s.set(v);
	} else if (s.kind == opt_settings::KIND_SLIDER) {
		if (opt_ui::Slider(settingLabel(s), &v, s.min, s.max, s.step)) s.set(v);
	} else {
		bool b = v != 0;
		if (opt_ui::Toggle(settingLabel(s), &b)) s.set(b ? 1 : 0);
	}
}
}  // namespace

static void openScreen(bool fromTitle)
{
	g_title = fromTitle;
	g_openRequest = false;
	Log(fromTitle ? "open (title Options row)" : "open (pause Options page detected)");
	opt_display::Refresh();
	for (int i = 0; i < opt_settings::g_settingCount && i < 16; i++) g_snapshot[i] = opt_settings::g_settings[i].get();
	g_page = 0;
	opt_ui::ResetFocus();
	opt_ui::SetBackdrop(fromTitle);
	InputCapture(true);
	g_closeWhenReleased = false;
	g_state = OPEN;
}

static void closeScreen(bool backToScript)
{
	opt_display::Flush();   // a pending window change is applied (or refused and reverted) before the values are compared
	for (int i = 0; i < opt_settings::g_settingCount && i < 16; i++)
		if (opt_settings::g_settings[i].get() != g_snapshot[i]) {
			opt_settings::g_settings[i].commit();
			Log("committed %s = %d", opt_settings::g_settings[i].id, opt_settings::g_settings[i].get());
		}
	opt_ui::SetBackdrop(false);
	opt_ui::Destroy();
	InputCapture(false);
	if (g_title) {
		opt_bridge::TitleRestoreMenu();   // the script's selection never left the Options row
		g_state = CLOSED;
		Log("closed (title)");
		return;
	}
	PauseRestoreFrame();
	if (backToScript) {
		PauseShowOptionsTitle();         // so the CLOSING wait can see the script hide it again
		InputInjectBack();
		g_closingFrames = 0;
		g_state = CLOSING;
	} else {
		g_state = CLOSED;
	}
	Log("closed");
}

extern "C" void port_options_frame(void)
{
	switch (g_state) {
	case CLOSED:
		if (opt_bridge::TitleMenuShown()) opt_bridge::TitleMovePrompt();
		if (g_openRequest && opt_bridge::TitleMenuShown()) openScreen(true);
		else if (!(PauseMenuShown() && PauseOptionsPageShown())) { g_openRequest = false; break; }
		else openScreen(false);
		// fall through: the screen runs in the frame it opens (else the game's own page-4 sprites were drawn for that one frame)
	case OPEN: {
		if (g_title) opt_bridge::TitleHideMenu();
		else if (!PauseMenuShown()) { closeScreen(false); break; }   // the pause menu went away under us
		else PauseHideOptionsPage();
		kDistLabels[9] = opt_strings::Str("OPT_FULL", "Full");
		opt_display::Tick();
		opt_ui::Begin(g_page ? opt_strings::Str("OPT_DISPLAY_TITLE", "DISPLAY") : opt_strings::Str("OPT_TITLE", "OPTIONS"));
		for (int i = 0; i < opt_settings::g_settingCount; i++) {
			if (opt_settings::g_settings[i].page != g_page) continue;
			if (strcmp(opt_settings::g_settings[i].id, "hudwidth") == 0 && !opt_display::WideScreen()) continue;   // nothing to limit up to 16:9
			drawSetting(opt_settings::g_settings[i]);
		}
		bool toDisplay = g_page == 0 && opt_ui::Button(opt_strings::Str("OPT_DISPLAY", "Display"));
		bool back = opt_ui::Button(opt_strings::Str("OPT_BACK", "Back"));
		opt_ui::End();
		back = back || opt_ui::BackRequested();
		if (toDisplay) { g_page = 1; opt_ui::ResetFocus(); back = false; }
		else if (back && g_page == 1) {
			g_page = 0;
			opt_ui::ResetFocus();
			opt_ui::SetFocus(pageRows(0));   // the Display entry, just after the page's settings
			back = false;
		}
		{ static int n; if (getenv("PORT_OPTIONS_DUMP") && ++n == 2) DebugDump(); }
		if (back) g_closeWhenReleased = true;
		if (g_closeWhenReleased && !InputAnyHeld()) closeScreen(true);
		break;
	}
	case CLOSING:
		if (!PauseOptionsPageShown()) { g_state = CLOSED; Log("script is back on the main page"); }
		else if (++g_closingFrames > 90) { Log("script did not leave page 4 within 90 frames"); g_state = CLOSED; }
		break;
	}
}

// ControlsManager::Poll: Accept on the title's Options row (selection index 3)
extern "C" void port_options_title_open(void) { g_openRequest = true; }

extern "C" const char *port_options_title_label(void) { return opt_strings::Str("OPT_TITLE_ENTRY", "Options"); }

extern "C" int port_options_is_menu_sprite(const char *name)
{
	return g_state == OPEN && name && strncmp(name, "port_opt_", 9) == 0;
}
