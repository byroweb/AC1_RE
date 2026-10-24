#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")

typedef struct {
    u32 uv0;                    /* +0x00 u0 v0 clut */
    u32 uv1;                    /* +0x04 u1 v1 tpage */
    u16 uv2;                    /* +0x08 */
    u16 tri;                    /* +0x0A nonzero: triangle */
    u16 uv3;                    /* +0x0C */
} PanelTex;

typedef struct {
    u32 *ot;                    /* +0x00 */
    u8 unk4[8];
    MATRIX m;                   /* +0x0C */
    SVECTOR sv;                 /* +0x2C */
} PanelScratch;

#define SCR ((PanelScratch *)0x1F800000)

/* 2D draw context + render globals block at 0x801ADCCC */
typedef struct {
    PrimBuf *buf;               /* +0x0000 */
    u32 *ot;                    /* +0x0004 */
    u8 unk8[0x72D0 - 8];
    MATRIX rot;                 /* +0x72D0 (0x801B4F9C) camera */
} DrawBlk;

extern DrawBlk D_801ADCCC;
extern s32 func_8005F770(SVECTOR *v, s16 *xy);

/* textured square (or triangle) of half-size s at world point v, rotated by rot */
void func_8005F7AC(SVECTOR *v, SVECTOR *rot, PanelTex *t, s32 s)
{
    s32 z;

    SCR->ot = D_801ADCCC.ot + 100;
    gte_ldv0(v);
    gte_rtv0();
    gte_stlvnl(0x1F800020);
    RotMatrixYXZ(rot, &SCR->m);
    MulMatrix2(&D_801ADCCC.rot, &SCR->m);
    SetRotMatrix(&SCR->m);
    SetTransMatrix(&SCR->m);
    if (t->tri == 0) {
        POLY_FT4 *p;
        SVECTOR *sv;

        p = (POLY_FT4 *)D_801ADCCC.buf->cur;
        D_801ADCCC.buf->cur = (u8 *)(p + 1);
        if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
            return;
        }
        *(u32 *)&p->u0 = t->uv0;
        *(u32 *)&p->u1 = t->uv1;
        *(u16 *)&p->u2 = t->uv2;
        *(u16 *)&p->u3 = t->uv3;
        *(u32 *)&p->r0 = 0x808080;
        SCR->sv.vz = 0;
        SCR->sv.vx = SCR->sv.vy = -s;
        sv = &SCR->sv;
        z = func_8005F770(&SCR->sv, &p->x0) >> 1;
        sv->vx = s;
        func_8005F770(&SCR->sv, &p->x1);
        SCR->sv.vy = s;
        func_8005F770(&SCR->sv, &p->x3);
        sv->vx = -s;
        func_8005F770(&SCR->sv, &p->x2);
        setlen(p, 9);
        setcode(p, 0x2C);
        if (z >= 0) {
            addPrim(SCR->ot + z, p);
        }
    } else {
        POLY_FT3 *q;
        SVECTOR *sv;

        q = (POLY_FT3 *)D_801ADCCC.buf->cur;
        D_801ADCCC.buf->cur = (u8 *)(q + 1);
        if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
            return;
        }
        *(u32 *)&q->u0 = t->uv0;
        *(u32 *)&q->u1 = t->uv1;
        *(u16 *)&q->u2 = t->uv2;
        *(u32 *)&q->r0 = 0x808080;
        SCR->sv.vz = 0;
        SCR->sv.vx = 0;
        SCR->sv.vy = -s;
        sv = &SCR->sv;
        z = func_8005F770(&SCR->sv, &q->x0) >> 1;
        sv->vx = -s;
        SCR->sv.vy = s;
        func_8005F770(&SCR->sv, &q->x1);
        sv->vx = s;
        func_8005F770(&SCR->sv, &q->x2);
        setlen(q, 7);
        setcode(q, 0x24);
        if (z >= 0) {
            addPrim(SCR->ot + z, q);
        }
    }
}
