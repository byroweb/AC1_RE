#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 0(%1)\n\tswc2 $14, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    u32 *ot;                    /* +0x00 */
    SVECTOR v[4];               /* +0x04 corners (v[1].vz..v[3] overlap the matrix) */
    VECTOR t;                   /* +0x24 = matrix +0x10's translation */
    s32 flag;                   /* +0x34 */
    s32 flag2;                  /* +0x38 */
} BbScr2;

#define BSCR ((BbScr2 *)0x1F800000)

typedef struct {
    u32 *ot;                    /* 0x801A5F28 */
    u8 pad04[0x78A0];
    MATRIX view;                /* 0x801AD7CC */
} OtView;

typedef struct {
    u16 clut;
    u16 tpage;
    u8 u, v;
    u8 w, h;
    u32 col;                    /* +0x08 colour + code word */
} SprCol;

extern OtView D_801A5F28;
extern DrawCtx D_801A5F24;
extern void RotTrans(SVECTOR *v0, VECTOR *v1, s32 *flag);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);

/* camera-facing sprite (POLY_FT4 0x2E) of w x h hanging at pos */
void func_800604B8(SVECTOR *pos, SprCol *spr, s32 w, s32 h, s32 z)
{
    POLY_FT4 *p;
    s32 otz;
    MATRIX *m;

    BSCR->ot = &D_801A5F28.ot[z];
    RotTrans(pos, &BSCR->t, &BSCR->flag2);
    m = (MATRIX *)&BSCR->v[1].vz;
    SetTransMatrix(m);
    SetRotMatrix(&D_801A5F28.view);
    p = (POLY_FT4 *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    BSCR->v[0].vx = BSCR->v[2].vx = -w;
    BSCR->v[1].vx = BSCR->v[3].vx = w;
    BSCR->v[0].vy = BSCR->v[1].vy = -h;
    BSCR->v[0].vz = m->m[0][0] = BSCR->v[2].vz = BSCR->v[3].vz = 0;
    BSCR->v[2].vy = BSCR->v[3].vy = 0;
    gte_ldv3(&BSCR->v[0], &BSCR->v[1], &BSCR->v[2]);
    gte_rtpt();
    p->u0 = spr->u;
    p->v0 = spr->v;
    p->u1 = spr->u + spr->w;
    p->v1 = spr->v;
    p->u2 = spr->u;
    p->v2 = spr->v + spr->h;
    p->u3 = spr->u + spr->w;
    p->v3 = spr->v + spr->h;
    gte_stszotz(&otz);
    if (otz != 0) {
        gte_stsxy3(&p->x0, &p->x1, &p->x2);
        gte_ldv0(&BSCR->v[3]);
        gte_rtps();
        *(u32 *)&p->r0 = spr->col;
        p->clut = spr->clut;
        p->tpage = spr->tpage;
        gte_stsxy(&p->x3);
        setlen(p, 9);
        setcode(p, 0x2E);
        otz >>= 1;
        if (otz >= 0) {
            addPrim(BSCR->ot + otz, p);
        }
    } else {
        D_801A5F24.buf->cur -= sizeof(POLY_FT4);
    }
}
