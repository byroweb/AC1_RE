#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    u32 *ot;        /* +0 */
    s16 sx, sy;     /* +4 */
    s32 sz;         /* +8 */
} BbScr;

#define SCR ((BbScr *)0x1F800000)

extern DrawCtx D_801A5F24;

/* billboard sprite (POLY_FT4) at a 3D point, size scaled by depth */
void func_8006023C(SVECTOR *pos, SprInfo *spr, u32 *col, s32 size, s32 z)
{
    POLY_FT4 *p;
    s32 w;
    s32 h;
    s32 oz;
    BbScr *s;

    s = SCR;

    s->ot = &D_801A5F24.ot[z];
    gte_ldv0(pos);
    gte_rtps();
    gte_stsxy(&s->sx);
    gte_stszotz(&s->sz);
    if (s->sz != 0) {
        if (size & 0x800) {
            w = ((size & 0x7FF) * 80) / s->sz;
            h = w >> 1;
        } else {
            h = (size * 80) / s->sz;
            w = h;
        }
        p = (POLY_FT4 *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur = (u8 *)(p + 1);
        if (D_801A5F24.buf->cur <= D_801A5F24.buf->end) {
            p->x0 = s->sx - w;
            p->y0 = s->sy - h;
            p->x1 = s->sx + w;
            p->y1 = s->sy - h;
            p->x2 = s->sx - w;
            p->y2 = s->sy + h;
            p->x3 = s->sx + w;
            p->y3 = s->sy + h;
            *(u32 *)&p->r0 = *col;
            p->clut = spr->clut;
            p->tpage = spr->tpage;
            p->u0 = spr->u;
            p->v0 = spr->v;
            p->u1 = spr->u + spr->w;
            p->v1 = spr->v;
            p->u2 = spr->u;
            p->v2 = spr->v + spr->h;
            p->u3 = spr->u + spr->w;
            p->v3 = spr->v + spr->h;
            setlen(p, 9);
            setcode(p, 0x2E);
            oz = s->sz >> 1;
            if (oz >= 0) {
                addPrim(s->ot + oz, p);
            }
        }
    }
}
