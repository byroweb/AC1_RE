#include "ac1/ac1.h"
/* ovl201 0x80066614 - queue one w x h glyph sprite (texture u, v) at screen
 * (x, y) relative to the view of the object's id (ids 0..20, two or three per
 * view) in the current buffer, lit with NCCS unless o->light is 21 (then
 * plain grey), at the object's depth. */

/* PsyQ inline_c.h-style GTE macros, local to this file. */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 )" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0( %0 )" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop;nop;.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0( %0 )" : : "r"(r0) : "memory")

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SPRT;

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

extern UiGfx D_801EF6C8;
extern UiViewSet D_801BCE88[2];
extern SVECTOR D_800B8D78;
extern u8 D_800B8D80[4];

void func_80066614(UiObj *o, s16 x, s16 y, s16 w, s16 h, s16 u, s16 v)
{
    SPRT *p;
    s16 depth;
    s16 k;

    p = (SPRT *)D_801EF6C8.prim;
    depth = o->unk14;
    D_801EF6C8.prim += sizeof(SPRT);
    setlen(p, 4);
    setcode(p, 0x64);
    switch (o->id) {
    case 0:
    case 1:
    case 2:
        k = 0;
        break;
    case 3:
    case 4:
        k = 1;
        break;
    case 5:
    case 6:
        k = 2;
        break;
    case 7:
    case 8:
        k = 3;
        break;
    case 9:
    case 10:
        k = 4;
        break;
    case 11:
    case 12:
        k = 5;
        break;
    case 13:
    case 14:
        k = 6;
        break;
    case 15:
    case 16:
        k = 7;
        break;
    case 17:
    case 18:
        k = 8;
        break;
    case 19:
    case 20:
        k = 9;
        break;
    }
    p->x0 = D_801BCE88[D_801EF6C8.idx].view[k].x + (x - D_801BCE88[D_801EF6C8.idx].view[k].ox);
    p->y0 = D_801BCE88[D_801EF6C8.idx].view[k].y + (y - D_801BCE88[D_801EF6C8.idx].view[k].oy);
    p->w = w;
    p->h = h;
    p->u0 = u;
    p->v0 = v;
    p->clut = 0x3817;
    if (o->light != 21) {
        gte_ldv0(&D_800B8D78);
        gte_ldrgb(D_800B8D80);
        gte_nccs();
        gte_strgb(&p->r0);
    } else {
        p->b0 = 128;
        p->g0 = 128;
        p->r0 = 128;
    }
    p->code &= ~2;
    addPrim(o->ot[D_801EF6C8.idx] + depth, p);
}
