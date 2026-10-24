#include "ac1/ac1.h"

typedef struct {
    u32 c[2];
} SkyCol;

typedef struct {
    SVECTOR pos;                /* +0x00 (0x801B4F8C) */
    SVECTOR rot;                /* +0x08 (0x801B4F94) */
} SkyPose;

extern s32 func_8005D928(s32 src, SkyCol *out);
extern void func_8005D784(s32 x, s32 y, s32 w, s32 h, SkyCol *c, s32 z, s32 abe);
extern void func_8005D5B0(s32 x, s32 y, s32 w, s32 h, SkyCol *c0, SkyCol *c1, SkyCol *c2, SkyCol *c3,
                          s32 z, s32 abe);
extern void func_8005D954(DVECTOR *pos, SprInfo *s, u32 *rgb, s32 z, s32 abe, s32 w, s32 h);
extern s32 ratan2(s32 y, s32 x);
extern MATRIX D_801B510C;
extern SVECTOR D_80089F58;
extern SprInfo D_80089F60;
extern SprInfo D_80089F6C;
extern u32 D_80089F78;
extern u8 D_80089F7C[];
extern SkyPose D_801B4F8C;

/* backdrop: sky gradient above the horizon, scrolling horizon sprite strip,
 * ground colour below (flags: 1 sky, 2 strip, 4 ground, 8 tall strip) */
void func_8005DAFC(s32 roll, u8 fl, s32 src)
{
    VECTOR v;
    SkyCol col[3];
    DVECTOR pos;
    SprInfo *tex;
    s32 h, hz, top, sky, th, a, f;
    u8 *p;

    func_8005D928(func_8005D928(func_8005D928(src, &col[0]), &col[1]), &col[2]);
    ApplyMatrix(&D_801B510C, &D_80089F58, &v);
    roll >>= 9;
    if (v.vz < 1024) {
        if (v.vy > 0) {
            h = 256;
        } else {
            h = -256;
        }
    } else {
        h = (v.vy * 160) / v.vz;
    }
    { s32 t = h + 62; hz = t + roll; }
    h = hz;
    if (fl & 2) {
        if (fl & 8) {
            th = 64;
            tex = &D_80089F6C;
            h = hz - 63;
        } else {
            th = 32;
            tex = &D_80089F60;
        }
    }
    sky = h - 384;
    if (sky > 0 && (fl & 1)) {
        if (sky >= 512) {
            sky = 511;
        }
        func_8005D784(0, 0, 158, sky, &col[0], 8191, -1);
    }
    if (hz > 0) {
        if (hz - th < 120 && (fl & 2)) {
            pos.vy = hz - th;
            a = ratan2(D_801B4F8C.rot.vx - D_801B4F8C.pos.vx, D_801B4F8C.rot.vz - D_801B4F8C.pos.vz) >> 1;
            f = a & 0x1FF;
            p = &D_80089F7C[f >> 7];
            f = a & 0x7F;
            pos.vx = 0;
            tex->u = f;
            tex->w = -128 - f;
            tex->v = *p++;
            func_8005D954(&pos, tex, &D_80089F78, 8191, 0, tex->w, th);
            pos.vx += tex->w;
            tex->u = 0;
            tex->w = 128;
            tex->v = *p++;
            func_8005D954(&pos, tex, &D_80089F78, 8191, 0, tex->w, th);
            pos.vx += 128;
            if (f < 64) {
                tex->w = f + 64;
                tex->v = *p;
                func_8005D954(&pos, tex, &D_80089F78, 8191, 0, tex->w, th);
            } else {
                tex->v = *p++;
                func_8005D954(&pos, tex, &D_80089F78, 8191, 0, tex->w, th);
                pos.vx += 128;
                tex->w = f - 64;
                tex->v = *p;
                func_8005D954(&pos, tex, &D_80089F78, 8191, 0, tex->w, th);
            }
        }
        if (sky < 120 && (fl & 1)) {
            if (sky < 0) {
                sky = 0;
            }
            func_8005D5B0(0, sky, 158, h, &col[0], &col[0], &col[1], &col[1], 8191, -1);
        }
    }
    if (hz < 120 && (fl & 4)) {
        if (hz < 0) {
            hz = 0;
        }
        func_8005D784(0, hz, 158, 120, &col[2], 8191, -1);
    }
}
