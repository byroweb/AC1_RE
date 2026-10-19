/* probe: a counting loop whose index only feeds stores is reversed (count down, bgez); a volatile target keeps it forward
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: any
 * a: addiu \$2,\$2,-1;bgez
 * b: slti \$2,\$3,4;bnez
 * not-b: bgez
 */
extern int A[]; extern volatile int VA[];
#ifndef VARIANT_B
void f(void) { int i; for (i = 0; i < 4; i++) A[i] = i; }
#else
void f(void) { int i; for (i = 0; i < 4; i++) VA[i] = i; }
#endif
