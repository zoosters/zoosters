#include "rw/rwcore.h"
// Actor local-point transform 0x42cf00. Stand-in class with raw offsets.
struct MV3 { float x, y, z; };
struct MMat { char b[0x40]; };


class ActorLocalPoint {
public:
	char pad[0x30];
	char m_matrix[0x30];   // +0x30 RwMatrix (right/up/at): passed whole to RwMatrixMultiply, its pos follows
	int m_matrixPosX, m_matrixPosY, m_matrixPosZ;   // +0x60 RwMatrix pos, cleared before use
	char pad2[0xbc - 0x6c];
	MV3 m_localPoint;   // +0xbc the point in the frame*matrix space
	void *m_frame;   // +0xc8 RwFrame (RwFrameGetLTM)
	unsigned char m_hasLocalPoint;   // +0xcc set once m_localPoint is valid
	void SetLocalPointFromWorld(const MV3 *point, int unused);
};

// 0x42cf00
void ActorLocalPoint::SetLocalPointFromWorld(const MV3 *point, int unused)
{
	m_matrixPosZ = 0;
	m_matrixPosY = 0;
	m_matrixPosX = 0;
	MMat world;
	MMat inv;
	MV3 pt;
	RwMatrixMultiply(reinterpret_cast<RwMatrix *>(&world), RwFrameGetLTM(static_cast<RwFrame *>(m_frame)), reinterpret_cast<const RwMatrix *>(&m_matrix));
	RwMatrixInvert(reinterpret_cast<RwMatrix *>(&inv), reinterpret_cast<const RwMatrix *>(&world));
	pt = *point;
	RwV3dTransformPoint(reinterpret_cast<RwV3d *>(&m_localPoint), reinterpret_cast<const RwV3d *>(&pt), reinterpret_cast<const RwMatrix *>(&inv));
	m_hasLocalPoint = 1;
}
