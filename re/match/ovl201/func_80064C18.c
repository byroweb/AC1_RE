#include "ac1/ac1.h"
/* ovl201 0x80064C18 - queue the sprite res (texture tpage res->tpage, window
 * D_8004B4FC) at the projected point *pos raised by 8, lit with NCCS, then a
 * draw-area packet restoring the buffer's area D_800B75A0[idx]. */

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 )" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0( %0 )" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop;nop;.word 0x4A180001")
#define gte_nccs() __asm__ volatile ("nop;nop;.word 0x4B08041B")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0( %0 )" : : "r"(r0) : "memory")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0( %0 )" : : "r"(r0) : "memory")

typedef struct {
    u32 tag;
    u32 code[2];
} DR_MODE;

typedef struct {
    u32 tag;
    u32 code[2];
} DR_AREA;

typedef struct {
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
    s16 x, y, w, h;
} RECT16;

typedef struct {
    s16 vx, vy;
} DVEC;

typedef struct {
    u8 idx;
    u8 pad[3];
    u8 *prim;
} UiGfx;

extern UiGfx D_801EF6C8;
extern SVECTOR D_800B8D78;
extern u8 D_800B91A4[4];
extern RECT16 D_8004B4FC;
extern RECT16 D_800B75A0[2];
extern void SetDrawMode(DR_MODE *p, s32 dfe, s32 dtd, s32 tpage, RECT16 *tw);
extern void SetDrawArea(DR_AREA *p, RECT16 *r);
extern s32 MargePrim(void *p0, void *p1);

void func_80064C18(UiObj *o, SVECTOR *pos, SprtRes *res)
{
    SprtPrim *p;
    DR_AREA *a;
    DVEC xy;
    SVECTOR v;
    RECT16 tw;
    RECT16 r;

    tw = D_8004B4FC;
    v = *pos;
    p = (SprtPrim *)D_801EF6C8.prim;
    v.vy -= 8;
    SetDrawMode(&p->mode, 0, 0, res->tpage, &tw);
    setlen(&p->sprt, 4);
    setcode(&p->sprt, 0x64);
    MargePrim(&p->mode, &p->sprt);
    gte_ldv0(&v);
    gte_rtps();
    gte_stsxy(&xy);
    p->sprt.x0 = xy.vx;
    p->sprt.y0 = xy.vy;
    p->sprt.t = res->t;
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B91A4);
    gte_nccs();
    gte_strgb(&p->sprt.r0);
    addPrim(o->ot[D_801EF6C8.idx] + 1, p);
    D_801EF6C8.prim += sizeof(SprtPrim);

    r = D_800B75A0[D_801EF6C8.idx];
    a = (DR_AREA *)D_801EF6C8.prim;
    D_801EF6C8.prim += sizeof(DR_AREA);
    SetDrawArea(a, &r);
    addPrim(o->ot[D_801EF6C8.idx] + 1, a);
}
