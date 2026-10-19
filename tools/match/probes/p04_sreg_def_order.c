/* probe: two call results kept across a call: the first defined lands in s1, the second in s0
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: subu \$2,\$17,\$16
 * b: subu \$2,\$16,\$17
 */
extern void g(int); extern int h(void);
#ifndef VARIANT_B
int f(void) { int a, b; a = h(); b = h(); g(0); return a - b; }
#else
int f(void) { int a, b; b = h(); a = h(); g(0); return a - b; }
#endif
