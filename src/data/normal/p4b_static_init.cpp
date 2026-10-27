// The game's static initialisers as one ordered list of named calls, maintained by hand. Normal build only; the address-bound builds
// run the .CRT$XCU table decoded from the image (port/runtime.cpp).
// The order is the game's .CRT$XCU order (the address above each row is its initialiser) and must be kept. A new row goes at its
// address. A class registration is `{ size_t sz = X::RegisteredSize; new RWS::CClassFactory::CRegistrar("X", &X::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("X", sz, <game literal>); n++; }`.
#include <new>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "framework/startup/win32/fileio_maps.h"   // FileMap, ReqMap
#include "zoosters/os.h"
#include "framework/core/cclassfactory.h"
#include "script/typeinit.h"
#include "modules/audio/audioglobalmixer.h"
#include "modules/fx/cfxcolorlight.h"
#include "modules/fx/cfxmotionblur.h"
#include "modules/fx/partspray.h"
#include "modules/tfb/cprotoactor.h"
#include "modules/tfb/ctfbmodel.h"
#include "modules/tfb/ctfbsound.h"
#include "modules/tfb/ctfbworld.h"
#include "framework/startup/win32/input/xboxcontroller.h"
#include "modules/tfb/cameradata.h"
#include "modules/tfb/gamecamera.h"
#include "modules/tfb/gamecontroller.h"
#include "modules/tfb/levelhub.h"
#include "modules/tfb/memcard.h"
#include "modules/tfb/radarrender.h"
#include "modules/tfb/shadowrender.h"
#include "modules/tfb/sprites/spritemanager.h"
#include "modules/tfb/sprites/spriteobject.h"
#include "modules/tfb/tfbshadowcamera.h"
#include "modules/tfb/worldobject.h"
#include "framework/startup/win32/systemhook.h"
#include "framework/mainloop/gameclock.h"
#include "tfb/level/resourcehandlers.h"
#include "framework/toolkits/rwaudresource/rwaudresource.h"
// A class registers its own sizeof (Class::RegisteredSize); the literal is the size the game registered (x86). A difference on x86 is a
// layout bug, logged and counted.
extern "C" FILE *port_log;                            // port/runtime.cpp
static int g_sizeMismatch;
static void CheckRegisteredSize(const char *cls, size_t got, size_t literal)
{
#ifndef ZOOSTERS_PTR64
	if (got != literal) { g_sizeMismatch++; fprintf(port_log ? port_log : stderr, "p4b_static_init: REGISTERED SIZE MISMATCH %s: sizeof 0x%x, game literal 0x%x\n", cls, (unsigned)got, (unsigned)literal); }
#else
	(void)cls; (void)got; (void)literal;
#endif
}
#include "framework/startup/win32/fileio.h"            // g_ioFn, g_pfnCreateFile / FileRead / FileWrite
#include "script/opcode.h"
extern Script::OpCode g_opcodeProto;                   // GLOBAL: 0x61f6e8 (src/data/normal/p4b_port_names.cpp)
extern "C" void port_script_assets_init(void);         // port/provisional/thunks.cpp

// the fileio.cpp maps, constructed by the CRT here (the game constructed them in the table)
FileMap g_fileMap;                 // GLOBAL: 0x62f184
ReqMap g_reqMap;                   // GLOBAL: 0x62f1b0
ReqMap::iterator g_reqCursor;      // GLOBAL: 0x62f1ac

// 219 table rows; skipped: 91 CEventId (zero state), 26 registrars, 11 atexit/CRT-helper rows.
extern "C" int port_game_static_init(void)
{
	int n = 0;
	// 0x5c54e0 Script::TypeRegistry_Init @0x617998
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5500 GameController::RegisterFactory
	{ size_t sz = RWS::TFB::GameController::RegisteredSize(); new RWS::CClassFactory::CRegistrar("GameController", &RWS::TFB::GameController::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("GameController", sz, 0xcc); n++; }
	// 0x5c5550 Script::TypeRegistry_Init @0x617bdc
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5570 CXboxStndController::RegisterFactory
	{ size_t sz = CXboxStndController::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CXboxStndController", &CXboxStndController::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CXboxStndController", sz, 0x250); n++; }
	// 0x5c55c0 Script::TypeRegistry_Init @0x617c0c
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c55e0 AudioGlobalMixer::RegisterFactory
	{ size_t sz = RWS::Audio::AudioGlobalMixer::RegisteredSize(); new RWS::CClassFactory::CRegistrar("AudioGlobalMixer", &RWS::Audio::AudioGlobalMixer::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("AudioGlobalMixer", sz, 0xa0); n++; }
	// 0x5c5650 Script::TypeRegistry_Init @0x617c48
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5670 CFXColorLight::RegisterFactory
	{ size_t sz = RWS::FX::CFXColorLight::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CFXColorLight", &RWS::FX::CFXColorLight::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CFXColorLight", sz, 0x38); n++; }
	// 0x5c5750 Script::TypeRegistry_Init @0x617c7c
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5790 Script::TypeRegistry_Init @0x617c88
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c57b0 CFXPartSpray::RegisterFactory
	{ size_t sz = RWS::FX::CFXPartSpray::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CFXPartSpray", &RWS::FX::CFXPartSpray::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CFXPartSpray", sz, 0x14c); n++; }
	// 0x5c5870 Script::TypeRegistry_Init @0x617cbc
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5890 CFXMotionBlur::RegisterFactory
	{ size_t sz = RWS::FX::CFXMotionBlur::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CFXMotionBlur", &RWS::FX::CFXMotionBlur::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CFXMotionBlur", sz, 0x58); n++; }
	// 0x5c5900 Script::TypeRegistry_Init @0x617dac
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5990 Script::TypeRegistry_Init @0x617e90
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c59b0 SpriteManager::RegisterFactory
	{ size_t sz = RWS::TFB::SpriteManager::RegisteredSize(); new RWS::CClassFactory::CRegistrar("SpriteManager", &RWS::TFB::SpriteManager::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("SpriteManager", sz, 0x50); n++; }
	// 0x5c59e0 SpriteObject::RegisterFactory
	{ size_t sz = RWS::TFB::SpriteObject::RegisteredSize(); new RWS::CClassFactory::CRegistrar("SpriteObject", &RWS::TFB::SpriteObject::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("SpriteObject", sz, 0x2c); n++; }
	// 0x5c5a10 Script::TypeRegistry_Init @0x617fd4
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5a50 CTFBSound::RegisterFactory
	{ size_t sz = RWS::TFB::CTFBSound::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CTFBSound", &RWS::TFB::CTFBSound::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CTFBSound", sz, 0x58); n++; }
	// 0x5c5aa0 Script::TypeRegistry_Init @0x617ff8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5ac0 LevelHub::RegisterFactory
	{ size_t sz = RWS::TFB::LevelHub::RegisteredSize(); new RWS::CClassFactory::CRegistrar("LevelHub", &RWS::TFB::LevelHub::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("LevelHub", sz, 0x20); n++; }
	// 0x5c5af0 GlobalScriptNames::RegisterFactory
	{ size_t sz = RWS::TFB::GlobalScriptNames::RegisteredSize(); new RWS::CClassFactory::CRegistrar("GlobalScriptNames", &RWS::TFB::GlobalScriptNames::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("GlobalScriptNames", sz, 0x20); n++; }
	// 0x5c5b40 Script::TypeRegistry_Init @0x618130
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5b60 CTFBWorld::RegisterFactory
	{ size_t sz = RWS::TFB::CTFBWorld::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CTFBWorld", &RWS::TFB::CTFBWorld::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CTFBWorld", sz, 0x58); n++; }
	// 0x5c5bb0 Script::TypeRegistry_Init @0x618180
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5bd0 WorldObject::RegisterFactory
	{ size_t sz = WorldObject::RegisteredSize(); new RWS::CClassFactory::CRegistrar("WorldObject", &WorldObject::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("WorldObject", sz, 0x88); n++; }
	// 0x5c5c00 Script::TypeRegistry_Init @0x618198
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5c40 CameraData::RegisterFactory
	{ size_t sz = RWS::TFB::CameraData::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CameraData", &RWS::TFB::CameraData::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CameraData", sz, 0x2c); n++; }
	// 0x5c5c90 Script::TypeRegistry_Init @0x6182e0
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5cb0 GameCamera::RegisterFactory
	{ size_t sz = RWS::TFB::GameCamera::RegisteredSize(); new RWS::CClassFactory::CRegistrar("GameCamera", &RWS::TFB::GameCamera::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("GameCamera", sz, 0xac); n++; }
	// 0x5c5d00 TFBShadowCamera::RegisterFactory
	{ size_t sz = RWS::TFB::TFBShadowCamera::RegisteredSize(); new RWS::CClassFactory::CRegistrar("TFBShadowCamera", &RWS::TFB::TFBShadowCamera::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("TFBShadowCamera", sz, 0x6c); n++; }
	// 0x5c5d50 Script::TypeRegistry_Init @0x618324
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5df0 Script::TypeRegistry_Init @0x618384
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5e10 CProtoActor::RegisterFactory
	{ size_t sz = RWS::TFB::CProtoActor::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CProtoActor", &RWS::TFB::CProtoActor::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CProtoActor", sz, 0x19c); n++; }
	// 0x5c5e40 
	{ port_script_assets_init(); n++; }
	// 0x5c5e80 Script::TypeRegistry_Init @0x61f510
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5ea0 Script::TypeRegistry_Init @0x61f654
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5ec0 Script::TypeRegistry_Init @0x61f6e4
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5ee0 Script::TypeRegistry_Init @0x61f894
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5f10 Script::TypeRegistry_Init @0x61f9c0
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5f30 Script::TypeRegistry_Init @0x61ff64
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5f50 Script::TypeRegistry_Init @0x620160
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5f70 Script::TypeRegistry_Init @0x621cc8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5f90 Script::TypeRegistry_Init @0x621f0c
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5fb0 Script::TypeRegistry_Init @0x621f3c
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c5ff0 Script::TypeRegistry_Init @0x622008
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6010 Script::TypeRegistry_Init @0x622114
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6030 Script::TypeRegistry_Init @0x6221a8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6050 Script::TypeRegistry_Init @0x6221bc
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6070 Script::TypeRegistry_Init @0x622398
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6090 Script::TypeRegistry_Init @0x6223a8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c60b0 Script::TypeRegistry_Init @0x6223a9
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c60f0 Script::TypeRegistry_Init @0x6223f4
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6110 RadarRender::RegisterFactory
	{ size_t sz = RadarRender::RegisteredSize(); new RWS::CClassFactory::CRegistrar("RadarRender", &RadarRender::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("RadarRender", sz, 0xc0); n++; }
	// 0x5c6160 Script::TypeRegistry_Init @0x622424
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6180 ShadowRender::RegisterFactory
	{ size_t sz = ShadowRender::RegisteredSize(); new RWS::CClassFactory::CRegistrar("ShadowRender", &ShadowRender::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("ShadowRender", sz, 0x78); n++; }
	// 0x5c61b0 Script::TypeRegistry_Init @0x622464
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c61d0 Script::TypeRegistry_Init @0x622468
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6210 CTFBModel::RegisterFactory
	{ size_t sz = CTFBModel::RegisteredSize(); new RWS::CClassFactory::CRegistrar("CTFBModel", &CTFBModel::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("CTFBModel", sz, 0x2f0); n++; }
	// 0x5c6280 Script::TypeRegistry_Init @0x626544
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c62a0 MemCard::RegisterFactory
	{ size_t sz = MemCard::RegisteredSize(); new RWS::CClassFactory::CRegistrar("MemCard", &MemCard::MakeNew, (unsigned)sz, 1); CheckRegisteredSize("MemCard", sz, 0x100); n++; }
	// 0x5c6310 Script::TypeRegistry_Init @0x62944c
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6350 Script::TypeRegistry_Init @0x629458
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6390 Script::TypeRegistry_Init @0x629464
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c63b0 DMorphResourceHandler::DMorphResourceHandler
	{ new DMorphResourceHandler; n++; }
	// 0x5c63f0 Script::TypeRegistry_Init @0x6296ac
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6430 RenderwareResourceHandler::RenderwareResourceHandler
	{ new RenderwareResourceHandler; n++; }
	// 0x5c6490 WaveDictResourceHandler::WaveDictResourceHandler
	{ new RWS::CAudioResource; n++; }
	// 0x5c64b0 TextResourceHandler::TextResourceHandler
	{ new TextResourceHandler; n++; }
	// 0x5c64f0 Script::TypeRegistry_Init @0x6296e8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6530 Script::TypeRegistry_Init @0x62974c
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c65b0 Script::TypeRegistry_Init @0x629774
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6630 SplineResourceHandler::SplineResourceHandler
	{ new SplineResourceHandler; n++; }
	// 0x5c6650 Script::TypeRegistry_Init @0x6297ac
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6670 KFsetResourceHandlerG::KFsetResourceHandlerG
	{ new KFsetResourceHandlerG; n++; }
	// 0x5c6690 ScriptResourceHandlerG::ScriptResourceHandlerG
	{ new ScriptResourceHandlerG; n++; }
	// 0x5c66d0 TextStringDictResourceHandler::TextStringDictResourceHandler
	{ new TextStringDictResourceHandler; n++; }
	// 0x5c66f0 Script::TypeRegistry_Init @0x629e5a
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6710 Script::TypeRegistry_Init @0x629f20
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6750 Script::TypeRegistry_Init @0x62a0ac
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6770 Script::TypeRegistry_Init @0x62a0b8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6790 Script::TypeRegistry_Init @0x62a0b9
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c67b0 Script::TypeRegistry_Init @0x62a0bc
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c67f0 Script::TypeRegistry_Init @0x62a0c8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6810 Script::TypeRegistry_Init @0x62a0d4
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6830 Script::TypeRegistry_Init @0x62a0d5
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6850 Script::TypeRegistry_Init @0x62a0d6
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6870 Script::TypeRegistry_Init @0x62a0d7
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6890 Script::TypeRegistry_Init @0x62a0d8
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c68b0 Script::TypeRegistry_Init @0x62a0f1
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c68d0 Script::TypeRegistry_Init @0x62a104
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c68f0 Script::TypeRegistry_Init @0x62a110
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6910 Script::TypeRegistry_Init @0x62a111
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6930 Script::TypeRegistry_Init @0x62a114
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6970 Script::TypeRegistry_Init @0x62a120
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6990 Script::TypeRegistry_Init @0x62a121
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c69b0 Script::TypeRegistry_Init @0x62a136
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c69d0 Script::TypeRegistry_Init @0x62a154
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6a50 Script::TypeRegistry_Init @0x62a304
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6a90 Script::TypeRegistry_Init @0x62a318
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6bf0 Script::TypeRegistry_Init @0x62a4a4
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6c10 
	{ g_fpsCounter.m_last = (unsigned)zoosters::os::TickMs64(); n++; }   // the real tick
	// PORT_FPS_LAST=<ms>: fixed start tick instead of the uptime, so two runs get the same once-a-second FrameRate::Update phase.
	{ const char *e = getenv("PORT_FPS_LAST"); if (e && *e) g_fpsCounter.m_last = (unsigned)strtoul(e, 0, 10); }
	// 0x5c6c20 TFBSystemHook::TFBSystemHook
	{ new (&TFBHook) TFBSystemHook(60.0f, 60.0f); n++; }   // args 0x42700000 twice
	// 0x5c6ca0 Script::TypeRegistry_Init @0x62a534
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6da0 Script::TypeRegistry_Init @0x62a570
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6e40 Script::TypeRegistry_Init @0x62a5ec
	{ static TypeInitHost host; host.Init(); n++; }
	// 0x5c6e80 ??0?$hash_map@I_NV?$hash_compare@IU?$less@I@std@@@stdext@@V?$allocator@U?$pair@$$CBI_N@std@@@std@@@stdext@@QAE@XZ: g_reqMap is a C++ static of the normal build (constructed by the CRT)
	// 0x5c6ea0 ??0?$hash_map@I_NV?$hash_compare@IU?$less@I@std@@@stdext@@V?$allocator@U?$pair@$$CBI_N@std@@@std@@@stdext@@QAE@XZ: g_fileMap is a C++ static of the normal build (constructed by the CRT)
	// 0x5c6ec0  ...
	{ FileIo_Reset(); n++; }   // stores g_ioFn[], g_pfnGetOverlappedResult, g_pfnCancelIo, g_pfnCloseHandle (the IAT-slot copies)
	// savefile.cpp names g_ioFn[0] / [1] separately (p4b_port_names.cpp)
	{ g_pfnCreateFile = (FileCreateFn)g_ioFn[0];
	  g_pfnFileRead = (FileRwFn)g_ioFn[1];
	  // g_ioFn[3] in the game is 0x60d824; here it is savefile.cpp's g_pfnFileWrite, slot 0 of the block in io_tables.cpp, whose static value is the
	  // game's WriteFile_thunk (an empty function off cl 13.10 x86, else SaveFileWriteAsync returned garbage on x64)
	  g_pfnFileWrite = (FileRwFn)g_ioFn[3]; }
	// 0x5c6ed0 : g_pfnCancelIo copied by FileIo_Reset above
	// 0x5c6ee0 : g_pfnCloseHandle copied by FileIo_Reset above
	// 0x5c6f50 Script::TypeRegistry_Init @0x62fef0
	{ static TypeInitHost host; host.Init(); n++; }
#ifndef ZOOSTERS_PTR64
	fprintf(port_log ? port_log : stderr, "p4b_static_init: %d class registrations, %d size mismatches against the game literals\n", 26, g_sizeMismatch);
#endif
	g_opcodeProto.m_flags = 0;   // the game prototype is zero data without a constructor (OpCode sets 1)
	return n;
}
