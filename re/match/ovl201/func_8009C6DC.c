/* ovl201 0x8009C6DC: matched (no extra flags). Loops 2/3 are call + while; hi/q are set in both arms of a free (s8)v < 0 test (jump2 merges the arms): the join hides q's constant from cse so q->lo stays 1(s1), and hi is an explicit variable reused in the switch */
#include "ac1/ac1.h"
/* ovl201 0x8009C6DC - new pilot: bump the D_80039D20 counter, reset the pilot
 * name (D_80031BE6, 18 bytes) from the template at D_8004D150 and append three
 * random SJIS full-width digits (0x82 0x4F+n); four random values instead
 * give fixed digit triples.  Then plays movie 6. */
typedef struct { u8 b[18]; } Name18;
extern u8 D_80039D20;
extern s32 D_80039D14;
extern Name18 D_8004D150;
extern Name18 D_80031BE6;
extern u8 D_80031BF0;
extern u8 D_80031BF1;
extern u8 D_80031BF2;
extern u8 D_80031BF3;
typedef struct { u8 hi; u8 lo; } SjisCh;
extern SjisCh D_80031BF4;
extern void func_8009C928(void);
extern void func_800A448C(s16 idx, u8 mode);

void func_8009C6DC(void)
{
    u8 *p = &D_80039D20;
    u8 v;
    u8 hi;
    SjisCh *q;

    if (*p != 0xFF) {
        *p = *p + 1;
    }
    D_80031BE6 = D_8004D150;
    while ((v = D_80039D14 & 0xF) >= 10) {
        func_8009C928();
    }
    D_80031BF0 = 0x82;
    D_80031BF1 = v + 0x4F;
    func_8009C928();
    while ((v = D_80039D14 & 0xF) >= 10) {
        func_8009C928();
    }
    D_80031BF2 = 0x82;
    D_80031BF3 = v + 0x4F;
    func_8009C928();
    while ((v = D_80039D14 & 0xF) >= 10) {
        func_8009C928();
    }
    if ((s8)v < 0) {
        hi = 0x82;
        q = &D_80031BF4;
    } else {
        hi = 0x82;
        q = &D_80031BF4;
    }
    q->hi = hi;
    q->lo = v + 0x4F;
    func_8009C928();
    switch ((u32)(D_80039D14 & 0xF)) {
    case 0:
        D_80031BF1 = 0x51;
        D_80031BF0 = hi;
        D_80031BF2 = hi;
        D_80031BF3 = 0x55;
        q->hi = hi;
        q->lo = 0x55;
        break;
    case 4:
        D_80031BF1 = 0x53;
        D_80031BF3 = 0x50;
        D_80031BF0 = hi;
        D_80031BF2 = hi;
        q->hi = hi;
        q->lo = 0x56;
        break;
    case 1:
        D_80031BF1 = 0x52;
        D_80031BF3 = 0x52;
        D_80031BF0 = hi;
        D_80031BF2 = hi;
        q->hi = hi;
        q->lo = 0x50;
        break;
    case 5:
        D_80031BF1 = 0x57;
        D_80031BF3 = 0x51;
        D_80031BF0 = hi;
        D_80031BF2 = hi;
        q->hi = hi;
        q->lo = 0x50;
        break;
    }
    func_800A448C(6, 0);
}
