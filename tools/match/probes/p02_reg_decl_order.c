/* probe: declaration order alone does not change allocation (only use order does)
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: same
 */
extern int h(void); extern int K;
#ifndef VARIANT_B
int f(void) { int a, b; a = h(); b = h(); return a + b + K; }
#else
int f(void) { int b, a; a = h(); b = h(); return a + b + K; }
#endif
