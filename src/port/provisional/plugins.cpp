// Stand-ins for game functions src/game has not written yet: the minimum that lets loading continue (stream readers skip their
// chunk, copies do nothing, sizes are 0). Plain C names: the linker tool maps every mangled spelling onto them (jmp trampolines).
extern "C" {
void *RwStreamSkip(void *stream, unsigned offset);

static void *skip_read(void *stream, int length, void *object, int offset, int size)
{
	(void)object; (void)offset; (void)size;
	return RwStreamSkip(stream, (unsigned)length);
}

// Only callbacks with NO src/game function at their address (the others resolve by address, see link_skeleton.py):
// TFB geometry plugin E1 getsize 0x45deb0, TFB world plugin FE copy 0x45fac0
int FUN_0045deb0(void *object, int offset, int size) { (void)object; (void)offset; (void)size; return 0; }
void *FUN_0045fac0(void *dst, void *src, int offset, int size) { (void)src; (void)offset; (void)size; return dst; }
}

int TFBPlugin_E1_GetSize(const void *object, int offset, int size) { return FUN_0045deb0((void *)object, offset, size); }
