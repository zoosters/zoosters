// ZOOSTERS_OPTIONS UI kit: immediate-mode widgets over a pool of the game's sprites (through the bridge only). Per frame:
//   Begin("TITLE"); Slider(...); Toggle(...); Selector(...); Button(...); End;
// Focus is a row index moved by up / down (or by the mouse hovering a row); left / right change the focused row's value, accept
// presses a button; a mouse click on the arrows / slider / toggle / button acts directly. Sprites not used in a frame are hidden.
#ifndef PORT_OPTIONS_UI_H
#define PORT_OPTIONS_UI_H

namespace opt_ui {

void Begin(const char *title);                                              // reads the input, draws the frame and the focus bar
bool Slider(const char *label, int *value, int lo, int hi, int step);       // true when the value changed
bool Toggle(const char *label, bool *value);
bool Selector(const char *label, int *index, const char *const *items, int count);   // "< item >", no wrap
bool Button(const char *label);                                             // true on accept / click
bool BackRequested();                                                       // the back key (Esc / triangle) was pressed this frame
void End();
void Destroy();                                                             // screen closed: removes the sprites (the next Begin recreates them)
void ResetFocus();
void SetBackdrop(bool on);                            // dims the screen behind the panel (title menu)
void SetFocus(int row);                               // after ResetFocus: the row (0-based) with the focus bar

}  // namespace opt_ui

#endif
