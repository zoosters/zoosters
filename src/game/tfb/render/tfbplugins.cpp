// Registration chain of the game's RenderWare plugins and the identity callback several of them share.
#include "rw/rpplugins.h"
#include "tfb/render/tfbatomic.h"
#include "tfb/render/tfbclump.h"
#include "tfb/render/tfbgeometry.h"
#include "tfb/render/tfblight.h"
#include "tfb/render/tfbmaterial.h"
#include "tfb/render/tfbplugins.h"
#include "tfb/render/tfbsector.h"
#include "tfb/render/tfbworld.h"

// GLOBAL: 0x4a10e0 RpMatFXPluginAttach

// 0x45e020
void *TFBPlugin_Identity(void *first)
{
	return first;
}

// 0x45f9a0
RwBool TFBPlugins_Attach()
{
	if (!RpMatFXPluginAttach())
		return 0;
	if (!TFBPlugin_F6_Attach())
		return 0;
	if (!TFBPlugin_E1_Attach())
		return 0;
	if (!TFBPlugin_FE_Attach())
		return 0;
	if (!TFBPlugin_FD_Attach())
		return 0;
	if (!TFBPlugin_C2_Attach())
		return 0;
	if (!TFBPlugin_B0_Attach())
		return 0;
	if (!TFBPlugin_B1_Attach())
		return 0;
	return 1;
}
