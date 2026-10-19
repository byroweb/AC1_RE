/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0012): hover
 * mover step (the think routine 801C5798 installs).  Sink when the probe below is
 * negative, else climb (vel Y capped at 9600 and -def+0x18); direction 129 first counts
 * down +0xD2 and then clears faction bit 4, direction 75 always climbs.  Directions
 * 65..72 accelerate along yaw + a fixed offset by def+0x1A, capped at def+0x1C; anything
 * else brakes with func_800143B8. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x16];
    u16 sink;           /* +0x16 */
    u16 floor;          /* +0x18 */
    u16 accel;          /* +0x1A */
    u16 max_speed;      /* +0x1C */
} Func801C5CB8Def;
typedef struct Func801C5CB8Obj Func801C5CB8Obj;
struct Func801C5CB8Obj {
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
    void (*fn_state)(Func801C5CB8Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vel[3];         /* +0xCC */
    s16 hold;           /* +0xD2 */
    u8 padd4[0x108 - 0xD4];
    Func801C5CB8Def *def; /* +0x108 */
};
extern void func_800784F8(Func801C5CB8Obj *o, s32 a, s32 b, s32 c);
extern void func_80075FC0(Func801C5CB8Obj *o);
extern void func_80075BC0(Func801C5CB8Obj *o, AcPos *probe);
extern void func_801C55B4(Func801C5CB8Obj *o);
extern void func_801C565C(Func801C5CB8Obj *o, Func801C5CB8Def *d);
extern void func_801C5634(Func801C5CB8Obj *o, Func801C5CB8Def *d);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 func_80027324(s32 v);
extern s32 func_800143B8(u16 *vel, s32 accel, s32 max_speed, u16 *out);
extern s32 func_8004F080(void *mover, AcPos *pos);
void func_801C5CB8(Func801C5CB8Obj *o)
{
    Func801C5CB8Def *d = o->def;
    AcPos dir;
    s32 accel, max_speed, r;
    s32 lim, sink;

    func_800784F8(o, 400, 160, 500);
    if (o->dir == 129) {
        if (o->hold != 0) {
            o->hold--;
            goto climb;
        }
        o->faction &= 0xFFEF;
    }
    func_80075FC0(o);
    if (o->dir == 75)
        goto climb;
    func_80075BC0(o, &dir);
    if (dir.y >= 0)
        goto climb;
    sink = d->sink;
    o->vel[1] -= sink;
    func_801C55B4(o);
    func_801C565C(o, d);
    goto clamp;
climb:
    o->vel[1] += 256;
    if ((s16)o->vel[1] > 9600)
        o->vel[1] = 9600;
    func_801C5634(o, d);
clamp:
    lim = -d->floor;
    if ((s16)o->vel[1] < lim)
        o->vel[1] = lim;
    accel = d->accel;
    max_speed = d->max_speed;
    switch (o->dir) {
    case 65:
        func_80014728(o->yaw, &dir);
    accelerate:
        o->vel[0] += (dir.x * accel) >> 12;
        o->vel[2] += (dir.z * accel) >> 12;
        r = func_80027324((s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2]);
        if (max_speed < r) {
            o->vel[0] = (s16)o->vel[0] * max_speed / r;
            o->vel[2] = (s16)o->vel[2] * max_speed / r;
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
        func_800143B8(o->vel, accel >> 2, max_speed, o->vel);
        break;
    }
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
