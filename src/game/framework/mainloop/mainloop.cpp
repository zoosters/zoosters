// GLOBAL: 0x62a49c RWS::ERROR_NOT_A_VALID_MESSAGE_ID_ML
#define MSG_INVALID ERROR_NOT_A_VALID_MESSAGE_ID_ML
#include <stdio.h>
#include <stdlib.h>
#include "framework/core/streamhandler.h"
#include "framework/mainloop/gameclock.h"
#include "framework/mainloop/mainloop.h"
#include "framework/toolkits/world/camerahelper.h"
#include "framework/startup/startup.h"
#include "framework/startup/win32/appstate.h"
#include "framework/startup/win32/textdict.h"
#include "framework/startup/win32/systemhook.h"
#include "framework/mainloop/mainloopmsg.h"
#include "modules/tfb/camera_views.h"
#include "modules/tfb/sprites/spritemanager_api.h"
#include "rw/rttoolkits.h"
#include "script/types/scriptsystem.h"
#include "tfb/collision/broadphase.h"
#include "tfb/level/levelmanager.h"
#include "tfb/render/postfx.h"
#include "zoosters/os.h"

void Level_ReleaseGameObjects();
namespace RWS { namespace RwsAudio { void Close(); } }
namespace RWS { namespace RwsAudio { void UnRegisterAudioObjects(); } }
namespace RWS { namespace RwsAudio { void Open(); } }
void Corpus_UnregisterMessages();
void Corpus_RegisterMessages();
unsigned GetTimeMs();
RwCamera *CreateRasterCamera(int w, int h, int z);
// Folded empty function (CEventId::~CEventId), called without a this.
// GLOBAL: 0x464ef0 Fold_Nop
void Fold_Nop();

namespace Hud2D {
void Close();
void Open();
}

namespace RWS {
namespace ClumpHelper {
void Open();
void Close();
}
}

static inline unsigned long GetTickCount() { return zoosters::os::TickMs(); }

// 0x461ae0
bool FrameRate::Update()
{
	bool newSecond = false;
	unsigned now = GetTickCount();
	unsigned dt = now - m_last;
	m_last = now;
	m_accum += dt;
	while (m_accum >= 1000) {
		m_accum -= 1000;
		if (m_accum < 1000) {
			m_prev = m_count;
			m_count = 0;
			newSecond = true;
		}
	}
	m_count++;
	return newSecond;
}

namespace RWS {
namespace MainLoop {
namespace anon {

bool systemStarted = false;  // GLOBAL: 0x62a49b RWS::MainLoop::anon::systemStarted (file-local in the original)

}  // namespace anon
}  // namespace MainLoop
}  // namespace RWS

// 0x461b80
bool MainLoop_IsStarted()
{
	return RWS::MainLoop::anon::systemStarted;
}

namespace RWS {
namespace MainLoop {
namespace anon {

// 0x461b90
void NewEntityCreatedCB()
{
	if (systemStarted) {
		SendMsg(iMsgStartSystem);
	}
}

// 0x461bd0
void StartSystem(RwChunkHeaderInfo &, RwStream *)
{
	if (!systemStarted) {
		systemStarted = true;
		SendMsg(iMsgStartSystem);
		Render::SetEnabled(1);
		Logic::SetEnabled(1);
	}
	TFBHook.Notify();
}

// 0x461e80
void StopSystem(RwChunkHeaderInfo &, RwStream *)
{
	if (systemStarted) {
		systemStarted = false;
		SendMsg(iMsgStopSystem);
		Render::SetEnabled(0);
		Logic::SetEnabled(0);
		if (TFBClockStart == 0)
			TFBClockStart = *Script::ScriptSystem::GetScriptTimePtr();
		TFBClockSum += TFBClockStart - TFBClockBase;
		TFBClockStart = 0;
		TFBClockBase = 0;
	}
}

// 0x461f10
void Reset(RwChunkHeaderInfo &h, RwStream *s)
{
	StopSystem(h, s);
	BroadPhase_Reset();
	CEventHandler::DeleteEventHandlers(iMsgDeleteEventHandler);
	CAttributeHandler::Purge();
	CResourceManager::Purge();
	Level_ReleaseGameObjects();
}

// Level reload (chunk 0x71d): like Reset, but purges everything.
// 0x461f40
void Init(RwChunkHeaderInfo &chunkHeader, RwStream *stream)
{
	StopSystem(chunkHeader, stream);
	BroadPhase_Reset();
	CAttributeHandler::PurgeAll();
	CEventHandler::DeleteEventHandlers(iMsgDeleteEventHandler);
	CResourceManager::PurgeAll();
	Level_ReleaseGameObjects();
}

}  // namespace anon

// 0x461f70
void Open(int width, int height, int zdepth)
{
	srand(GetTimeMs() + rand());
	Render::SetCamera(CreateRasterCamera(width, height, zdepth));
	Rt2dOpen(Render::GetCamera());
	LoadScreen_Show(Render::GetCamera(), 0);
	RWS::RwsAudio::Open();
	CEventHandler::Open(0, 0);
	CEventHandler::RegisterStreamChunkHandlers();
	CLinearMemoryManager::RegisterStreamChunkHandlers();
	CAttributeHandler::SetNewAttributeHandlerCallBack(anon::NewEntityCreatedCB);
	CAttributeHandler::RegisterStreamChunkHandlers();
	CStreamHandler::RegisterStreamChunkHandlers();
	CResourceManager::Open();
	CResourceManager::RegisterStreamChunkHandlers();
	ClumpHelper::Open();
	Logic::Open();
	Logic::RegisterStreamChunkHandlers();
	Logic::Running();
	Render::Open();
	Fold_Nop();
	CEventHandler::RegisterMsg(iMsgStartSystem, "iMsgStartSystem", 0);
	CEventHandler::RegisterMsg(iMsgStopSystem, "iMsgStopSystem", 0);
	Corpus_RegisterMessages();
	RwRGBA black;
	black.red = 0;
	black.green = 0;
	black.blue = 0;
	black.alpha = 0;
	RwCameraClear(Render::GetCamera(), &black, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
	Render::Poll();
	RwCameraClear(Render::GetCamera(), &black, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
	Render::Poll();
	CStreamHandler::RegisterChunkHandler(0x70b, anon::StartSystem);
	CStreamHandler::RegisterChunkHandler(0x70c, anon::StopSystem);
	CStreamHandler::RegisterChunkHandler(0x700, anon::Reset);
	CStreamHandler::RegisterChunkHandler(0x71d, anon::Init);
	Hud2D::Open();
	UpdatePixelFogCutoff(Render::GetCamera());
}

// 0x462590
void Close()
{
	BroadPhase_Reset();
	Hud2D::Close();
	CEventHandler::DeleteEventHandlers(iMsgDeleteEventHandler);
	CAttributeHandler::PurgeAll();
	CResourceManager::PurgeAll();
	Corpus_UnregisterMessages();
	CStreamHandler::UnRegisterStreamChunkHandlers();
	Logic::UnRegisterStreamChunkHandlers();
	Logic::Close();
	Fold_Nop();
	Render::Close();
	ClumpHelper::Close();
	CAttributeHandler::UnRegisterStreamChunkHandlers();
	CAttributeHandler::Purge();
	CAttributeHandler::SetNewAttributeHandlerCallBack(0);
	CResourceManager::UnRegisterStreamChunkHandlers();
	CResourceManager::Close();
	CEventHandler::UnRegisterMsg(iMsgStartSystem);
	CEventHandler::UnRegisterMsg(iMsgStopSystem);
	CEventHandler::UnRegisterStreamChunkHandlers();
	CLinearMemoryManager::DestroyLinearMemoryManager();
	CEventHandler::Close();
	RWS::RwsAudio::Close();
	RWS::RwsAudio::UnRegisterAudioObjects();
	Rt2dClose();
	CameraHelper::Destroy(Render::GetCamera());
	Render::SetCamera(0);
	CStreamHandler::UnRegisterChunkHandler(0x70b);
	CStreamHandler::UnRegisterChunkHandler(0x70c);
	CStreamHandler::UnRegisterChunkHandler(0x700);
	CStreamHandler::UnRegisterChunkHandler(0x71d);
}

}  // namespace MainLoop
}  // namespace RWS

static inline void Sleep(unsigned long ms) { zoosters::os::SleepMs((unsigned)ms); }

extern int g_vsyncFlipCount;          // GLOBAL: 0x6221b8 g_vsyncFlipCount
extern int g_currentLevel;            // GLOBAL: 0x606a38 g_currentLevel
extern unsigned char g_shotRequest;   // GLOBAL: 0x62a490 g_shotRequest
extern int g_shotNo;                  // GLOBAL: 0x62a494 g_shotNo
extern char g_shotDir[];              // GLOBAL: 0x62a390 g_shotDir
extern unsigned char g_fpsShown;      // GLOBAL: 0x62a498 g_fpsShown
extern unsigned char g_fpsCreated;    // GLOBAL: 0x62a49a g_fpsCreated
extern int g_errTimer;                // GLOBAL: 0x606c44 g_errTimer

int SaveDiskFreeSpace(bool clamp);
bool Sprite_IsVisible(RWS::TFB::GameSprite *sprite);

// The build text and the level-change chunk header share one slot, as in the game.
union TextOrHdr { char build[32]; RwChunkHeaderInfo hdr; };

#ifdef ZOOSTERS_DEBUGUI
bool port_debugui_logic_allowed();
void port_debugui_loading(int on);   // the script registry is half built between the release and the end of LevelLoad: the variable panel waits
#endif
// 0x4620d0
void RWS::MainLoop::Poll()
{
	int steps = g_vsyncFlipCount;
	float dt = TFBHook.Tick();
	TextOrHdr textOrHdr;
	if (!(dt > 0.0f) && (dt != 0.0f || MainLoop_RenderIsEnabled())) {
		if (dt < 0.0f) {

			char fpsText[32];
			if (g_fpsCounter.Update()) {
				unsigned fps = g_fpsCounter.m_prev;
				if (!fps)
					fps = g_fpsCounter.m_count;
				sprintf(fpsText, "FPS:%u", fps);
				sprintf(textOrHdr.build, "%s%04u", "Build FC", 25);
				if (!g_fpsCreated) {
					SpriteMgr_CreateTextSprite("bx_fps", "FPS:???", 8.0f, 8.0f, 128.0f, 32.0f);
					SpriteMgr_CreateTextSprite("bx_build", textOrHdr.build, 8.0f, 28.0f, 128.0f, 32.0f);
					g_fpsCreated = 1;
				}
				if (g_fpsShown) {
					SpriteMgr_SetTextSprite("bx_fps", fpsText, 8.0f, 8.0f, 128.0f, 32.0f);
					SpriteMgr_SetTextSprite("bx_build", textOrHdr.build, 8.0f, 28.0f, 128.0f, 32.0f);
				} else {
					SpriteMgr_SetTextSprite("bx_fps", "", 8.0f, 8.0f, 128.0f, 32.0f);
					SpriteMgr_SetTextSprite("bx_build", "", 8.0f, 28.0f, 128.0f, 32.0f);
				}
			}
			RWS::MainLoop::Render::Poll();
			if (g_shotRequest && (unsigned)(SaveDiskFreeSpace(true) / 1024) < 8000) {
				SpriteMgr_CreateTextSprite("bx_screenerror", TextDict_Find(g_dict, "ID_SCREENSHOTERROR"), 8.0f, 48.0f, 128.0f, 32.0f);
				g_errTimer = 100;
				g_shotRequest = 0;
			} else if (g_shotRequest) {
				{
					char name[0x118];
					RWS::MainLoop::Render::GetCamera();
					RWS::MainLoop::Render::GetCamera();
					sprintf(name, "%sScreen%03d.bmp", g_shotDir, g_shotNo);
					RwRaster *raster = CameraView(RWS::MainLoop::Render::GetCamera())->frameBuffer;
					if (raster) {
						RwImage *img = RwImageCreate(RasterView(raster)->width, RasterView(raster)->height, 32);
						if (img) {
							RwImageAllocatePixels(img);
							RwImageSetFromRaster(img, raster);
							if (!RwImageWrite(img, name)) {
								SpriteMgr_CreateTextSprite("bx_screenerror", TextDict_Find(g_dict, "ID_SCREENSHOTERROR"), 8.0f, 48.0f, 128.0f, 32.0f);
								g_errTimer = 100;
							} else {
								RwImageDestroy(img);
								g_shotNo++;
							}
						}
					}
					g_shotRequest = 0;
				}
			}
			if (g_errTimer != -1) {
				if (g_errTimer == 0) {
					RemoveSpriteByName("bx_screenerror");
					g_errTimer = -1;
				} else {
					g_errTimer--;
				}
			}
			if (g_currentLevel == 5) {
				RWS::TFB::GameSprite *oopsSprite = FindSpriteByName("Instruct_Oops_Equal");
				if (oopsSprite && Sprite_IsVisible(oopsSprite)) {
					SpriteMgr_CreateTextSprite("bx_RAVE_KEY_1", TextDict_Find(g_dict, "ID_RAVE_KEY_1"), 160.0f, 338.0f, 128.0f, 32.0f);
					SpriteMgr_CreateTextSprite("bx_RAVE_KEY_2", TextDict_Find(g_dict, "ID_RAVE_KEY_2"), 160.0f, 368.0f, 128.0f, 32.0f);
				} else {
					RemoveSpriteByName("bx_RAVE_KEY_1");
					RemoveSpriteByName("bx_RAVE_KEY_2");
				}
			}
		} else {
			Sleep(0);
		}
		return;
	}
	{
#ifdef ZOOSTERS_DEBUGUI   // pause / frame advance / slow motion hold the logic step
		if (!port_debugui_logic_allowed())
			return;
#endif
		do {
			if (LevelManager()) {
				Cursor_HideSave();
				RWS::MainLoop::anon::StopSystem(textOrHdr.hdr, 0);
				BroadPhase_Reset();
				RWS::CEventHandler::DeleteEventHandlers(RWS::iMsgDeleteEventHandler);
				RWS::CAttributeHandler::Purge();
				RWS::CResourceManager::Purge();
#ifdef ZOOSTERS_DEBUGUI
				port_debugui_loading(1);
#endif
				Level_ReleaseGameObjects();
				bool loaded = LevelLoad();
#ifdef ZOOSTERS_DEBUGUI
				port_debugui_loading(0);
#endif
				if (loaded) {
					g_fpsCreated = 0;
					RWS::MainLoop::anon::StartSystem(textOrHdr.hdr, 0);
					Sleep(200);
					Cursor_Restore();
					return;
				}
			}
			RWS::MainLoop::Logic::Poll();
		} while (--steps > 0);
	}
}

// GLOBAL: 0x464f00 TFBSystemHook::Notify
// GLOBAL: 0x62a4a8 g_fpsCounter
