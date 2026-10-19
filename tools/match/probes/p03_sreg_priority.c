/* probe: s0 goes to the value with more uses per live range; equal ones: x->s1, y->s0
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: move \$17,\$4
 * a: move \$16,\$5
 * b: move \$16,\$4
 * b: move \$17,\$5
 */
extern void g(int);
#ifndef VARIANT_B
int f(int x, int y) { g(0); return x - y; }
#else
int f(int x, int y) { g(0); g(x); return x - y; }
#endif
