#include "tfb/level/levelmanager.h"
#include "framework/core/loadhooks.h"
#include "framework/mainloop/gameclock.h"
#include "framework/mainloop/mainloop.h"
#include "framework/startup/startup.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/platform.h"
#include "modules/audio/audioglobalmixer.h"
#include "rw/rttoolkits.h"
#include "script/symbolregistry.h"
#include "tfb/level/bink.h"
#include "script/ops_message.h"
#include "script/types/scriptsystem.h"
#include "tfb/hud/hud2d.h"
#include "tfb/render/shadow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: 0x606a40 g_LevelTable
// GLOBAL: 0x606a38 g_currentLevel
// GLOBAL: 0x62a2f8 g_nextLevel
// GLOBAL: 0x6221b4 g_playMode
// GLOBAL: 0x602ea0 g_globalSymbolCount
// GLOBAL: 0x621f38 g_typeSymCount
// GLOBAL: 0x5d5a90 g_introVideos
// GLOBAL: 0x62a1e0 g_loadStartTime
// GLOBAL: 0x62a1e4 g_loadDuration
// GLOBAL: 0x62a1e8 g_loadOfsX
// GLOBAL: 0x62a1ec g_loadOfsY
// GLOBAL: 0x62f1e0 g_dsoundObj
// GLOBAL: 0x5ca284 _imp__BinkSetSoundSystem
// GLOBAL: 0x5ca280 _imp__BinkOpenDirectSound
// GLOBAL: 0x5ca248 _imp__SetCapture

extern const char *g_introVideos[4];   // bgm\02.krn .. bgm\05.krn
extern int g_typeSymCount;             // script symbols of the built-in types (the first global symbol after them)
extern unsigned g_loadStartTime;       // the loading progress (framework/loadprogress.cpp)
extern unsigned g_loadDuration;
extern float g_loadOfsX;
extern float g_loadOfsY;
#include "zoosters/os.h"
static inline void SetCapture(void *wnd) { zoosters::os::CaptureMouse(wnd, wnd != 0); }

int PlayIntroVideo(const char *path);           // 0x460c50 (video.cpp)
int PlayVideo(const char *path, bool skippable); // 0x460b90 (video.cpp): 0 when it was skipped
unsigned GetTimeMs();                           // 0x44d2c0
int ControllerElapsedMs(char reset);   // 0x402860
float GetPlayerInput(int input, float threshold, int player);   // 0x402880
void BlockInputsUntilReleased(int ms);          // 0x4028a0
void CtlRefresh();                              // 0x406370 reads the controllers
int SymbolRegistry_GetSize();                   // 0x432cb0
void *GetActorSerialCounter();   // 0x42f360
Script::ScriptReference *TFBType_User();        // 0x43c530
Script::ScriptReference *TFBType_Value();       // 0x42f900

// Pad_SelectControlBuffer layouts: the levels with their own controls, and the intro videos'.
enum ControlLayout {
	CONTROLS_DEFAULT = 0,
	CONTROLS_MAP = 2,
	CONTROLS_RAVE = 3,
	CONTROLS_GOLF = 4,
	CONTROLS_SHUFFLE = 5,
	CONTROLS_VIDEO = 7
};

static const unsigned kDemoPromptDelayMs = 4000; // the demo's start screen waits this long before it shows the prompt
static const unsigned kDemoBlinkMs = 500;         // then the prompt blinks with this period (shown in the even halves)
static const int kInputBlockMs = 250;             // inputs held over a video stay blocked until released, or this long
static const int kLocalSymbolBase = 0x3fc0d;      // script reference index of the running script's first local object
static const int kActorSerialsPerLevel = 800;     // each level numbers its actors from level * 800
static const float kPerScreenUnitX = 0.0015625f;  // 1 / 640: the loading screen's 640 x 480 units to pixels
static const float kPerScreenUnitY = 0.0020833334f; // 1 / 480

static void PrepareControls(int level)
{
	g_shadowSkip = level == LEVEL_ESCAPE;
	switch (level) {
	case LEVEL_MAP:
		Pad_SelectControlBuffer(CONTROLS_MAP);
		break;
	case LEVEL_RAVE:
		Pad_SelectControlBuffer(CONTROLS_RAVE);
		break;
	case LEVEL_SHUFFLE:
		Pad_SelectControlBuffer(CONTROLS_SHUFFLE);
		break;
	case LEVEL_GOLF:
		Pad_SelectControlBuffer(CONTROLS_GOLF);
		break;
	default:
		Pad_SelectControlBuffer(CONTROLS_DEFAULT);
		break;
	}
}

// Boot of a normal start: whether there is a title level, the global stream (its symbols are the global script symbols), then the four intro videos; a key pressed during a skipped video skips the rest.
static void BootNormal()
{
	char path[0x100];
	sprintf(path, "LEVELS\\%s", g_LevelTable[LEVEL_TITLE].m_stream);
	if (!RWS::StartUp::LoadGameDatabaseFile(path, true))
		g_playMode = PLAYMODE_NO_TITLE;
	RWS::StartUp::LoadGameDatabaseFile("LEVELS\\GLOBAL.STREAM", false);
	g_globalSymbolCount = SymbolRegistry_GetSize();
	RWS::Audio::AudioGlobalMixer::StopAllStreams(true);
	Pad_SelectControlBuffer(CONTROLS_VIDEO);
	DSound_Init(g_mainWnd);
	BinkSetSoundSystem((void *)BinkOpenDirectSound, g_dsoundObj);
	// startup logos only with ZOOSTERS_BINK_LOGOS
#ifdef ZOOSTERS_BINK_LOGOS
	for (int i = 0; i < 4; i++) {
		if (!PlayVideo(g_introVideos[i], true) && GetPlayerInput(0, 0.0f, 0) != 0.0f)
			break;
	}
#endif
	DSound_Release(); // the game also calls an empty function (0x5074b0) around this
	Pad_SelectControlBuffer(CONTROLS_DEFAULT);
	BlockInputsUntilReleased(kInputBlockMs);
}

// The boot sequence of the kiosk demo: the global stream, then the demo picture until start is pressed (after 4 s a blinking
// "PRESS START BUTTON"). The 2D device draws to the main camera meanwhile.
static void BootDemo(Script::ScriptSystem::ExposeVSyncFlipCount &flipCount)
{
	RwCamera *camera = RWS::MainLoop::Render::GetCamera();
	RWS::StartUp::LoadGameDatabaseFile("LEVELS\\GLOBAL.STREAM", false);
	g_globalSymbolCount = SymbolRegistry_GetSize();
	Hud2DObject prompt;
	prompt.m_font = Hud2D::GetFont(0, "tsa16");
	prompt.m_boxWidth = 400.0f;
	prompt.m_boxHeight = 400.0f;
	prompt.m_anchor = HUD2D_ANCHOR_CENTRE;
	prompt.m_drawY = 50.0f;
	RwCamera *camera2d = Rt2dDeviceGetCamera();
	Rt2dDeviceSetCamera(camera);
	Rt2dDeviceSetMetric(0.0f, 0.0f, 640.0f, 480.0f);
	LoadScreen_LoadImage("demo.png");
	ControllerElapsedMs(1);
	flipCount.SetValue(1, &flipCount);
	unsigned start = GetTimeMs();
	for (;;) {
		CtlRefresh();
		LoadScreen_Draw(camera);
		unsigned elapsed = GetTimeMs() - start;
		if (elapsed > kDemoPromptDelayMs) {
			if ((elapsed / kDemoBlinkMs & 1) == 0 && prompt.m_font) {
				prompt.SetFormat("PRESS START BUTTON");
				prompt.MeasureText(&prompt.m_width, &prompt.m_height);
				prompt.GetAnchorOffset(&prompt.m_offsetX, HUD2D_ANCHOR_CENTRE);
				Hud2D::BeginFrame(camera);
				prompt.DrawText();
				Hud2D::EndFrame(camera);
			}
			if (GetPlayerInput(0, 0.25f, 0) != 0.0f)
				break;
		}
		RwCameraShowRaster(camera, 0, rwRASTERFLIPWAITVSYNC);
	}
	flipCount.SetValue(2, &flipCount);
	Rt2dDeviceSetCamera(camera2d);
	if (camera2d)
		Rt2dDeviceSetMetric(0.0f, 0.0f, 640.0f, 480.0f);
	LoadScreen_DestroyRaster();
}

// When another level was requested: prepares the switch (controls, vsync flip count of the loading screens, the boot sequence on the first call, the video between battle and the map) and returns 1 so the main loop loads it. No switch in the demo after the first level. Otherwise 0; on the title screen the controller clock pauses while the game clock is stopped.
// 0x4610b0
int LevelManager()
{
	if (g_nextLevel == g_currentLevel) {
		if (g_currentLevel == LEVEL_TITLE) {
			if (TFBClock_IsStopped()) {
				ControllerElapsedMs(1);
				return 0;
			}
			ControllerElapsedMs(0);
		}
		return 0;
	}
	Script::ScriptSystem::ExposeVSyncFlipCount flipCount;
	flipCount.SetValue(2, &flipCount);
	PrepareControls(g_nextLevel);
	if (g_currentLevel != LEVEL_NONE) {
		if (g_playMode >= PLAYMODE_DEMO)
			return 0;
		if (g_currentLevel == LEVEL_BATTLE && g_nextLevel == LEVEL_AFTER_BATTLE) {
			PlayIntroVideo("bgm\\01.krn");
			g_nextLevel = LEVEL_MAP;
		}
	} else if (g_playMode == PLAYMODE_NORMAL) {
		BootNormal();
	} else if (g_playMode == PLAYMODE_DEMO) {
		BootDemo(flipCount);
	}
	srand(GetTimeMs() + rand());
	return 1;
}

// The level a request leads to: on the first call the play mode's first level, from the title the new game's first level.
static int ResolveRequestedLevel()
{
	if (g_currentLevel == LEVEL_NONE) {
		if (g_playMode == PLAYMODE_NORMAL)
			return LEVEL_TITLE;
		return g_playMode != PLAYMODE_NO_TITLE ? LEVEL_BEACH : LEVEL_KINGOFNY;
	}
	if (g_currentLevel == LEVEL_TITLE && g_nextLevel == LEVEL_NEW_GAME)
		return LEVEL_KINGOFNY;
	return g_nextLevel;
}

// Back on the title, every global user value of the scripts starts over: scalars are set to 0, arrays cleared.
static void ResetGlobalUserValues()
{
	int count = g_globalSymbolCount;
	for (int i = g_typeSymCount; i < count; i++) {
		Script::ScriptReference *object;
		if (i >= kLocalSymbolBase) {
			object = g_curScript->m_locals[i - kLocalSymbolBase];
		} else {
			uintptr_t slot = (*g_symRegistryY)[i].obj;
			if (slot & 1) // not resolved yet
				continue;
			object = (Script::ScriptReference *)slot;
		}
		if (!object || object->Deref(0, 0) != object)
			continue;
		if (object->GetType() != TFBType_User() || !object->IsKindOf(TFBType_Value()))
			continue;
		Script::UserReference *value = static_cast<Script::UserReference *>(object);
		if (value->m_count == 0)
			value->SetValue(0, value);
		else
			memset(value->m_data, 0, value->m_dataSize);
	}
}

#ifdef ZOOSTERS_WIDESCREEN
extern "C" void port_wide2d_hud(float W, float H, float *B, float *D);   // port/fixes/widescreen.cpp
#endif
// Loads the level LevelManager let through: its loading screen (a request past the table loads the map behind the zoo shop picture), loading progress, actors numbered from level * 800, global user values reset on the title, then streams LEVELS\<stream>. A new game plays the opening video and sets the game clock (0.15 when the video was skipped). Returns whether the stream loaded.
// 0x4614d0
bool LevelLoad()
{
	int level = ResolveRequestedLevel();
	g_currentLevel = level;
	const char *loadScreen;
	if (level >= LEVEL_COUNT) {
		g_nextLevel = LEVEL_MAP;
		g_currentLevel = LEVEL_MAP;
		g_loadOfsX = 204.0f;
		g_loadOfsY = 178.0f;
		loadScreen = "zooshop.png";
	} else {
		g_loadOfsX = g_LevelTable[level].m_progressX;
		g_loadOfsY = g_LevelTable[level].m_progressY;
		loadScreen = g_LevelTable[level].m_loadScreen;
	}
	RwCamera *camera = RWS::MainLoop::Render::GetCamera();
#ifdef ZOOSTERS_WIDESCREEN   // widescreen port build only: the progress leaves on the loading picture's centred 4:3 grid
	{
		float B, D;
		port_wide2d_hud((float)RwRasterGetWidth(RwCameraGetRaster(camera)), (float)RwRasterGetHeight(RwCameraGetRaster(camera)), &B, &D);
		g_loadOfsX = B + D * g_loadOfsX;
	}
#else
	g_loadOfsX = (float)RwRasterGetWidth(RwCameraGetRaster(camera)) * g_loadOfsX * kPerScreenUnitX;
#endif
	g_loadOfsY = (float)RwRasterGetHeight(RwCameraGetRaster(camera)) * g_loadOfsY * kPerScreenUnitY;
	LoadScreen_LoadImage(loadScreen);
	for (int buffer = 0; buffer < 2; buffer++) { // both buffers of the flip chain
		LoadScreen_Draw(RWS::MainLoop::Render::GetCamera());
		RwCameraShowRaster(RWS::MainLoop::Render::GetCamera(), 0, rwRASTERFLIPWAITVSYNC);
	}
	LoadScreen_Begin((unsigned)(g_LevelTable[g_currentLevel].m_loadTime * 250.0f));
	*(int *)GetActorSerialCounter() = g_currentLevel * kActorSerialsPerLevel;
	if (g_currentLevel == LEVEL_TITLE)
		ResetGlobalUserValues();

	char path[0x100];
	sprintf(path, "LEVELS\\%s", g_LevelTable[g_currentLevel].m_stream);
	bool loaded = RWS::StartUp::LoadGameDatabaseFile(path, false);
	g_loadStartTime = 0;
	TFBLoadingProgress(); // with the start time cleared it draws at once (twice: both buffers)
	TFBLoadingProgress();
	g_loadDuration = 0;
	LoadScreen_DestroyRaster();
	if (loaded && g_currentLevel == LEVEL_KINGOFNY && g_nextLevel == LEVEL_NEW_GAME && g_playMode == PLAYMODE_NORMAL) {
		int played = PlayIntroVideo("bgm\\06.krn");
		BlockInputsUntilReleased(kInputBlockMs);
		Script::ScriptSystem::ExposeGameTime gameTime;
		gameTime.SetValue(played != 1 ? 0.15f : 0.0f, &gameTime);
	}
	g_nextLevel = g_currentLevel;
	ControllerElapsedMs(1);
	SetCapture(0);
	return loaded;
}
