#include "ac1/ac1.h"
/* ovl202 0x80080DA0: ground mover step of a mission actor (port of fdat000 0x801C59EC with
 * the def in globals): directions 1..8 at +0x3C accelerate along yaw + offset, else brake;
 * lean +0x10E follows the speed; footstep sounds while inside the height band. */
typedef struct {
    u16 accel;          /* 0x800411B0 */
    u16 max_speed;      /* 0x800411B2 */
} MoveDef;
typedef struct {
    u8 n;               /* 0x8004124F */
    u8 s[1][2];
} SoundSet;
extern MoveDef D_800411B0;
extern u16 D_800411B4;
extern u16 D_800411B6;
extern SoundSet D_8004124F;
extern u8 *D_801A5DC0;
extern s32 D_801A5DC4;
extern void func_80079798(AcEntity *, s32, s32);
extern void func_80014728(s32 angle, AcPos *dir);
extern s32 func_80027324(s32 v);
extern s32 func_800143B8(u16 *vel, s32 accel, s32 max_speed, u16 *out);
extern s32 func_80013390(s32 cur, s32 target, s32 step);
extern s32 func_8002A4A0(void);
extern void func_80078250(AcEntity *o, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, u8 *buf);
extern s32 func_8004F080(AcMover *mover, AcPos *pos);

#define VEL(o) ((u16 *)((u8 *)(o) + 0xCC))
#define DIR(o) (((u8 *)(o))[0x3C])
#define LEAN(o) (*(s16 *)((u8 *)(o) + 0x10E))
#define BAND(o) (*(u16 *)((u8 *)(o) + 0x2A))
#define TOP(o) (*(s16 *)((u8 *)(o) + 0x2E))

void func_80080DA0(AcEntity *o)
{
    AcPos dir;
    s32 r;
    u8 *snd;

    func_80079798(o, D_800411B4, D_800411B6);
    switch (DIR(o)) {
    case 1:
        func_80014728(o->yaw, &dir);
    accelerate:
        VEL(o)[0] += (dir.x * D_800411B0.accel) >> 12;
        VEL(o)[2] += (dir.z * D_800411B0.accel) >> 12;
        r = func_80027324((s16)VEL(o)[0] * (s16)VEL(o)[0] + (s16)VEL(o)[2] * (s16)VEL(o)[2]);
        if (D_800411B0.max_speed < r) {
            VEL(o)[0] = (s16)VEL(o)[0] * D_800411B0.max_speed / r;
            VEL(o)[2] = (s16)VEL(o)[2] * D_800411B0.max_speed / r;
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
        r = func_800143B8(VEL(o), D_800411B0.accel, D_800411B0.max_speed, VEL(o));
        break;
    }
    LEAN(o) = func_80013390(LEAN(o), r, 512);
    if (r != 0 && TOP(o) <= o->pos.y && !(TOP(o) < o->pos.y - BAND(o)) && D_8004124F.n != 0) {
        snd = D_8004124F.s[D_801A5DC4 % D_8004124F.n];
        func_80078250(o, snd[0], snd[1], 0, 140, (func_8002A4A0() & 0x7F) + 250, 66,
                      D_801A5DC0 + 0x1F00);
    }
    if (!func_8004F080(&o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
