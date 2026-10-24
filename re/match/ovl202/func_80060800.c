#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0tr() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")

typedef struct {
    u32 *ot;                    /* +0x00 */
    u8 pad04[8];
    MATRIX m;                   /* +0x0C */
    SVECTOR v;                  /* +0x2C */
} SprScr;

#define SSCR ((SprScr *)0x1F800000)

typedef struct {
    u32 *ot;                    /* 0x801A5F28 */
    u8 pad04[0x7750];
    MATRIX cam;                 /* 0x801AD67C */
} OtCam;

typedef struct {
    u32 uv0;                    /* u0 v0 clut */
    u32 uv1;                    /* u1 v1 tpage */
    u16 uv2;
    u16 tri;                    /* non-zero: triangle */
    u16 uv3;
} SprUV;

extern OtCam D_801A5F28;
extern DrawCtx D_801A5F24;
extern s32 func_800607C4(SVECTOR *v, s16 *sxy);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);

/* textured sprite (quad or triangle) of half size s at pos, rotated by rot */
void func_80060800(SVECTOR *pos, SVECTOR *rot, SprUV *uv, s32 s)
{
    POLY_FT4 *p;
    POLY_FT3 *t;
    s32 z;
    SVECTOR *v;
    SVECTOR *w;
    s16 n;
    s32 x;
    s32 x2;

    SSCR->ot = D_801A5F28.ot + 100;
    gte_ldv0(pos);
    gte_rtv0tr();
    gte_stlvnl(SSCR->m.t);
    RotMatrixYXZ(rot, &SSCR->m);
    MulMatrix2(&D_801A5F28.cam, &SSCR->m);
    SetRotMatrix(&SSCR->m);
    SetTransMatrix(&SSCR->m);
    if (uv->tri == 0) {
        p = (POLY_FT4 *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur = (u8 *)(p + 1);
        if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
            return;
        }
        *(u32 *)&p->u0 = uv->uv0;
        *(u32 *)&p->u1 = uv->uv1;
        *(u16 *)&p->u2 = uv->uv2;
        *(u16 *)&p->u3 = uv->uv3;
        *(u32 *)&p->r0 = 0x808080;
        n = -s;
        SSCR->v.vz = 0;
        SSCR->v.vy = n;
        SSCR->v.vx = n;
        v = &SSCR->v;
        z = func_800607C4(&SSCR->v, &p->x0) >> 1;
        x = s;
        v->vx = x;
        func_800607C4(&SSCR->v, &p->x1);
        SSCR->v.vy = x;
        func_800607C4(&SSCR->v, &p->x3);
        v->vx = n;
        func_800607C4(&SSCR->v, &p->x2);
        setlen(p, 9);
        setcode(p, 0x2C);
        if (z >= 0) {
            addPrim(SSCR->ot + z, p);
        }
    } else {
        t = (POLY_FT3 *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur = (u8 *)(t + 1);
        if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
            return;
        }
        *(u32 *)&t->u0 = uv->uv0;
        *(u32 *)&t->u1 = uv->uv1;
        *(u16 *)&t->u2 = uv->uv2;
        *(u32 *)&t->r0 = 0x808080;
        x2 = -s;
        SSCR->v.vz = 0;
        SSCR->v.vx = 0;
        SSCR->v.vy = x2;
        w = &SSCR->v;
        z = func_800607C4(&SSCR->v, &t->x0) >> 1;
        w->vx = x2;
        x = s;
        SSCR->v.vy = x;
        func_800607C4(&SSCR->v, &t->x1);
        w->vx = x;
        func_800607C4(&SSCR->v, &t->x2);
        setlen(t, 7);
        setcode(t, 0x24);
        if (z >= 0) {
            addPrim(SSCR->ot + z, t);
        }
    }
}
