/* probe: LICM: a global read in the body is hoisted after the guard; a local copy is loaded before it
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: blez \$5,[^;]*;move \$3,\$0;lui \$6,0x8010;lw \$6,3840
 * b: lw \$6,3840\(\$6\);blez
 */
extern int K, N; extern int A[];
#ifndef VARIANT_B
void f(void) { int i; for (i = 0; i < N; i++) A[i] = K; }
#else
void f(void) { int i, k = K, m = N; for (i = 0; i < m; i++) A[i] = k; }
#endif
