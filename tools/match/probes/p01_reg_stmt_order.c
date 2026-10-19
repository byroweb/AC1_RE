/* probe: statement (first-use) order picks v0/v1: swapping two loads swaps the registers
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: subu \$2,\$3,\$2
 * b: subu \$2,\$2,\$3
 */
extern int A[];
#ifndef VARIANT_B
int f(int i, int j) { int a, b; a = A[i]; b = A[j]; return a - b; }
#else
int f(int i, int j) { int a, b; b = A[j]; a = A[i]; return a - b; }
#endif
