/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0018): flying
 * mover step.  Climb or sink (vel Y, clamped to 9600 and -def+0x18) depending on the
 * probe from func_80075BC0, drive the same phase-keyed helpers as 801C6598, then
 * accelerate along the facing by def+0x1A and cap the horizontal speed at def+0x1C. */
#include "ac1/ac1.h"
typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Func801C674CVec;
typedef struct {
    u8 pad0[0x16];
    u16 sink;           /* +0x16 */
    u16 floor;          /* +0x18 */
    u16 accel;          /* +0x1A */
    u16 max_speed;      /* +0x1C */
    u8 pad1e[0x2C - 0x1E];
    u8 b2c;             /* +0x2C */
    u8 b2d;             /* +0x2D */
    u8 pad2e[0x3C - 0x2E];
    Func801C674CVec aim; /* +0x3C */
    u8 pad42[0x48 - 0x42];
    u8 lo;              /* +0x48 */
    u8 hi;              /* +0x49 */
    u8 b4a;             /* +0x4A */
} Func801C674CDef;
typedef struct {
    u8 m64[0x6E - 0x64];
    s16 phase;          /* +0x6E */
} Func801C674CMover;
typedef struct Func801C674CObj Func801C674CObj;
struct Func801C674CObj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x41 - 0x14];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Func801C674CObj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    Func801C674CMover mover; /* +0x64 */
    u8 pad70[0xCC - 0x70];
    u16 vel[3];         /* +0xCC */
    u8 padd2[0x108 - 0xD2];
    Func801C674CDef *def; /* +0x108 */
    u8 x10c[4];         /* +0x10C */
};
extern void func_80075FC0(Func801C674CObj *o);
extern void func_800784F8(Func801C674CObj *o, s32 a, s32 b, s32 c);
extern void func_80075BC0(Func801C674CObj *o, AcPos *probe);
extern void func_801C565C(Func801C674CObj *o, Func801C674CDef *d);
extern void func_801C5634(Func801C674CObj *o, Func801C674CDef *d);
extern void func_8004CE44(s32 id, AcPos *pos, s32 a, s32 b);
extern void func_801C56C8(Func801C674CObj *o, Func801C674CDef *d);
extern void func_8007A0E4();
extern void func_801C62A8(Func801C674CObj *o, Func801C674CDef *d, Func801C674CVec *aim);
extern void func_800745D0(Func801C674CObj *o, void *p, s32 a, s32 b, s32 c, s32 e, s32 f);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 func_80027324(s32 v);
extern s32 func_8004F080(void *mover, AcPos *pos);
void func_801C674C(Func801C674CObj *o)
{
    Func801C674CDef *d = o->def;
    AcPos a;
    Func801C674CVec aim;
    s32 accel, max_speed, r;
    s32 lim;

    func_80075FC0(o);
    func_800784F8(o, 400, 160, 500);
    func_80075BC0(o, &a);
    if (a.y < 0) {
        o->vel[1] -= d->sink << 2;
        func_801C565C(o, d);
    } else {
        o->vel[1] += 256;
        if ((s16)o->vel[1] > 9600)
            o->vel[1] = 9600;
        func_801C5634(o, d);
    }
    lim = -d->floor;
    if ((s16)o->vel[1] < lim)
        o->vel[1] = lim;
    if (o->mover.phase >= 0 && o->mover.phase <= 10) {
        if (o->mover.phase == 5)
            func_8004CE44(d->b4a, &o->pos, 120, 0);
        func_801C56C8(o, d);
    } else {
        func_8007A0E4(o, d);
    }
    if (o->mover.phase > d->lo && o->mover.phase < d->hi) {
        aim = d->aim;
        aim.y <<= 2;
        func_801C62A8(o, d, &aim);
    } else if (o->mover.phase == d->lo) {
        func_800745D0(o, o->x10c, d->b2c, d->b2d, 2, 0, 1);
    }
    accel = d->accel;
    max_speed = d->max_speed;
    func_80014728(o->yaw, &a);
    o->vel[0] += (a.x * accel) >> 12;
    o->vel[2] += (a.z * accel) >> 12;
    r = func_80027324((s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2]);
    if (max_speed < r) {
        o->vel[0] = (s16)o->vel[0] * max_speed / r;
        o->vel[2] = (s16)o->vel[2] * max_speed / r;
    }
    if (!func_8004F080(&o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
