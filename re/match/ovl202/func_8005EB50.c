#include "ac1/ac1.h"
/* ovl202 0x8005EB50 - draw the sky backdrop: the horizon line from the camera pitch
 * (direction D_8008E4EC through matrix D_801AD7EC), an upper flat band (flag 1), a
 * row of horizon sprites scrolled by the camera heading (flag 2, tall with flag 8),
 * the gradient below them (flag 1) and the ground band (flag 4).  Three colours
 * are read from src. */

extern u8 *func_8005E97C(u8 *src, u8 *dst);
extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern s32 ratan2(s32 y, s32 x);
extern void func_8005E7D8(s32 x0, s32 y0, s32 x1, s32 y1, u8 *rgb, s32 z, s32 abr);
extern void func_8005E604(s32 x0, s32 y0, s32 x1, s32 y1, u8 *c0, u8 *c1, u8 *c2, u8 *c3,
                          s32 z, s32 abr);
extern void func_8005E9A8(DVECTOR *pos, SprInfo *s, u32 *rgb, s32 z, s32 abe, s32 w, s32 h);
extern MATRIX D_801AD7EC;
extern SVECTOR D_8008E4EC;
extern SprInfo D_8008E4F4;
extern SprInfo D_8008E500;
extern u32 D_8008E50C;
extern u8 D_8008E510[];
extern s16 D_801AD66C;
extern s16 D_801AD670;
extern s16 D_801AD674;
extern s16 D_801AD678;

void func_8005EB50(s32 h, u8 flags, u8 *src)
{
    VECTOR v;
    u8 c0[8];
    u8 c1[8];
    u8 c2[8];
    DVECTOR pos;
    s32 y;
    s32 hz;
    s32 top;
    s32 sh;
    s32 ang;
    s32 frac;
    u8 *tab;
    SprInfo *spr;
    DVECTOR *pp;
    u32 *rgb;

    src = func_8005E97C(src, c0);
    src = func_8005E97C(src, c1);
    func_8005E97C(src, c2);
    ApplyMatrix(&D_801AD7EC, &D_8008E4EC, &v);
    h >>= 9;
    if (v.vz < 1024) {
        if (v.vy > 0) {
            y = 256;
        } else {
            y = -256;
        }
    } else {
        y = v.vy * 320 / v.vz;
    }
    {
        s32 t = y + 122;
        hz = t + h;
    }
    y = hz;
    if (flags & 2) {
        if (flags & 8) {
            sh = 64;
            spr = &D_8008E500;
            y = hz - 63;
        } else {
            sh = 32;
            spr = &D_8008E4F4;
        }
    }
    top = y - 384;
    if (top > 0 && (flags & 1)) {
        if (top >= 512) {
            top = 511;
        }
        func_8005E7D8(0, 0, 320, top, c0, 8191, -1);
    }
    if (hz > 0) {
        if (hz - sh < 240 && (flags & 2)) {
            pos.vy = hz - sh;
            ang = ratan2(D_801AD674 - D_801AD66C, D_801AD678 - D_801AD670) >> 1;
            frac = ang & 0x1FF;
            tab = &D_8008E510[frac >> 7];
            frac = ang & 0x7F;
            pos.vx = 0;
            pp = &pos;
            rgb = &D_8008E50C;
            spr->u = frac;
            spr->w = -128 - frac;
            spr->v = *tab++;
            func_8005E9A8(pp, spr, rgb, 8191, 0, spr->w, sh);
            pos.vx += spr->w;
            spr->u = 0;
            spr->w = 128;
            spr->v = *tab++;
            func_8005E9A8(pp, spr, rgb, 8191, 0, spr->w, sh);
            pos.vx += 128;
            if (frac < 64) {
                spr->w = frac + 64;
                spr->v = *tab;
                func_8005E9A8(pp, spr, rgb, 8191, 0, spr->w, sh);
            } else {
                spr->v = *tab++;
                func_8005E9A8(pp, spr, rgb, 8191, 0, spr->w, sh);
                pos.vx += 128;
                spr->w = frac - 64;
                spr->v = *tab;
                func_8005E9A8(pp, spr, rgb, 8191, 0, spr->w, sh);
            }
        }
        if (top < 240 && (flags & 1)) {
            if (top < 0) {
                top = 0;
            }
            func_8005E604(0, top, 320, y, c0, c0, c1, c1, 8191, -1);
        }
    }
    if (hz < 240 && (flags & 4)) {
        if (hz < 0) {
            hz = 0;
        }
        func_8005E7D8(0, hz, 320, 240, c2, 8191, -1);
    }
}
