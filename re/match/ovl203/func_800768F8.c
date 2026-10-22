/* ovl203 func_800768F8: versus weapon fire dispatcher (variadic).  For each of the n
 * weapon records (w, a count, then per-record extra ints on the stack), get the muzzle
 * pair (func_800732FC) and the shot direction, then spawn by the record's class
 * (flags & 0x7000) and sub-type (flags & 0xF00). */
#include "ac1/ac1.h"

typedef struct {
    u8 a;               /* +0 */
    s16 b;              /* +2 */
    u8 c;               /* +4 */
} V768Shot;

typedef struct {
    u16 flags;          /* +0x00 */
    u16 unk2;           /* +0x02 */
    s16 unk4;           /* +0x04 */
    s16 unk6;           /* +0x06 */
    s16 unk8;           /* +0x08 */
    s16 unkA;           /* +0x0A */
    s16 unkC;           /* +0x0C */
    s16 unkE;           /* +0x0E */
    s16 unk10;          /* +0x10 */
    u8 unk12[6];        /* +0x12 */
    u8 unk18[4];        /* +0x18 */
    u8 unk1C[4];        /* +0x1C */
} V768Wpn;

typedef struct {
    u8 pad0[0x12];
    u16 yaw;            /* +0x12 */
    u8 pad14[0x60 - 0x14];
    s32 unk60;          /* +0x60 */
    u8 pad64[0x76 - 0x64];
    u16 unk76;          /* +0x76 */
    u16 unk78;          /* +0x78 */
    s16 unk7A;          /* +0x7A */
} V768Obj;

extern s32 func_800732FC();
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern void func_80014AE4(s32 dx, s32 dy, s32 dz, SVECTOR *ang);
extern void func_80014338(SVECTOR *v, s32 a, SVECTOR *out);
extern void VectorNormalSS(SVECTOR *v, SVECTOR *out);
extern void func_80084B44();
extern void func_80084D98();
extern void func_80084F94();
extern void func_800850F4();
extern void func_80086A98();
extern void func_80087660();
extern void func_80087E08();
extern void func_800870B8();

void func_800768F8(V768Obj *o, s32 a, s32 b, s32 c, V768Wpn *w, s32 n)
{
    s32 *ap;
    s32 fx;
    s32 x, y;
    s32 k;
    SVECTOR pr[2];
    SVECTOR spare;
    SVECTOR d;
    SVECTOR ang;
    V768Shot t;

    ap = &n;
    t.a = a;
    t.b = b;
    t.c = c;
    while (--n != -1) {
        if ((w->flags & 0x8000) || o->unk60 == 0)
            goto alt;
        x = *++ap;
        y = *++ap;
        if (func_800732FC(o, pr, w->flags & 0xFF, w->unk2, 2, x, y) == 0)
            continue;
        d.vx = pr[1].vx - pr[0].vx;
        d.vy = pr[1].vy - pr[0].vy;
        d.vz = pr[1].vz - pr[0].vz;
    dispatch:
        switch (w->flags & 0x7000) {
        case 0x0000:
            if (fx != 255)
                fx = w->unk10;
            switch (w->flags & 0xF00) {
            case 0x000:
                func_80084B44(o, &t, pr, &d, w->unk4, w->unk6, w->unkC, w->unk12,
                              w->unk8, w->unkA, w->unkE, w->unk18, w->unk1C, 0, fx);
                break;
            case 0x100:
                func_80084D98(o, &t, pr, &d, w->unk4, w->unk6, w->unkC,
                              w->unk8, w->unkA, w->unkE, w->unk18, w->unk1C, 0, fx);
                break;
            case 0x800:
                func_80014AE4(d.vx, d.vy, d.vz, &ang);
                ang.vx += *++ap;
                ang.vy += *++ap;
                func_80084F94(o, &t, pr, &ang, &d, w->unkC, w->unkA, w->unkE, *++ap);
                break;
            case 0x900:
                func_80086A98(o, pr, &d, &t, w->unkA, 52, 0);
                break;
            case 0xA00:
                func_80014AE4(d.vx, d.vy, d.vz, &ang);
                func_80014338(&d, w->unk4, &d);
                func_800850F4(o, &t, pr, &ang, &d, w->unkC, w->unkA, w->unkE);
                break;
            }
            fx = 255;
            break;
        case 0x2000:
            func_80087660(o, pr, &d, &t, w->unk6, (w->flags & 0xF00) >> 8);
            break;
        case 0x4000:
            func_80014AE4(d.vx, d.vy, d.vz, &ang);
            func_80087E08(o, pr, &d, &t, w->unk6, (w->flags & 0xF00) >> 8, 0, &ang);
            break;
        case 0x6000:
            func_800870B8(o, pr, &d, &t, w->unk6, (w->flags & 0xF00) >> 8);
            break;
        case 0x7000:
            k = (s16)(w->unk6 / 600);
            VectorNormalSS(&d, &pr[1]);
            pr[1].vx = (pr[1].vx * 75) >> 9;
            pr[1].vy = (pr[1].vy * 75) >> 9;
            pr[1].vz = (pr[1].vz * 75) >> 9;
            func_80014AE4(d.vx, d.vy, d.vz, &ang);
            func_800850F4(o, &t, pr, &ang, &pr[1], 49, 0, k);
            break;
        }
        continue;
    alt:
        x = *++ap;
        y = *++ap;
        if (func_800732FC(o, pr, w->flags & 0xFF, w->unk2, 1, x) == 0)
            continue;
        ang.vx = -o->unk76;
        if (o->unk7A != 0)
            ang.vy = o->yaw - o->unk7A;
        else
            ang.vy = o->yaw - o->unk78;
        func_80014774(&ang, &d);
        goto dispatch;
    }
}
