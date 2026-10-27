#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_CAMERAHELPER_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_WORLD_CAMERAHELPER_H
#include "rw/rpworld.h"
#include "framework/core/cattributehandler.h"

namespace RWS {
namespace CameraHelper {
RwBool Destroy(RwCamera *pCamera);
RwCamera *CreateSubRasterCamera(RwRect &rect);   // game code
void SetFieldOfView(RwReal rViewWindow, RwCamera *pCamera);   // game code, cdecl
}
namespace MainLoop { namespace Render { RwCamera *GetCamera(); } }
}
#endif
