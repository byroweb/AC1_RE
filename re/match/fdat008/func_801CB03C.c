/* fdat008: AI movement state machine (fn_move), smaller sibling of
 * fdat000 0x801C981C.  Template: re/match/fdat000/func_801C981C.c. */
#include "ac1/ac1.h"
typedef struct {
    u8 hold;            /* +0x00 */
    u8 fx1[8];          /* +0x01 */
    u8 fx9[4];          /* +0x09 */
    u8 fxd[9];          /* +0x0D */
    u16 sink;           /* +0x16 */
    u8 pad18[0x22 - 0x18];
    u16 speed;          /* +0x22 */
} Func801CB03CDef;
typedef struct {
    u8 pad0[8];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x28 - 0x14];
    u16 h28;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    s16 top;            /* +0x2C */
    s16 floor;          /* +0x2E */
    u8 pad30[0x36 - 0x30];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 move;            /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[0x41 - 0x3E];
    u8 state;           /* +0x41 */
    u8 pad42[0x64 - 0x42];
    u8 mode;            /* +0x64 AcMover.mode */
    u8 pad65[0x72 - 0x65];
    u16 h72;            /* +0x72 */
    u8 pad74[0xCC - 0x74];
    AcPos vel;          /* +0xCC */
    u8 padd4[0x108 - 0xD4];
    Func801CB03CDef *def; /* +0x108 */
    u8 pad10c[0x148 - 0x10C];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
} Func801CB03CObj;
extern s32 D_801A5DC8;
extern AcPos D_80039C78;
extern s32 func_8004EAD0(u8 *mover);
extern s32 func_8004EBF8(u8 *mover);
extern void func_8004DE88(u8 *mover, AcPos *vel);
extern void func_8004EA3C(u8 *mover, s32 a, u8 *b);
extern void func_8004CE44(s32 a, AcPos *pos, s32 b, s32 c);
extern s32 func_80074FA4(Func801CB03CObj *o, AcPos *at);
extern void func_8007A02C(Func801CB03CObj *o);
extern void func_801C7034(Func801CB03CObj *o, Func801CB03CDef *d);
extern void func_801C7198(Func801CB03CObj *o, s32 anim);
extern void func_801C7234(Func801CB03CObj *o, s32 anim, s32 a, s32 n, AcPos *vel);
extern s32 func_801CAFB8(Func801CB03CObj *o);
extern void func_800766E0(Func801CB03CObj *o, s32 anim, s32 a, s32 b, s32 c, u8 *fx, void *fn);
extern void func_80076578(Func801CB03CObj *o, s32 anim, s32 a, void *fn);
extern void func_800765E8(Func801CB03CObj *o, s32 anim, s32 a, void *fn);
extern void func_80076A84(Func801CB03CObj *o, s32 anim, s32 a, s32 b, u8 *fx, void *fn);
extern void func_800775B8(Func801CB03CObj *o, s32 anim, s32 a, s32 n, AcPos *vel, void *fn);
extern void func_8007790C(Func801CB03CObj *o, s32 anim, s32 a, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_800778B8(Func801CB03CObj *o, s32 anim, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_8007847C(Func801CB03CObj *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_80076014();
extern void func_80076078();
extern void func_800760DC();
extern void func_800764C4();
extern void func_801C76B8();
extern void func_801C7F98();
extern void func_801C814C();
void func_801CB03C(Func801CB03CObj *o, s32 event)
{
    Func801CB03CDef *d = o->def;
    AcPos at;
    s32 r;
    s32 *flag;

    if (o->faction & 0x10)
        event = o->move;
    if (o->mode == 22) {
        if (func_8004EAD0(&o->mode)) {
            func_8004DE88(&o->mode, &o->vel);
            o->vel.x <<= 6;
            o->vel.y <<= 6;
            o->vel.z <<= 6;
            goto hover;
        }
    } else if (o->kind != 192 && event != 96 && event != 97) {
        if (o->state == 64)
            goto stop;
        if (o->top - o->pos.y >= 50) {
            at.x = o->pos.x;
            at.y = o->pos.y + 50;
            at.z = o->pos.z;
            flag = &D_801A5DC8;
            *flag = 1;
            r = func_80074FA4(o, &at);
            *flag = 0;
            if (r == 0) {
                event = 75;
                o->vel.y = 256;
                o->vel.z = 0;
                o->vel.x = 0;
                o->move = 75;
                o->state = 75;
                o->faction &= ~0x10;
            stop:
                o->fire_flags = 0;
                o->flags = 0;
                switch (o->mode) {
                case 0:
                    break;
                case 2:
                    o->vel.y = 0;
                    func_801C7198(o, 26);
                    return;
                case 4:
                    o->vel.y = 0;
                    func_801C7198(o, 32);
                    return;
                case 6:
                    o->vel.y = 0;
                    func_801C7198(o, 33);
                    return;
                case 15:
                    o->vel.y = 0;
                    func_801C7198(o, 27);
                    return;
                case 17:
                    o->vel.y = 0;
                    func_801C7198(o, 34);
                    return;
                case 19:
                    o->vel.y = 0;
                    func_801C7198(o, 35);
                    return;
                case 9:
                    o->vel.y = 0;
                    func_801C7198(o, 28);
                    return;
                case 12:
                    o->vel.y = 0;
                    func_801C7198(o, 29);
                    return;
                case 21:
                    o->vel = D_80039C78;
                    if (o->kind == 2) {
                        func_800775B8(o, 30, 1, 512, &o->vel, func_801C76B8);
                        r = 73;
                    } else {
                        func_800775B8(o, 31, 1, 512, &o->vel, func_801C76B8);
                        r = 74;
                    }
                    o->move = r;
                    o->state = r;
                    return;
                }
            }
        }
    }
    switch (event) {
    case 1:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 2, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 2, 0, 256, &o->vel);
            break;
        }
        break;
    case 2:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 4, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 4, 0, 256, &o->vel);
            break;
        }
        break;
    case 3:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 6, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 6, 0, 256, &o->vel);
            break;
        }
        break;
    case 6:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 15, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 15, 0, 256, &o->vel);
            break;
        }
        break;
    case 7:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 17, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 17, 0, 256, &o->vel);
            break;
        }
        break;
    case 8:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 19, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 19, 0, 256, &o->vel);
            break;
        }
        break;
    case 4:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 9, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 9, 0, 256, &o->vel);
            break;
        }
        break;
    case 5:
        switch (o->mode) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 12, 0, 256, &o->vel);
            break;
        default:
            if (func_801CAFB8(o))
                func_801C7234(o, 12, 0, 256, &o->vel);
            break;
        }
        break;
    case 0:
        switch (o->mode) {
        default:
            if (!func_801CAFB8(o))
                break;
        case 0:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_801C7234(o, 0, 0, 256, &o->vel);
            break;
        }
        break;
    case 64:
        if (o->kind != 192) {
            if (d->hold == 1) {
                o->vel.y = 0;
                goto hover;
            }
            if (func_801CAFB8(o)) {
                if (o->mode == 56) {
                    func_80076A84(o, 22, 1, 255, d->fxd, func_800760DC);
                    o->h72 = 1024;
                } else {
                    func_80076A84(o, 22, 1, 255, d->fxd, func_800760DC);
                }
            }
        } else {
        hover:
            func_800775B8(o, 25, 1, d->speed, &o->vel, func_801C76B8);
        }
        break;
    case 65:
        func_800775B8(o, 26, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 66:
        func_800775B8(o, 32, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 67:
        func_800775B8(o, 33, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 68:
        func_800775B8(o, 27, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 69:
        func_800775B8(o, 34, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 70:
        func_800775B8(o, 35, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 71:
        func_800775B8(o, 28, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 72:
        func_800775B8(o, 29, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 73:
        func_800775B8(o, 30, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 74:
        func_800775B8(o, 31, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 75:
        func_800775B8(o, 24, 1, d->speed, &o->vel, func_801C76B8);
        break;
    case 96:
        func_8007A02C(o);
        if (o->mode != 23) {
            o->flags = 0;
            o->fire_flags = 0;
            func_8004CE44(14, &o->pos, 60, 0);
        }
        switch (o->mode) {
        case 24:
        case 25:
            func_801C7234(o, 0, 0, 256, &o->vel);
            event = 0;
            o->faction &= ~0x10;
            break;
        case 26:
            func_801C7234(o, 2, 0, 256, &o->vel);
            event = 1;
            o->faction &= ~0x10;
            break;
        case 32:
            func_801C7234(o, 4, 0, 256, &o->vel);
            event = 2;
            o->faction &= ~0x10;
            break;
        case 33:
            func_801C7234(o, 6, 0, 256, &o->vel);
            event = 3;
            o->faction &= ~0x10;
            break;
        case 27:
            func_801C7234(o, 15, 0, 256, &o->vel);
            event = 6;
            o->faction &= ~0x10;
            break;
        case 34:
            func_801C7234(o, 17, 0, 256, &o->vel);
            event = 7;
            o->faction &= ~0x10;
            break;
        case 35:
            func_801C7234(o, 19, 0, 256, &o->vel);
            event = 8;
            o->faction &= ~0x10;
            break;
        case 28:
            func_801C7234(o, 9, 0, 256, &o->vel);
            event = 4;
            o->faction &= ~0x10;
            break;
        case 29:
            func_801C7234(o, 12, 0, 256, &o->vel);
            event = 5;
            o->faction &= ~0x10;
            break;
        default:
            if (o->mode == 23) {
                if (func_8004EBF8(&o->mode)) {
                    o->faction &= ~0x10;
                    func_801C7234(o, 0, 0, 256, &o->vel);
                } else {
                    func_800765E8(o, 23, 1, func_80076078);
                }
            } else {
                func_800765E8(o, 23, 1, func_80076078);
                o->vel = D_80039C78;
            }
            o->h72 = 512;
            break;
        }
        break;
    case 97:
        func_801C7034(o, d);
        if (o->mode == 36) {
            if (func_8004EBF8(&o->mode)) {
                o->faction &= ~0x10;
                func_80076578(o, 0, 1, func_80076014);
            } else {
                func_800765E8(o, 36, 1, func_80076078);
                o->h72 = 512;
            }
        } else {
            o->flags = 0;
            o->fire_flags = 0;
            func_800765E8(o, 36, 1, func_80076078);
            o->h72 = 512;
            func_8004EA3C(&o->mode, 1, d->fx9);
            o->vel = D_80039C78;
            if (o->floor <= o->pos.y && o->floor >= o->pos.y - o->h2a)
                func_8007847C(o, 100, 5, 300, 700, 100, 300, 64);
        }
        break;
    case 32:
        if (o->mode == 56 && func_8004EBF8(&o->mode))
            o->faction &= ~0x10;
        func_800766E0(o, 56, 1, 0, 0, 0, func_801C7F98);
        o->h72 = 1024;
        break;
    case 33:
        if (o->mode == 47 && func_8004EBF8(&o->mode))
            o->faction &= ~0x10;
        func_800775B8(o, 47, 1, 1024, &o->vel, func_801C814C);
        break;
    case 128:
        func_8007790C(o, o->mode, 1, 1024, &o->vel, -1, func_800764C4);
        break;
    case 129:
        func_800778B8(o, o->mode, 1024, &o->vel, -1, func_801C76B8);
        break;
    }
    if (!(o->faction & 0x10))
        o->move = event;
}
