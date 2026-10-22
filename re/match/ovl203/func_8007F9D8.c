#include "ac1/ac1.h"
/* ovl203 ground mover step (cf. fdat000 0x801C59EC): the tuning lives in the
 * record at *D_80039D18 instead of o->def. */
typedef struct {
    u8 pad0[0x210];
    u16 accel;          /* +0x210 */
    u16 max_speed;      /* +0x212 */
    u16 snd_a;          /* +0x214 */
    u16 snd_b;          /* +0x216 */
    u8 pad218[0x2AF - 0x218];
    u8 n_sounds;        /* +0x2AF */
    u8 sounds[1][2];    /* +0x2B0 */
} Mv3Def;
typedef struct Mv3Obj Mv3Obj;
struct Mv3Obj {
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
    void (*fn_state)(Mv3Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vel[3];         /* +0xCC */
    u8 padd2[0x10E - 0xD2];
    s16 lean;           /* +0x10E */
};
extern Mv3Def *D_80039D18;
extern u8 *D_8019D838;
extern s32 D_8019D83C;
extern void func_80078258(Mv3Obj *o, s32 a, s32 b);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 func_80027324(s32 v);
extern s32 func_800143B8(u16 *vel, s32 accel, s32 max_speed, u16 *out);
extern s32 func_80013390(s32 cur, s32 target, s32 step);
extern s32 func_8002A4A0(void);
extern void func_80076FA0(Mv3Obj *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, u8 *buf);
extern s32 func_8004F29C(void *mover, AcPos *pos);
void func_8007F9D8(Mv3Obj *o)
{
    AcPos dir;
    s32 r;
    u8 *snd;

    func_80078258(o, D_80039D18->snd_a, D_80039D18->snd_b);
    switch (o->dir) {
    case 1:
        func_80014728(o->yaw, &dir);
    accelerate:
        o->vel[0] += (dir.x * D_80039D18->accel) >> 12;
        o->vel[2] += (dir.z * D_80039D18->accel) >> 12;
        r = func_80027324((s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2]);
        if (D_80039D18->max_speed < r) {
            o->vel[0] = (s16)o->vel[0] * D_80039D18->max_speed / r;
            o->vel[2] = (s16)o->vel[2] * D_80039D18->max_speed / r;
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
        r = func_800143B8(o->vel, D_80039D18->accel, D_80039D18->max_speed, o->vel);
        break;
    }
    o->lean = func_80013390(o->lean, r, 512);
    if (r != 0 && o->top <= o->pos.y && !(o->top < o->pos.y - o->band) && D_80039D18->n_sounds != 0) {
        snd = D_80039D18->sounds[D_8019D83C % D_80039D18->n_sounds];
        func_80076FA0(o, snd[0], snd[1], 0, 140, (func_8002A4A0() & 0x7F) + 250, 66,
                      D_8019D838 + 0x1F00);
    }
    if (!func_8004F29C(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
