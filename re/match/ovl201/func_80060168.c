#include "ac1_types.h"
/* ovl201 0x80060168  size 500  score 8.527  2026-10-18
 * Tile a 5 x 4 grid of 64x64 sprites (DR_MODE + SPRT pairs), colour them
 * through the GTE (NCCS) and add them to the object's ordering table. */

/* PsyQ inline_c.h-style GTE macros, local to this file. */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 )" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0( %0 )" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop;nop;.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0( %0 )" : : "r"(r0) : "memory")

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} P_TAG;

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
} Func80060168Prim;

typedef struct {
    u8 pad[0x14];
    u16 depth;      /* +0x14 */
    u8 pad16[2];
    u32 *ot[2];     /* +0x18 */
} Func80060168Obj;

typedef struct {
    u8 idx;         /* 0x801EF6C8 */
    u8 pad[3];
    u8 *prim;       /* 0x801EF6CC */
} Func80060168Gfx;

extern u8 D_801A2578;
extern Func80060168Gfx D_801EF6C8;
extern u8 D_800B8E44[];
typedef struct {
    u16 h0;
    u16 clut;       /* 0x800B758A */
    u32 w4;
    s32 tpage;      /* 0x800B7590 */
} Func80060168Tex;

extern Func80060168Tex D_800B7588;
extern s16 D_800B8D78[];
extern u8 D_800B8D80[];
extern void func_80028234(void *p);
extern void func_8002CEF4(DR_MODE *p, s32 dfe, s32 dtd, s32 tpage, void *tw);
extern void func_8002B8C4(void *p0, void *p1);

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))
#define getaddr(p) (u32)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)

void func_80060168(Func80060168Obj *obj)
{
    Func80060168Prim *p;
    s16 x;
    s16 y;
    s16 depth;

    depth = obj->depth;
    x = 0;
    if (D_801A2578 == 0) {
        p = (Func80060168Prim *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(Func80060168Prim) * 20;
        func_80028234(D_800B8E44);
        x = 0;
        do {
            y = 0;
            do {
                func_8002CEF4(&p->mode, 0, 0, D_800B7588.tpage, 0);
                ((P_TAG *)&p->sprt)->len = 4;
                p->sprt.code = 0x64;
                func_8002B8C4(&p->mode, &p->sprt);
                p->sprt.w = 64;
                p->sprt.h = 64;
                p->sprt.x0 = x;
                p->sprt.y0 = y;
                p->sprt.u0 = 0;
                p->sprt.v0 = 192;
                p->sprt.clut = D_800B7588.clut;
                gte_ldv0(D_800B8D78);
                gte_ldrgb(D_800B8D80);
                gte_nccs();
                gte_strgb(&p->sprt.r0);
                p->sprt.code &= ~2;
                addPrim(obj->ot[D_801EF6C8.idx] + depth, p);
                p++;
                y += 64;
            } while (y <= 192);
            x += 64;
        } while (x <= 256);
    }
}
