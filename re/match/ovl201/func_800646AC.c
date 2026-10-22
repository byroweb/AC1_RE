#include "ac1/ac1.h"
/* ovl201 0x800646AC - queue one textured sprite spanning the projected
 * corners v[0]..v[3] of a frame piece, texture window tw (u, v from tw),
 * lit with NCCS, one step deeper than the object's ordering-table head. */

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
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SPRT;

typedef struct {
    DR_MODE mode;
    SPRT sprt;
} SprtPrim;

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
extern u8 D_800B7594[4];
extern void SetDrawMode(DR_MODE *p, s32 dfe, s32 dtd, s32 tpage, RECT16 *tw);
extern s32 MargePrim(void *p0, void *p1);

void func_800646AC(UiObj *o, SVECTOR *v, RECT16 *tw)
{
    SprtPrim *p;
    DVEC a;
    DVEC b;

    p = (SprtPrim *)D_801EF6C8.prim;
    SetDrawMode(&p->mode, 0, 0, 0x45, tw);
    setlen(&p->sprt, 4);
    setcode(&p->sprt, 0x64);
    MargePrim(&p->mode, &p->sprt);
    gte_ldv0(&v[0]);
    gte_rtps();
    gte_stsxy(&a);
    gte_ldv0(&v[3]);
    gte_rtps();
    gte_stsxy(&b);
    p->sprt.x0 = a.vx;
    p->sprt.y0 = a.vy;
    p->sprt.w = b.vx - a.vx;
    p->sprt.h = b.vy - a.vy;
    p->sprt.u0 = tw->x;
    p->sprt.v0 = tw->y;
    p->sprt.clut = 0x3817;
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B7594);
    gte_nccs();
    gte_strgb(&p->sprt.r0);
    addPrim(o->ot[D_801EF6C8.idx] + 1, p);
    D_801EF6C8.prim += sizeof(SprtPrim);
}
