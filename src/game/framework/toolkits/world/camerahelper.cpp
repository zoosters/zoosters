#include "framework/toolkits/world/camerahelper.h"
#include "framework/toolkits/world/framehelper.h"

namespace RWS {
namespace CameraHelper {

// 0x44e850
RwBool Destroy(RwCamera *camera)
{
	RwFrame *frame = RwCameraGetFrame(camera);
	if (frame) {
		RwFrameForAllChildren(frame, FrameHelper::RemoveChildFrame, 0);
		if (RwFrameGetParent(frame))
			RwFrameRemoveChild(frame);
		RwCameraSetFrame(camera, 0);
		FrameHelper::FrameDestroy(frame);
	}

	RpWorld *world = RwCameraGetWorld(camera);
	if (world)
		RpWorldRemoveCamera(world, camera);

	RwRaster *raster = RwCameraGetRaster(camera);
	if (raster) {
		RwRasterDestroy(raster);
		RwCameraSetRaster(camera, 0);
	}

	RwRaster *zRaster = RwCameraGetZRaster(camera);
	if (zRaster) {
		RwRasterDestroy(zRaster);
		RwCameraSetZRaster(camera, 0);
	}

	return RwCameraDestroy(camera);
}

} // namespace CameraHelper
} // namespace RWS
