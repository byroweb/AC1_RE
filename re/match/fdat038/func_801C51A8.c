#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x41];
    u8 state;           /* +0x41 */
    u8 pad42[0x8A];
    u16 cc;             /* +0xCC */
    u16 ce;             /* +0xCE */
    u16 d0;             /* +0xD0 */
} Ent;
extern s32 func_80014DFC();
extern s32 func_80073714();
extern void func_800759A4();
extern u8 D_80039C78[];
extern u8 D_801C649C[];

s32 func_801C51A8(Ent *e, s32 msg, ...)
{
    s32 t;
    s32 v;

    t = func_80073714(e, 0x7FFFFFFF);
    if (msg == 5) {
        goto c5;
    }
    if (msg < 6) {
        if (msg == 0) {
            goto c0;
        }
    }
    return 0;
c5:
        e->d0 = 0;
        e->ce = 0;
        e->cc = 0;
        switch (e->state) {
        case 1:
            e->state = 6;
            break;
        case 2:
            e->state = 8;
            break;
        case 3:
            e->state = 7;
            break;
        case 4:
            e->state = 5;
            break;
        case 5:
            e->state = 4;
            break;
        case 6:
            e->state = 1;
            break;
        case 7:
            e->state = 3;
            break;
        case 8:
            e->state = 2;
            break;
        }
        return 0;
c0:
        if (t >= 0x1389) {
            func_800759A4(e, 1, D_80039C78);
        } else {
            v = func_80014DFC(10, D_801C649C, 1);
            if (v != 0) {
                func_800759A4(e, D_801C649C[v + 0xC], D_80039C78);
            }
        }
        return 0;
}
