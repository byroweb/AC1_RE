#include "ac1/ac1.h"

extern s32 rand(void);
extern s32 func_80073714(AcEntity *, s32);
extern void func_801C5D58(AcEntity *, s32);

s32 func_801C6008(AcEntity *e, s32 msg, ...)
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
            func_801C5D58(e, 1);
        } else if (dist < 1000) {
            func_801C5D58(e, 2);
        } else {
            r = rand() & 0x1F;
            if (r >= 0) {
                if (r < 6) {
                    func_801C5D58(e, 1);
                } else if (r < 8) {
                    func_801C5D58(e, 2);
                }
            }
        }
    }
    return 0;
}
