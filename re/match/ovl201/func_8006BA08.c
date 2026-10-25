#include "ac1/ac1.h"

typedef struct { u8 r, g, b, a; } EmbRgb;

typedef struct EmbImg {
    u8 pad[0x8];
    u8 zoom;            /* +0x08 (edit +0x84) */
    u8 pad9[0xA];
    s8 fade;            /* +0x13 (edit +0x8F) 1 fade in, else fade out; 1/-1 when done */
} EmbImg;

typedef struct EmbCur {
    u8 pad[2];
    s16 x;              /* +0x02 (obj +0x7E) */
    s16 y;              /* +0x04 (obj +0x80) */
} EmbCur;

typedef struct EmbVw {
    s16 x0;             /* +0x00 (obj +0x7C) */
    s16 y0;             /* +0x02 (obj +0x7E) */
} EmbVw;

typedef struct EmbEdit {
    u8 hdr[0x7C];
    EmbImg img;         /* +0x7C */
    u8 pad90[0x88 - 0x90 + 0x14];
} EmbEditX;

typedef struct EmbEd {
    u8 hdr[0x7C];
    u8 w[0xC];          /* +0x7C */
    u32 *pixels;        /* +0x88 */
    u8 pad8c[0xC];
    UiObj *cursor;      /* +0x98 */
    UiObj *view;        /* +0x9C */
} EmbEd;

extern EmbRgb D_8017E5D8[];     /* editor palette */
extern EmbRgb D_800B7E48;       /* background / colour 0 */
extern EmbRgb D_800B7E54;
extern EmbRgb D_800B7E58;
extern RECT D_800B80FC;
extern RECT D_800B7E4C;
extern MATRIX D_800B8DC4[];     /* light matrices */
int LoadImage(RECT *, u32 *);
int DrawSync(int);
extern void func_8006C554(EmbEd *e);
extern void func_8006C934(EmbEd *e);
extern u8 func_8006C4C0(EmbImg *img, s32 x, s32 y);

void func_8006BA08(EmbEd *e)
{
    EmbImg *img = (EmbImg *)e->w;
    EmbCur *c = (EmbCur *)&e->cursor->w;
    EmbVw *v = (EmbVw *)&e->view->w;
    u16 out[16];
    s16 i;
    u8 n;
    s16 x, y;
    u32 t;
    u8 ci;
    EmbRgb *pal;
    u8 r0;

    LoadImage(&D_800B80FC, e->pixels);
    for (i = 0; i < 16; i++) {
        if (i == 0) {
            out[i] = 0;
        } else {
            out[i] = 0;
            out[i] = (D_8017E5D8[i].b >> 3) << 10 | (D_8017E5D8[i].g >> 3) << 5 | (D_8017E5D8[i].r >> 3);
            if (D_8017E5D8[i].r == 0 && D_8017E5D8[i].g == 0 && D_8017E5D8[i].b == 0)
                out[i] |= 0x8000;
        }
    }
    LoadImage(&D_800B7E4C, (u32 *)out);
    func_8006C554(e);
    func_8006C934(e);
    D_800B7E58.r += 16;
    D_800B7E58.g += 16;
    D_800B7E54.r += 8;
    D_800B7E54.g += 16;
    D_800B7E54.b += 24;
    if (img->fade == 1) {
        u32 sr, sg, sb;
        __asm__("" : "=r"(pal) : "0"(D_8017E5D8));
        r0 = pal->r;
        t = r0 >> 4;
        sr = 1;
        if (t) {
            sr = t;
        }
        t = pal->g >> 4;
        sg = 1;
        if (t) {
            sg = t;
        }
        t = pal->b >> 4;
        sb = 1;
        if (t) {
            sb = t;
        }
        if (r0 - D_800B7E48.r >= (s32)sr) {
            D_800B7E48.r += sr;
        } else {
            D_800B7E48.r = r0;
        }
        if (D_8017E5D8[0].g - D_800B7E48.g >= (s32)sg) {
            D_800B7E48.g += sg;
        } else {
            D_800B7E48.g = D_8017E5D8[0].g;
        }
        if (D_8017E5D8[0].b - D_800B7E48.b >= (s32)sb) {
            D_800B7E48.b += sb;
        } else {
            D_800B7E48.b = D_8017E5D8[0].b;
        }
        if (D_800B7E48.r == D_8017E5D8[0].r && D_800B7E48.g == D_8017E5D8[0].g && D_800B7E48.b == D_8017E5D8[0].b) {
            img->fade = -1;
        }
    } else {
        u32 sr, sg, sb;
        __asm__("" : "=r"(pal) : "0"(D_8017E5D8));
        t = pal->r >> 4;
        sr = 1;
        if (t) {
            sr = t;
        }
        t = pal->g >> 4;
        sg = 1;
        if (t) {
            sg = t;
        }
        t = pal->b >> 4;
        sb = 1;
        if (t) {
            sb = t;
        }
        if (D_800B7E48.r >= sr) {
            D_800B7E48.r -= sr;
        } else {
            D_800B7E48.r = 0;
        }
        if (D_800B7E48.g >= sg) {
            D_800B7E48.g -= sg;
        } else {
            D_800B7E48.g = 0;
        }
        if (D_800B7E48.b >= sb) {
            D_800B7E48.b -= sb;
        } else {
            D_800B7E48.b = 0;
        }
        if (D_800B7E48.r == 0 && D_800B7E48.g == 0 && D_800B7E48.b == 0) {
            img->fade = 1;
        }
    }
    x = v->x0 + (c->x - 172) / img->zoom;
    y = v->y0 + (c->y - 36) / img->zoom;
    D_800B8DC4[10] = D_800B8DC4[0];
    if ((u16)(c->x - 172) < 128 && (u16)(c->y - 36) < 128) {
        ci = func_8006C4C0(img, x, y);
        n = D_8017E5D8[ci].r >= 121;
        if (D_8017E5D8[ci].g >= 121) {
            n++;
        }
        if (D_8017E5D8[ci].b >= 121) {
            n++;
        }
        if (n >= 2) {
            D_800B8DC4[10].m[0][2] = 0;
            D_800B8DC4[10].m[1][2] = 0;
            D_800B8DC4[10].m[2][2] = 0;
        } else {
            D_800B8DC4[10].m[0][2] = 4096;
            D_800B8DC4[10].m[1][2] = 4096;
            D_800B8DC4[10].m[2][2] = 4096;
        }
    }
    DrawSync(0);
}
