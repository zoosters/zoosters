# Notices

Zoosters is licensed under the GNU GPL v3 ([LICENSE](LICENSE)). It reimplements the game's code and contains no game assets
and no source code from the game's developers or the RenderWare SDKs. A few tables (names, constants, data formats) are taken
from the original executable to read the game's data. Requires your own copy of the original PC game.

The arrow images in `src/port/options/assets` are our own.

*Madagascar* is a trademark of DreamWorks Animation; the game is published by Activision. Zoosters is an unofficial fan
project and is not affiliated with or endorsed by DreamWorks Animation, Activision or Toys for Bob.

## Third-party code and algorithms

**fdlibm.** `gameAcos` and `gameSinK` in `src/port/shim/rwcore.cpp` follow fdlibm's single-precision `acosf` and
`__kernel_sinf`:

> Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
>
> Developed at SunPro, a Sun Microsystems, Inc. business.
> Permission to use, copy, modify, and distribute this software is freely granted, provided that this notice is preserved.

**Collision algorithms.** The oriented-box tests in `src/game/tfb/collision/` implement the separating-axis and contact
algorithms David Eberly published with his Magic Software library (1999-2000) and his paper "Dynamic Collision Detection
using Oriented Bounding Boxes". Our code is an independent implementation of those algorithms; credit for the methods goes
to him.

**FFmpeg (Bink decoder).** `src/port/bink/binkdec.cpp` and `binkdec.h` are derived from FFmpeg 7.1 (`libavformat/bink.c`,
`libavcodec/bink.c`, `binkdsp.c`, `binkdata.h`, `binkaudio.c`, `wma_freqs.c`) and stay under the GNU Lesser General Public
License 2.1 or later, as the originals:

> Copyright (c) 2009 Konstantin Shishkov; Copyright (c) 2007-2011 Peter Ross; Copyright (c) 2009 Daniel Verkamp.
> FFmpeg is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as
> published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

Bink is a format of RAD Game Tools; no RAD code or headers are used.

## Libraries used at build or run time (not part of this repository)

| library | licence | how it is used |
|---|---|---|
| [librw](https://github.com/aap/librw) (our fork [zoosters/librw](https://github.com/zoosters/librw), branch `zoosters`) | MIT | RenderWare reimplementation; git submodule `vendor/librw` |
| [Dear ImGui](https://github.com/ocornut/imgui) (shipped with librw) | MIT | debug overlay, only in `ZOOSTERS_DEBUGUI` builds |
| [SDL 3](https://github.com/libsdl-org/SDL) | zlib | window, input, game controllers |
| [OpenAL Soft](https://github.com/kcat/openal-soft) | LGPL 2.0 or later | audio; linked dynamically |

Binary releases ship these libraries' licence texts.
