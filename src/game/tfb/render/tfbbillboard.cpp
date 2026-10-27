// TFBAtomic::UpdateBillboardFrame: faces a billboard atomic's frame towards the camera (TFB_BILLBOARD_*).
// The frame's matrix is parent * local translation * local rotation * (world rotation)^-1 * facing * world rotation; facing looks at the camera's position or along its viewing direction, around a fixed up axis or the camera's up.
#include "rw/rwcore.h"
#include "tfb/render/tfbatomic.h"

// Identity matrix with RW's "orthonormal + internal identity" type flags (0x20003) set.
#define BB_IDENT(m) \
	do { \
		(m)->right.x = (m)->up.y = (m)->at.z = 1.0f; \
		(m)->right.y = (m)->right.z = (m)->up.x = 0.0f; \
		(m)->up.z = (m)->at.x = (m)->at.y = 0.0f; \
		(m)->pos.x = (m)->pos.y = (m)->pos.z = 0.0f; \
		(m)->flags = (m)->flags | 0x20003; \
	} while (0)

// 0x45c400
void TFBAtomic::UpdateBillboardFrame(RpAtomic *atomic, RwCamera *camera)
{
	RwFrame *frame = RpAtomicGetFrame(atomic);
	if (!frame)
		return;
	RwMatrix *camLtm = RwFrameGetLTM(RwCameraGetFrame(camera));
	RwFrame *parentFrame = (RwFrame *)frame->parent;
	RwMatrix *parentLtm = 0;
	if (parentFrame)
		parentLtm = RwFrameGetLTM(parentFrame);
	unsigned mode;
	if (g_tfbFdOfs > 0)
		mode = TFBAtomicGetData(atomic)->m_billboardFlags;
	else
		mode = 0;
	RwMatrix result;
	BB_IDENT(&result);
	RwMatrix *src;
	if (g_tfbFdOfs > 0)
		src = TFBAtomicGetData(atomic)->m_matrix;
	else
		src = 0;
	RwMatrix local = *src;
	RwMatrix world = local;
	if (parentLtm)
		RwMatrixTransform(&world, parentLtm, rwCOMBINEPOSTCONCAT);
	RwMatrix worldInv;
	RwMatrixInvert(&worldInv, &world);
	RwMatrix worldRot = world;
	RwMatrix worldInvRot = worldInv;
	worldRot.pos.x = worldRot.pos.y = worldRot.pos.z = 0.0f;
	worldInvRot.pos.x = worldInvRot.pos.y = worldInvRot.pos.z = 0.0f;
	RwMatrixUpdate(&worldRot);
	RwMatrixUpdate(&worldInvRot);
	RwV3d worldPos = world.pos;
	RwMatrix localRot = local;
	localRot.pos.x = localRot.pos.y = localRot.pos.z = 0.0f;
	RwMatrix localTrans;
	BB_IDENT(&localTrans);
	RwMatrixTranslate(&localTrans, &local.pos, rwCOMBINEPOSTCONCAT);
	RwMatrix localRotInv;
	RwMatrixInvert(&localRotInv, &localRot);
	RwV3d upAxis;
	if (mode & TFB_BILLBOARD_FIXED_UP) {
		if (mode & TFB_BILLBOARD_UP_FROM_PARENT)
			upAxis = worldRot.up;
		else {
			upAxis.x = 0.0f;
			upAxis.y = 1.0f;
			upAxis.z = 0.0f;
		}
	}
	RwMatrix face;
	BB_IDENT(&face);
	RwMatrixUpdate(&face);
	if (mode & TFB_BILLBOARD_FACE_POSITION) {
		if (mode & TFB_BILLBOARD_FIXED_UP) {
			face.up = upAxis;
			face.at.x = camLtm->pos.x - worldPos.x;
			face.at.y = camLtm->pos.y - worldPos.y;
			face.at.z = camLtm->pos.z - worldPos.z;
			RwV3dNormalize(&face.at, &face.at);
			face.right.x = face.up.y * face.at.z - face.up.z * face.at.y;
			face.right.y = face.up.z * face.at.x - face.up.x * face.at.z;
			face.right.z = face.up.x * face.at.y - face.up.y * face.at.x;
			RwV3dNormalize(&face.right, &face.right);
			face.at.x = face.right.y * face.up.z - face.right.z * face.up.y;
			face.at.y = face.right.z * face.up.x - face.right.x * face.up.z;
			face.at.z = face.right.x * face.up.y - face.right.y * face.up.x;
			RwV3dNormalize(&face.at, &face.at);
		} else {
			BB_IDENT(&face);
			RwMatrixUpdate(&face);
			face.at.x = camLtm->pos.x - worldPos.x;
			face.at.y = camLtm->pos.y - worldPos.y;
			face.at.z = camLtm->pos.z - worldPos.z;
			RwV3dNormalize(&face.at, &face.at);
			face.up = camLtm->up;
			face.right.x = face.up.y * face.at.z - face.up.z * face.at.y;
			face.right.y = face.up.z * face.at.x - face.up.x * face.at.z;
			face.right.z = face.up.x * face.at.y - face.up.y * face.at.x;
			RwV3dNormalize(&face.right, &face.right);
		}
	} else {
		BB_IDENT(&face);
		RwMatrixUpdate(&face);
		if (mode & TFB_BILLBOARD_FIXED_UP) {
			face.up = upAxis;
			face.at.x = -camLtm->at.x;
			face.at.y = -camLtm->at.y;
			face.at.z = -camLtm->at.z;
			face.right.x = face.up.y * face.at.z - face.up.z * face.at.y;
			face.right.y = face.up.z * face.at.x - face.up.x * face.at.z;
			face.right.z = face.up.x * face.at.y - face.up.y * face.at.x;
			RwV3dNormalize(&face.right, &face.right);
			face.at.x = face.right.y * face.up.z - face.right.z * face.up.y;
			face.at.y = face.right.z * face.up.x - face.right.x * face.up.z;
			face.at.z = face.right.x * face.up.y - face.right.y * face.up.x;
			RwV3dNormalize(&face.at, &face.at);
		} else {
			face.at.x = -camLtm->at.x;
			face.at.y = -camLtm->at.y;
			face.at.z = -camLtm->at.z;
			face.up = camLtm->up;
			face.right.x = face.up.y * face.at.z - face.up.z * face.at.y;
			face.right.y = face.up.z * face.at.x - face.up.x * face.at.z;
			face.right.z = face.up.x * face.at.y - face.up.y * face.at.x;
			RwV3dNormalize(&face.right, &face.right);
		}
	}
	RwMatrixTransform(&result, &worldRot, rwCOMBINEPOSTCONCAT);
	RwMatrixTransform(&result, &face, rwCOMBINEPOSTCONCAT);
	RwMatrixTransform(&result, &worldInvRot, rwCOMBINEPOSTCONCAT);
	RwMatrixTransform(&result, &localRot, rwCOMBINEPOSTCONCAT);
	RwMatrixTransform(&result, &localTrans, rwCOMBINEPOSTCONCAT);
	if (parentLtm)
		RwMatrixTransform(&result, parentLtm, rwCOMBINEPOSTCONCAT);
	*RwFrameGetLTM(frame) = result;
	RwMatrixUpdate(RwFrameGetLTM(frame));
	if (parentFrame && parentLtm) {
		RwMatrix parentInv;
		RwMatrixInvert(&parentInv, parentLtm);
		RwMatrix *model = &frame->modelling;
		RwMatrixMultiply(model, &result, &parentInv);
		RwMatrixUpdate(model);
	}
	RwFrameUpdateObjects(frame);
}
