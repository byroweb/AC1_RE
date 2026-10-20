/* fdat000 (mission library, entry 0 @ 0x801C4B40; 29 twins, group g0042): the AI
 * movement state machine, fn_move (+0x54) of the mission-library units.  `event` is
 * the requested move; while faction bit 0x10 is set the unit stays on its saved move
 * (+0x3C) and only stores a new one when the bit is clear again (tail).
 * Mode 22 (falling) lands into the hover routine 801C5CB8 once the mover reports
 * ground (func_8004EAD0).  A unit with def+0x16 set that is not already in the air
 * (kind 192) probes 50 units above itself (func_80074FA4) and, when blocked, stops
 * into the matching hover start (801C5798) for its current walk mode.
 * Events 1..8 pick a walk/turn animation from the current mode; 0 idles; 32..35
 * jump, boost and fly; 48..56 strafe (56 by the velocity direction); 64..75 hover;
 * 96/97 land; 128/129 fall.  Each switch case makes its own call: cross-jumping
 * folds them into the copies retail keeps.  Matching levers: the probe flag goes
 * through a local pointer (retail keeps &D_801A5DC8 in s0 across the call), and `r`
 * is reused for the probe's z and the direction angle, which makes it a global
 * pseudo so the angle lands in v1 as in retail.  Retail's 12 jump tables are
 * 8-aligned inside the object, hence --no-rodata-align-fix in the manifest row. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0;
    u8 fx1[8];          /* +0x01 */
    u8 fx9[4];          /* +0x09 */
    u8 fxd[9];          /* +0x0D */
    u16 sink;           /* +0x16 */
    u8 pad18[0x22 - 0x18];
    u16 speed;          /* +0x22 */
    u8 pad24[0x38 - 0x24];
    u16 mask;           /* +0x38 */
} Func801C981CDef;
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
    u8 pad65[0x6A - 0x65];
    u16 h6a;            /* +0x6A */
    u16 h6c;            /* +0x6C */
    u8 pad6e[2];
    s16 h70;            /* +0x70 */
    u16 h72;            /* +0x72 */
    u8 pad74[0xCC - 0x74];
    AcPos vel;          /* +0xCC */
    u8 padd4[0x108 - 0xD4];
    Func801C981CDef *def; /* +0x108 */
    u8 pad10c[0x135 - 0x10C];
    u8 b135;            /* +0x135 */
    u8 pad136[0x148 - 0x136];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
} Func801C981CObj;
int abs(int);
extern s32 D_801A5DC8;
extern AcPos D_80039C78;
extern s32 func_8004EAD0(u8 *mover);
extern s32 func_8004EBF8(u8 *mover);
extern void func_8004DE88(u8 *mover, AcPos *vel);
extern void func_8004EA3C(u8 *mover, s32 a, u8 *b);
extern void func_8004CE44(s32 a, AcPos *pos, s32 b, s32 c);
extern s32 func_80074FA4(Func801C981CObj *o, AcPos *at);
extern void func_801C5798(Func801C981CObj *o, s32 anim);
extern void func_801C5634(Func801C981CObj *o, Func801C981CDef *d);
extern s32 func_801C96F4(Func801C981CObj *o);
extern void func_800766E0(Func801C981CObj *o, s32 anim, s32 a, s32 b, s32 c, u8 *fx, void *fn);
extern void func_80076578(Func801C981CObj *o, s32 anim, s32 a, void *fn);
extern void func_800765E8(Func801C981CObj *o, s32 anim, s32 a, void *fn);
extern void func_80076A84(Func801C981CObj *o, s32 anim, s32 a, s32 b, u8 *fx, void *fn);
extern void func_800775B8(Func801C981CObj *o, s32 anim, s32 a, s32 n, AcPos *vel, void *fn);
extern void func_8007732C(Func801C981CObj *o, s32 anim, s32 a, s32 n, AcPos *vel, void *fn);
extern void func_80077170(Func801C981CObj *o, s32 anim, s32 a, AcPos *vel, void *fn);
extern void func_8007790C(Func801C981CObj *o, s32 anim, s32 a, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_800778B8(Func801C981CObj *o, s32 anim, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_8007847C(Func801C981CObj *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_80076014();
extern void func_80076078();
extern void func_800760DC();
extern void func_800764C4();
extern void func_801C95B8();
extern void func_801C5CB8();
extern void func_801C5FA4();
extern void func_801C6598();
extern void func_801C674C();
extern void func_801C6A28();
void func_801C981C(Func801C981CObj *o, s32 event)
{
    Func801C981CDef *d = o->def;
    s32 ev = event;
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
    } else if (d->sink != 0 && o->kind != 192 && event != 96 && event != 97) {
        if (o->state == 64 && o->kind == 176)
            goto stop;
        if (o->top - o->pos.y >= 50) {
            at.x = o->pos.x;
            at.y = o->pos.y + 50;
            r = (u16)o->pos.z;
            at.z = r;
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
            stop:
                o->fire_flags = 0;
                o->flags = 0;
                switch (o->mode) {
                case 1:
                case 2:
                case 48:
                    func_801C5798(o, 26);
                    return;
                case 3:
                case 4:
                case 52:
                    func_801C5798(o, 32);
                    return;
                case 5:
                case 6:
                case 53:
                    func_801C5798(o, 33);
                    return;
                case 14:
                case 15:
                case 49:
                    func_801C5798(o, 27);
                    return;
                case 16:
                case 17:
                case 54:
                    func_801C5798(o, 34);
                    return;
                case 18:
                case 19:
                case 55:
                    func_801C5798(o, 35);
                    return;
                case 8:
                case 9:
                case 50:
                    func_801C5798(o, 28);
                    return;
                case 11:
                case 12:
                case 51:
                    func_801C5798(o, 29);
                    return;
                case 45:
                case 46:
                    o->faction &= ~0x10;
                    func_801C5798(o, 24);
                    return;
                }
            }
        }
    }
    switch (event) {
    case 1:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 2, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 1:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 2, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 1, 1, 0, 0, 0, func_800760DC);
            break;
        case 3:
        case 5:
            func_800766E0(o, 1, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 1, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 2:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 4, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 3:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 4, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 3, 1, 0, 0, 0, func_800760DC);
            break;
        case 1:
        case 5:
            func_800766E0(o, 3, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 3, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 3:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 6, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 5:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 6, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 5, 1, 0, 0, 0, func_800760DC);
            break;
        case 1:
        case 3:
            func_800766E0(o, 5, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 5, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 6:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 15, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 14:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 15, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 14, 1, 0, 0, 0, func_800760DC);
            break;
        case 16:
        case 18:
            func_800766E0(o, 14, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 14, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 7:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 17, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 16:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 17, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 16, 1, 0, 0, 0, func_800760DC);
            break;
        case 14:
        case 18:
            func_800766E0(o, 16, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 16, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 8:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 19, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 18:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 19, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 18, 1, 0, 0, 0, func_800760DC);
            break;
        case 14:
        case 16:
            func_800766E0(o, 18, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 18, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 4:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 9:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 9, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 8:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 9, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 8, 1, 0, 0, 0, func_800760DC);
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 8, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 5:
        switch (o->mode) {
        case 2:
        case 4:
        case 6:
        case 12:
        case 15:
        case 17:
        case 19:
            func_800766E0(o, 12, 1, 1, 0, 0, func_800760DC);
            o->h72 = d->speed;
            break;
        case 11:
            if (func_8004EAD0(&o->mode))
                func_800766E0(o, 12, 1, 0, 2, d->fx1, func_801C95B8);
            else
                func_800766E0(o, 11, 1, 0, 0, 0, func_800760DC);
            break;
        default:
            if (func_801C96F4(o)) {
                func_800766E0(o, 11, 1, 0, 0, 0, func_800760DC);
                o->h72 = d->speed;
            }
            break;
        }
        break;
    case 0:
        if (func_801C96F4(o)) {
            func_80076578(o, 0, 1, func_80076014);
            o->h72 = d->speed;
        }
        break;
    case 64:
        if (o->kind != 192) {
            if (func_801C96F4(o)) {
                if (o->mode == 56) {
                    func_80076A84(o, 22, 1, 255, d->fxd, func_800760DC);
                    o->h72 = 1024;
                } else {
                    func_80076A84(o, 22, 1, 255, d->fxd, func_800760DC);
                }
            }
        } else {
        hover:
            func_800775B8(o, 25, 1, d->speed, &o->vel, func_801C5CB8);
        }
        break;
    case 65:
        func_800775B8(o, 26, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 66:
        func_800775B8(o, 32, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 67:
        func_800775B8(o, 33, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 68:
        func_800775B8(o, 27, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 69:
        func_800775B8(o, 34, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 70:
        func_800775B8(o, 35, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 71:
        func_800775B8(o, 28, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 72:
        func_800775B8(o, 29, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 75:
        func_800775B8(o, 24, 1, d->speed, &o->vel, func_801C5CB8);
        break;
    case 96:
        func_801C5634(o, d);
        if (o->mode != 23) {
            o->flags = 0;
            o->fire_flags = 0;
            func_8004CE44(14, &o->pos, 60, 0);
        }
        switch (ev) {
        case 1:
            func_800766E0(o, 2, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        case 2:
            func_800766E0(o, 4, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        case 3:
            func_800766E0(o, 6, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        case 6:
            func_800766E0(o, 15, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        case 7:
            func_800766E0(o, 17, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        case 8:
            func_800766E0(o, 19, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        case 4:
            func_800766E0(o, 9, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        case 5:
            func_800766E0(o, 12, 1, 0, 2, d->fx1, func_801C95B8);
            event = ev;
            o->h72 = d->speed;
            o->faction &= ~0x10;
            break;
        default:
            if (o->mode == 23) {
                if (func_8004EBF8(&o->mode)) {
                    o->faction &= ~0x10;
                    func_80076578(o, 0, 1, func_80076014);
                } else {
                    func_800765E8(o, 23, 1, func_80076078);
                }
            } else {
                func_800765E8(o, 23, 1, func_80076078);
                o->vel = D_80039C78;
            }
            o->h72 = d->speed;
            break;
        }
        break;
    case 97:
        func_801C5634(o, d);
        if (o->mode == 36) {
            if (func_8004EBF8(&o->mode)) {
                o->faction &= ~0x10;
                func_80076578(o, 0, 1, func_80076014);
            } else {
                func_800765E8(o, 36, 1, func_80076078);
            }
        } else {
            o->flags = 0;
            o->fire_flags = 0;
            func_800765E8(o, 36, 1, func_80076078);
            func_8004EA3C(&o->mode, 1, d->fx9);
            o->vel = D_80039C78;
            if (o->floor <= o->pos.y && o->floor >= o->pos.y - o->h2a)
                func_8007847C(o, 100, 5, 300, 700, 100, 300, 64);
        }
        break;
    case 48:
        func_8007732C(o, 48, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 49:
        func_8007732C(o, 52, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 50:
        func_8007732C(o, 53, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 53:
        func_8007732C(o, 49, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 55:
        func_8007732C(o, 55, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 51:
        func_8007732C(o, 50, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 52:
        func_8007732C(o, 51, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 54:
        func_8007732C(o, 54, 1, d->speed, &o->vel, func_801C5FA4);
        break;
    case 56:
        if (abs(o->vel.x) >= 21 || abs(o->vel.z) >= 21) {
            r = -ratan2(o->vel.x, o->vel.z) - o->yaw;
            switch (((r + 256) >> 9) & 7) {
            case 0:
                func_8007732C(o, 49, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            case 1:
                func_8007732C(o, 55, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            case 2:
                func_8007732C(o, 51, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            case 3:
                func_8007732C(o, 53, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            case 4:
                func_8007732C(o, 48, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            case 5:
                func_8007732C(o, 52, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            case 6:
                func_8007732C(o, 50, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            case 7:
                func_8007732C(o, 54, 1, d->speed, &o->vel, func_801C5FA4);
                break;
            }
        } else {
            func_80076578(o, 0, 1, func_80076014);
            func_801C5634(o, d);
            o->h72 = d->speed;
        }
        break;
    case 32:
        if (o->mode == 56 && func_8004EBF8(&o->mode))
            o->faction &= ~0x10;
        func_800766E0(o, 56, 1, 0, 0, 0, func_801C6598);
        o->h72 = 1024;
        break;
    case 33:
        if (o->mode == 47 && func_8004EBF8(&o->mode))
            o->faction &= ~0x10;
        func_800775B8(o, 47, 1, 1024, &o->vel, func_801C674C);
        break;
    case 34:
        if (o->mode != 45 && o->mode != 46)
            o->vel = D_80039C78;
        func_80077170(o, o->b135, 1, &o->vel, func_801C6A28);
        break;
    case 35:
        if (o->h70 < 0) {
            if (func_8004EBF8(&o->mode))
                o->faction &= ~0x10;
            func_80077170(o, o->b135, -1, &o->vel, func_801C6A28);
        } else {
            func_80077170(o, o->b135, 1, &o->vel, func_801C6A28);
            o->h70 = -1;
            o->h6a = o->h28;
            o->h6c = o->h2a;
        }
        break;
    case 128:
        func_8007790C(o, o->mode, 1, 1024, &o->vel, -1, func_800764C4);
        break;
    case 129:
        func_800778B8(o, o->mode, 1024, &o->vel, -1, func_801C5CB8);
        break;
    }
done:
    if (!(o->faction & 0x10))
        o->move = event;
}
