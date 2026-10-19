/* probe: frameless leaf: a single-exit result variable fills jr's slot (jr ra; move v0,..); early returns leave jr ra; nop
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: jr \$31;move \$2,\$3$
 * b: jr \$31;nop$
 */
extern int K;
#ifndef VARIANT_B
int f(int x) { int r = 0; if (x) r = 1; else K = 2; return r; }
#else
int f(int x) { if (x) return 1; K = 2; return 0; }
#endif
