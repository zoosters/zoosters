// Device vertex types for the headless (RW_NULL) build: librw's null platform defines no Im2DVertex / Im3DVertex / RWDEVICE, but
// the shims build immediate-mode vertices with them. Plain copies of librw's d3d9 layout (src/d3d/rwd3d.h); the null engine draws
// nothing. Include after <rw.h>.
#pragma once
#if defined(RW_NULL) && !defined(RWDEVICE)
namespace rw {
namespace portnull {

struct Im3DVertex
{
	V3d position;
	V3d normal;
	uint32 color;
	float32 u, v;

	void setX(float32 x) { position.x = x; }
	void setY(float32 y) { position.y = y; }
	void setZ(float32 z) { position.z = z; }
	void setNormalX(float32 x) { normal.x = x; }
	void setNormalY(float32 y) { normal.y = y; }
	void setNormalZ(float32 z) { normal.z = z; }
	void setColor(uint8 r, uint8 g, uint8 b, uint8 a) { color = (uint32)a << 24 | (uint32)r << 16 | (uint32)g << 8 | b; }
	void setU(float32 u_) { u = u_; }
	void setV(float32 v_) { v = v_; }
	float getX(void) { return position.x; }
	float getY(void) { return position.y; }
	float getZ(void) { return position.z; }
	float getU(void) { return u; }
	float getV(void) { return v; }
};

struct Im2DVertex
{
	float32 x, y, z;
	float32 w;
	uint32 color;
	float32 u, v;

	void setScreenX(float32 x_) { x = x_; }
	void setScreenY(float32 y_) { y = y_; }
	void setScreenZ(float32 z_) { z = z_; }
	void setCameraZ(float32 z_) { w = z_; }
	void setRecipCameraZ(float32 recipz) { w = 1.0f / recipz; }
	void setColor(uint8 r, uint8 g, uint8 b, uint8 a) { color = (uint32)a << 24 | (uint32)r << 16 | (uint32)g << 8 | b; }
	void setU(float32 u_, float) { u = u_; }
	void setV(float32 v_, float) { v = v_; }
	float getScreenX(void) { return x; }
	float getScreenY(void) { return y; }
	float getScreenZ(void) { return z; }
	float getCameraZ(void) { return w; }
	float getU(void) { return u; }
	float getV(void) { return v; }
};

}  // namespace portnull
}  // namespace rw
#define RWDEVICE portnull
#endif
