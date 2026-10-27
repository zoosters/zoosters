#ifndef ZOOSTERS_TFB_LEVEL_RESOURCEHANDLERS_VIEWS_H
#define ZOOSTERS_TFB_LEVEL_RESOURCEHANDLERS_VIEWS_H

#include "rw/rwcore.h"

// Rt2dFont is opaque in RW's API; the font loader scales a new font's height and replaces its page's raster.
// The unnamed members are the font's flags / glyph data (RW 3.6 Rt2dFont).
struct Rt2dFontView {
	char reserved00[4];
	RwReal height;
	char reserved08[0xc];
	RwTexture *texture;  // the font's page
};

#endif
