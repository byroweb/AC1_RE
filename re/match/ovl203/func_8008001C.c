#include "ac1/ac1.h"
/* ovl203: movement state machine (fn_move) of a player AC in 2-player mode;
 * the AC counterpart of the mission-library unit machine fdat000 0x801C981C.
 * `event` is the requested move; while bit 0x10 of +0x36 is set the AC keeps its
 * saved move (+0x3C) and only stores a new one when the bit clears (tail).
 * Mode 22 (falling) lands into hover once the mover reports ground; a grounded
 * AC probes 50 units above itself (func_80073CD0, flag D_8019D840) and, when
 * blocked, stops into the matching hover start (func_8007A448) for its walk mode. */
typedef struct {
    u8 pad0[8];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x2A - 0x10];
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
    u8 pad65;
    u8 b66;             /* +0x66 */
    u8 pad67[0x70 - 0x67];
    s16 h70;            /* +0x70 */
    u16 h72;            /* +0x72 */
    u8 pad74[0xCC - 0x74];
    AcPos vel;          /* +0xCC */
    u8 padd4[0x148 - 0xD4];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 flags;          /* +0x15C */
} MvObj3;
typedef struct {
    u8 pad0[0x20];
    u16 h20;            /* +0x20 */
    u8 pad22[0x4C - 0x22];
    u8 *fx;             /* +0x4C */
    u8 pad50[0x214 - 0x50];
    u16 side_a;         /* +0x214 */
    u16 side_b;         /* +0x216 */
    u8 pad218[0x27A - 0x218];
    u8 b27A;            /* +0x27A */
} MvSide3;
extern MvSide3 *D_80039D18;
extern s32 D_8019D840;
extern AcPos D_80039C78;
extern s32 func_8004ECEC(u8 *mover);
extern s32 func_8004EE14(u8 *mover);
extern void func_8004E0A4(u8 *mover, AcPos *vel);
extern void func_8004EC58(u8 *mover, s32 a, u8 *b);
extern void func_8004D04C(s32 a, AcPos *pos, s32 b, s32 c);
extern s32 func_80073CD0(MvObj3 *o, AcPos *at);
extern void func_8007A448(MvObj3 *o, s32 anim);
extern s32 func_8007FF5C(MvObj3 *o);
extern void func_8007FCCC(MvObj3 *o, s32 anim, s32 a, s32 b, AcPos *vel);
extern void func_800752C8(MvObj3 *o, s32 a, void *fn);
extern void func_800753D8(MvObj3 *o, s32 anim, s32 a, s32 b, s32 c, s32 d, void *fn);
extern void func_80075804(MvObj3 *o, s32 anim, s32 a, s32 b, u8 *fx, void *fn);
extern void func_80076338(MvObj3 *o, s32 anim, s32 a, s32 n, AcPos *vel, void *fn);
extern void func_80078B0C(MvObj3 *o);
extern void func_80075368(MvObj3 *o, s32 anim, s32 a, void *fn);
extern void func_800771CC(MvObj3 *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_8007A590(void);
extern void func_80075460(MvObj3 *o, s32 anim, s32 a, s32 b, s32 c, s32 d, void *fn);
extern void func_8007668C(MvObj3 *o, s32 anim, s32 a, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_80076638(MvObj3 *o, s32 anim, s32 n, AcPos *vel, s32 t, void *fn);
extern void func_8007888C();
extern void func_800789A4();
extern void func_8007843C();
extern void func_8007A09C();
extern void func_800783C0();
extern void func_80079144();
extern void func_800797D0();
extern void func_8007A3C8();
void func_8008001C(MvObj3 *o, s32 event)
{
    AcPos at;
    s32 r;

    if (o->faction & 0x10)
        event = o->move;
    if (o->mode == 22) {
        if (func_8004ECEC(&o->mode)) {
            func_8004E0A4(&o->mode, &o->vel);
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
            if (o->kind != 176 || (o->vel.x == 0 && o->vel.z == 0))
                D_8019D840 = 1;
            r = func_80073CD0(o, &at);
            D_8019D840 = 0;
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
                D_80039D18->h20 &= ~2;
                switch (o->mode) {
                case 0:
                    break;
                case 2:
                    o->vel.y = 0;
                    func_8007A448(o, 26);
                    return;
                case 4:
                    o->vel.y = 0;
                    func_8007A448(o, 32);
                    return;
                case 6:
                    o->vel.y = 0;
                    func_8007A448(o, 33);
                    return;
                case 15:
                    o->vel.y = 0;
                    func_8007A448(o, 27);
                    return;
                case 17:
                    o->vel.y = 0;
                    func_8007A448(o, 34);
                    return;
                case 19:
                    o->vel.y = 0;
                    func_8007A448(o, 35);
                    return;
                case 9:
                    o->vel.y = 0;
                    func_8007A448(o, 28);
                    return;
                case 12:
                    o->vel.y = 0;
                    func_8007A448(o, 29);
                    return;
                case 21:
                    o->vel = D_80039C78;
                    if (o->kind == 2) {
                        func_80076338(o, 30, 1, 512, &o->vel, func_8007A09C);
                        r = 73;
                    } else {
                        func_80076338(o, 31, 1, 512, &o->vel, func_8007A09C);
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
            func_8007FCCC(o, 2, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 2, 0, 256, &o->vel);
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
            func_8007FCCC(o, 4, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 4, 0, 256, &o->vel);
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
            func_8007FCCC(o, 6, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 6, 0, 256, &o->vel);
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
            func_8007FCCC(o, 15, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 15, 0, 256, &o->vel);
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
            func_8007FCCC(o, 17, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 17, 0, 256, &o->vel);
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
            func_8007FCCC(o, 19, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 19, 0, 256, &o->vel);
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
            func_8007FCCC(o, 9, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 9, 0, 256, &o->vel);
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
            func_8007FCCC(o, 12, 0, 256, &o->vel);
            break;
        default:
            if (func_8007FF5C(o))
                func_8007FCCC(o, 12, 0, 256, &o->vel);
            break;
        }
        break;
    case 0:
        switch (o->mode) {
        default:
            if (!func_8007FF5C(o))
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
            func_8007FCCC(o, 0, 0, 256, &o->vel);
            break;
        }
        break;
    case 9:
        if (o->kind != 3) {
            if (o->mode == 21) {
                func_800752C8(o, 2, func_8007888C);
                o->h70 = 1;
                o->b66 = 1;
                break;
            }
            if (!func_8007FF5C(o))
                break;
        }
        func_800753D8(o, 21, 1, 2, D_80039D18->side_a, D_80039D18->side_b, func_8007888C);
        break;
    case 10:
        if (o->kind != 2) {
            if (o->mode == 21) {
                func_800752C8(o, 3, func_800789A4);
                o->h70 = 1;
                o->b66 = 1;
                break;
            }
            if (!func_8007FF5C(o))
                break;
        }
        func_800753D8(o, 21, 1, 3, -D_80039D18->side_a, -D_80039D18->side_b, func_800789A4);
        break;
    case 64:
        if (o->kind != 192) {
            if (D_80039D18->b27A == 1) {
                o->vel.y = 0;
                goto hover;
            }
            if (func_8007FF5C(o)) {
                if (o->mode == 56) {
                    func_80075804(o, 22, 1, 255, D_80039D18->fx + 12, func_8007843C);
                    o->h72 = 1024;
                } else {
                    func_80075804(o, 22, 1, 255, D_80039D18->fx + 12, func_8007843C);
                }
            }
        } else {
        hover:
            func_80076338(o, 25, 1, 512, &o->vel, func_8007A09C);
        }
        break;
    case 65:
        func_80076338(o, 26, 1, 512, &o->vel, func_8007A09C);
        break;
    case 66:
        func_80076338(o, 32, 1, 512, &o->vel, func_8007A09C);
        break;
    case 67:
        func_80076338(o, 33, 1, 512, &o->vel, func_8007A09C);
        break;
    case 68:
        func_80076338(o, 27, 1, 512, &o->vel, func_8007A09C);
        break;
    case 69:
        func_80076338(o, 34, 1, 512, &o->vel, func_8007A09C);
        break;
    case 70:
        func_80076338(o, 35, 1, 512, &o->vel, func_8007A09C);
        break;
    case 71:
        func_80076338(o, 28, 1, 512, &o->vel, func_8007A09C);
        break;
    case 72:
        func_80076338(o, 29, 1, 512, &o->vel, func_8007A09C);
        break;
    case 73:
        func_80076338(o, 30, 1, 512, &o->vel, func_8007A09C);
        break;
    case 74:
        func_80076338(o, 31, 1, 512, &o->vel, func_8007A09C);
        break;
    case 75:
        func_80076338(o, 24, 1, 512, &o->vel, func_8007A09C);
        break;
    case 96:
        func_80078B0C(o);
        if (o->mode != 23) {
            o->flags = 0;
            o->fire_flags = 0;
            func_8004D04C(14, &o->pos, 60, 0);
        }
        switch (o->mode) {
        case 24:
        case 25:
            func_8007FCCC(o, 0, 0, 256, &o->vel);
            event = 0;
            o->faction &= ~0x10;
            break;
        case 26:
            func_8007FCCC(o, 2, 0, 256, &o->vel);
            event = 1;
            o->faction &= ~0x10;
            break;
        case 32:
            func_8007FCCC(o, 4, 0, 256, &o->vel);
            event = 2;
            o->faction &= ~0x10;
            break;
        case 33:
            func_8007FCCC(o, 6, 0, 256, &o->vel);
            event = 3;
            o->faction &= ~0x10;
            break;
        case 27:
            func_8007FCCC(o, 15, 0, 256, &o->vel);
            event = 6;
            o->faction &= ~0x10;
            break;
        case 34:
            func_8007FCCC(o, 17, 0, 256, &o->vel);
            event = 7;
            o->faction &= ~0x10;
            break;
        case 35:
            func_8007FCCC(o, 19, 0, 256, &o->vel);
            event = 8;
            o->faction &= ~0x10;
            break;
        case 28:
            func_8007FCCC(o, 9, 0, 256, &o->vel);
            event = 4;
            o->faction &= ~0x10;
            break;
        case 29:
            func_8007FCCC(o, 12, 0, 256, &o->vel);
            event = 5;
            o->faction &= ~0x10;
            break;
        case 30:
            func_800753D8(o, 21, 1, 2, D_80039D18->side_a, D_80039D18->side_b, func_8007888C);
            event = 9;
            o->faction &= ~0x10;
            break;
        case 31:
            func_800753D8(o, 21, 1, 3, -D_80039D18->side_a, -D_80039D18->side_b, func_800789A4);
            event = 10;
            o->faction &= ~0x10;
            break;
        default:
            if (o->mode == 23) {
                if (func_8004EE14(&o->mode)) {
                    o->faction &= ~0x10;
                    func_8007FCCC(o, 0, 0, 256, &o->vel);
                } else {
                    func_80075368(o, 23, 1, func_800783C0);
                }
            } else {
                func_80075368(o, 23, 1, func_800783C0);
                o->vel = D_80039C78;
            }
            o->h72 = 512;
            break;
        }
        break;
    case 97:
        func_80078B0C(o);
        if (o->mode == 36) {
            if (func_8004EE14(&o->mode)) {
                event = 0;
                o->faction &= ~0x10;
                func_8007FCCC(o, 0, 0, 512, &o->vel);
            } else {
                func_80075368(o, 36, 1, func_800783C0);
                o->h72 = 512;
            }
        } else {
            o->flags = 0;
            o->fire_flags = 0;
            func_80075368(o, 36, 1, func_800783C0);
            o->h72 = 512;
            func_8004EC58(&o->mode, 1, D_80039D18->fx + 8);
            o->vel = D_80039C78;
            if (o->floor <= o->pos.y && o->floor >= o->pos.y - o->h2a)
                func_800771CC(o, 100, 5, 300, 700, 100, 300, 64);
        }
        break;
    case 32:
        if (o->mode == 56 && func_8004EE14(&o->mode)) {
            o->faction &= ~0x10;
            func_8007A590();
        }
        func_80075460(o, 56, 1, 0, 0, 0, func_80079144);
        o->h72 = 1024;
        break;
    case 33:
        if (o->mode == 47 && func_8004EE14(&o->mode)) {
            o->faction &= ~0x10;
            func_8007A590();
        }
        func_80076338(o, 47, 1, 1024, &o->vel, func_800797D0);
        break;
    case 128:
        func_8007668C(o, o->mode, 1, 1024, &o->vel, -1, func_8007A3C8);
        break;
    case 129:
        func_80076638(o, o->mode, 1024, &o->vel, -1, func_8007A09C);
        break;
    }
    if (!(o->faction & 0x10))
        o->move = event;
}
