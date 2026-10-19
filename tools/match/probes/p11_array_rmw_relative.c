/* probe: extern array: element RMW in a branch + re-read after the join makes A[-16] base-relative (-32); without the RMW it stays direct
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: lh \$5,-32\(\$4\)
 * b: lh \$3,2016\(\$3\)
 */
extern short A[]; extern int K;
#ifndef VARIANT_B
void f(void) { if (A[0] != A[-16]) A[0] += 8; if (A[0] == A[-16]) K = 1; }
#else
void f(void) { if (A[0] != A[-16]) K = 2; if (A[0] == A[-16]) K = 1; }
#endif
