#include "ac1/ac1.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
} Pos16;

extern s16 D_80041B90;
extern s16 D_80041B92;
extern s16 D_80041B94;
extern s16 D_80041B9A;
extern s32 D_801A5DC4;
extern int rsin(int a);
extern int rcos(int a);
extern long ratan2(long y, long x);
extern void func_8004D558(s32 id, s32 sub, s32 a, s32 b, s32 c);

void func_8004D610(Pos16 *lo, Pos16 *hi, s32 range, s32 sfx, s32 pan, s32 vol, s32 pitch, s32 wobble)
{
    s32 d[6];
    s32 vl;
    s32 vr;
    s32 max;
    s32 i;
    s32 n;
    s32 v;
    s32 a;
    s32 *p;
    s32 *q;
    s32 ang;
    s32 *fc;

    if (pan == 0xFF) {
        pan = 0;
    } else {
        pan += 0x4000;
    }
    d[0] = D_80041B90 - lo->x;
    d[1] = D_80041B92 - lo->y;
    d[2] = D_80041B94 - lo->z;
    d[3] = D_80041B90 - hi->x;
    d[4] = D_80041B92 - hi->y;
    d[5] = D_80041B94 - hi->z;
    if (d[0] < 0 || d[1] < 0 || d[2] < 0 || d[3] > 0 || d[4] > 0 || d[5] > 0) {
        goto outside;
    }
    vl = vol;
    vr = vl;
play:
    if (wobble == 1) {
        fc = &D_801A5DC4;
        pitch += rsin((*fc & 0x7F) << 5) >> 6;
        pitch += rsin(*(u8 *)fc << 4) >> 7;
    }
    func_8004D558(sfx + 110, pan, vl, vr, pitch);
    return;
outside:
    n = -range;
    if (d[0] < n || d[1] < n || d[2] < n || range < d[3] || range < d[4] || range < d[5]) {
        return;
    }
    max = 0;
    p = &d[0];
    q = &d[3];
    for (i = 3; i != 0; i--) {
        a = *p++;
        v = *q++;
        if (a < 0) {
            v = -a;
        } else if (v <= 0) {
            continue;
        }
        if (max < v) {
            max = v;
        }
    }
    vol = (range - max) * vol / range;
    if (vol < 10) {
        return;
    }
    ang = (1024 - ratan2(((lo->x + hi->x) >> 1) - D_80041B90, ((lo->z + hi->z) >> 1) - D_80041B94) - D_80041B9A) & 0xFFF;
    if (ang >= 0x800) {
        ang = 0x1000 - ang;
    }
    ang = (((ang >> 1) - 512) * max) / range + 512;
    vl = (vol * rsin(ang)) >> 12;
    vr = (vol * rcos(ang)) >> 12;
    goto play;
}
