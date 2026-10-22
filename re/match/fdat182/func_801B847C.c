#include "ac1/ac1.h"

typedef struct {
    u8 kind;            /* +0x00 */
    u8 idx;             /* +0x01 */
    u8 pad2[0x2A];
} Slot;
typedef struct {
    u16 w0;             /* +0x00 */
    u8 idx;             /* +0x02 */
    u8 f3;
    u8 f4;
    u8 pad5;
    u16 f6;
    u8 f8;
    u8 f9;
} Def;
typedef struct {
    u8 pad0[0x2C];
    s32 timer;          /* +0x2C */
    u8 pad30[8];
    Def *def;           /* +0x38 */
} Obj;
extern Slot D_8019B4F8[];
extern s32 func_8006C8FC();
extern void func_80088D2C();

void func_801B847C(Obj *o, s32 a1, s32 a2, s32 a3)
{
    Def *d;
    Slot *s;
    s32 kind;
    u8 *a2p;
    u8 *e;

    d = o->def;
    a2p = (u8 *)D_8019B4F8;
    s = &D_8019B4F8[d->idx];
    kind = s->kind;
    if (kind == 3) {
    body:
        o->timer = 0;
        if (func_8006C8FC(o, kind, a2p, a3) != 0) {
            func_80088D2C(d->f3, d->f4, d->f6, d->f8, d->f9);
        }
    } else if (kind == 1) {
        a3 = d->w0;
        a2p = (u8 *)D_8019B4F8 + 0x58 + s->idx * 0x170;
        if ((a3 != 1 || a2p[0x40] == 0xFF) && *(u16 *)(a2p + 0x160) >= a3 - 1) {
            return;
        }
        goto body;
    }
}
