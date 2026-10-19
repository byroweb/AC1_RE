/* probe: if (n > 0) do {} while: same guard as for(), but no empty frame
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: \$29
 * b: ^blez \$4
 * not-b: \$29
 */
extern int A[];
#ifndef VARIANT_B
void f(int n) { int i; for (i = 0; i < n; i++) A[i] = 0; }
#else
void f(int n) { int i = 0; if (n > 0) do { A[i] = 0; i++; } while (i < n); }
#endif
