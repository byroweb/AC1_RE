/* probe: only ra saved and a final store through a register: lw ra; sw; jr ra; addiu sp (filled). A final global store (lui $at macro) gives addiu sp; jr ra; nop
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: lw \$31,16\(\$29\);sw \$3,116\(\$2\);jr \$31;addiu \$29,\$29,24$
 * b: addiu \$29,\$29,24;jr \$31;nop$
 */
extern int *hp(void); extern int K;
#ifndef VARIANT_B
void f(void) { int *p = hp(); p[29] = 5; }
#else
void f(void) { int *p = hp(); K = 5; }
#endif
