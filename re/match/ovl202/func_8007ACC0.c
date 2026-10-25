/* ovl202 0x8007ACC0: hover AI chase step (wave8 a3 match). */
#include "ac1/ac1.h"

typedef struct {
    u16 sink;           /* 0x800411A8 */
    u16 floor;          /* 0x800411AA */
    u16 accel;          /* 0x800411AC */
    u16 max_speed;      /* 0x800411AE */
    u8 pad[4];
    u16 a;              /* 0x800411B4 */
    u16 b;              /* 0x800411B6 */
} HoverCfg;

typedef struct {
    s16 vx, vy, vz;
} V3;

typedef struct Obj Obj;
struct Obj {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x28 - 0x14];
    u16 h28;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    u8 pad2c[0x41 - 0x2C];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0x6E - 0x64]; /* +0x64 AcMover */
    u16 mode;           /* +0x6E */
    u8 pad70[0xCC - 0x70];
    s16 vel[3];         /* +0xCC */
};

extern u16 D_800411A8;
extern u16 D_800411AA;
extern u16 D_800411AE;
extern u16 D_800411B6;
extern Obj *D_80041024;
extern s16 D_80041034;
extern s16 D_80041036;
extern s16 D_80041038;
extern V3 D_80041258;
extern s32 D_80040FA8;
extern u16 D_80040FC0;
extern u16 D_8004123A;
extern u8 D_8004124A;
extern u8 D_8004124B;
extern SVECTOR D_80040FD8;
extern s16 D_80040FDA;
extern s16 D_80040FE2;
extern void func_800784F8(Obj *o, s32 a, s32 b, s32 c);
extern void func_80015258(SVECTOR *a, s32 b, s32 c, SVECTOR *d, SVECTOR *e, SVECTOR *f, s32 g, s32 h);
extern long ratan2(long y, long x);
extern s16 func_800132CC(s16 a, s16 b, s16 c);
extern void func_80014728(s32 angle, SVECTOR *dir);
extern void func_8007A19C(Obj *o);
extern void func_8007A058(Obj *o);
extern void func_8007A02C(Obj *o);
extern void func_8007A0C4(Obj *o);
extern void func_8007A0E4(Obj *o);
extern void func_8004CE44(s32 a, AcPos *p, s32 b, s32 c);
extern void func_8007AB04(Obj *o, V3 *v);
extern s32 func_800745D0(Obj *e, SVECTOR *out, s32 a, s32 b, s32 c, s32 d, s32 f);
extern s32 func_8004F080(void *mover, AcPos *pos);

void func_8007ACC0(Obj *o)
{
    SVECTOR dir;
    SVECTOR a;
    SVECTOR b;
    SVECTOR c;
    SVECTOR out;
    V3 v;
    Obj *t;
    s32 ang;
    s32 k;
    s32 d;
    s32 e;
    s32 x;
    s32 w;
    s32 q;
    u16 z;
    s32 s;

    t = D_80041024;
    func_800784F8(o, 400, 160, 500);
    if (t != 0) {
        a.vx = o->pos.x;
        a.vy = o->pos.y - (o->h2a >> 1);
        a.vz = o->pos.z;
        b.vx = t->pos.x;
        b.vy = t->pos.y - (t->h2a >> 1);
        b.vz = t->pos.z;
        c.vx = b.vx - D_80041034;
        c.vy = b.vy - D_80041036;
        c.vz = b.vz - D_80041038;
        func_80015258(&a, 0, D_800411AE >> 6, &b, &c, &out, 0, 5);
        *(SVECTOR *)&D_80041034 = b;
        ang = -ratan2(out.vx - o->pos.x, out.vz - o->pos.z);
        o->yaw = func_800132CC(o->yaw, ang, D_800411B6 << 2);
        func_80014728(ang, &dir);
        do {
        k = o->h28 + t->h28;
        k += k >> 2;
        q = dir.vx;
        d = dir.vz * k;
        w = (u16)out.vx - ((q * k) >> 12);
        *(volatile s16 *)&out.vx = w;
        x = (s16)w;
        z = *(volatile u16 *)&out.vz;
        q = d >> 12;
        out.vz = z - q;
        } while (0);
        e = (x - a.vx) << 6;
        if (e > D_800411AE) {
            o->vel[0] = D_800411AE;
        } else if (e < -D_800411AE) {
            o->vel[0] = -D_800411AE;
        } else {
            o->vel[0] = e;
        }
        e = (out.vz - a.vz) << 6;
        if (e > D_800411AE) {
            o->vel[2] = D_800411AE;
        } else if (e < -D_800411AE) {
            o->vel[2] = -D_800411AE;
        } else {
            o->vel[2] = e;
        }
        e = (out.vy - a.vy) << 6;
        if (e > 256)
            goto climb;
        if (e < -D_800411A8 * 4) {
            o->vel[1] -= D_800411A8 * 4;
            goto sink;
        }
        o->vel[1] = e;
    } else if ((D_80040FA8 & D_8004123A) && !(D_80040FC0 & 1)) {
        o->vel[1] -= D_800411A8;
    sink:
        func_8007A19C(o);
        func_8007A058(o);
    } else {
    climb:
        o->vel[1] += 256;
        if (o->vel[1] > 9600)
            o->vel[1] = 9600;
        func_8007A02C(o);
    }
clamp:
    if (o->vel[1] < -D_800411AA) {
        o->vel[1] = -D_800411AA;
    }
    if (o->mode < 11) {
        if ((s16)o->mode == 5) {
            func_8004CE44(15, &o->pos, 120, 0);
        }
        func_8007A0C4(o);
    } else {
        func_8007A0E4(o);
    }
    if ((u16)(o->mode - 6) < 2) {
        v = D_80041258;
        v.vy <<= 2;
        func_8007AB04(o, &v);
    } else if ((s16)o->mode == 5) {
        func_800745D0(o, &D_80040FD8, D_8004124A, D_8004124B, 2, 0, 1);
        D_80040FDA = D_80040FE2 = o->pos.y - (o->h2a >> 1);
    }
    if (!func_8004F080(o->mover, &o->pos) && o->fn_state) {
        o->fn_state(o, o->state);
    }
}
