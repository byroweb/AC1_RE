#include "ac1/ac1.h"
/* ovl201 0x8007F30C - queue a 15x15 TILE in the colour picked by the owner's two letter codes */

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

typedef struct {
    u8 idx;                     /* 0x801EF6C8 ordering table in use */
    u8 pad[3];
    u8 *prim;                   /* 0x801EF6CC primitive cursor */
} UiGfx;

typedef struct {                /* 100 bytes: one 2D view */
    s16 x, y;                   /* +0x00 screen origin */
    u8 pad04[4];
    s16 ox, oy;                 /* +0x08 world point at the origin */
    u8 pad0C[0x58];
} UiView2D;

typedef struct {                /* 103512 bytes per buffer */
    UiView2D view[1035];
    u8 pad[12];
} UiViewSet;

typedef struct {
    u8 pad7c[9];
    u8 part;                    /* +0x85 */
    u8 slot;                    /* +0x86 */
    u8 pad87[5];
    UiObj *owner;               /* +0x8C */
} PaintTileWork;

extern UiGfx D_801EF6C8;
extern UiViewSet D_801BCE88[2];
extern u8 D_80031A64[4][4][3];

void func_8007F30C(UiObj *o)
{
    UiObj *src;
    PaintTileWork *w;
    UiObj *owner;
    TILE *p;
    u8 a;
    u8 b;
    u8 r, g, bl;

    src = o->w.list.src;
    do {    /* the do/while (0) block decides the register of a (a1) */
        w = (PaintTileWork *)&src->w;
        owner = w->owner;
        switch (w->part) {
    case 'H':
        a = 0;
        break;
    case 'C':
    case 'G':
        a = 1;
        break;
    case 'A':
        a = 2;
        break;
    case 'B':
        a = 3;
        break;
        }
    } while (0);
    switch (w->slot) {
    case 'B':
        b = 0;
        break;
    case 'O':
        b = 1;
        break;
    case 'D':
        b = 2;
        break;
    case 'J':
        b = 3;
        break;
    }
    r = D_80031A64[a][b][0];
    g = D_80031A64[a][b][1];
    p = (TILE *)D_801EF6C8.prim;
    bl = D_80031A64[a][b][2];
    D_801EF6C8.prim += sizeof(TILE);
    setlen(p, 3);
    p->code = 0x60;
    p->x0 = (s16)(D_801BCE88[D_801EF6C8.idx].view[0].x + 170) - D_801BCE88[D_801EF6C8.idx].view[0].ox;
    p->y0 = (s16)(D_801BCE88[D_801EF6C8.idx].view[0].y + 189) - D_801BCE88[D_801EF6C8.idx].view[0].oy;
    p->w = 15;
    p->h = 15;
    if (r >= 63) {
        r = 63;
    }
    if (g >= 63) {
        g = 63;
    }
    if (bl >= 63) {
        bl = 63;
    }
    p->r0 = r * 4;
    p->g0 = g * 4;
    p->b0 = bl * 4;
    addPrim(owner->ot[D_801EF6C8.idx] + 1, p);
}
