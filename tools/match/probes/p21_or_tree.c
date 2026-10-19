/* probe: OR trees are not reassociated: a|b|c|d is a left chain, (a|b)|(c|d) is balanced
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: or \$2,\$4,\$5;or \$2,\$2,\$6;jr \$31;or \$2,\$2,\$7
 * b: or \$4,\$4,\$5;or \$2,\$6,\$7;jr \$31;or \$2,\$4,\$2
 */
#ifndef VARIANT_B
int f(int a, int b, int c, int d) { return a | b | c | d; }
#else
int f(int a, int b, int c, int d) { return (a | b) | (c | d); }
#endif
