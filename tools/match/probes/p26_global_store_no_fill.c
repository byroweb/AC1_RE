/* probe: a store to a bare global is a lui $at macro and never fills a delay slot; volatile makes no difference
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: any
 * a: bnez \$2,[^;]*;nop
 * b: bnez \$2,[^;]*;nop
 */
extern int K; extern volatile int V;
#ifndef VARIANT_B
void f(void) { int i; for (i = 0; i < 4; i++) K = i; }
#else
void f(void) { int i; for (i = 0; i < 4; i++) V = i; }
#endif
