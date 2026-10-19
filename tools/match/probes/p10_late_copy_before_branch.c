/* probe: re-reading a field into a local before an if leaves a register copy in the branch delay slot (move, then and)
 * defsym: K=0x80100F00 K2=0x80100F04 V=0x80100F08 N=0x80100F0C HV=0x800411C2 HT=0x800411C4 GP=0x80040FAC G=0x80040FA8 H=0x800411C0 A=0x80100800 VA=0x80100900 g=0x80100A00 h=0x80100B00 hp=0x80100B40
 * bytes: differ
 * a: bne \$6,\$4,0x[0-9a-f]+;move \$2,\$5;
 * a: and \$2,\$7,\$2
 * b: bne \$6,\$4,0x[0-9a-f]+;and \$2,\$7,\$5;
 */
typedef struct { unsigned int cur; unsigned int prev; char pad[0x292]; unsigned short r; char p2[2]; unsigned short l; } Pad;
typedef struct { short pos, vel, timer; } Sc;
extern Pad G; extern Sc H; extern short HV, HT; extern unsigned int GP;
#ifndef VARIANT_B
void f(void) { unsigned int raw, mask, cur, b; short t; raw = G.cur; mask = G.l | G.r; cur = raw & mask;
    if (cur != (GP & mask)) { Sc *p = &H; p->timer = 0; } t = HT;
    if (t == 0) { b = G.cur; if (cur == mask) { HV = 0; H.pos = 0; HT = 10; } else { if (G.l & b) HV += 8; else if (G.r & G.cur) HV -= 8; H.pos += H.vel; } } else HT = t - 1; }
#else
void f(void) { unsigned int raw, mask, cur; short t; raw = G.cur; mask = G.l | G.r; cur = raw & mask;
    if (cur != (GP & mask)) { Sc *p = &H; p->timer = 0; } t = HT;
    if (t == 0) { if (cur == mask) { HV = 0; H.pos = 0; HT = 10; } else { if (G.l & G.cur) HV += 8; else if (G.r & G.cur) HV -= 8; H.pos += H.vel; } } else HT = t - 1; }
#endif
