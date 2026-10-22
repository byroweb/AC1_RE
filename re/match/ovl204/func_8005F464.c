#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 0(%1)\n\tswc2 $14, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    SprInfo s;                  /* +0x00 */
    u32 rgb;                    /* +0x08 */
} SprTex;

typedef struct {
    u32 *ot;                    /* +0x00 */
    SVECTOR sv[4];              /* +0x04 quad corners (sv[1].vz overlaps m) */
} SprScratch;

#define SCR ((SprScratch *)0x1F800000)

/* 2D draw context + render globals block at 0x801ADCCC */
typedef struct {
    PrimBuf *buf;               /* +0x0000 */
    u32 *ot;                    /* +0x0004 */
    u8 unk8[0x7420 - 8];
    MATRIX rot;                 /* +0x7420 (0x801B50EC) */
} DrawBlk;

extern DrawBlk D_801ADCCC;

/* upright sprite (w x h, anchored at its bottom edge) at world point v */
void func_8005F464(SVECTOR *v, SprTex *t, s32 w, s32 h, s32 z)
{
    POLY_FT4 *p;
    s32 otz;
    MATRIX *tm;

    SCR->ot = D_801ADCCC.ot + z;
    RotTrans(v, (VECTOR *)0x1F800024, (long *)0x1F800038);
    tm = (MATRIX *)0x1F800010;
    SetTransMatrix(tm);
    SetRotMatrix(&D_801ADCCC.rot);
    p = (POLY_FT4 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    SCR->sv[0].vx = SCR->sv[2].vx = -w;
    SCR->sv[1].vx = SCR->sv[3].vx = w;
    SCR->sv[0].vy = SCR->sv[1].vy = -h;
    SCR->sv[0].vz = tm->m[0][0] = SCR->sv[2].vz = SCR->sv[3].vz = 0;
    SCR->sv[2].vy = SCR->sv[3].vy = 0;
    gte_ldv3(&SCR->sv[0], &SCR->sv[1], &SCR->sv[2]);
    gte_rtpt();
    p->u0 = t->s.u;
    p->v0 = t->s.v;
    p->u1 = t->s.u + t->s.w;
    p->v1 = t->s.v;
    p->u2 = t->s.u;
    p->v2 = t->s.v + t->s.h;
    p->u3 = t->s.u + t->s.w;
    p->v3 = t->s.v + t->s.h;
    gte_stszotz(&otz);
    if (otz != 0) {
        gte_stsxy3(&p->x0, &p->x1, &p->x2);
        gte_ldv0(&SCR->sv[3]);
        gte_rtps();
        *(u32 *)&p->r0 = t->rgb;
        p->clut = t->s.clut;
        p->tpage = t->s.tpage;
        gte_stsxy(&p->x3);
        setlen(p, 9);
        setcode(p, 0x2E);
        otz >>= 1;
        if (otz >= 0) {
            addPrim(SCR->ot + otz, p);
        }
    } else {
        D_801ADCCC.buf->cur -= sizeof(POLY_FT4);
    }
}
