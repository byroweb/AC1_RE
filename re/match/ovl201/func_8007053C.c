#include "ac1_types.h"

typedef struct {
    u8 r, g, b, cd;
} CVECTOR;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))
#define getaddr(p) (u32)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setlen(p, _len) (((P_TAG *)(p))->len = (u8)(_len))
#define setcode(p, _code) (((P_TAG *)(p))->code = (u8)(_code))
#define setTile(p) setlen(p, 3), setcode(p, 0x60)

typedef struct {
    u8 pad[0x18];
    u32 *ot[2];             /* +0x18 UiObj ordering tables */
} EmbOtObj;

typedef struct {
    u8 pad[0x7C];
    s16 x0;                 /* +0x7C first canvas column on screen */
    s16 y0;                 /* +0x7E */
} EmbView;

typedef struct {
    u8 pad[0x8];
    u8 zoom;                /* +0x08 */
    u8 pad9[0x13];
    EmbOtObj *cursor;       /* +0x1C */
    EmbView *view;          /* +0x20 */
} EmbImage;

typedef struct {
    u16 x;      /* +0x00 */
    u16 y;      /* +0x02 */
    u8 pad4[4];
    u16 w;      /* +0x08 */
    u16 h;      /* +0x0A */
    u8 padC[103512 - 0xC];
} Screen;

typedef struct {
    u8 player;  /* +0x00 */
    u8 pad[3];
    TILE *prim; /* +0x04 */
} Gpu;

extern Gpu D_801EF6C8;
extern Screen D_801BCEEC[];

void func_8007053C(EmbImage *img, s16 x, s32 y, s16 layer, CVECTOR *rgb, s16 big)
{
    EmbView *v = img->view;
    u16 zoom = img->zoom;
    TILE *prim;
    s16 vx, vy;
    s32 ox, oy, n, z, t;

    if ((u16)x < 64 && (u16)y < 64) {
        vx = v->x0;
        vy = v->y0;
        ox = -D_801BCEEC[D_801EF6C8.player].w + D_801BCEEC[D_801EF6C8.player].x;
        oy = -D_801BCEEC[D_801EF6C8.player].h + D_801BCEEC[D_801EF6C8.player].y;
        prim = D_801EF6C8.prim;
        D_801EF6C8.prim = prim + 1;
        setTile(prim);
        prim->h = 1;
        prim->w = 1;
        prim->x0 = x + (s16)(ox + 40);
        prim->y0 = y + (s16)(oy + 48);
        *(CVECTOR *)&prim->r0 = *rgb;
        addPrim(&img->cursor->ot[D_801EF6C8.player][layer], prim);
        if (big && x >= vx && (s16)y >= vy) {
            t = zoom >> 1;
            t = (u16)t;
            n = 64 / t;
            z = (s16)zoom;
            z = (s16)z;
            if (x < vx + n && (s16)y < vy + n) {
                prim = D_801EF6C8.prim;
                D_801EF6C8.prim = prim + 1;
                setTile(prim);
                prim->h = zoom;
                prim->w = zoom;
                prim->x0 = (x - vx) * z + (s16)(ox + 172);
                prim->y0 = ((s16)y - vy) * z + (s16)(oy + 36);
                *(CVECTOR *)&prim->r0 = *rgb;
                addPrim(&img->cursor->ot[D_801EF6C8.player][layer], prim);
            }
        }
    }
}
