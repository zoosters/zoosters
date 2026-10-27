// ZOOSTERS_OPTIONS display settings: resolution, window mode, HUD scale and width. Window mode and resolution apply a moment after the
// last change (a held arrow key steps through the list: one mode switch, not one per step) or when the page closes; a change the window
// system refuses falls back to the previous state. HUD scale and width go to the store right away (widescreen.cpp reads them there).
#ifndef PORT_OPTIONS_DISPLAY_H
#define PORT_OPTIONS_DISPLAY_H

namespace opt_display {

enum { MODE_WINDOWED, MODE_BORDERLESS, MODE_FULLSCREEN, MODE_COUNT };

void Refresh();                     // page opens: reads the current state, rebuilds the resolution list
void Tick();                        // once per frame: applies a pending change after a short pause
void Flush();                       // applies a pending change now

// Resolution as w << 16 | h, 0 = desktop (borderless always 0: the window is the desktop)
int Resolution();
void SetResolution(int code);
void CommitResolution();
int ResolutionCount();              // the list of the selector: Desktop first, then the display's modes, smallest first
const char *const *ResolutionLabels();
int ResolutionIndex();
int ResolutionCodeAt(int index);

int ModeCount();                    // 2 on macOS (no exclusive full screen there), else 3
bool WideScreen();                  // the window is wider than 16:9 (the HUD Width row is offered)
int WindowMode();
void SetWindowMode(int mode);
void CommitWindowMode();

int HudScale();                     // percent, 0 = automatic
void SetHudScale(int percent);
int HudWidth();                     // 0 full, 1 16:9
void SetHudWidth(int v);

}  // namespace opt_display

#endif
