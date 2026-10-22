#include "ac1/ac1.h"

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
    u8 pad3d[0x40 - 0x3D];
    u8 ai_mode;         /* +0x40 */
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 */
    u16 vel[3];         /* +0xCC */
    s16 hold;           /* +0xD2 */
    u8 padd4[0x15C - 0xD4];
    u32 flags;          /* +0x15C */
};
extern s32 D_801A5DC0;
extern s32 func_80027324(s32);
extern void func_80014728(s32, AcPos *);
extern void func_800143B8(void *, s32, s32, void *);
extern void func_8004C98C(s32, s32, void *, void *, s32, s32, s32, s32, s32);
extern s32 func_8004F080(void *, void *);
extern void func_80075BC0(Obj *, AcPos *);
extern void func_80075FC0(Obj *);
extern void func_800784F8(Obj *, s32, s32, s32);

void func_801C4D30(Obj *o)
{
    AcPos dir;
    u16 t0;
    u16 t1;
    s32 r;

    func_800784F8(o, 400, 300, 500);
    if (o->dir == 129) {
        if (o->hold != 0) {
            o->hold--;
            goto climb;
        }
        o->faction &= 0xFFEF;
    }
    func_80075FC0(o);
    func_80075BC0(o, &dir);
    if (dir.y <= 0 || (s16)o->vel[1] > 1200) {
        o->vel[1] -= 64;
        if ((s16)o->vel[1] < -1920) {
            *(s16 *)&o->vel[1] = -1920;
        }
    } else {
climb:
        o->vel[1] += 256;
        if ((s16)o->vel[1] > 9600) {
            o->vel[1] = 9600;
        }
    }
    switch (o->dir) {
    case 2:
        func_80014728(o->yaw, &dir);
accel:
        dir.x >>= 3;
        dir.z >>= 3;
        t0 = dir.x;
        t1 = dir.z;
        o->vel[0] += t0;
        o->vel[2] += dir.z;
        r = (s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2];
        if (r > 0x01900000) {
            r = func_80027324(r);
            o->vel[0] = (s16)o->vel[0] * 5120 / r;
            o->vel[2] = (s16)o->vel[2] * 5120 / r;
        }
        break;
    case 3:
        func_80014728(o->yaw + 2048, &dir);
        goto accel;
    case 4:
        func_80014728(o->yaw + 1024, &dir);
        goto accel;
    case 5:
        func_80014728(o->yaw - 1024, &dir);
        goto accel;
    default:
        func_800143B8(o->vel, 64, 5120, o->vel);
        break;
    }
    func_8004C98C(0x80, D_801A5DC0 + 0x1000, &o->pos, o->vel, 100, 8000, 8500, 0, 50);
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state) {
        o->fn_state(o, o->state);
    }
}
