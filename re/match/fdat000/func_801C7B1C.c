/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0020): aim a
 * weapon.  Get the muzzle point (func_800745D0), trace towards the target with the
 * weapon's spread limits (func_80073C2C); on a usable hit count the burst/round
 * counters at +0x133/+0x134, optionally lead the target (func_80015258, remembering
 * its position at +0x10C) and store the aim vector at +0x76..+0x7A.  Returns 1 on a
 * hit, 0 (vector cleared) otherwise. */
#include "ac1/ac1.h"
typedef struct {
    u16 mode;           /* +0x00 */
    u8 pad2[3];
    u8 b5;              /* +0x05 */
    u8 b6;              /* +0x06 */
    u8 pad7[0x11 - 0x07];
    u8 lead;            /* +0x11 */
    s16 yaw_min;        /* +0x12 */
    s16 yaw_max;        /* +0x14 */
    s16 pitch_min;      /* +0x16 */
    s16 pitch_max;      /* +0x18 */
    s16 range;          /* +0x1A */
    s16 rounds;         /* +0x1C */
    s16 burst;          /* +0x1E */
} Func801C7B1CWeapon;
typedef struct {
    u8 pad0[0x10];
    AcPos pos;          /* +0x10 hit point; w = flags */
    AcPos aim;          /* +0x18 */
    u8 pad20[0x30 - 0x20];
} Func801C7B1CHit;
typedef struct {
    u8 pad0[0x10];
    u8 x10[2];          /* +0x10 */
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
    u8 pad38[0x48 - 0x38];
    void *target;       /* +0x48 */
    u8 pad4c[0x76 - 0x4C];
    s16 aim_x;          /* +0x76 */
    s16 aim_y;          /* +0x78 */
    s16 aim_z;          /* +0x7A */
    u8 pad7c[0x10C - 0x7C];
    AcPos last;         /* +0x10C */
    u8 pad114[0x133 - 0x114];
    u8 shots;           /* +0x133 */
    u8 rounds;          /* +0x134 */
} Func801C7B1CObj;
extern s32 func_800745D0(Func801C7B1CObj *o, AcPos *muzzle, s32 a, s32 b, s32 c, s32 d);
extern s32 func_80073C2C(void *target, AcPos *muzzle, s32 yaw, s32 zero, s32 flags, s32 faction,
                         s16 yaw_min, s16 yaw_max, s16 pitch_min, s16 pitch_max, s16 range,
                         Func801C7B1CHit *hit);
extern void func_80015258(AcPos *muzzle, void *vel, void *arg, AcPos *at, AcPos *delta,
                          AcPos *tmp, AcPos *out, s32 lead);
s32 func_801C7B1C(Func801C7B1CObj *o, Func801C7B1CWeapon *w, s32 flags, void *arg)
{
    Func801C7B1CHit hit;
    AcPos muzzle;
    AcPos delta;
    AcPos tmp;
    AcPos out;

    if (!func_800745D0(o, &muzzle, w->b5, w->b6, 1, 1))
        goto fail;
    if (!func_80073C2C(o->target, &muzzle, o->yaw, 0, (u16)flags, o->faction, w->yaw_min,
                       -w->yaw_max, w->pitch_min, -w->pitch_max, w->range, &hit)
        || !(hit.pos.w & 2))
        goto miss;
    if (o->shots >= w->burst) {
        if (w->mode == 1) {
            if (o->rounds < w->rounds)
                o->rounds++;
            o->shots = 0;
        }
    } else {
        o->shots++;
    }
    if (w->lead) {
        delta.x = hit.pos.x - o->last.x;
        delta.y = hit.pos.y - o->last.y;
        delta.z = hit.pos.z - o->last.z;
        o->last = hit.pos;
        func_80015258(&muzzle, o->x10, arg, &hit.pos, &delta, &tmp, &out, w->lead);
    } else {
        out = hit.aim;
    }
    if (w->mode) {
        o->aim_x = out.x;
        o->aim_y = -out.y;
        o->aim_z = 0;
    } else {
        o->aim_x = out.x;
        o->aim_z = -out.y;
        o->aim_y = 0;
    }
    return 1;
miss:
    o->shots = 0;
    o->rounds = 0;
fail:
    o->aim_z = 0;
    o->aim_y = 0;
    o->aim_x = 0;
    return 0;
}
