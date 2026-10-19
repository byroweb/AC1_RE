/* probe: p = &G; p->x is folded back to direct lui/%lo accesses (same bytes as G.x)
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: same
 * a: lhu \$2,4678\(\$2\)
 */
typedef struct { unsigned int cur; unsigned int prev; char pad[0x292]; unsigned short r; char p2[2]; unsigned short l; } Pad;
typedef struct { short pos, vel, timer; } Sc;
extern Pad G;
#ifndef VARIANT_B
unsigned int f(void) { return (G.l | G.r) & G.cur; }
#else
unsigned int f(void) { Pad *p = &G; return (p->l | p->r) & p->cur; }
#endif
