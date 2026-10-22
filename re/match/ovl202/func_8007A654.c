#include "ac1/ac1.h"

typedef struct {
    s16 vx, vy, vz;
} V3;

typedef struct Obj Obj;
struct Obj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 h10[2];          /* +0x10 */
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
    u8 pad38[0x41 - 0x38];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[2];        /* +0x64 AcMover */
    u8 b66;             /* +0x66 */
    u8 pad67[0x6E - 0x67];
    u16 mode;           /* +0x6E */
    u8 pad70[0xCC - 0x70];
    SVECTOR vel;        /* +0xCC */
};

extern Obj *D_80041024;
extern s16 D_800411B6;
extern u8 D_80040FE8;
extern u8 D_800411F7;
extern u8 D_80041225;
extern s32 D_80040FA8;
extern u16 D_8004123A;
extern u16 D_80041230;
extern u8 D_8004124A;
extern u8 D_8004124B;
typedef struct {
    SVECTOR lo;         /* 0x80040FD8 muzzle */
    u8 pad[0x278];
    V3 v;               /* 0x80041258 */
} AimBlk;
extern AimBlk D_80040FD8;
extern SVECTOR D_80039C78;
extern V3 D_8004B4D8;
extern V3 D_8004B4E0;
extern V3 D_8004B4E8;
extern long ratan2(long y, long x);
extern s16 func_800132CC(s16 a, s16 b, s16 c);
extern void func_80014728(s32 angle, SVECTOR *dir);
extern void func_8007A19C(Obj *o);
extern void func_8007A058(Obj *o);
extern void func_8007A02C(Obj *o);
extern void func_8007A0C4(Obj *o);
extern void func_8007A0E4(Obj *o);
extern void func_8004CE44(s32 a, AcPos *p, s32 b, s32 c);
extern void func_8007A4F4(Obj *o, V3 *v);
extern void func_8008915C(Obj *o, SVECTOR *muzzle, SVECTOR *dir, V3 *v, s32 speed, s32 a, s32 b, void *c);
extern s32 func_800745D0(Obj *e, SVECTOR *out, s32 a, s32 b, s32 c, s32 d, s32 f);
extern s32 func_800143B8(SVECTOR *vel, s32 accel, s32 max_speed, SVECTOR *out);
extern s32 func_8004F080(void *mover, AcPos *pos);

void func_8007A654(Obj *o)
{
    SVECTOR d[3];
    Obj *t;
    s32 yaw;
    s32 i;

    t = D_80041024;
    if (t != 0) {
        o->yaw = func_800132CC(o->yaw, -ratan2(t->pos.x - o->pos.x, t->pos.z - o->pos.z), D_800411B6);
    }
    if ((s16)o->mode == 0) {
        D_80040FE8 = 1;
    }
    if ((u16)(o->mode - 3) < 9) {
        func_8007A19C(o);
        func_8007A058(o);
    } else {
        func_8007A02C(o);
    }
    if (o->mode < 21) {
        if ((s16)o->mode == 8) {
            func_8004CE44(15, &o->pos, 120, 0);
        }
        func_8007A0C4(o);
    } else {
        func_8007A0E4(o);
    }
    if ((u16)(o->mode - 7) < 5) {
        func_8007A4F4(o, &D_80040FD8.v);
        if (D_800411F7 != 0 && D_80040FE8 == 1 && (D_80040FA8 & D_8004123A)) {
            yaw = o->yaw;
            func_80014728(yaw - 128, &d[0]);
            func_80014728(yaw, &d[1]);
            func_80014728(yaw + 128, &d[2]);
            switch (D_80041225) {
            case 0: {
                V3 v = D_8004B4D8;
                func_8008915C(o, &D_80040FD8.lo, &d[1], &v, 3600, 7, 49, o->h10);
                break;
            }
            case 1: {
                V3 v = D_8004B4E0;
                func_8008915C(o, &D_80040FD8.lo, &d[1], &v, 3600, 7, 49, o->h10);
                break;
            }
            case 2: {
                V3 v = D_8004B4D8;
                for (i = 2; i != -1; i--) {
                    func_8008915C(o, &D_80040FD8.lo, &d[i], &v, 3600, 7, 49, o->h10);
                }
                break;
            }
            case 3: {
                V3 v = D_8004B4E8;
                func_8008915C(o, &D_80040FD8.lo, &d[1], &v, 10000, 6, 49, o->h10);
                break;
            }
            }
        }
    } else if ((s16)o->mode == 6) {
        func_800745D0(o, &D_80040FD8.lo, D_8004124A, D_8004124B, 2, 0, 1);
    } else if ((s16)o->mode >= 12) {
        if (o->state != 0) {
            o->b66 = 1;
            o->faction &= ~0x10;
            func_8007A0E4(o);
        }
    }
    if ((s16)o->mode >= 12) {
        o->vel = D_80039C78;
    } else if ((s16)o->mode >= 7 && !(D_80040FA8 & D_80041230)) {
        func_800143B8(&o->vel, 1280, 7680, &o->vel);
    }
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state) {
        o->fn_state(o, o->state);
    }
    if (D_80040FA8 & D_8004123A) {
        D_80040FE8 = 0;
    }
}
