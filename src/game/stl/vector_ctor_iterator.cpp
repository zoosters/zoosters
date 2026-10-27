// CRT `vector constructor iterator`, no EH.
typedef void (__fastcall *VecCtorFn)(void *);

// 0x406810
void __stdcall VecCtorIter(void *ptr, unsigned size, int count, VecCtorFn ctor)
{
	while (--count >= 0) {
		ctor(ptr);
		ptr = (char *)ptr + size;
	}
}
