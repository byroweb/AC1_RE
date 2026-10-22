#include "ac1/ac1.h"

typedef struct {
    u16 sink;           /* 0x800411A8 */
    u16 floor;          /* 0x800411AA */
    u16 accel;          /* 0x800411AC */
    u16 max_speed;      /* 0x800411AE */
    u8 pad[4];
    u16 a;              /* 0x800411B4 */
    u16 b;              /* 0x800411B6 */
} HoverCfg;

typedef struct Obj Obj;
struct Obj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 pad3d[0x41 - 0x3D];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vel[3];         /* +0xCC */
    s16 hold;           /* +0xD2 */
};

extern HoverCfg D_800411A8;
extern s32 D_80040FA8;
extern u16 D_80040FC0;
extern u16 D_8004123A;
extern void func_800784F8(Obj *o, s32 a, s32 b, s32 c);
extern void func_80079798(Obj *o, s32 a, s32 b);
extern void func_8007A19C(Obj *o);
extern void func_8007A058(Obj *o);
extern void func_8007A02C(Obj *o);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 func_80027324(s32 v);
extern s32 func_800143B8(u16 *vel, s32 accel, s32 max_speed, u16 *out);
extern s32 func_8004F080(void *mover, AcPos *pos);

void func_8007B548(Obj *o)
{
    AcPos dir;
    s32 r;

    func_800784F8(o, 400, 160, 500);
    if (o->dir == 129) {
        if (o->hold != 0) {
            o->hold--;
            goto climb;
        }
        o->faction &= 0xFFEF;
    }
    func_80079798(o, D_800411A8.a, D_800411A8.b);
    if (!(D_80040FA8 & D_8004123A)) {
        goto climb;
    }
    if (D_80040FC0 & 1) {
        goto climb;
    }
    o->vel[1] -= D_800411A8.sink;
    func_8007A19C(o);
    func_8007A058(o);
    goto clamp;
climb:
    o->vel[1] += 256;
    if ((s16)o->vel[1] > 9600) {
        o->vel[1] = 9600;
    }
    func_8007A02C(o);
clamp:
    if ((s16)o->vel[1] < -D_800411A8.floor) {
        o->vel[1] = -D_800411A8.floor;
    }
    switch (o->dir) {
    case 65:
        func_80014728(o->yaw, &dir);
    accelerate:
        o->vel[0] += (dir.x * D_800411A8.accel) >> 12;
        o->vel[2] += (dir.z * D_800411A8.accel) >> 12;
        r = func_80027324((s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2]);
        if (D_800411A8.max_speed < r) {
            o->vel[0] = (s16)o->vel[0] * D_800411A8.max_speed / r;
            o->vel[2] = (s16)o->vel[2] * D_800411A8.max_speed / r;
        }
        break;
    case 66:
        func_80014728(o->yaw + 512, &dir);
        goto accelerate;
    case 67:
        func_80014728(o->yaw - 512, &dir);
        goto accelerate;
    case 68:
        func_80014728(o->yaw + 2048, &dir);
        goto accelerate;
    case 69:
        func_80014728(o->yaw + 1536, &dir);
        goto accelerate;
    case 70:
        func_80014728(o->yaw + 2560, &dir);
        goto accelerate;
    case 71:
        func_80014728(o->yaw + 1024, &dir);
        goto accelerate;
    case 72:
        func_80014728(o->yaw - 1024, &dir);
        goto accelerate;
    default:
        func_800143B8(o->vel, D_800411A8.accel >> 2, D_800411A8.max_speed, o->vel);
        break;
    }
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state) {
        o->fn_state(o, o->state);
    }
}
