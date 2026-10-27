#ifndef ZOOSTERS_MODULES_FX_CFXCOLORLIGHT_H
#define ZOOSTERS_MODULES_FX_CFXCOLORLIGHT_H
#include "modules/fx/cfxbaselight.h"
#include "modules/tfb/ctfbcommandstub.h"
namespace RWS {
namespace FX {
class GameLight;
// PS2: CFXBaseLight@0, CTFBCommand@52.
class CFXColorLight : public CFXBaseLight, public TFB::CTFBCommand {
public:
	RWS_MAKENEWCLASS(CFXColorLight);
	CFXColorLight(const CAttributePacket &attr);
	~CFXColorLight();
	virtual void HandleAttributes(const CAttributePacket &attr);
private:
	GameLight *m_gameLight;   // the script object, allocated by the constructor
};
ZOOSTERS_ASSERT_SIZE(CFXColorLight, 0x38);
}}
#endif
