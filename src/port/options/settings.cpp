// ZOOSTERS_OPTIONS settings table (see settings.h).
#include <string.h>
#include "settings.h"
#include "bridge/options_bridge.h"
#include "display.h"

namespace opt_settings {
namespace {
using namespace opt_bridge;

int getMusic() { return SettingGet(KEY_MUSIC); }
void setMusic(int v) { SettingSet(KEY_MUSIC, v); }
void commitMusic() { SettingCommit(KEY_MUSIC, "Settings\\Options\\Music"); }

int getEffects() { return SettingGet(KEY_EFFECTS); }
void setEffects(int v) { SettingSet(KEY_EFFECTS, v); }
void commitEffects() { SettingCommit(KEY_EFFECTS, "Settings\\Options\\Effect"); }

int getVibration() { return SettingGet(KEY_RUMBLE_ON); }              // the registry holds the inverse: Options\RumbleOff
void setVibration(int v) { SettingSet(KEY_RUMBLE_ON, v); }
void commitVibration() { SettingCommit(KEY_RUMBLE_ON, "Settings\\Options\\RumbleOff"); }

int getShadows() { return SettingGet(KEY_SHADOWS); }
void setShadows(int v) { SettingSet(KEY_SHADOWS, v); }
void commitShadows() { SettingCommit(KEY_SHADOWS, "Settings\\Display\\DetailedShadows"); }

// One control for two registry values: 0 = Full (DistanceClipping 0, Distance untouched), 1..100 = clipping on at that Distance.
int getDistance() { return SettingGet(KEY_DIST_CLIP) ? SettingGet(KEY_DISTANCE) : 0; }
void setDistance(int v)
{
	if (v <= 0) { SettingSet(KEY_DIST_CLIP, 0); return; }
	SettingSet(KEY_DIST_CLIP, 1);
	SettingSet(KEY_DISTANCE, v);
}
void commitDistance()
{
	SettingCommit(KEY_DIST_CLIP, "Settings\\Display\\DistanceClipping");
	SettingCommit(KEY_DISTANCE, "Settings\\Display\\Distance");
}
// Pad camera (port/provisional/gamepad_*.cpp): invert x / y against the default (modern horizontal, the game's vertical)
extern "C" int port_pad_invert_x, port_pad_invert_y;
extern "C" void port_gamepad_invert_camera(int axis, int value);
int getInvertX() { port_gamepad_invert_camera(0, -1); return port_pad_invert_x; }
void setInvertX(int v) { port_gamepad_invert_camera(0, v); }
void commitInvertX() { ConfigSetInt("Settings\\Port\\PadInvertCameraX", getInvertX()); }
int getInvertY() { port_gamepad_invert_camera(0, -1); return port_pad_invert_y; }
void setInvertY(int v) { port_gamepad_invert_camera(1, v); }
void commitInvertY() { ConfigSetInt("Settings\\Port\\PadInvertCameraY", getInvertY()); }

// Display page: the selectors work on codes (resolution w << 16 | h, 0 = desktop; HUD scale in percent, 0 = automatic)
int getResolution() { return opt_display::Resolution(); }
void setResolution(int v) { opt_display::SetResolution(v); }
int getWindowMode() { return opt_display::WindowMode(); }
void setWindowMode(int v) { opt_display::SetWindowMode(v); }
int getHudScale() { return opt_display::HudScale(); }
void setHudScale(int v) { opt_display::SetHudScale(v); }
void commitHudScale() { ConfigSetInt("Settings\\Port\\HudScale", getHudScale()); }
int getHudWidth() { return opt_display::HudWidth(); }
void setHudWidth(int v) { opt_display::SetHudWidth(v); }
void commitHudWidth() { ConfigSetInt("Settings\\Port\\HudWidth", getHudWidth()); }
}  // namespace

const Setting g_settings[] = {
	{ "music",    "Music Volume",    "Settings\\Options\\Music",             KIND_SLIDER, 0, 100, 5, 75, APPLY_LIVE, getMusic, setMusic, commitMusic },
	{ "effects",  "Sound Volume",    "Settings\\Options\\Effect",            KIND_SLIDER, 0, 100, 5, 75, APPLY_LIVE, getEffects, setEffects, commitEffects },
	{ "vibration", "Vibration",      "Settings\\Options\\RumbleOff",         KIND_TOGGLE, 0, 1, 1, 1, APPLY_LIVE, getVibration, setVibration, commitVibration },
	{ "invertx",  "Invert Camera X", "Settings\\Port\\PadInvertCameraX",       KIND_TOGGLE, 0, 1, 1, 0, APPLY_LIVE, getInvertX, setInvertX, commitInvertX },
	{ "inverty",  "Invert Camera Y", "Settings\\Port\\PadInvertCameraY",       KIND_TOGGLE, 0, 1, 1, 0, APPLY_LIVE, getInvertY, setInvertY, commitInvertY },
	{ "shadows",  "Detailed Shadows", "Settings\\Display\\DetailedShadows",  KIND_TOGGLE, 0, 1, 1, 1, APPLY_LIVE, getShadows, setShadows, commitShadows },
	{ "distance", "Draw Distance",   "Settings\\Display\\Distance",          KIND_CHOICE, 0, 100, 10, 0, APPLY_LIVE, getDistance, setDistance, commitDistance },
	{ "windowmode", "Window Mode",   "Settings\\Port\\WindowMode",          KIND_CHOICE, 0, 2, 1, 0, APPLY_LIVE, getWindowMode, setWindowMode, opt_display::CommitWindowMode, 1 },
	{ "resolution", "Resolution",    "Settings\\Display\\Resolution",       KIND_CHOICE, 0, 0, 1, 0, APPLY_LIVE, getResolution, setResolution, opt_display::CommitResolution, 1 },
	{ "hudscale", "HUD Scale",       "Settings\\Port\\HudScale",            KIND_CHOICE, 0, 100, 10, 0, APPLY_LIVE, getHudScale, setHudScale, commitHudScale, 1 },
	{ "hudwidth", "HUD Width",       "Settings\\Port\\HudWidth",            KIND_CHOICE, 0, 1, 1, 0, APPLY_LIVE, getHudWidth, setHudWidth, commitHudWidth, 1 },
};
const int g_settingCount = sizeof(g_settings) / sizeof(g_settings[0]);

const Setting *Find(const char *id)
{
	for (int i = 0; i < g_settingCount; i++)
		if (strcmp(g_settings[i].id, id) == 0) return &g_settings[i];
	return 0;
}

}  // namespace opt_settings
