#include "ac1/ac1.h"
/* ovl201 0x800514AC - start async loads of sound files id..id+2 (archive 6) into the
 * idle bank; func_800515F8 / 51608 / 51618 clear the three busy flags */
typedef struct SndCtl {
    s16 vab[2];                 /* +0x00 */
    s16 seq[2];                 /* +0x04 */
    u8 *a[2];                   /* +0x08 */
    u8 *b[2];                   /* +0x10 */
    u8 *c[2];                   /* +0x18 */
    u8 cur;                     /* +0x20 */
} SndCtl;
extern SndCtl D_801BC450;
extern u8 D_801BC471;
extern u8 D_801BC472;
extern u8 D_801BC473;
s32 func_80015C9C(s32 a, u16 b, u8 *loc);
u8 *func_80050FC4(s32 size);
void func_80015B78(s32 a, u8 *loc, s32 size, u8 *buf, void (*cb)());
void func_800515F8();
void func_80051608();
void func_80051618();

void func_800514AC(s32 id)
{
    u8 loc[8];
    s32 n;
    u8 *p;
    s32 k;

    D_801BC473 = 1;
    D_801BC472 = 1;
    D_801BC471 = 1;
    k = D_801BC450.cur == 0;
    n = func_80015C9C(6, id, loc);
    p = func_80050FC4(n);
    D_801BC450.a[k] = p;
    func_80015B78(16, loc, n, p, func_800515F8);
    n = func_80015C9C(6, id + 1, loc);
    p = func_80050FC4(n);
    D_801BC450.b[k] = p;
    func_80015B78(16, loc, n, p, func_80051608);
    n = func_80015C9C(6, id + 2, loc);
    p = func_80050FC4(n);
    D_801BC450.c[k] = p;
    func_80015B78(16, loc, n, p, func_80051618);
}
