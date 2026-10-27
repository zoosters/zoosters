// ZOOSTERS_OPTIONS bridge: the only code of the in-game options (port/options/) that touches game internals (sprites, input,
// settings store, pause script state). Everything above it (ui/, pages/, settings) sees plain ints, floats and handles.
// Coordinates are the game's 640x480 HUD grid (the widescreen HUD mapping applies on top).
#ifndef PORT_OPTIONS_BRIDGE_H
#define PORT_OPTIONS_BRIDGE_H

namespace opt_bridge {

// ---- sprites (run-time sprites of the game's SpriteManager; the handle is an index, 0 = none) ----------------------------------------
typedef int Sprite;
enum Anchor { ANCHOR_TOP_LEFT = 0, ANCHOR_TOP_RIGHT = 1, ANCHOR_TOP_CENTRE = 2, ANCHOR_BOTTOM_LEFT = 3, ANCHOR_BOTTOM_RIGHT = 4,
              ANCHOR_BOTTOM_CENTRE = 5, ANCHOR_MIDDLE_LEFT = 6, ANCHOR_MIDDLE_RIGHT = 7, ANCHOR_CENTRE = 8 };

Sprite SpriteCreate(const char *name);              // the name must stay valid (static); an existing sprite of that name is reused; hidden at first
void SpriteDestroy(Sprite s);
void SpriteSetQuad(Sprite s, const char *texture);  // a textured quad (a texture of the level's dictionary), size = the texture's until SetSize
void SpriteSetText(Sprite s, const char *text);     // a text sprite (the menu font tsa16); no '%' in the text
void SpriteSetAnchor(Sprite s, int anchor);         // which point of the sprite its position gives
void SpriteSetPos(Sprite s, float x, float y);
void SpriteSetScale(Sprite s, float sx, float sy);  // 1.0 = the texture / font size
void SpriteSetSize(Sprite s, float w, float h);     // quads: size in grid units (derives the scale from the texture size)
void SpriteSetTint(Sprite s, unsigned argb);        // 0xAARRGGBB; for quads the game's neutral colour is 0x808080
void SpriteSetPriority(Sprite s, int priority);     // draw order: higher draws earlier (behind); negative = in front of the pause menu (0..3)
void SpriteSetVisible(Sprite s, bool visible);
bool SpriteBounds(Sprite s, float *x, float *y, float *w, float *h);   // the box drawn last frame (grid units); false until resolved
bool TextureSize(const char *texture, float *w, float *h);              // false when the level has no such texture
void SpritesFlush();                                // once per frame after the changes: tells the SpriteManager (list sort / re-resolve)

// ---- input: one-press events from the game's menu control buffer (arrows = DPad, Enter / Space = accept, Esc = back)
// ---------------------
enum Nav { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT, NAV_ACCEPT, NAV_BACK, NAV_COUNT };
void InputPoll();                                   // once per frame: reads the buffer, makes events (held keys repeat after a delay)
bool InputAnyHeld();                                // a menu input (arrows, accept, back) is down (as of the last InputPoll)
bool InputPressed(int nav);                         // the event of this frame (consumed by InputPoll of the next frame)
void InputCapture(bool on);                         // true: the pause script no longer sees the pad / keys (its input is blocked)
void InputInjectBack();                             // one script-side "back" (triangle) press: the pause script pops its Options page
struct MouseState { bool valid; float x, y; bool moved, down, pressed; };   // grid units; valid = the cursor is shown
MouseState Mouse();
void PlaySound(int which);                          // 0 move, 1 accept, 2 cancel: the pause script's UI_Select / UI_Accept / UI_Cancel sounds; no-op when the level has none

// ---- the pause menu (script PR_Pause.AI) ----------------------------------------------------------------------------------------------
bool PauseOptionsPageShown();                       // the script has switched to page 4 (new_menu_list_ID 4): its title sprite is visible
bool PauseMenuShown();                              // the pause menu is up (menu_master visible)
void PauseHideOptionsPage();                        // hides the original page-4 sprites (every frame while our screen is up)
void PauseRestoreFrame();                           // shows the pause menu's frame strips again (hidden while our screen is up)
void PauseShowOptionsTitle();                       // shows the page-4 title again (the script hides it itself when it leaves the page)

// ---- the title menu (its list is a script set; the options entry is a spawned row, see ReferenceSet::Insert) -------------------------------
bool TitleMenuShown();                              // the title's New Game / Load Game list is on screen
void TitleMovePrompt();                             // the button prompt below the four rows (every frame the list is shown)
void TitleHideMenu();                               // hides the list, its title, the selection indicator and the prompt (every frame while our screen is up)
void TitleRestoreMenu();                            // gives them the visibility they had

// ---- mod files (our own art / text, never game data)
// --------------------------------------------------------------------------------- Looked up as `<game dir>\Mod\<rel>` first,
// then `<dir of the port exe>\Mod\<rel>` (PORT_MOD_DIR overrides both): the game dir may stay untouched.
bool ModFile(const char *rel, char *out, int outSize);   // true + the path of an existing file
bool LoadModAssets();                               // loads Mod\Levels\options.stream (textures, global = stays resident) once; false when absent

// ---- screen (the game's screen-sized state; display changes at run time)
// -------------------------------------------------------------------------------------------------------------------
bool ScreenSize(int *w, int *h);                    // the main camera's raster size in pixels; false before the camera exists
void ScreenResized(int w, int h);                   // camera rasters, the game's resolution globals and the cached 2D fonts take the new size

// ---- settings (game variables; the registry is written only by Commit)
// -------------------------------------------------------------------
enum SettingKey { KEY_MUSIC, KEY_EFFECTS, KEY_RUMBLE_ON, KEY_SHADOWS, KEY_DIST_CLIP, KEY_DISTANCE, KEY_COUNT };
int  SettingGet(int key);                           // the live value (music / effects 0..100, booleans 0/1, distance 1..100)
void SettingSet(int key, int value);                // live, in memory only
void SettingCommit(int key, const char *regPath);   // writes the current value to the settings store under regPath ("Settings\\Options\\Music")
int  ConfigGetInt(const char *regPath, int def);    // CfgObj::GetInt
void ConfigSetInt(const char *regPath, int value);  // CfgObj::SetOption
bool ConfigGetString(const char *regPath, char *out, int size);   // false when missing
void ConfigSetString(const char *regPath, const char *value);
void DebugDump();                                   // logs every visible sprite of ours (debugging)
void Log(const char *fmt, ...);                     // port log

}  // namespace opt_bridge

#endif
