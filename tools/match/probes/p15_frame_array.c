/* probe: an unused local array is allocated anyway: int u[4] gives an empty 16-byte frame
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * not-a: \$29
 * b: addiu \$29,\$29,-16
 * not-b: \(\$29\)
 */
extern int K;
#ifndef VARIANT_B
void f(void) { K = 1; }
#else
void f(void) { int u[4]; K = 1; }
#endif
