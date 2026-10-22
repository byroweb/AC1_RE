/* ovl203 func_80079144: versus walker/ground mover step.  Face the lock target, run
 * the per-state (+0x6E) helpers; in states 7..11 aim, and while the fire buttons
 * (player +0x29A) are held with the shot latch (+0x48) armed, fire the pattern picked
 * by player +0x285 (func_80087E08 along yaw-128 / yaw / yaw+128).  State 6 refreshes the
 * aim pair; from 12 on the velocity is reset, else braked unless +0x290 buttons. */
#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    s16 a, b, c;
} V79Trip;

typedef struct V79Obj V79Obj;
struct V79Obj {
    u8 pad0[0x08];
    SVECTOR pos;        /* +0x08 */
    s16 unk10;          /* +0x10 */
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
    u8 pad38[0x41 - 0x38];
    u8 state;           /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_state)(V79Obj *, u8);  /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0x66 - 0x64]; /* +0x64 AcMover */
    u8 unk66;           /* +0x66 */
    u8 pad67[0x6E - 0x67];
    s16 st;             /* +0x6E */
    u8 pad70[0xCC - 0x70];
    SVECTOR vel;        /* +0xCC */
};

typedef struct {
    u8 pad0[0x08];
    u32 pad_cur;        /* +0x008 */
    u8 pad0c[0x38 - 0x0C];
    SVECTOR aim[2];     /* +0x038 */
    u8 fire_latch;      /* +0x048 */
    u8 pad49[0x84 - 0x49];
    V79Obj *tgt;        /* +0x084 */
    u8 pad88[0x216 - 0x88];
    s16 speed_cap;      /* +0x216 */
    u8 pad218[0x257 - 0x218];
    u8 unk257;          /* +0x257 */
    u8 pad258[0x285 - 0x258];
    u8 pattern;         /* +0x285 */
    u8 pad286[0x290 - 0x286];
    u16 brake_mask;     /* +0x290 */
    u8 pad292[0x29A - 0x292];
    u16 fire_mask;      /* +0x29A */
    u8 pad29c[0x2AA - 0x29C];
    u8 aim_arg[2];      /* +0x2AA */
    u8 pad2ac[0x2B8 - 0x2AC];
    V79Trip trip;       /* +0x2B8 */
} V79P;

#define P ((V79P *)D_80039D18)

extern V79Trip D_8004B15C;
extern V79Trip D_8004B164;
extern V79Trip D_8004B16C;
extern SVECTOR D_80039C78;

extern s32 ratan2(s32 y, s32 x);
extern s16 func_800132CC(s16 a, s16 b, s16 c);
extern void func_80014728(s32 angle, SVECTOR *dir);
extern void func_80078C8C(V79Obj *o);
extern void func_80078B38(V79Obj *o);
extern void func_80078B0C(V79Obj *o);
extern void func_80078BA8(V79Obj *o);
extern void func_80078BCC(V79Obj *o);
extern void func_8004D04C(s32 a, SVECTOR *pos, s32 b, s32 c);
extern void func_80078FE4(V79Obj *o, V79Trip *t);
extern s32 func_800732FC(V79Obj *o, SVECTOR *aim, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void func_80087E08(V79Obj *o, SVECTOR *aim, SVECTOR *dir, V79Trip *t, s32 a, s32 b, s32 c, s16 *d);
extern s32 func_800143B8(SVECTOR *vel, s32 accel, s32 max_speed, SVECTOR *out);
extern s32 func_8004F29C(void *mover, SVECTOR *pos);

void func_80079144(V79Obj *o)
{
    V79Obj *tgt;
    SVECTOR d[3];
    s16 yaw;
    u8 k;
    s32 i;
    u16 st;

    tgt = P->tgt;
    if (tgt != 0)
        o->yaw = func_800132CC(o->yaw, -ratan2(tgt->pos.vx - o->pos.vx, tgt->pos.vz - o->pos.vz), P->speed_cap);
    if (o->st == 0)
        P->fire_latch = 1;
    if ((u16)(o->st - 3) < 9) {
        func_80078C8C(o);
        func_80078B38(o);
    } else {
        func_80078B0C(o);
    }
    if ((u16)o->st < 21) {
        if (o->st == 8)
            func_8004D04C(15, &o->pos, 120, 0);
        func_80078BA8(o);
    } else {
        func_80078BCC(o);
    }
    if ((u16)(o->st - 7) < 5) {
        func_80078FE4(o, &P->trip);
        if (P->unk257 != 0 && (k = P->fire_latch) == 1 && (P->pad_cur & P->fire_mask)) {
            yaw = o->yaw;
            func_80014728(yaw - 128, &d[0]);
            func_80014728(yaw, &d[1]);
            func_80014728(yaw + 128, &d[2]);
            switch (P->pattern) {
            case 0: {
                V79Trip t = D_8004B15C;
                func_80087E08(o, P->aim, &d[1], &t, 3600, 7, 49, &o->unk10);
                break;
            }
            case 1: {
                V79Trip t = D_8004B164;
                func_80087E08(o, P->aim, &d[1], &t, 3600, 7, 49, &o->unk10);
                break;
            }
            case 2: {
                V79Trip t = D_8004B15C;
                for (i = 2; i != -1; i--)
                    func_80087E08(o, P->aim, &d[i], &t, 3600, 7, 49, &o->unk10);
                break;
            }
            case 3: {
                V79Trip t = D_8004B16C;
                func_80087E08(o, P->aim, &d[1], &t, 10000, 6, 49, &o->unk10);
                break;
            }
            }
        }
    } else if (o->st == 6) {
        func_800732FC(o, P->aim, P->aim_arg[0], P->aim_arg[1], 2, 0, 1);
    } else if (o->st >= 12 && o->state != 0) {
        o->unk66 = 1;
        o->faction &= 0xFFEF;
        func_80078BCC(o);
    }
    if (o->st >= 12) {
        o->vel = D_80039C78;
    } else if (o->st >= 7 && !(P->pad_cur & P->brake_mask)) {
        func_800143B8(&o->vel, 1280, 7680, &o->vel);
    }
    if (!func_8004F29C(o->mover, &o->pos) && o->fn_state)
        o->fn_state(o, o->state);
    if (P->pad_cur & P->fire_mask)
        P->fire_latch = 0;
}
