#include "ac1_types.h"
/* ovl202 0x8007B160  size 1000: per-frame update of a ground unit: pad
 * steering, hover effects, then an 8-way move / brake selected by state
 * 48..56 (jump table at 0x8004B4F0), then the path/end callback. */
typedef struct {
    s16 vx, vy, vz, pad;
} SVECTOR;
typedef struct Func8007B160Obj Func8007B160Obj;
struct Func8007B160Obj {
    u8 pad0[8];
    u8 f08[2];          /* +0x08 */
    s16 f0A;            /* +0x0A */
    u8 pad0C[0x12 - 0xC];
    s16 dir;            /* +0x12 */
    u8 pad14[0x2A - 0x14];
    u16 f2A;            /* +0x2A */
    u8 pad2C[2];
    s16 f2E;            /* +0x2E */
    u8 pad30[0x36 - 0x30];
    u16 flags;          /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 state;           /* +0x3C */
    u8 pad3D[0x41 - 0x3D];
    u8 arg;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*cb)(Func8007B160Obj *, u8); /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 f64[0xCC - 0x64]; /* +0x64 */
    s16 vx;             /* +0xCC */
    s16 padCE;
    s16 vz;             /* +0xD0 */
};
extern struct {
    u16 accel;          /* 0x800411B0 */
    u16 maxspd;         /* 0x800411B2 */
    u16 b4;             /* 0x800411B4 */
    u16 b6;             /* 0x800411B6 */
} D_800411B0;
extern u16 D_80040FC0;
extern u8 D_80041250;
extern u8 D_80041251;
extern u8 D_80041252;
extern u8 D_80041253;
extern u8 *D_801A5DC0;
extern void func_80079798(Func8007B160Obj *, int, int);
extern void func_8007A19C(Func8007B160Obj *);
extern void func_8007A058(Func8007B160Obj *);
extern void func_8007A02C(Func8007B160Obj *);
extern void func_80078250(Func8007B160Obj *, int, int, int, int, int, int, void *);
extern void func_8007832C(Func8007B160Obj *, int, int);
extern void func_80014728(int, SVECTOR *);
extern int func_8004F080(void *, void *);
extern int rand(void);
extern long SquareRoot0(long a);

void func_8007B160(Func8007B160Obj *o)
{
    SVECTOR sv;
    int len;
    int d;

    if (o->state != 32) {
        o->flags &= ~0x10;
    }
    func_80079798(o, D_800411B0.b4, D_800411B0.b6);
    if (!(D_80040FC0 & 1)) {
        func_8007A19C(o);
        func_8007A058(o);
    } else {
        func_8007A02C(o);
    }
    if (o->f2E <= o->f0A && o->f2E >= o->f0A - o->f2A) {
        func_80078250(o, D_80041250, D_80041251, 0, 160, (rand() & 0x7F) + 300, 66, D_801A5DC0 + 0x1F00);
        func_80078250(o, D_80041252, D_80041253, 0, 160, (rand() & 0x7F) + 300, -1, 0);
    } else {
        func_8007832C(o, D_80041250, D_80041251);
        func_8007832C(o, D_80041252, D_80041253);
    }
    switch (o->state) {
    case 48:
        func_80014728(o->dir, &sv);
    accel:
        o->vx += (sv.vx * D_800411B0.accel) >> 12;
        o->vz += (sv.vz * D_800411B0.accel) >> 12;
        d = SquareRoot0(o->vx * o->vx + o->vz * o->vz);
        if (d > D_800411B0.maxspd) {
            o->vx = o->vx * D_800411B0.maxspd / d;
            o->vz = o->vz * D_800411B0.maxspd / d;
        }
        break;
    case 49:
        func_80014728(o->dir + 512, &sv);
        goto accel;
    case 50:
        func_80014728(o->dir - 512, &sv);
        goto accel;
    case 53:
        func_80014728(o->dir + 2048, &sv);
        goto accel;
    case 54:
        func_80014728(o->dir + 1536, &sv);
        goto accel;
    case 55:
        func_80014728(o->dir + 2560, &sv);
        goto accel;
    case 51:
        func_80014728(o->dir + 1024, &sv);
        goto accel;
    case 52:
        func_80014728(o->dir - 1024, &sv);
        goto accel;
    case 56:
        len = SquareRoot0(o->vx * o->vx + o->vz * o->vz);
        d = len - D_800411B0.accel;
        if (d <= 0) {
            o->vx = o->vz = 0;
        } else {
            o->vx = o->vx * d / len;
            o->vz = o->vz * d / len;
        }
        break;
    }
    if (func_8004F080(o->f64, o->f08) == 0) {
        if (o->cb != 0) {
            o->cb(o, o->arg);
        }
    }
}
