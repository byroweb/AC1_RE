/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0013): walker
 * step.  Clear faction bit 4 unless direction is 32, run the flag/colour pair, start
 * the def's two sounds (+0x30/+0x32) inside the height band at +0x2A/+0x2E or stop
 * them outside it, then directions 48..55 accelerate along yaw + a fixed offset by
 * def+0x1E, capped at def+0x20; 56 brakes with func_800143B8. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x1E];
    u16 accel;          /* +0x1E */
    u16 max_speed;      /* +0x20 */
    u8 pad22[0x30 - 0x22];
    u8 snd_a[2];        /* +0x30 */
    u8 snd_b[2];        /* +0x32 */
} Func801C5FA4Def;
typedef struct Func801C5FA4Obj Func801C5FA4Obj;
struct Func801C5FA4Obj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x2A - 0x14];
    u16 band;           /* +0x2A */
    u8 pad2c[2];
    s16 top;            /* +0x2E */
    u8 pad30[0x36 - 0x30];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 pad3d[0x41 - 0x3D];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Func801C5FA4Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vel[3];         /* +0xCC */
    u8 padd2[0x108 - 0xD2];
    Func801C5FA4Def *def; /* +0x108 */
};
extern u8 *D_801A5DC0;
extern void func_80075FC0(Func801C5FA4Obj *o);
extern void func_801C55B4(Func801C5FA4Obj *o);
extern void func_801C565C(Func801C5FA4Obj *o, Func801C5FA4Def *d);
extern s32 func_8002A4A0(void);
extern void func_80078250(Func801C5FA4Obj *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, u8 *buf);
extern void func_8007832C(Func801C5FA4Obj *o, s32 a, s32 b);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 func_80027324(s32 v);
extern s32 func_800143B8(u16 *vel, s32 accel, s32 max_speed, u16 *out);
extern s32 func_8004F080(void *mover, AcPos *pos);
void func_801C5FA4(Func801C5FA4Obj *o)
{
    Func801C5FA4Def *d = o->def;
    AcPos dir;
    s32 accel, max_speed, r, vol;

    if (o->dir != 32)
        o->faction &= 0xFFEF;
    func_80075FC0(o);
    func_801C55B4(o);
    func_801C565C(o, d);
    if (o->top <= o->pos.y && !(o->top < o->pos.y - o->band)) {
        vol = 160;
        func_80078250(o, d->snd_a[0], d->snd_a[1], 0, vol, (func_8002A4A0() & 0x7F) + 300, 66,
                      D_801A5DC0 + 0x1F00);
        func_80078250(o, d->snd_b[0], d->snd_b[1], 0, vol, (func_8002A4A0() & 0x7F) + 300, -1, 0);
    } else {
        func_8007832C(o, d->snd_a[0], d->snd_a[1]);
        func_8007832C(o, d->snd_b[0], d->snd_b[1]);
    }
    accel = d->accel;
    max_speed = d->max_speed;
    switch (o->dir) {
    case 48:
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
    case 49:
        func_80014728(o->yaw + 512, &dir);
        goto accelerate;
    case 50:
        func_80014728(o->yaw - 512, &dir);
        goto accelerate;
    case 53:
        func_80014728(o->yaw + 2048, &dir);
        goto accelerate;
    case 54:
        func_80014728(o->yaw + 1536, &dir);
        goto accelerate;
    case 55:
        func_80014728(o->yaw + 2560, &dir);
        goto accelerate;
    case 51:
        func_80014728(o->yaw + 1024, &dir);
        goto accelerate;
    case 52:
        func_80014728(o->yaw - 1024, &dir);
        goto accelerate;
    case 56:
        func_800143B8(o->vel, accel, max_speed, o->vel);
        break;
    }
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
