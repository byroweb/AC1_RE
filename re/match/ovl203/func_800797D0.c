/* matched (wave4 a5): target-sink arm does its store then goto the shared call pair (a label there keeps sched1 from hoisting move a0,s1; jump2 cross-jumps the subu/sh); aim[0].vy = aim[1].vy chain after the call */
/* ovl203 func_800797D0: versus hover mover step with a lock target.  With a target
 * (player +0x84), lead it (func_80015258 from the last target position at +0x94), turn
 * toward the lead point and steer the velocity at it, clamped to player +0x20E; without
 * one, sink/climb like func_8007A09C.  Then the state (+0x6E) side effects and the move. */
#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    s16 a;
    u16 b;
    s16 c;
} V797Trip;

typedef struct V797Obj V797Obj;
struct V797Obj {
    u8 pad0[0x08];
    SVECTOR pos;        /* +0x08 */
    u8 pad10[2];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x28 - 0x14];
    u16 r;              /* +0x28 */
    u16 h;              /* +0x2A */
    u8 pad2c[0x41 - 0x2C];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(V797Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0x6E - 0x64]; /* +0x64 AcMover */
    s16 st;             /* +0x6E */
    u8 pad70[0xCC - 0x70];
    u16 vel[3];         /* +0xCC */
};

typedef struct {
    u8 pad0[0x08];
    u32 pad_cur;        /* +0x008 */
    u8 pad0c[0x20 - 0x0C];
    u16 flags20;        /* +0x020 */
    u8 pad22[0x38 - 0x22];
    SVECTOR aim[2];     /* +0x038 */
    u8 pad48[0x84 - 0x48];
    V797Obj *tgt;       /* +0x084 */
    u8 pad88[0x94 - 0x88];
    SVECTOR prev;       /* +0x094 */
    u8 pad9c[0x208 - 0x9C];
    u16 sink;           /* +0x208 */
    u16 floor;          /* +0x20A */
    u16 accel;          /* +0x20C */
    u16 maxv;           /* +0x20E */
    u8 pad210[0x216 - 0x210];
    u16 speed_cap;      /* +0x216 */
    u8 pad218[0x29A - 0x218];
    u16 sink_mask;      /* +0x29A */
    u8 pad29c[0x2AA - 0x29C];
    u8 aim_arg[2];      /* +0x2AA */
    u8 pad2ac[0x2B8 - 0x2AC];
    V797Trip trip;      /* +0x2B8 */
} V797P;

#define P ((V797P *)D_80039D18)

extern void func_80077248(V797Obj *o, s32 a, s32 b, s32 c);
extern void func_80015258(SVECTOR *a, s32 b, s32 c, SVECTOR *d, SVECTOR *e, SVECTOR *f, s32 g, s32 h);
extern s32 ratan2(s32 y, s32 x);
extern s16 func_800132CC(s16 a, s16 b, s16 c);
extern void func_80014728(s32 angle, SVECTOR *dir);
extern void func_80078C8C(V797Obj *o);
extern void func_80078B38(V797Obj *o);
extern void func_80078B0C(V797Obj *o);
extern void func_80078BA8(V797Obj *o);
extern void func_80078BCC(V797Obj *o);
extern void func_8004D04C(s32 a, SVECTOR *pos, s32 b, s32 c);
extern void func_80079624(V797Obj *o, V797Trip *t);
extern s32 func_800732FC(V797Obj *o, SVECTOR *aim, s32 a, s32 b, s32 c, s32 d, s32 e);
extern s32 func_8004F29C(void *mover, SVECTOR *pos);

void func_800797D0(V797Obj *o)
{
    V797Obj *tgt;
    SVECTOR dir;
    SVECTOR a;
    SVECTOR b;
    SVECTOR d;
    SVECTOR r;
    V797Trip t;
    s32 ang;
    s32 k;
    s32 v;
    u16 m;
    u16 s;
    s16 y;

    tgt = P->tgt;
    func_80077248(o, 400, 160, 500);
    if (tgt != 0) {
        a.vx = o->pos.vx;
        a.vy = o->pos.vy - (o->h >> 1);
        a.vz = o->pos.vz;
        b.vx = tgt->pos.vx;
        b.vy = tgt->pos.vy - (tgt->h >> 1);
        b.vz = tgt->pos.vz;
        d.vx = b.vx - P->prev.vx;
        d.vy = b.vy - P->prev.vy;
        d.vz = b.vz - P->prev.vz;
        func_80015258(&a, 0, P->maxv >> 6, &b, &d, &r, 0, 5);
        P->prev = b;
        ang = ratan2(r.vx - o->pos.vx, r.vz - o->pos.vz);
        o->yaw = func_800132CC(o->yaw, -ang, P->speed_cap * 4);
        func_80014728(-ang, &dir);
        k = o->r + tgt->r;
        k = k + (k >> 2);
        r.vx -= (dir.vx * k) >> 12;
        r.vz -= (dir.vz * k) >> 12;
        v = (r.vx - a.vx) << 6;
        if (P->maxv < v)
            o->vel[0] = P->maxv;
        else if (v < -P->maxv)
            o->vel[0] = -P->maxv;
        else
            o->vel[0] = v;
        v = (r.vz - a.vz) << 6;
        if (P->maxv < v)
            o->vel[2] = P->maxv;
        else if (v < -P->maxv)
            o->vel[2] = -P->maxv;
        else
            o->vel[2] = v;
        v = (r.vy - a.vy) << 6;
        if (v > 256)
            goto climb;
        if (v < -P->sink * 4) {
            o->vel[1] -= P->sink * 4;
            goto sink;
        }
        o->vel[1] = v;
    } else if ((P->pad_cur & P->sink_mask) && !(P->flags20 & 1)) {
        o->vel[1] -= P->sink;
    sink:
        func_80078C8C(o);
        func_80078B38(o);
    } else {
    climb:
        o->vel[1] += 256;
        if ((s16)o->vel[1] > 9600)
            o->vel[1] = 9600;
        func_80078B0C(o);
    }
    m = P->floor;
    if ((s16)o->vel[1] < -m)
        o->vel[1] = -m;
    if ((u16)o->st < 11) {
        if (o->st == 5)
            func_8004D04C(15, &o->pos, 120, 0);
        func_80078BA8(o);
    } else {
        func_80078BCC(o);
    }
    if ((u16)(o->st - 6) < 2) {
        t = P->trip;
        t.b *= 4;
        func_80079624(o, &t);
    } else if (o->st == 5) {
        func_800732FC(o, P->aim, P->aim_arg[0], P->aim_arg[1], 2, 0, 1);
        P->aim[0].vy = P->aim[1].vy = o->pos.vy - (o->h >> 1);
    }
    if (!func_8004F29C(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
}
