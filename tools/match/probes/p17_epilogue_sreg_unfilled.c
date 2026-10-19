/* probe: a second saved register (s0) restores after the store: always addiu sp; jr ra; nop
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: jr \$31;addiu \$29,\$29,24$
 * b: lw \$16,16\(\$29\);addiu \$29,\$29,24;jr \$31;nop$
 */
extern int *hp(void);
#ifndef VARIANT_B
void f(void) { int *p = hp(); p[29] = 5; }
#else
void f(int x) { int *p = hp(); p[29] = x; }
#endif
