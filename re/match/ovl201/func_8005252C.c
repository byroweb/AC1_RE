#include "ac1/ac1.h"

extern u8 D_80031A94[2][0xA0];
extern char D_80031BD4[];
extern s32 D_80039CA4;
extern u16 D_80041B3C;
extern u16 D_80041B40;
extern u16 D_80041B44;
extern char D_8017E448[12];
extern char D_8017E454[7][4];
extern u8 D_8004AFC0[];
extern u8 D_8004AFC8[];
extern u8 D_8004AFD0[];
extern u8 D_8004AFE0[];
extern u8 D_8004AFEC[];
extern u8 D_8004AFF8[];
extern u8 D_8004B004[];
extern u8 D_8004B010[];
extern u8 D_8004B014[];

extern void func_8005BA2C(void);
extern void func_80052BD0(UiObj *o);
extern UiObj *func_8005CB94(void *s, s32 x, s32 y, s32 n);
extern void func_8005D80C(s32 a, void *s, s32 b, s32 c, s32 x, s32 y);
extern void func_800519A4(char *buf, s16 n);
extern void func_80051A74(char *buf, s32 n);
extern u8 func_800B203C(void);
extern s16 func_800B1ED0(void);
extern s32 func_80086750(void);

#define HAS(off) (D_80031A94[0][off] != 0 || D_80031A94[1][off] != 0)

void func_8005252C(UiObj *o)
{
    s32 credits;
    u32 rank;
    u16 a;
    u16 b;
    u16 c;
    s16 n;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        func_8005CB94(D_80031BD4 + 0x12, 8, 8, 6);
        func_8005CB94(D_80031BD4, 8, 0x18, 6);
        func_8005D80C(1, D_8004AFC0, 0x10, 8, 0x28, 6);
        func_8005D80C(1, D_8004AFC8, 0x10, 8, 0x38, 6);
        func_8005D80C(1, D_8004AFD0, 0x10, 8, 0x48, 6);
        func_8005CB94(D_8004AFE0, 8, 0x58, 6);
        func_8005CB94(D_8004AFEC, 8, 0x68, 6);
        func_8005CB94(D_8004AFF8, 8, 0x78, 6);
        func_8005CB94(D_8004B004, 8, 0x88, 6);
        credits = D_80039CA4;
        n = 0;
        rank = func_800B203C();
        a = D_80041B3C;
        b = D_80041B40;
        c = D_80041B44;
        if (HAS(0x92)) {
            n = 1;
        }
        if (HAS(0x02)) {
            n++;
        }
        if (HAS(0x59)) {
            n++;
        }
        if (HAS(0x5F)) {
            n++;
        }
        if (HAS(0x6D)) {
            n++;
        }
        if (HAS(0x88)) {
            n++;
        }
        if (HAS(0x8E)) {
            n++;
        }
        if (HAS(0x7A)) {
            n++;
        }
        if (HAS(0x48)) {
            n++;
        }
        if (HAS(0x7D)) {
            n++;
        }
        if (HAS(0x43)) {
            n++;
        }
        n += func_800B1ED0();
        func_80051A74(D_8017E448, credits);
        func_800519A4(D_8017E454[0], rank);
        func_800519A4(D_8017E454[1], a);
        func_800519A4(D_8017E454[2], b);
        if (a == 0) {
            func_800519A4(D_8017E454[3], 0);
        } else {
            func_800519A4(D_8017E454[3], b * 100 / a);
        }
        func_800519A4(D_8017E454[4], c);
        if (a == 0) {
            func_800519A4(D_8017E454[5], 0);
        } else {
            func_800519A4(D_8017E454[5], 100 - b * 100 / a);
        }
        func_800519A4(D_8017E454[6], (n + func_80086750()) * 100 / 63);
        func_8005D80C(1, D_8017E448, 0x10, 0x40, 0x28, 6);
        if (rank < 10) {
            func_8005D80C(1, D_8017E454[0], 0x10, 0x40, 0x38, 6);
        } else {
            func_8005D80C(1, D_8017E454[0], 0x10, 0x48, 0x38, 6);
        }
        func_8005D80C(1, D_8017E454[1], 0x10, 0x40, 0x58, 6);
        func_8005D80C(1, D_8017E454[2], 0x10, 0x40, 0x68, 6);
        func_8005D80C(1, D_8017E454[3], 0x10, 0x60, 0x68, 6);
        func_8005D80C(1, D_8017E454[4], 0x10, 0x40, 0x78, 6);
        func_8005D80C(1, D_8017E454[5], 0x10, 0x60, 0x78, 6);
        func_8005D80C(1, D_8017E454[6], 0x10, 0x68, 0x88, 6);
        func_8005D80C(1, D_8004B010, 0x10, 0x58, 0x68, 6);
        func_8005D80C(1, D_8004B010, 0x10, 0x58, 0x78, 6);
        func_8005D80C(1, D_8004B014, 0x10, 0x78, 0x68, 6);
        func_8005D80C(1, D_8004B014, 0x10, 0x78, 0x78, 6);
        func_8005D80C(1, D_8004B014, 0x10, 0x80, 0x88, 6);
        o->update = func_80052BD0;
    }
}
