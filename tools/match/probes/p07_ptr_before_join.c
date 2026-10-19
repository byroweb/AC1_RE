/* probe: a pointer set before a join addresses every field through the base after it; set after the join only offset 0 does
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: lhu \$3,2\(\$4\)
 * b: lhu \$4,4546\(\$4\)
 */
typedef struct { unsigned int cur; unsigned int prev; char pad[0x292]; unsigned short r; char p2[2]; unsigned short l; } Pad;
typedef struct { short pos, vel, timer; } Sc;
extern Sc H; extern int K;
#ifndef VARIANT_B
void f(void) { Sc *s = &H; if (K) H.vel = 8; else H.vel = -8; s->pos += s->vel; }
#else
void f(void) { Sc *s; if (K) H.vel = 8; else H.vel = -8; s = &H; s->pos += s->vel; }
#endif
