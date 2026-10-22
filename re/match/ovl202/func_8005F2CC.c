#include "ac1/ac1.h"

typedef struct {                /* libgpu LINE_F2 */
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;
    s32 xy1;
} LineF2;

typedef struct {                /* libgpu DR_MODE */
    u32 tag;
    u32 code[2];
} DrMode;

typedef struct {
    u32 *ot;                    /* +0x00 */
    s16 depth;                  /* +0x04 */
    u8 pad06[4];
    u16 tpage;                  /* +0x0A */
    u8 pad0c[4];
    u32 col;                    /* +0x10 */
    SVECTOR a;                  /* +0x14 */
    SVECTOR b;                  /* +0x1C */
    u8 pad24[0x44];
    s32 sxy0;                   /* +0x68 */
    s32 sxy1;                   /* +0x6C */
    s32 sz0;                    /* +0x70 */
    s32 sz1;                    /* +0x74 */
    u8 pad78[4];
    s32 half;                   /* +0x7C */
} LineScr;

#define LSCR ((LineScr *)0x1F800000)

extern DrawCtx D_801A5F24;
extern void SetDrawMode(DrMode *p, s32 dfe, s32 dtd, s32 tpage, RECT *tw);
extern void func_8005EF38(SVECTOR *a);

/* draw a 3D line (already projected into the scratchpad); split it in two
 * halves through func_8005EF38 when an end is near the camera */
void func_8005F2CC(SVECTOR *a, SVECTOR *b, SprInfo *spr, u32 *col)
{
    LineF2 *p;
    DrMode *dm;
    s32 z;

    if (LSCR->sz0 <= 320 || LSCR->sz1 <= 320) {
        LSCR->depth = 2;
        LSCR->col = *col;
        LSCR->a = *a;
        LSCR->b.vx = (a->vx + b->vx) >> 1;
        LSCR->b.vy = (a->vy + b->vy) >> 1;
        LSCR->b.vz = (a->vz + b->vz) >> 1;
        LSCR->tpage = spr->tpage;
        LSCR->half = 0;
        func_8005EF38(&LSCR->a);
        LSCR->a = LSCR->b;
        LSCR->b = *b;
        LSCR->half = 1;
        func_8005EF38(&LSCR->a);
        return;
    }
    p = (LineF2 *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur <= D_801A5F24.buf->end) {
        p->xy0 = LSCR->sxy0;
        p->xy1 = LSCR->sxy1;
        *(u32 *)&p->r0 = *col;
        setlen(p, 3);
        setcode(p, 0x42);
        z = (LSCR->sz0 + LSCR->sz1) >> 2;
        if (z >= 0) {
            addPrim(LSCR->ot + z, p);
        }
        dm = (DrMode *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur = (u8 *)(dm + 1);
        if (D_801A5F24.buf->cur <= D_801A5F24.buf->end) {
            SetDrawMode(dm, 0, 1, spr->tpage, 0);
            if (z >= 0) {
                addPrim(LSCR->ot + z, dm);
            }
        }
    }
}
