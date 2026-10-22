#include "ac1/ac1.h"
/* ovl202 0x8007EAE8: player AC init: derive the movement/energy parameters (0x800411A4..BE)
 * from the AC stat bytes (0x800411F7..), copy weapon slots, set up the player entity and
 * start its mode (0/3 -> 0x80080D20, 1/2 -> 0x80082BC4). */
typedef struct {
    u8 pad[0x0E];
    u16 a;              /* +0x0E */
    u16 b;              /* +0x10 */
    u8 pad12[0x3C - 0x12];
} Slot3C;
typedef struct {
    u8 pad[20];
} Rec20;
extern s8 D_80040FB8;
extern s8 D_80040FB9;
extern u16 D_80040FBE;
extern s16 D_80040FC0;
extern Rec20 *D_80040FEC;
extern s32 D_800411A4;
extern s16 D_800411A8;
extern s16 D_800411AA;
extern s16 D_800411AC;
extern s16 D_800411AE;
extern s16 D_800411B0;
extern s16 D_800411B2;
extern s16 D_800411B4;
extern s16 D_800411B6;
extern s16 D_800411BE;
extern u8 D_800411F7;
extern u8 D_800411F9;
extern u8 D_800411FA;
extern u8 D_800411FB;
extern u8 D_800411FC;
extern u8 D_800411FD;
extern u8 D_800411FE;
extern u8 D_800411FF;
extern u8 D_80041200;
extern u8 D_80041201;
extern u16 D_80041204;
extern u16 D_80041206;
extern u16 D_80041208;
extern u16 D_8004120C;
extern u16 D_80041210;
extern u16 D_80041212[];
extern u8 D_80041216[];
extern u8 D_8004121A[];
extern u8 D_8004121B;
extern volatile u16 D_80041226;
extern u8 D_80041278;
extern Slot3C D_8004127A[];
extern Rec20 D_8008E684[];
extern void func_8007DF08(void);
extern void func_80079728(void);
extern void func_8007BA2C(void);
extern void func_80080D20(AcEntity *o);
extern void func_80082BC4(AcEntity *o);

void func_8007EAE8(AcEntity *o)
{
    int i;
    Slot3C *s;

    D_80040FB9 = 0;
    D_80040FB8 = 0;
    func_8007DF08();
    func_80079728();
    D_80040FBE = D_80041204;
    D_80040FC0 = 0;
    D_800411A8 = D_800411F9 * 2;
    D_800411AA = D_800411FA * 200;
    D_800411AC = D_800411FB * 14;
    D_800411AE = D_800411FC * 140;
    D_800411B0 = D_800411FD * 14;
    D_800411B2 = D_800411FE * 140;
    D_800411B4 = (D_800411FF * 5U) >> 4;
    D_800411B6 = (D_80041200 * 15) >> 4;
    D_800411BE = D_80041201 * 4;
    D_80040FEC = &D_8008E684[D_8004121B];
    D_800411A4 = D_80041210 * D_80041210;
    if (D_800411F7 >= 6) {
        D_80041206 >>= 1;
        D_80041208 <<= 1;
        D_8004120C <<= 1;
        if (D_80041208 > D_80041206) D_80041208 = D_80041206;
        if (D_8004120C > D_80041206) D_8004120C = D_80041206;
    }
    i = D_80041278;
    s = D_8004127A;
    while (--i != -1) {
        s->b = s->a;
        s++;
    }
    D_80040FB8 = 0;
    func_8007BA2C();
    o->unk3a[0] = 7;
    o->faction = 1;
    o->ap = o->ap_max = D_80041212[0];
    *(u16 *)&o->unk164[0] = (D_80041216[0] * 25U) >> 2;
    *(u16 *)((u8 *)o + 0x38) = D_80041226 & ~3;
    D_80041226 &= ~8;
    ((u8 *)&o->mover)[4] = 2;
    *(s32 *)((u8 *)o + 0x110) = 0;
    switch (D_8004121A[0]) {
    case 0:
    case 3:
        if (D_800411F7 >= 4) {
            D_8004121A[0] = 3;
        }
        func_80080D20(o);
        break;
    case 1:
    case 2:
        func_80082BC4(o);
        break;
    }
}
