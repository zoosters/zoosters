// Bounds accumulator and shadow-pass callback pointer.

// The game stores the address of this member function (ShadowPass::RenderActorShadow, 0x4437f0) in a data word.
struct SLink;
struct ShadowPass { void RenderActorShadow(SLink *link); };

// shadowcam2.cpp passes the address of this word on (declared there as char[]). The literals behind it are not part of it.
// ALONE: DAT_005d4dc8 C string, only read through a pointer up to its NUL
// GLOBAL: 0x5d4dc8 DAT_005d4dc8
void (ShadowPass::*DAT_005d4dc8)(SLink *) = &ShadowPass::RenderActorShadow;   // MSVC single-inheritance member pointer is a plain address

