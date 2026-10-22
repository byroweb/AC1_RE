#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"
#include "ac1/gfxframe.h"

/* per-player record (0x39C, array at 0x80040FA0) */
typedef struct {
    s32 now;                    /* +0x000 */
    s32 old;                    /* +0x004 */
    s32 x;                      /* +0x008 pad bits used this frame */
    s32 xold;                   /* +0x00C */
    u8 unk10[0x254 - 0x10];
    u32 flags;                  /* +0x254 */
    u8 unk258[0x398 - 0x258];
    CamParm *cam;               /* +0x398 */
} PadRec;

typedef struct {
    u8 unk0[0xC0];
    DRAWENV env;                /* +0xC0 */
    u8 unk11c[0x13C - 0x11C];
} FrameBuf;

extern PadRec D_80040FA0[2];
extern s32 D_801ADA48;
extern FrameBuf D_801ADA4C[2];
extern u8 D_80041BC2;
extern u16 D_80041BBE;
extern u8 D_80041BBB;
extern struct { s32 v; } D_80041BC4;  /* CD play-time accumulator */
extern s32 func_8001628C(void);
extern void func_80077C30(void);
extern void DrawSync(s32 mode);
extern void PutDrawEnv(DRAWENV *env);
extern void DrawPrim(void *p);
extern void SsSetMVol(s32 l, s32 r);
extern s32 CdControl(s32 com, u8 *param, u8 *result);
extern s32 CdRead2(s32 mode);
extern s32 VSync(s32 mode);

/* 2-player pause: freeze until START, draw the PAUSE plate, stop CD audio */
void func_800771D0(void)
{
    POLY_FT4 p;
    POLY_FT4 *pp = &p;
    volatile u8 *cd;
    FrameBuf *fb;
    s32 t;
    s32 x;

    if (!(D_80040FA0[0].x & 0x800)) {
        if (!(D_80040FA0[1].x & 0x800)) {
            return;
        }
    } else {
        D_80040FA0[0].flags &= ~0x10000;
        D_80040FA0[0].cam->mode &= ~1;
    }
    if (D_80040FA0[1].x & 0x800) {
        D_80040FA0[1].flags &= ~0x10000;
        D_80040FA0[1].cam->mode &= ~1;
    }
    DrawSync(0);
    fb = &D_801ADA4C[0];
    if (D_801ADA48 == 0) {
        fb = &D_801ADA4C[1];
    }
    PutDrawEnv(&fb->env);
    pp->r0 = 0;
    pp->g0 = 250;
    pp->b0 = 0;
    pp->clut = 32016;
    pp->tpage = 30;
    pp->x0 = 121;
    pp->y0 = 109;
    pp->x1 = 199;
    pp->y1 = 109;
    pp->x2 = 121;
    pp->y2 = 131;
    pp->x3 = 199;
    pp->y3 = 131;
    pp->u0 = 49;
    pp->v0 = 201;
    pp->u1 = 127;
    pp->v1 = 201;
    pp->u2 = 49;
    pp->v2 = 223;
    pp->u3 = 127;
    pp->v3 = 223;
    setlen(pp, 9);
    setcode(pp, 0x2E);
    DrawPrim(pp);
    DrawSync(0);
    SsSetMVol(0, 0);
    cd = &D_80041BC2;
    if (*cd == 0xFF) {
        do {
        } while (*cd == 0xFF);
    }
    if (D_80041BBE != 0xFFFF) {
        t = func_8001628C();
        while (CdControl(9, 0, 0) == 0) {
        }
    }
    while (1) {
        VSync(2);
        func_80077C30();
        if (!(D_80040FA0[0].x & 0x800) && !(D_80040FA0[1].x & 0x800)) {
            break;
        }
        if (D_80040FA0[0].x == 0x890) {
            D_80040FA0[0].flags |= 0x10000;
        }
        if (D_80040FA0[0].x == 0x860) {
            D_80040FA0[0].cam->mode |= 1;
        }
        x = D_80040FA0[1].x;
        if (x == 0x890) {
            D_80040FA0[1].flags |= 0x10000;
        }
        if (x == 0x860) {
            D_80040FA0[1].cam->mode |= 1;
        }
    }
    do {
        VSync(2);
        func_80077C30();
    } while (!(D_80040FA0[0].x & 0x800) && !(D_80040FA0[1].x & 0x800));
    do {
        VSync(2);
        func_80077C30();
    } while ((D_80040FA0[0].x & 0x800) || (D_80040FA0[1].x & 0x800));
    if (D_80041BBE != 0xFFFF) {
        while (CdRead2(200) == 0) {
        }
        D_80041BC4.v += func_8001628C() - t;
    }
    SsSetMVol(D_80041BBB, D_80041BBB);
    D_80040FA0[0].x = 0;
    D_80040FA0[1].x = 0;
}
