#include "ac1/ac1.h"

typedef struct {                /* 0x78, copied from 0x80041B10 */
    s8 id;                      /* +0x00 finished entry, -1 none */
    u8 pad1;
    s8 rank;                    /* +0x02 */
    u8 pad3;
    s8 b4;                      /* +0x04 */
    s8 b5;                      /* +0x05 */
    u16 h6;                     /* +0x06 */
    u16 h8;                     /* +0x08 */
    u16 add[15];                /* +0x0A */
    u8 pad28[0x28 - 0x28];
    s32 w28;                    /* +0x28 */
    u8 pad2C[4];
    u32 n30;                    /* +0x30 */
    u32 n34;                    /* +0x34 */
    u8 flags[64];               /* +0x38 */
} Prog;

typedef struct {
    Prog p;                     /* +0x000 */
    volatile u8 busy;           /* +0x078 */
    s8 cursor;                  /* +0x079 */
    s8 top;                     /* +0x07A */
} Garage3750;

extern Garage3750 D_801F3750;
extern Prog D_80041B10;
typedef struct { s32 failed; s32 bonus; } Result;
extern Result D_80048610;
extern u8 D_80039D22;
typedef struct { s32 total; s32 x[15]; } Money;
extern Money D_80049E70;
extern u8 D_80031A64[2][0xA0];  /* part stock: [0] one table, [1] the other (0xA0 apart) */
extern s32 func_800867C4(void);

void func_80086838(void)
{
    s32 i;
    u8 *p;
    u8 v;

    D_801F3750.p = D_80041B10;
    if (D_801F3750.p.id == -1) return;
    if (D_80048610.failed == 0) {
        if (D_801F3750.p.b4 == 1 && (D_801F3750.p.id == 21 || D_801F3750.p.id == 22
                || D_801F3750.p.id == 47 || D_801F3750.p.id == 48 || D_801F3750.p.id == 49)) {
            D_801F3750.p.rank++;
        }
        if ((u8)D_801F3750.p.id == 48 || (u8)D_801F3750.p.id == 49) {
            D_801F3750.p.flags[48] |= 2;
            D_801F3750.p.flags[49] |= 2;
        } else {
            D_801F3750.p.flags[D_801F3750.p.id] |= 2;
        }
        if (++D_801F3750.p.n30 >= 1000) {
            D_801F3750.p.n30 = 999;
        }
        if (func_800867C4() != 0) {
            D_80039D22 = 1;
        }
        if (D_801F3750.p.id == 2) {
            p = &D_80031A64[0][0x4E]; v = *p; if (v == 0 && D_80031A64[1][0x4E] == 0) *p = v + 1;
        } else if (D_801F3750.p.id == 13) {
            p = &D_80031A64[0][0x80]; v = *p; if (v == 0 && D_80031A64[1][0x80] == 0) *p = v + 1;
        } else if (D_801F3750.p.id == 36) {
            p = &D_80031A64[0][0x82]; v = *p; if (v == 0 && D_80031A64[1][0x82] == 0) *p = v + 1;
        }
        if (D_801F3750.p.b4 == 1) {
            if (D_801F3750.p.id == 26) {
                D_80049E70.x[7] = 0;
            } else if (D_801F3750.p.id == 27) {
                D_80049E70.x[2] = 0;
            } else if (D_801F3750.p.id == 47) {
                D_80049E70.x[12] = 0;
            }
            D_80049E70.total += D_801F3750.p.h6;
            D_80049E70.total += D_80048610.bonus;
            D_801F3750.p.w28 += D_801F3750.p.b5;
        }
    } else {
        if ((u8)D_801F3750.p.id == 48 || (u8)D_801F3750.p.id == 49) {
            D_801F3750.p.flags[48] |= 4;
            D_801F3750.p.flags[49] |= 4;
        } else {
            D_801F3750.p.flags[D_801F3750.p.id] |= 4;
        }
        if (++D_801F3750.p.n34 >= 1000) {
            D_801F3750.p.n34 = 999;
        }
        if (D_801F3750.p.b4 == 1) {
            D_80049E70.total += D_801F3750.p.h8;
            D_801F3750.p.w28 -= D_801F3750.p.b5;
        }
    }
    if (D_801F3750.p.b4 == 1) {
        if (D_801F3750.p.rank >= 11) {
            if (D_80031A64[1][0x4F] == 0 && D_80031A64[0][0x4F] == 0) D_80031A64[1][0x4F] = 1;
            if (D_80031A64[1][0x58] == 0 && D_80031A64[0][0x58] == 0) D_80031A64[1][0x58] = 1;
            if (D_80031A64[1][0x67] == 0 && D_80031A64[0][0x67] == 0) D_80031A64[1][0x67] = 1;
            if (D_80031A64[1][0x6C] == 0 && D_80031A64[0][0x6C] == 0) D_80031A64[1][0x6C] = 1;
        }
        if (D_801F3750.p.rank >= 21) {
            if (D_80031A64[1][0x93] == 0 && D_80031A64[0][0x93] == 0) D_80031A64[1][0x93] = 2;
            if (D_80031A64[2][0x0] == 0 && D_80031A64[1][0x0] == 0) D_80031A64[2][0x0] = 2;
            if (D_80031A64[2][0x3] == 0 && D_80031A64[1][0x3] == 0) D_80031A64[2][0x3] = 2;
        }
    }
    if (D_801F3750.p.b4 == 1) {
        for (i = 14; i != -1; i--) {
            if (D_80049E70.x[i] != 0) {
                D_80049E70.x[i] += D_801F3750.p.add[i];
            }
        }
    }
    D_801F3750.p.id = -1;
    D_80041B10.id = -1;
    D_801F3750.cursor = 0;
    D_801F3750.top = 0;
}
