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
    u8 pad3d[0x41 - 0x3D];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 */
    u16 vel[3];         /* +0xCC */
    s16 hold;           /* +0xD2 */
};
extern s32 D_801A5DC0;
extern s32 D_801A5DC4;
extern s32 rand(void);
extern s32 SquareRoot0(s32);
extern void func_800143B8(void *, s32, s32, void *);
extern void func_80014728(s32, AcPos *);
extern s32 func_8004F080(void *, void *);
extern void func_80075FC0(void);
extern void func_80078250(Obj *, s32, s32, s32, s32, s32, s32, s32);

void func_801C4CFC(Obj *o)
{
    AcPos dir;
    s32 a;
    s32 b;
    s32 r;

    func_80075FC0();
    switch (D_801A5DC4 % 3) {
    case 0:
        a = 9;
        b = 0;
        break;
    case 1:
        a = 8;
        b = 1;
        break;
    case 2:
        a = 5;
        b = 4;
        break;
    }
    func_80078250(o, a, b, 0, 140, (rand() & 0x7F) + 250, 66, D_801A5DC0 + 0x1F00);
    switch (o->dir) {
    case 1:
        func_80014728(o->yaw, &dir);
tail:
        o->vel[0] += (dir.x * 5) >> 6;
        o->vel[2] += (dir.z * 5) >> 6;
        r = SquareRoot0((s16)o->vel[0] * (s16)o->vel[0] + (s16)o->vel[2] * (s16)o->vel[2]);
        if (r > 3200) {
            o->vel[0] = (s16)o->vel[0] * 3200 / r;
            o->vel[2] = (s16)o->vel[2] * 3200 / r;
        }
        break;
    case 2:
        func_80014728(o->yaw + 512, &dir);
        goto tail;
    case 3:
        func_80014728(o->yaw - 512, &dir);
        goto tail;
    case 6:
        func_80014728(o->yaw + 2048, &dir);
        goto tail;
    case 4:
        func_80014728(o->yaw + 1536, &dir);
        goto tail;
    case 5:
        func_80014728(o->yaw + 2560, &dir);
        goto tail;
    case 7:
        if (o->hold == 0) {
            o->faction &= 0xFFEF;
        } else {
            o->hold--;
        }
        goto tail;
    default:
        func_800143B8(o->vel, 320, 3200, o->vel);
        break;
    }
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state) {
        o->fn_state(o, o->state);
    }
}
