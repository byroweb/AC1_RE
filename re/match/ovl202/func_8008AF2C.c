#include "ac1/ac1.h"
/* ovl202 0x8008AF2C: mission result (payout) screen: reward, special bonus, ammo and
 * repair costs, credit total clamped to -999999..99999999, then the fade-in / line-by-line
 * reveal / wait-for-button / fade-out loop. */
typedef struct {
    s32 reward;         /* +0x00 */
    s32 bonus;          /* +0x04 */
    s32 ammo;           /* +0x08 */
    s32 repair;         /* +0x0C */
    s32 other;          /* +0x10 */
    s32 total;          /* +0x14 */
    s32 credits;        /* +0x18 */
    s32 show[7];        /* +0x1C */
} Payout;
typedef struct {
    u8 pad[0x0E];
    u16 max;            /* +0x0E */
    u16 left;           /* +0x10 */
    u16 price;          /* +0x12 */
    u8 pad14[0x24 - 0x14];
    u8 flags;           /* +0x24 */
    u8 pad25[0x3C - 0x25];
} AmmoSlot;
typedef struct {
    u8 pad[0xC];
    void (*report)(s32 *a, s32 *b);  /* +0x0C */
} Objective;
extern u8 D_8019F518;
extern Objective *D_8019F51C;
extern s32 D_80039CA4;
extern s32 D_8004860C;
extern u32 D_80048608;
extern u16 D_80041212;
extern u8 D_80041278;
extern AmmoSlot D_8004127A[];
extern u8 D_800B17C8[];
extern void func_800165E4(s32 a, s32 b, void *c);
extern void func_80011FD8(void *a);
extern void func_8005715C(void);
extern void func_800571F0(void);
extern void func_8008ABD4(s32 a);
extern void func_8008ABF4(void);
extern void func_8008AE44(Payout *p);
extern s32 func_80016A84(s32 a);

void func_8008AF2C(void)
{
    Payout r;
    AmmoSlot *s;
    s32 i;
    s32 k;
    s32 j;
    s32 prev, old;

    if (D_8019F518 == 70)
        return;
    if (D_8019F518 == 0) {
        D_80039CA4 = D_8004860C;
        return;
    }
    r.bonus = 0;
    r.other = 0;
    D_8019F51C->report(&r.bonus, &r.other);
    if (g_mission_flags & 0x80) {
        r.reward = D_8004860C;
    } else {
        r.bonus = 0;
        r.reward = 0;
    }
    if (D_8019F518 == 48 || D_8019F518 == 49)
        return;
    if ((D_8019F518 == 47 || D_8019F518 == 21 || D_8019F518 == 22) && (g_mission_flags & 0x200))
        return;
    r.ammo = 0;
    j = D_80041278;
    s = D_8004127A;
    while (--j != -1) {
        if (s->flags & 0x10) {
            r.ammo -= (s->max - s->left) * s->price;
        }
        s++;
    }
    r.repair = -((u32)((D_80041212 - g_ac_entities[0].ap) * (D_80048608 / 100)) / 8000);
    r.total = r.reward + r.bonus + r.ammo + r.repair + r.other;
    D_80039CA4 += r.total;
    if (D_80039CA4 > 99999999) {
        D_80039CA4 = 99999999;
    } else if (D_80039CA4 < -999999) {
        D_80039CA4 = -999999;
    }
    r.credits = D_80039CA4;
    r.show[6] = 0;
    r.show[5] = 0;
    r.show[4] = 0;
    r.show[3] = 0;
    r.show[2] = 0;
    r.show[1] = 0;
    r.show[0] = 0;
    func_800165E4(1, 103, D_800B17C8);
    func_80011FD8(D_800B17C8);
    i = 0;
    do {
        func_8005715C();
        func_8008ABD4(i);
        i += 12;
        func_8008ABF4();
        func_800571F0();
    } while (i < 128);
    func_8008ABD4(128);
    prev = func_80016A84(1);
    k = 0;
    while (1) {
        func_8005715C();
        func_8008AE44(&r);
        func_8008ABF4();
        func_800571F0();
        if (k == 0) r.show[0] = 1;
        else if (k == 5) r.show[1] = 1;
        else if (k == 10) r.show[2] = 1;
        else if (k == 15) r.show[3] = 1;
        else if (k == 20) r.show[4] = 1;
        else if (k == 30) r.show[5] = 1;
        else if (k == 45) r.show[6] = 1;
        old = prev;
        prev = func_80016A84(1);
        if (k >= 46 && old == 0 && (prev & 0x9F0))
            break;
        k++;
    }
    i = 128;
    do {
        func_8005715C();
        func_8008ABD4(i);
        func_8008AE44(&r);
        i -= 12;
        func_8008ABF4();
        func_800571F0();
    } while (i >= 0);
    func_8008ABD4(0);
    func_8005715C();
    func_8008ABF4();
    func_800571F0();
    func_8005715C();
    func_8008ABF4();
    func_800571F0();
}
