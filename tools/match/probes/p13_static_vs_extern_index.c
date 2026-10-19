/* probe: static vs extern array changes only the relocation, not the instruction shape (the static's own .bss relocation stays masked)
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: any
 * masked-ok: yes
 * a: ^sll \$4,\$4,0x2;lui \$1,0x[0-9a-f]+;addu \$1,\$1,\$4;lw \$2,\d+\(\$1\);jr \$31;nop$
 * b: ^sll \$4,\$4,0x2;lui \$1,0x[0-9a-f]+;addu \$1,\$1,\$4;lw \$2,\d+\(\$1\);jr \$31;nop$
 */
extern int A[]; static int SA[8];
#ifndef VARIANT_B
int f(int i) { return A[i]; }
#else
int f(int i) { return SA[i]; }
#endif
