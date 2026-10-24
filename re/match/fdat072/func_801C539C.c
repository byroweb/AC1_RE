#include "ac1/ac1.h"

extern s32 func_80014DFC();
extern s32 func_80073714();
extern void func_800759A4();
extern void func_80075B08();
extern s32 func_801C4F3C();
extern u8 D_80039C78[];
extern u8 D_801C6AC0[];
extern u8 D_801C6AD0[];

s32 func_801C539C(AcEntity *e, s32 msg, ...)
{
    s16 p[3];
    s32 v;
    u8 w;
    u8 t;
    s32 *ap;

    ap = &msg;
    func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 5:
        t = e->state;
        if (t == 1) {
            w = 6;
            goto st;
        }
        if (t == 6) {
            w = 1;
        st:
            e->state = w;
        }
        break;
    case 8:
        p[0] = (((s16 *)ap[1])[0] + ((s16 *)ap[1])[4]) >> 1;
        p[2] = (((s16 *)ap[1])[2] + ((s16 *)ap[1])[6]) >> 1;
        p[1] = ((s16 *)ap[1])[5] - 0x3E8;
        func_80075B08(e, 1, p);
        break;
    case 0:
        if (func_801C4F3C(e, 0x2EE0, 0x640) != 0) {
            func_800759A4(e, D_801C6AC0[func_80014DFC(8, D_801C6AC0, 1) + 8], D_80039C78);
        } else {
            v = func_80014DFC(0xA, D_801C6AD0, 1);
            if (v != 0) {
                func_800759A4(e, D_801C6AD0[v + 0xC], D_80039C78);
            }
        }
        break;
    }
    return 0;
}
