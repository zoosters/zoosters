#include "modules/audio/audioglobalmixer.h"
namespace RWS { namespace Audio {
// 0x406e60
CAttributeHandler *(*g_keepMakeNew_AudioGlobalMixer)(const CAttributePacket &) = &AudioGlobalMixer::MakeNew;
}}
