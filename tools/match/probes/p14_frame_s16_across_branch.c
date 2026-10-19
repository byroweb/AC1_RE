/* probe: an s16 local live across a branch leaves an unused 8-byte stack slot (addiu sp,-8 with no sp loads/stores); int does not
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * not-a: \$29
 * b: addiu \$29,\$29,-8
 * not-b: \(\$29\)
 */
typedef struct { short pos, vel, timer; } Sc; extern Sc H;
#ifndef VARIANT_B
void f(void) { int v = H.vel; if (v > 0) H.vel = v - 16; }
#else
void f(void) { short v = H.vel; if (v > 0) H.vel = v - 16; }
#endif
