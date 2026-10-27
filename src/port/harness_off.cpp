// The development harness (src/dev, ZOOSTERS_DEV_HARNESS) is left out: the hooks the runtime calls into it, inactive.
// rand / srand stay the game's own LCG (libcmt's: state * 214013 + 2531011, bits 16..30); the C library's rand differs per platform.
#include <stddef.h>

extern "C" {
void *port_gaddr(unsigned) { return 0; }
int port_gaddr_selftest(void) { return 0; }
void port_gtrace_init(void) {}
int port_gtrace_wanted(void) { return 0; }
int port_gtrace_base(void) { return -1; }
int port_gtrace_trace_frame(void) { return -1; }
void port_rnglog_init(void) {}
void port_rnglog_rp(unsigned, void *) {}
unsigned port_rnglog_seed(unsigned seed) { return seed; }
void port_semtrace_init(void) {}
void port_semtrace_query(int, const float *, int, int, int, int) {}
unsigned port_rq[24];
#ifdef RW_D3D9
int port_d3dcap_ndraw(void) { return -1; }
void port_d3dcap_after_present(int) {}
#endif
}

static __declspec(thread) unsigned t_holdrand = 1;
extern "C" int __cdecl rand(void)
{
	t_holdrand = t_holdrand * 0x343FDu + 0x269EC3u;
	return (int)((t_holdrand >> 16) & 0x7FFF);
}
extern "C" void __cdecl srand(unsigned seed) { t_holdrand = seed; }
