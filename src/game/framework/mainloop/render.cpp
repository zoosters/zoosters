// GLOBAL: 0x62a52c RWS::ERROR_NOT_A_VALID_MESSAGE_ID_RD
#define MSG_INVALID ERROR_NOT_A_VALID_MESSAGE_ID_RD
#include "framework/mainloop/mainloop.h"
#include "framework/mainloop/mainloopmsg.h"

namespace RWS {
namespace MainLoop {
namespace Render {

extern RwCamera *g_pCamera;   // GLOBAL: 0x62a524 RWS::MainLoop::Render::g_pCamera
int gbEnabled = 1;            // GLOBAL: 0x606c68 RWS::MainLoop::Render::gbEnabled (file-local in the original; the address-bound builds bind the image's)
extern int sm_frame_count;    // GLOBAL: 0x62a528 RWS::MainLoop::Render::sm_frame_count

// 0x462660
RwCamera *GetCamera()
{
	return g_pCamera;
}

// 0x462670
void SetCamera(RwCamera *camera)
{
	g_pCamera = camera;
}

// 0x462680
void Open()
{
	CEventHandler::RegisterMsg(iMsgDoRender, "iMsgDoRender", "RwCamera*");
	CEventHandler::RegisterMsg(iMsgDoRenderDirectorsCamera, "iMsgDoRenderDirectorsCamera", "RwCamera*");
	CEventHandler::RegisterMsg(iMsgPreShowRaster, "iMsgPreShowRaster", "RwCamera*");
}

// 0x4626c0
void Close()
{
	CEventHandler::UnRegisterMsg(iMsgDoRender);
	CEventHandler::UnRegisterMsg(iMsgDoRenderDirectorsCamera);
	CEventHandler::UnRegisterMsg(iMsgPreShowRaster);
}

// 0x4626f0
void SetEnabled(int enabled)
{
	gbEnabled = enabled;
}

// 0x462710
unsigned GetFrameCount()
{
	return sm_frame_count;
}

// 0x462720
void Poll()
{
	if (!gbEnabled)
		return;
	sm_frame_count++;
	CMsg msg1;
	msg1.Id = iMsgDoRender.pEventId;
	msg1.pData = g_pCamera;
	SendMsg(msg1);
	CMsg msg2;
	msg2.Id = iMsgPreShowRaster.pEventId;
	msg2.pData = g_pCamera;
	SendMsg(msg2);
	if (g_pCamera)
		RwCameraShowRaster(g_pCamera, 0, 1);
}

}  // namespace Render
}  // namespace MainLoop
}  // namespace RWS

// 0x462700
int MainLoop_RenderIsEnabled()
{
	return RWS::MainLoop::Render::gbEnabled;
}

