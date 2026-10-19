/* probe: early returns with identical tails are cross-jumped into one store (shared exit); a result variable keeps one store per arm
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * count-a: 1 sw \$2,3840\(\$1\)
 * count-b: 2 sw \$2,3840\(\$1\)
 */
extern int K, K2;
#ifndef VARIANT_B
int f(int x) { if (x) { K = 1; return 1; } if (K2) { K = 2; return 2; } return 0; }
#else
int f(int x) { int r; if (x) { K = 1; r = 1; } else if (K2) { K = 2; r = 2; } else r = 0; return r; }
#endif
