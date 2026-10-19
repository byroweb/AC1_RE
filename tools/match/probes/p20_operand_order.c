/* probe: commutative operands keep source order: a & b -> and v0,a0,a1; b & a -> and v0,a1,a0
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: and \$2,\$4,\$5
 * b: and \$2,\$5,\$4
 */
#ifndef VARIANT_B
int f(int a, int b) { return a & b; }
#else
int f(int a, int b) { return b & a; }
#endif
