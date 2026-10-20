/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0011): ground
 * mover step.  Directions 1..8 at +0x3C accelerate along yaw + a fixed offset by
 * def+0x1E and cap the speed at def+0x20; anything else brakes with func_800143B8.
 * Then turn +0x122 towards the speed, and, while moving inside the height band at
 * +0x2A/+0x2E, play one of the def's sound pairs (+0x2F count, pairs from +0x30). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x1E];
    u16 accel;          /* +0x1E */
    u16 max_speed;      /* +0x20 */
    u8 pad22[0x2F - 0x22];
    u8 n_sounds;        /* +0x2F */
    u8 sounds[1][2];    /* +0x30 */
} Func801C59ECDef;
typedef struct Func801C59ECObj Func801C59ECObj;
struct Func801C59ECObj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x2A - 0x14];
    u16 band;           /* +0x2A */
    u8 pad2c[2];
    s16 top;            /* +0x2E */
    u8 pad30[0x3C - 0x30];
    u8 dir;             /* +0x3C */
    u8 pad3d[0x41 - 0x3D];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Func801C59ECObj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vel[3];         /* +0xCC */
    u8 padd2[0x108 - 0xD2];
    Func801C59ECDef *def; /* +0x108 */
    u8 pad10c[0x122 - 0x10C];
    s16 lean;           /* +0x122 */
};
extern u8 *D_801A5DC0;
extern s32 D_801A5DC4;
extern void func_80075FC0(Func801C59ECObj *o);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 func_80027324(s32 v);
extern s32 func_800143B8(u16 *vel, s32 accel, s32 max_speed, u16 *out);
extern s32 func_80013390(s32 cur, s32 target, s32 step);
extern s32 func_8002A4A0(void);
extern void func_80078250(Func801C59ECObj *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, u8 *buf);
extern s32 func_8004F080(void *mover, AcPos *pos);
void func_801C59EC(Func801C59ECObj *o)
{
    Func801C59ECDef *d = o->def;
    AcPos dir;
    s32 r;
    u8 *snd;

    func_80075FC0(o);
    switch (o->dir) {
    case 1:
        func_80014728(o->yaw, &dir);
    accelerate:
        o->vel[0] += (dir.x * d->accel) >> 12;
        o->vel[2] += (dir.z * d->accel) >> 12;
        r = func_80027324((s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2]);
        if (d->max_speed < r) {
            o->vel[0] = (s16)o->vel[0] * d->max_speed / r;
            o->vel[2] = (s16)o->vel[2] * d->max_speed / r;
        }
        break;
    case 2:
        func_80014728(o->yaw + 512, &dir);
        goto accelerate;
    case 3:
        func_80014728(o->yaw - 512, &dir);
        goto accelerate;
    case 6:
        func_80014728(o->yaw + 2048, &dir);
        goto accelerate;
    case 7:
        func_80014728(o->yaw + 1536, &dir);
        goto accelerate;
    case 8:
        func_80014728(o->yaw + 2560, &dir);
        goto accelerate;
    case 4:
        func_80014728(o->yaw + 1024, &dir);
        goto accelerate;
    case 5:
        func_80014728(o->yaw - 1024, &dir);
        goto accelerate;
    default:
        r = func_800143B8(o->vel, d->accel, d->max_speed, o->vel);
        break;
    }
    o->lean = func_80013390(o->lean, r, 512);
    if (r != 0 && o->top <= o->pos.y && !(o->top < o->pos.y - o->band) && d->n_sounds != 0) {
        snd = d->sounds[D_801A5DC4 % d->n_sounds];
        func_80078250(o, snd[0], snd[1], 0, 140, (func_8002A4A0() & 0x7F) + 250, 66,
                      D_801A5DC0 + 0x1F00);
    }
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
