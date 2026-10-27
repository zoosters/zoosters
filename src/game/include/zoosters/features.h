// Port feature macros, derived from the build tier (--tier <vanilla|fixes|qol> passes /DZOOSTERS_TIER=<0|1|2> and
// force-includes this header). Code tests the feature macro (#ifdef ZOOSTERS_FIX_BUGS), never ZOOSTERS_TIER. Tiers are
// cumulative. A new feature = one line here. Without ZOOSTERS_TIER (the cl 13.10 build, any tool that never sees this header)
// the result is vanilla.
#ifndef ZOOSTERS_FEATURES_H
#define ZOOSTERS_FEATURES_H

#if defined(ZOOSTERS_FIX_BUGS) || defined(ZOOSTERS_WIDESCREEN) || defined(ZOOSTERS_OPTIONS) || defined(ZOOSTERS_MOUSE_LOOK)
#error "feature macros come from zoosters/features.h: pick a tier with build option --tier"
#endif

#ifndef ZOOSTERS_TIER
#define ZOOSTERS_TIER 0   // 0 vanilla, 1 fixes, 2 qol
#endif

#if ZOOSTERS_TIER >= 1
// Behaviour that is clearly unintended and harmful (crashes, hazards, plain logic errors). Covers: no display mode change (WinMain never
// calls ChangeDisplaySettingsA, the stored resolution is fitted to the screen: port/fixes/display.cpp).
#define ZOOSTERS_FIX_BUGS
#endif

#if ZOOSTERS_TIER >= 2
// Modern display basics: Hor+ camera, 4:3 HUD grid with edge anchoring, pillarboxed loading screen, desktop resolution when none is
// stored, DPI awareness (port/fixes/widescreen.cpp).
#define ZOOSTERS_WIDESCREEN

// In-game options: the pause menu's Options page is replaced by a C++ screen, which the title menu opens too through its own Options row
// (port/options/).
#define ZOOSTERS_OPTIONS

// Mouse look on by default (Settings\Display\MouseLook missing = on): the original launcher wrote 1, the game's own default is 0, so a port
// without the launcher started with the mouse ignored. MouseLook=0 still turns it off.
#define ZOOSTERS_MOUSE_LOOK
#endif

#endif
