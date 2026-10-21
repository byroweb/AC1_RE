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
extern void func_800125D0(char *, s32);
extern s32 func_8005145C(s32, s32, s32, s32, s32, char *);
extern void func_800517CC(s32, s32, s32, s32, ...);
extern void func_80051FB0(s32, s32, s16);
extern void func_80051FF8(s32, s32, s32);

void func_80052020(HudTag *o) {
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
            func_80051FB0(sx, sy, 12);
            func_8005145C(sx + 22, sy + 7, 16, 56, 0, D_8004ADB0);
            func_800517CC(14, sx, sy, 2, 16, 4, 22, 12, 38, 12);
            func_80051FF8(sx, sy, o->icon);
        } else {
            char buf[16];

            func_80051FB0(sx, sy, 11);
            func_800125D0(buf, o->value);
            func_800517CC(13, sx, sy, 2, 16, 4, 22, 12,
                          func_8005145C(sx + 23, sy + 7, 16, 56, 0, buf) + 22, 12);
        }
    }
}
