/* probe: one indexed access: lui/addu/lw %lo (split); two accesses to the same array: lui/addiu then addu/lw 0 (la form)
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: lui \$1,0x8010;addu \$1,\$1,\$4;lw \$2,2048\(\$1\)
 * b: lui \$2,0x8010;addiu \$2,\$2,2048
 * b: lw \$2,4\(\$2\)
 */
extern int A[];
#ifndef VARIANT_B
int f(int i) { return A[i]; }
#else
int f(int i) { return A[i] + A[i + 1]; }
#endif
