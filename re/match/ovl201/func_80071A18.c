#include "ac1/ac1.h"

/* part tables: name, price (s32 +0x24), then u16 stats; stride differs per category */
#define PART(n, sz) typedef struct { char name[0x24]; s32 price; u16 weight; u16 s2a; u16 s2c; u16 s2e; u16 s30; u16 s32; u8 pad[sz - 0x34]; } n
PART(Part3C, 0x3C);
PART(Part38, 0x38);
PART(Part48, 0x48);
typedef struct { char name[0x24]; s32 price; u16 weight; u16 s2a; u16 s2c; u16 s2e; u16 s30; u16 s32; } Part34;
typedef struct { char name[0x24]; s32 price; u16 weight; u16 s2a; u16 s2c; u16 s2e; } Part30;
typedef struct { char name[0x24]; s32 price; u16 weight; u16 s2a; u16 s2c; u16 s2e; u8 pad[0x60 - 0x30]; } Part60;
typedef struct { char name[0x24]; s32 price; u16 weight; u16 s2a; } Part2C;
typedef struct { u16 v[12]; } CatList;

extern u8 D_80031A58[];
typedef struct {
    s32 a38, a3c, a40, a44, a48, a4c;
    u16 a50, a52, a54;
} AcStats;
extern AcStats D_8004AC38;
extern CatList D_8004BDC0;
extern Part3C D_800B91F4[];
extern Part38 D_800B944C[];
extern Part48 D_800B94F4[];
extern Part34 D_800B9DF4[];
extern Part2C D_800BA134[];
extern Part34 D_800BA3F4[];
extern Part34 D_800BAADC[];
extern Part30 D_800BABAC[];
extern Part60 D_800BACCC[];
extern Part30 D_800BAFCC[];
s32 func_80083C00(s32, s32);

void func_80071A18(void)
{
    CatList cat;
    s16 i;
    u8 flag;
    s32 small;
    u16 id;
    u16 c;
    u8 legs;
    u8 d7;
    s32 a0, a1, a2, a3, t0, t1;

    cat = D_8004BDC0;
    legs = D_80031A58[3];
    flag = 1;
    D_8004AC38.a38 = 0;
    D_8004AC38.a3c = 0;
    D_8004AC38.a40 = 0;
    D_8004AC38.a44 = 0;
    D_8004AC38.a48 = 0;
    D_8004AC38.a4c = 0;
    D_8004AC38.a50 = 0;
    D_8004AC38.a52 = 0;
    D_8004AC38.a54 = 0;
    if (D_80031A58[0] == 255) {
        D_8004AC38.a4c = 0;
        D_8004AC38.a54 = 0;
    } else {
        D_8004AC38.a4c = D_800B944C[D_80031A58[0]].s32;
        D_8004AC38.a54 = D_800B944C[D_80031A58[0]].s32;
    }
    if (D_80031A58[11] == 255) {
        D_8004AC38.a48 = 0;
        D_8004AC38.a52 = 0;
    } else {
        D_8004AC38.a52 = D_8004AC38.a48 = D_800B94F4[D_80031A58[11]].s32;
        flag = 1;
    }
    if (D_80031A58[8] == 255) {
        D_8004AC38.a38 = 0;
        D_8004AC38.a50 = 0;
    } else {
        D_8004AC38.a50 = D_8004AC38.a38 = D_800BAFCC[D_80031A58[8]].s2a;
    }
    i = 0;
    small = legs < 8;
    D_8004AC38.a40 = 0;
    D_8004AC38.a3c = 0;
    do {
        id = D_80031A58[i];
        c = cat.v[i];
        if (id != 255) {
            a3 = 0;
            t1 = 0;
            t0 = 0;
            a1 = 0;
            switch ((s16)c) {
            case 4:
                a1 = D_800B91F4[id].weight;
                t1 = D_800B91F4[id].s2c;
                a3 = D_800B91F4[id].s2a;
                a2 = D_800B91F4[id].price;
                a0 = a1;
                break;
            case 0:
                a1 = D_800B944C[id].weight;
                t1 = D_800B944C[id].s2c;
                a3 = D_800B944C[id].s2a;
                a2 = D_800B944C[id].price;
                a0 = a1;
                break;
            case 1:
                a0 = D_800B94F4[id].weight;
                t1 = D_800B94F4[id].s2c;
                a3 = D_800B94F4[id].s2a;
                a2 = D_800B94F4[id].price;
                break;
            case 2:
                t0 = D_800B9DF4[id].weight;
                t1 = D_800B9DF4[id].s2c;
                a3 = D_800B9DF4[id].s2a;
                a2 = D_800B9DF4[id].price;
                a0 = t0;
                a1 = t0;
                break;
            case 7:
                if (small)
                    t0 = D_800BA134[id].weight;
                else
                    t0 = 0;
                a1 = D_800BA134[id].weight;
                a3 = D_800BA134[id].s2a;
                a2 = D_800BA134[id].price;
                a0 = a1;
                break;
            case 6:
                a1 = D_800BA3F4[id].weight;
                a3 = D_800BA3F4[id].s2a;
                a2 = D_800BA3F4[id].price;
                a0 = a1;
                break;
            case 3:
                if (small)
                    t0 = D_800BAADC[id].weight;
                else
                    t0 = 0;
                a1 = D_800BAADC[id].weight;
                a3 = D_800BAADC[id].s2a;
                a2 = D_800BAADC[id].price;
                a0 = a1;
                break;
            case 5:
                a1 = D_800BABAC[id].weight;
                a3 = D_800BABAC[id].s2a;
                a2 = D_800BABAC[id].price;
                a0 = a1;
                break;
            case 8:
                a1 = D_800BACCC[id].weight;
                a3 = D_800BACCC[id].s2a;
                a2 = D_800BACCC[id].price;
                a0 = a1;
                break;
            case 10:
                a1 = D_800BAFCC[id].weight;
                a2 = D_800BAFCC[id].price;
                a0 = a1;
                break;
            default:
                goto none;
            }
        } else {
        none:
            a2 = 0;
            a3 = 0;
            t1 = 0;
            a0 = 0;
            t0 = 0;
            a1 = 0;
        }
        if (a0 != -1)
            D_8004AC38.a40 += a0;
        if (t1 != -1)
            D_8004AC38.a3c += t1;
        if (a2 != -1)
            D_8004AC38.a44 += a2;
        if (a3 != -1)
            D_8004AC38.a38 -= a3;
        if (flag && a1 != -1)
            D_8004AC38.a48 -= a1;
        if (t0 != -1)
            D_8004AC38.a4c -= t0;
        i++;
    } while (i < 12);
    id = D_80031A58[6];
    d7 = D_80031A58[7];
    if (id != 255 && id == d7 && (u8)func_80083C00(6, id) == 1) {
        D_8004AC38.a40 -= D_800BA3F4[id].weight;
        D_8004AC38.a44 -= D_800BA3F4[id].price;
        D_8004AC38.a38 += D_800BA3F4[id].s2a;
        D_8004AC38.a48 += D_800BA3F4[id].weight;
    }
}
