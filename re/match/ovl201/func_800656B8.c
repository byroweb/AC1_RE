#include "ac1/ac1.h"
/* ovl201 0x800656B8 - queue the object's sprite (o->res: uv, clut, size and
 * texture page) opaque at its position in the view of its id, lit
 * with NCCS unless o->light is 21; the opaque copy of func_800653D4. */

/* PsyQ inline_c.h-style GTE macros, local to this file. */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 )" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0( %0 )" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop;nop;.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0( %0 )" : : "r"(r0) : "memory")

typedef struct {
    u32 tag;
    u32 code[2];
} DR_MODE;

typedef struct {                /* sprite texture part, as stored in o->res */
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SprtTex;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    SprtTex t;
} SPRT;

typedef struct {
    DR_MODE mode;
    SPRT sprt;
} SprtPrim;

typedef struct {
    SprtTex t;
    s32 tpage;
} SprtRes;

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
extern void func_8002CEF4(DR_MODE *p, s32 dfe, s32 dtd, s32 tpage, void *tw);
extern void func_8002B8C4(void *p0, void *p1);

void func_800656B8(UiObj *o)
{
    SprtPrim *p;
    SprtRes *res;
    s16 depth;
    s16 k;

    depth = o->unk14;
    res = o->res;
    p = (SprtPrim *)D_801EF6C8.prim;
    D_801EF6C8.prim += sizeof(SprtPrim);
    func_8002CEF4(&p->mode, 0, 0, res->tpage, 0);
    setlen(&p->sprt, 4);
    setcode(&p->sprt, 0x64);
    func_8002B8C4(&p->mode, &p->sprt);
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
    p->sprt.x0 = D_801BCE88[D_801EF6C8.idx].view[k].x + (o->pos.vx - D_801BCE88[D_801EF6C8.idx].view[k].ox);
    p->sprt.y0 = D_801BCE88[D_801EF6C8.idx].view[k].y + (o->pos.vy - D_801BCE88[D_801EF6C8.idx].view[k].oy);
    p->sprt.t = res->t;
    if (o->light != 21) {
        gte_ldv0(&D_800B8D78);
        gte_ldrgb(D_800B8D80);
        gte_nccs();
        gte_strgb(&p->sprt.r0);
    } else {
        p->sprt.b0 = 128;
        p->sprt.g0 = 128;
        p->sprt.r0 = 128;
    }
    p->sprt.code &= ~2;
    addPrim(o->ot[D_801EF6C8.idx] + depth, p);
}
