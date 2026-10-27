// Application state of the start-up layer: window, settings, quit / frame flags, mouse pointer. The addresses are annotated in config.cpp.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_APPSTATE_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_APPSTATE_H

#include "zoosters/base.h"

struct CfgObj;
struct TextDict;
namespace RWS { namespace TFB { class GameSprite; } }

extern void *g_mainWnd;   // HWND of the window the platform layer created
extern void *DAT_0062a5dc;   // the same window, kept by WinMain
namespace RWS { namespace Win { void *GetMainWindow(); } }
extern TextDict *g_dict;
extern int g_effectsVolume;   // Settings\Options\Effect
extern int g_musicVolume;   // Settings\Options\Music
extern bool g_detailedShadows;   // Settings\Display\DetailedShadows
extern bool g_camDirectTurn;   // Settings\Display\MouseLook
extern unsigned char g_unlockLevels;   // Settings\Options\UnlockLevels
extern unsigned g_resWidth;   // Settings\Display\Resolution, "<w>x<h>"
extern unsigned g_resHeight;
extern unsigned char g_distClip;   // Settings\Display\DistanceClipping
extern int g_distance;   // Settings\Display\Distance
extern unsigned char g_wndWindowed;   // WinMain clears it: the PC game is always full screen
extern unsigned char g_mainLoopEntered;
extern unsigned char g_quitRequested;   // WM_CLOSE / WM_DESTROY seen
extern unsigned char g_frameReady;   // the window is active (App_SetActive): run frames
extern int g_globalSymbolCount;   // script symbols after levels\global.stream

// Requests of the window procedure, polled elsewhere.
extern unsigned char g_wndRequestFlag;
extern int g_wndRequestA;
extern int g_wndRequestB;
extern unsigned char g_cfgQuitOnInput;   // debug option: a key / click quits
extern unsigned char g_cfgKeyReqA;   // debug option: a key press requests 0x10
extern unsigned char g_cfgKeyReqB;   // debug option: a key press requests 4
extern unsigned char g_mouseButtonDown;   // left button held
extern unsigned char g_cursorShown;   // the mouse pointer sprite is on (framework/cursor.cpp)
extern RWS::TFB::GameSprite *g_cursorSprite;   // the sprite "bx_MousePointer"
extern unsigned char g_cursorSaved;   // g_cursorShown saved by Cursor_HideSave

CfgObj *Settings_Create();
// Pointer-sized on x64: a 32-bit lParam would truncate WM_CREATE's CREATESTRUCT pointer.
#if   !defined(ZOOSTERS_PTR64)
typedef unsigned WinWParam;
typedef long WinLParam;
typedef long WinLResult;
#else
typedef unsigned long long WinWParam;
typedef long long WinLParam;
typedef long long WinLResult;
#endif
WinLResult __stdcall GameWndProc(void *hwnd, unsigned msg, WinWParam wParam, WinLParam lParam);
extern "C"   // MSVC gives WinMain C linkage implicitly; port/loader/port_run.cpp calls it by that name
int __stdcall WinMain(void *hInstance, void *hPrev, char *cmdLine, int showCmd);
void Cursor_Show(unsigned char show);
void Cursor_HideSave();
void Cursor_Restore();
void Pad_SelectControlBuffer(int index);
// menupointer.cpp
bool HudButtonHitTest(int x, int y, RWS::TFB::GameSprite *button);
int MenuPointer_Update(int x, int y, bool down, bool click);

#endif
