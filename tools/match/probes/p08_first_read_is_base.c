/* probe: when fields go base-relative, the field read first is the base (cur first: 670/666/0; masks first: 0/-4/-670)
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: addiu \$5,\$5,4008;lhu \$3,670\(\$5\);lhu \$2,666\(\$5\)
 * b: addiu \$5,\$5,4678;lhu \$3,0\(\$5\);lhu \$2,-4\(\$5\)
 * b: lw \$3,-670\(\$5\)
 */
typedef struct { unsigned int cur; unsigned int prev; char pad[0x292]; unsigned short r; char p2[2]; unsigned short l; } Pad;
typedef struct { short pos, vel, timer; } Sc;
extern Pad G; extern Sc H;
#ifndef VARIANT_B
void f(void) { unsigned int raw, mask, cur; raw = G.cur; mask = G.l | G.r; cur = raw & mask;
    if (cur != (G.prev & mask)) H.timer = 0;
    if (H.timer == 0) { if (cur == mask) H.vel = 0; else if (G.cur & G.l) H.vel = 1; else if (G.cur & G.r) H.vel = 2; } }
#else
void f(void) { unsigned int mask, cur; mask = G.l | G.r; cur = G.cur & mask;
    if (cur != (G.prev & mask)) H.timer = 0;
    if (H.timer == 0) { if (cur == mask) H.vel = 0; else if (G.l & G.cur) H.vel = 1; else if (G.r & G.cur) H.vel = 2; } }
#endif
