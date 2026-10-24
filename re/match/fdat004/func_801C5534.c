/* fdat004 mission_turret10_event_handler: varargs message handler (e, msg, ...)
 * in the style of fdat012 0x801C5FB4.  msg 4 resets, 1/2 start, everything else
 * runs the distance-band state machine on +0x42 and a random attack pick.
 * Retail quirks kept: the ally count re-tests slot 1 fifteen times, and
 * state 3 dispatches on an uninitialised pick. */
#include "ac1/ac1.h"

extern s32 rand(void);
extern s32 func_80014D78(s32 n);
extern s32 func_80073714(AcEntity *, s32);
extern s16 func_80073904(void);
extern s32 func_80073924(s32 t);
extern void func_801C53F4(AcEntity *, s32);
extern void func_801C5464(AcEntity *, s32);
extern void func_801C54CC(AcEntity *, s32);
extern s32 func_801C5534(AcEntity *e, s32 msg, ...);

s32 func_801C5534(AcEntity *e, s32 msg, ...)
{
    s32 dist;
    u32 v;
    AcEntity *p;
    s32 n;
    s32 i;

    dist = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 4:
        if (e->ai_mode != 0xFF) {
            e->ai_mode = 0;
        }
        e->unk42[0] = 0;
        e->faction &= ~0x10;
        *(u16 *)&e->unk42[2] = func_80073904();
        goto stop;
    case 1:
    case 2:
        if (e->unk42[0] != 2) {
            e->unk42[0] = 1;
            *(u16 *)&e->unk42[2] = func_80073904();
        }
        v = rand() & 0xF;
        goto dispatch;
    default:
        *(s16 *)((u8 *)e + 0x7C) = -1;
        if (dist < 7000) {
            *(s16 *)((u8 *)e + 0x7C) = 0;
        }
        switch (e->unk42[0]) {
        case 0:
            if (dist < 8000 && rand() < 26214) {
            start:
                e->unk42[0] = 1;
                *(u16 *)&e->unk42[2] = func_80073904();
                v = rand() & 3;
                goto dispatch;
            }
            if (dist < 32000) {
                dist >>= 5;
                if (func_80014D78(dist * dist * dist >> 14) < 476) {
                    p = &g_ac_entities[1];
                    for (i = 15, n = 0; i != 0; i--) {
                        if (p->def != 0 && p->fn_move == (void *)func_801C5534 && p->unk42[0] != 0) {
                            n++;
                        }
                    }
                    if (n < 2) {
                        goto start;
                    }
                }
            }
        stop:
            e->state = 0;
            return 0;
        case 1:
        case 2:
            if (dist > 8000) {
                e->unk42[0] = 3;
                *(u16 *)&e->unk42[2] = func_80073904();
                goto near;
            }
            if (e->unk42[0] == 2) {
                if (func_80073924(*(u16 *)&e->unk42[2]) > 300) {
                    e->unk42[0] = 1;
                    *(u16 *)&e->unk42[2] = func_80073904();
                }
            } else if (*(s16 *)((u8 *)e + 0x2E) != 0x7FFF && rand() < 3277) {
                e->unk42[0] = 2;
                *(u16 *)&e->unk42[2] = func_80073904();
            }
            v = rand() & 0x3F;
        dispatch:
            switch (v) {
            case 0:
            case 1:
            case 2:
                func_801C53F4(e, 2);
                break;
            case 3:
                func_801C53F4(e, 3);
                break;
            case 4:
            case 5:
                func_801C5464(e, 2);
                break;
            case 6:
            case 7:
                func_801C54CC(e, 2);
                break;
            case 8:
                func_801C5464(e, 3);
                break;
            case 11:
                func_801C54CC(e, 3);
                break;
            case 12:
                func_801C53F4(e, 4);
                break;
            case 13:
                func_801C53F4(e, 5);
                break;
            case 14:
                func_801C53F4(e, 1);
                break;
            }
            break;
        case 3:
            if (dist < 4000) {
                e->unk42[0] = 1;
                *(u16 *)&e->unk42[2] = func_80073904();
                goto dispatch;
            }
        near:
            func_801C53F4(e, 2);
            break;
        }
        break;
    case 5:
        break;
    }
    return 0;
}
