#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x41];
    u8 state;           /* +0x41 */
    u8 f42;
    u8 pad43[0x89];
    u16 cc;
    u16 ce;
    u16 d0;
} Ent;
extern u32 func_80014D78();
extern s32 func_80073714();
extern void func_801C5418();
extern s32 rand();

s32 func_801C547C(Ent *e, s32 msg, ...)
{
    s32 t;
    u8 v;
    u32 r;

    t = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 5:
        e->d0 = 0;
        e->ce = 0;
        e->cc = 0;
        switch (e->state) {
        case 1:
            e->state = 6;
            break;
        case 2:
            e->state = 5;
            break;
        case 3:
            e->state = 4;
            break;
        case 6:
            e->state = 1;
            break;
        case 4:
            e->state = 3;
            break;
        case 5:
            e->state = 2;
            break;
        }
        return 0;
    case 1:
    case 2:
        if (rand() < 0x2A3E) {
            if (rand() < 0x4000) {
                func_801C5418(e, 1);
            } else {
                func_801C5418(e, 6);
            }
        }
        return 0;
    default:
        if (e->f42 == 1) {
            if (t >= 0x1771) {
                if (rand() < 0x199A) {
                    func_801C5418(e, 1);
                    return 0;
                }
            } else {
                if (t < 0xBB8 && rand() < 0x199A) {
                    func_801C5418(e, 6);
                    return 0;
                }
            }
            r = func_80014D78(0x1E);
            switch (r) {
            case 0:
            case 1:
                func_801C5418(e, 1);
                break;
            case 2:
            case 3:
                func_801C5418(e, 2);
                break;
            case 4:
            case 5:
                func_801C5418(e, 3);
                break;
            case 6:
            case 7:
                func_801C5418(e, 6);
                break;
            case 8:
            case 9:
                func_801C5418(e, 4);
                break;
            case 10:
            case 11:
                func_801C5418(e, 5);
                break;
            }
        }
    }
    return 0;
}
