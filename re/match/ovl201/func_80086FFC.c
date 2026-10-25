#include "ac1/ac1.h"

typedef struct {
    s8 lo;                      /* lowest rank that may see the entry */
    s8 hi;                      /* highest rank */
    s8 kind;
    s8 pad;
} ListEnt;

typedef struct {
    u8 pad0[2];
    s8 rank;                    /* +0x002 0x801F3752 */
    u8 pad3;
    s8 b4;                      /* +0x004 0x801F3754 show-all switch */
    u8 pad5[0x28 - 5];
    s32 w28;                    /* +0x028 0x801F3778 */
    u8 pad2C[0x38 - 0x2C];
    u8 flags[64];               /* +0x038 0x801F3788 */
    u8 pad78[0x84 - 0x78];
    s8 order[64];               /* +0x084 0x801F37D4 */
    ListEnt ent[64];            /* +0x0C4 0x801F3814 */
} Garage3750;

extern Garage3750 D_801F3750;
extern u8 D_80031B0E[];
extern void func_80086EC8(void);
extern s8 D_801F3754;
extern s8 D_801F37D4[];
extern ListEnt D_801F3814[];
extern s32 D_801F3778;
extern u8 D_801F37C8[];

void func_80086FFC(void)
{
    s32 n;
    s32 rank;
    s32 i;
    s32 idx;
    s32 k;
    s32 j;
    s32 e;
    s32 c;
    u8 *fl;
    s32 neg;
    u8 *q;
    s32 t;

    n = 0;
    rank = D_801F3750.rank;
    do {
        for (i = 64; i != 0; i--) {
            neg = -1;
            fl = D_801F37C8;
            if (D_801F3754 == 1) {
                if (rank < 0) {
                    e = 0;
                    idx = 64 - i;
                } else {
                    e = 0;
                    idx = 64 - i;
                }
                if (rank < D_801F3750.ent[idx].lo) continue;
                if (D_801F3750.ent[idx].hi < rank) continue;
                if (idx == 17) {
                    if (D_80031B0E[0] != 0) continue;
                    if (D_80031B0E[0xA0] != 0) continue;
                }
                c = 64 - i;
                if (i == c + i - 27) {
                    if (D_801F3750.flags[8] & 4) {
                        if (!(D_801F3750.flags[8] & 2)) continue;
                    }
                }
                c = 64 - i;
                if (i == c + i - 50) {
                    if (!(D_801F3750.flags[8] & 4)) continue;
                    if (D_801F3750.flags[8] & 2) continue;
                }
                c = 0;
                if (rank >= 26) {
                    if (D_801F3750.w28 > 0) {
                        do { j = 64 - i; } while (0);
                        if (D_801F3750.ent[j].kind == 0) goto ok;
                        if ((u8)(D_801F3750.ent[j].kind - 1) < 2) goto ok;
                    skip:
                        idx = 0;
                        continue;
                    } else if (D_801F3750.w28 < 0) {
                        do { j = 64 - i; } while (0);
                        if (D_801F3750.ent[j].kind == 0) goto ok;
                        if ((u8)(D_801F3750.ent[j].kind + 2) < 2) goto ok;
                        goto skip;
                    } else {
                        do { j = 64 - i; } while (0);
                        k = D_801F3750.ent[j].kind;
                        if (k != 0 && k != 3 && k != 1 && k != neg) continue;
                    }
                }
            ok:
                if (rank < 0) {
                    do { q = fl - i; } while (0);
                    t = *q;
                } else {
                    do { q = fl - i; } while (0);
                    t = *q;
                }
                if (!(t & 2)) {
                    do { j = 64 - i; } while (0);
                    do { D_801F37D4[n++] = j; } while (0);
                }
            } else {
                idx = 0;
                do { e = 64 - i; } while (0);
                do {
                    do {
                        do {
                            if (D_801F3750.ent[e].lo != neg) {
                                D_801F3750.order[n++] = e;
                            }
                        } while (0);
                    } while (0);
                } while (0);
            }
        }
        neg = -1;
        fl = D_801F37C8;
        rank++;
        if (rank == D_801F3750.rank) {
            func_80086EC8();
        }
        rank &= 63;
    } while (n == 0);
    if (D_801F3750.b4 == 1) {
        D_801F3750.rank = rank - 1;
    }
}
