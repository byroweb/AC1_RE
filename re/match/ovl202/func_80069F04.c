#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    u8 b[8];
} Blk8;

typedef struct {                /* 0x18 per corner */
    Blk8 uv;                    /* +0x00 */
    s32 w;                      /* +0x08 */
    u32 rgb;                    /* +0x0C */
    s32 xy;                     /* +0x10 */
    s32 z;                      /* +0x14 */
} Corner;

typedef struct {
    u8 pad00[0x2C];
    u32 rgb;                    /* +0x2C flat colour + code */
    u8 pad30[4];
    Corner c[3];                /* +0x34 */
    u8 work[0x48];              /* +0x7C */
    Corner *cp[3];              /* +0xC4 */
    u8 padd0[0x358 - 0xD0];
    u8 *normals;                /* +0x358 */
    u8 *uvtab;                  /* +0x35C */
    u16 clut_lo, clut;          /* +0x360 */
    u16 tpage_lo, tpage;        /* +0x364 */
    u8 pad368[4];
    u32 *otbase;                /* +0x36C */
    u32 *ot;                    /* +0x370 */
    u8 pad374[0x1C];
    u8 rgb_ft[3];               /* +0x390 base colour */
    u8 code_ft;                 /* +0x393 */
} Spad;

#define SP ((Spad *)0x1F800000)

typedef struct {
    s32 w0;                     /* +0x00 (u16 +0x02 clut) */
    s32 w1;                     /* +0x04 (u16 +0x06 tpage) */
    s32 w2;                     /* +0x08 */
    u16 n;                      /* +0x0C normal byte offset */
    u16 v[3];                   /* +0x0E vertex byte offsets */
} Face;

typedef struct {
    Face *f;
    s8 ot;                      /* +0x04 */
    u8 pad5;
    u8 arg;                     /* +0x06 */
    u8 flags;                   /* +0x07 */
} FaceRef;

typedef struct {
    s32 xy;
    s16 z;
    s16 pad;
    u8 rgb[4];
} Vtx;

extern u8 D_801A6A48[];
extern void func_80069A5C(void *work, s32 arg);

void func_80069F04(FaceRef *r)
{
    Face *f;
    Vtx *a;
    Vtx *b;
    Vtx *c;
    Spad *sp;
    u8 *vt;

    SP->cp[0] = &SP->c[0];
    SP->cp[1] = &SP->c[1];
    SP->cp[2] = &SP->c[2];
    SP->tpage_lo = 0;
    SP->clut_lo = 0;
    sp = SP;
    vt = D_801A6A48;
    while (r->f != 0) {
        f = r->f;
        a = (Vtx *)(vt + f->v[0]);
        b = (Vtx *)(vt + f->v[1]);
        c = (Vtx *)(vt + f->v[2]);
        gte_ldv0(sp->normals + f->n);
        gte_ldrgb(sp->rgb_ft);
        gte_nccs();
        sp->c[0].xy = a->xy;
        sp->c[1].xy = b->xy;
        sp->c[2].xy = c->xy;
        sp->c[0].z = a->z;
        sp->c[1].z = b->z;
        sp->c[2].z = c->z;
        sp->c[0].uv = *(Blk8 *)(sp->uvtab + (f->v[0] >> 1));
        sp->c[1].uv = *(Blk8 *)(sp->uvtab + (f->v[1] >> 1));
        sp->c[2].uv = *(Blk8 *)(sp->uvtab + (f->v[2] >> 1));
        sp->c[0].w = f->w0;
        sp->c[1].w = f->w1;
        sp->c[2].w = f->w2;
        sp->clut = ((u16 *)f)[1];
        sp->tpage = ((u16 *)f)[3];
        gte_strgb(&sp->rgb);
        sp->code_ft = (r->flags & 2) | 0x24;
        sp->ot = sp->otbase + r->ot * 4;
        func_80069A5C(sp->work, r->arg);
        r++;
    }
}
