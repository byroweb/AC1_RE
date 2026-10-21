#include "ac1/ac1.h"

typedef struct {
    s32 on;                     /* +0x00 */
    s32 value;                  /* +0x04 number shown when flag 8 is clear */
    u8 unk08[4];
    s16 sx;                     /* +0x0C smoothed screen x */
    s16 sy;                     /* +0x0E smoothed screen y */
    SVECTOR pos;                /* +0x10; pos.pad holds the flags */
    u8 unk18[0x0E];
    s16 icon;                   /* +0x26 */
} HudTag;

extern char D_8004ADB0[];
extern void RotTrans(SVECTOR *v0, VECTOR *v1, long *flag);
extern s32 func_800125D0(char *, s32);
extern s32 func_8005145C(s32, s32, s32, s32, s32, char *);
extern void func_800517CC(s32, s32, s32, s32, ...);
extern void func_80051CCC(s32, s32, s16);

void func_80051D14(HudTag *o) {
    VECTOR v;
    long flag;
    s32 sx;
    s32 sy;
    s32 d;

    if (o->on == 0) {
        return;
    }
    RotTrans(&o->pos, &v, &flag);
    if (v.vz <= 0) {
        return;
    }
    sx = (v.vx * 160) / v.vz;
    sy = (v.vy * 160) / v.vz;
    d = (sx - o->sx) >> 2;
    if (d == 0) {
        o->sx = sx;
    } else {
        sx = (o->sx += d);
    }
    d = (sy - o->sy) >> 2;
    if (d == 0) {
        o->sy = sy;
    } else {
        sy = (o->sy += d);
    }
    if ((u32)(sx + 107) < 215 && (u32)(sy + 74) < 149) {
        sx += 79;
        sy += 60;
        if (o->pos.pad & 8) {
            func_80051CCC(sx, sy, 12);
            func_8005145C(sx + 14, sy - 15, 16, 56, 0, D_8004ADB0);
            func_800517CC(14, sx, sy, 2, 8, -6, 14, -10, 30, -10);
        } else {
            char buf[16];
            s32 w;

            func_80051CCC(sx, sy, 11);
            func_800125D0(buf, o->value);
            func_800517CC(13, sx, sy, 2, 8, -6, 14, -10,
                          func_8005145C(sx + 15, sy - 15, 16, 56, 0, buf) + 14, -10);
            if (o->pos.pad & 4) {
                w = func_800125D0(buf, o->icon) * 4 + 13;
                func_800517CC(13, sx, sy, 2, -8, 6, -14, 10,
                              -14 - func_8005145C(sx - w, sy + 5, 16, 56, 0, buf), 10);
            }
        }
    }
}
