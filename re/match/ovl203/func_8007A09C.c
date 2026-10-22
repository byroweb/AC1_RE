/* ovl203 func_8007A09C: versus-mode hover mover step (port of fdat000 func_801C5CB8).
 * Sink while the current player holds the +0x29A buttons (and flags20 bit 0 is clear),
 * else climb (vel Y capped at 9600, floored at -player+0x20A); directions 65..71
 * accelerate along yaw + offset by player+0x20C, capped at +0x20E; else brake. */
#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

#define VP16(off) (*(u16 *)((u8 *)D_80039D18 + (off)))

typedef struct V7A09CObj V7A09CObj;
struct V7A09CObj {
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
    void (*fn_state)(V7A09CObj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 AcMover */
    u16 vel[3];         /* +0xCC */
    s16 hold;           /* +0xD2 */
};
extern void func_80077248(V7A09CObj *o, s32 a, s32 b, s32 c);
extern void func_80078258(V7A09CObj *o, s32 a, s32 b);
extern void func_80078C8C(V7A09CObj *o);
extern void func_80078B38(V7A09CObj *o);
extern void func_80078B0C(V7A09CObj *o);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 SquareRoot0(s32 v);
extern s32 func_800143B8(u16 *vel, s32 accel, s32 max_speed, u16 *out);
extern s32 func_8004F29C(void *mover, AcPos *pos);

void func_8007A09C(V7A09CObj *o)
{
    AcPos dir;
    s32 r;
    u16 lim;

    func_80077248(o, 400, 160, 500);
    if (o->dir == 129) {
        if (o->hold != 0) {
            o->hold--;
            goto climb;
        }
        o->faction &= 0xFFEF;
    }
    func_80078258(o, VP16(0x214), D_80039D18->speed_cap);
    if ((D_80039D18->pad_cur & VP16(0x29A)) && !(D_80039D18->flags20 & 1)) {
        o->vel[1] -= VP16(0x208);
        func_80078C8C(o);
        func_80078B38(o);
    } else {
climb:
        o->vel[1] += 256;
        if ((s16)o->vel[1] > 9600)
            o->vel[1] = 9600;
        func_80078B0C(o);
    }
    lim = VP16(0x20A);
    if ((s16)o->vel[1] < -lim)
        o->vel[1] = -lim;
    switch (o->dir) {
    case 65:
        func_80014728(o->yaw, &dir);
    accelerate:
        o->vel[0] += (dir.x * VP16(0x20C)) >> 12;
        o->vel[2] += (dir.z * VP16(0x20C)) >> 12;
        r = SquareRoot0((s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2]);
        if (VP16(0x20E) < r) {
            o->vel[0] = (s16)o->vel[0] * VP16(0x20E) / r;
            o->vel[2] = (s16)o->vel[2] * VP16(0x20E) / r;
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
        func_800143B8(o->vel, VP16(0x20C) >> 2, VP16(0x20E), o->vel);
        break;
    }
    if (!func_8004F29C(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
