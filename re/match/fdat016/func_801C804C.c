#include "ac1/ac1.h"

typedef struct {
    u8 state;           /* +0x00 */
    u8 idx;             /* +0x01 index into D_8019FAB8.ents */
    u8 pad2[8];
    s16 link;           /* +0x0A -1 = none */
    u8 pad0c[0x16];
    u16 wid;            /* +0x22 index into g_world_structs */
    u8 pad24[8];
} Slot2C;               /* 0x2C */
typedef struct {
    s32 flags;
    u8 pad[0x3C];
} WSf;                  /* 0x40 */

extern s32 rand(void);
extern s32 func_80014D78(s32);
extern void func_8004C318(s32);
extern s32 func_80052338(void);
extern void func_800529F0(s32, s32, s32);
extern void func_80078CFC(Slot2C *);
extern void func_8008A798();
extern void func_8008C2FC(s32);
extern WSf D_801D0B9C[];
typedef struct {
    Slot2C slots[256];              /* 0x8019FAB8 */
    AcEntity ents[16];              /* 0x801A26B8 */
} World;
extern World D_8019FAB8;
extern u16 D_8019F528;
extern u16 D_8019F52A;
extern s16 D_8019F52E;
extern s16 D_8019F532;
extern s32 D_801AC80C;
extern s16 D_801CE2FC;
extern s16 D_801CE300;
extern s32 D_801CE304;
extern s32 D_801CE30C;
extern s32 D_801CE310;
extern s32 D_801CE314;
extern s32 D_801CE318;
extern s32 D_801CE31C;
extern s32 D_801CE320;

void func_801C804C(void)
{
    s32 n;
    s32 i;
    s32 j;
    s32 k;
    s32 c;
    s32 st;
    s32 idxv;
    s32 cnt;
    s16 *px;
    s16 *py;
    u16 *pf;
    Slot2C *s;
    Slot2C *q;
    AcEntity *en;
    AcEntity *e2;

    if (rand() < 656) {
        D_801CE2FC = func_80014D78(513) - 256;
        D_801CE300 = func_80014D78(513) - 256;
    }
    px = &D_8019F52E;
    if (*px < D_801CE2FC) {
        *px = *px + 1;
    } else if (D_801CE2FC < *px) {
        *px = *px - 1;
    }
    py = &D_8019F532;
    if (*py < D_801CE300) {
        *py = *py + 1;
    } else if (D_801CE300 < *py) {
        *py = *py - 1;
    }
    n = D_801CE304;
    if (n < 4096) {
        D_801CE304 = n + 1;
        func_8008A798(n);
    }
    pf = &D_8019F52A;
    if (*pf == 0xFFFE) {
        *pf = 0;
        D_801AC80C = 1;
    }
    if (D_8019F528 != 0) {
        return;
    }
    if (D_801CE30C == 0) {
        j = 0;
        c = 0;
        do {
            if (D_8019FAB8.slots[11 + j].link != -1 && D_8019FAB8.slots[11 + j].state == 3) {
                c++;
            }
            j++;
        } while (j < 4);
        if (c == 3) {
            i = 5;
            s = &D_8019FAB8.slots[5];
            do {
                func_800529F0(0, 0, 4);
                D_801D0B9C[s->wid].flags |= 0xFFF;
                func_80078CFC(s);
                s++;
                i--;
            } while (i != 0);
            D_801CE30C++;
        }
        return;
    }
    if (D_801CE310 < 2) {
        if (D_801CE318 == 0 && D_801CE314 == 0) {
            e2 = &D_8019FAB8.ents[D_8019FAB8.slots[5].idx];
            if (e2->ap <= (e2->ap_max >> 1)) {
                D_801CE318 = 1;
                func_800529F0(0, 0, 6);
            }
        }
        if (D_801CE310 < 2 && D_801CE314 == 0 && D_8019FAB8.slots[5].link != -1 && D_8019FAB8.slots[5].state == 3) {
            D_801CE314 = 1;
            func_800529F0(0, 0, 109);
            func_8004C318(256);
            goto after;
        }
    }
    en = &D_8019FAB8.ents[D_8019FAB8.slots[5].idx];
    switch (D_801CE310) {
    case 0:
        D_801CE310 = 1;
        if (en->pos.x < -13999) {
            q = &D_8019FAB8.slots[5];
            k = 5;
            do {
                if (q->state == 1) {
                    D_8019FAB8.ents[q->idx].faction &= 0xFFF8;
                }
                k--;
                q++;
            } while (k != 0);
        }
        break;
    case 1:
        if (en->pos.x < -24999) {
            D_801CE310 = 2;
            func_800529F0(0, 0, 108);
        }
        break;
    }
after:
    if (D_801CE30C == 0xB0) {
        func_800529F0(0, 0, 107);
    } else if (D_801CE30C == 0x134) {
        func_8008C2FC(0);
    }
    if (D_801CE30C <= 0xFFFE) {
        D_801CE30C++;
    }
    if (D_801CE31C == 0 && D_8019FAB8.slots[10].link != -1) {
        if (D_8019FAB8.ents[D_8019FAB8.slots[10].idx].def == (void *)&D_8019FAB8.slots[10] && D_8019FAB8.ents[D_8019FAB8.slots[10].idx].ai_mode != 0xFF) {
            D_801CE31C = 1;
        }
    }
    if (D_801CE310 >= 2 && func_80052338() != 0) {
        if (D_8019FAB8.slots[10].link != -1 && D_8019FAB8.slots[10].state == 3) {
            if (D_801CE320 == 10) {
                func_800529F0(0, 0, 104);
                func_8004C318(128);
            }
            D_801CE320++;
            return;
        }
        D_801CE310 = 3;
    }
}
