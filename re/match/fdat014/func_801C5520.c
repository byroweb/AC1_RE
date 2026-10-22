#include "ac1/ac1.h"

extern s32 rand(void);
extern s32 func_80073714(AcEntity *, s32);
extern s32 func_80073904(void);
extern s32 func_80073924(u16);
extern void func_800759A4(AcEntity *, s32, void *);
extern void func_801C52E8(AcEntity *, s32);
extern u8 D_80039C78[];

s32 func_801C5520(AcEntity *e, s32 msg, ...)
{
    s32 dist;
    s32 r;

    dist = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 5:
        switch (e->state) {
        case 1:
            e->state = 2;
            break;
        case 2:
            e->state = 1;
            break;
        }
        return 0;
    }
    if (e->unk42[0] == 0) {
        if (dist > 6000) {
            func_801C52E8(e, 1);
        } else if (dist < 1000) {
            func_801C52E8(e, 2);
        } else if (dist < 5000 && func_80073924(*(u16 *)&e->unk42[2]) > 120) {
            func_800759A4(e, 4, D_80039C78);
            *(u16 *)&e->unk42[2] = func_80073904() + (rand() & 0x7F);
        } else {
            r = rand() & 0x1F;
            if (r >= 0) {
                if (r < 6) {
                    func_801C52E8(e, 1);
                } else if (r < 8) {
                    func_801C52E8(e, 2);
                }
            }
        }
    }
    return 0;
}
