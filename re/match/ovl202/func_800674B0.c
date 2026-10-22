#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0tr() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")
#define gte_ldlvl(r0) __asm__ volatile ("lwc2 $9, 0(%0)\n\tlwc2 $10, 4(%0)\n\tlwc2 $11, 8(%0)" : : "r"(r0))
#define gte_sqr12() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AA00428")
#define gte_ldlzc(r0) __asm__ volatile ("mtc2 %0, $30\n\tnop" : : "r"(r0))
#define gte_stlzc(r0) __asm__ volatile ("swc2 $31, 0(%0)" : : "r"(r0) : "memory")
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ldcv(r0) __asm__ volatile ("lbu $12, 0(%0)\n\tlbu $13, 1(%0)\n\tlbu $14, 2(%0)\n\tmtc2 $12, $9\n\tmtc2 $13, $10\n\tmtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_gpf12() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B98003D")
#define gte_gpl12() __asm__ volatile ("nop\n\tnop\n\t.word 0x4BA8003E")
#define gte_stcv(r0) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsb $12, 0(%0)\n\tsb $13, 1(%0)\n\tsb $14, 2(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct {                /* 0x30 point light / sphere (0x801AE82C) */
    void *owner;                /* +0x00 */
    SVECTOR c;                  /* +0x04 centre (pad = off) */
    u8 pad0c[0x18];
    u8 col[4];                  /* +0x24 */
    s16 pad28;
    u16 flags;                  /* +0x2A bit 0 = lights structures */
    s16 r;                      /* +0x2C */
    s16 pad2e;
} PLight;

typedef struct {
    PLight l[16];
    s32 unk300;
    s32 n;                      /* +0x304 (0x801AEB30) */
} PLightSet;

typedef struct {
    s16 min[3];
    s16 mesh;
    s16 max[3];
    s16 n_parts;
} WsBox;

typedef struct {
    u8 pad00[0x10];
    SVECTOR org;                /* +0x10 (pad = rotation) */
    WsBox box;                  /* +0x18 */
    u8 pad28[8];
    u32 flags;                  /* +0x30 colour table in the top nibble */
} LWorld;

typedef struct {                /* 0x10 */
    s32 sxy;
    s32 otz;
    u8 col[3];                  /* +0x08 */
    u8 lit;                     /* +0x0B */
    s32 dp;
} LVtx;

typedef struct {
    s32 n;                      /* +0x00 */
    s32 lit;                    /* +0x04 */
    s32 cnt;                    /* +0x08 */
    PLight *l;                  /* +0x0C */
    SVECTOR org;                /* +0x10 */
    WsBox box;                  /* +0x18 */
    SVECTOR d;                  /* +0x28 */
    VECTOR lp;                  /* +0x30 */
    LVtx *vp;                   /* +0x40 */
    VECTOR dv;                  /* +0x44 */
    VECTOR sq;                  /* +0x54 */
    MATRIX m;                   /* +0x64 */
    u8 *lt;                     /* +0x84 */
} LScr;

#define LSCR ((LScr *)0x1F800000)

extern PLightSet D_801AE82C;
extern LVtx D_801A6A48[];
extern SVECTOR *D_801A6544;
extern u8 D_80090A28[][128];
extern u16 D_80031564[];
extern void func_80013AFC(s16 a, MATRIX *m);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);

/* add the point lights touching a world structure's box to its vertex
 * colours (base colours from table D_80090A28 by the vertex pad) */
s32 func_800674B0(LWorld *ws, s32 n)
{
    LScr *scr;
    PLight *l;
    SVECTOR *v;
    LVtx *o;
    s32 k;
    s32 *p;
    s16 lo[4];
    s16 hi[4];
    s32 lz;
    s32 sum;
    s32 sh;
    s32 t;

    LSCR->lit = 0;
    scr = LSCR;
    scr->n = D_801AE82C.n;
    if (scr->n != 0) {
    LSCR->l = D_801AE82C.l;
    LSCR->box = ws->box;
    do {
        l = scr->l;
        if (!(l->flags & 1)) {
            goto next;
        }
        if (l->owner == ws) {
            goto next;
        }
        if (l->c.pad != 0) {
            goto next;
        }
        if (!(scr->box.min[0] < l->c.vx + l->r)) {
            goto next;
        }
        if (!(scr->box.max[0] > l->c.vx - l->r)) {
            goto next;
        }
        if (!(scr->box.min[1] < l->c.vy + l->r)) {
            goto next;
        }
        if (!(scr->box.max[1] > l->c.vy - l->r)) {
            goto next;
        }
        if (!(scr->box.min[2] < l->c.vz + l->r)) {
            goto next;
        }
        if (!(scr->box.max[2] > l->c.vz - l->r)) {
            goto next;
        }
        if (scr->lit == 0) {
            k = n;
            o = D_801A6A48;
            scr->lt = D_80090A28[ws->flags >> 28];
            scr->org = ws->org;
            v = D_801A6544;
            do {
                *(u32 *)o->col = *(u32 *)(scr->lt + v->pad);
                if (v->pad != 0) {
                    o->lit = 1;
                }
                o++;
                v++;
            } while (--k);
            func_80013AFC(scr->org.pad << 10, &scr->m);
            SetRotMatrix(&scr->m);
            scr->m.t[2] = 0;
            scr->m.t[1] = 0;
            scr->m.t[0] = 0;
            SetTransMatrix(&scr->m);
        }
        scr->d.vx = (scr->l->c.vx - scr->org.vx) << 2;
        scr->d.vy = (scr->l->c.vy - scr->org.vy) << 2;
        scr->d.vz = (scr->l->c.vz - scr->org.vz) << 2;
        gte_ldv0(&scr->d);
        gte_rtv0tr();
        gte_stlvnl(&scr->lp);
        lo[0] = scr->lp.vx - (scr->l->r << 2);
        hi[0] = scr->lp.vx + (scr->l->r << 2);
        lo[1] = scr->lp.vy - (scr->l->r << 2);
        hi[1] = scr->lp.vy + (scr->l->r << 2);
        lo[2] = scr->lp.vz - (scr->l->r << 2);
        hi[2] = scr->lp.vz + (scr->l->r << 2);
        scr->vp = D_801A6A48;
        v = D_801A6544;
        scr->cnt = n;
        do {
            if (v->vx > lo[0] && v->vx < hi[0] && v->vy > lo[1] && v->vy < hi[1]
                && v->vz > lo[2] && v->vz < hi[2]) {
                scr->dv.vx = v->vx - scr->lp.vx;
                scr->dv.vy = v->vy - scr->lp.vy;
                scr->dv.vz = v->vz - scr->lp.vz;
                gte_ldlvl(&scr->dv);
                gte_sqr12();
                gte_stlvnl(&scr->sq);
                sum = scr->sq.vx + scr->sq.vy + scr->sq.vz;
                gte_ldlzc(sum);
                gte_stlzc(&lz);
                lz &= ~1;
                t = lz - 24;
                sh = (31 - lz) >> 1;
                if (t <= 0) {
                    t = sum >> (24 - lz);
                } else {
                    t = sum << t;
                }
                sum = D_80031564[t] << sh;
                sum = (sum >> 2) / scr->l->r;
                if (sum > 4096) {
                    sum = 4096;
                }
                gte_lddp(4096 - sum);
                gte_ldcv(scr->l->col);
                gte_gpf12();
                gte_lddp(sum);
                gte_ldcv(scr->vp->col);
                gte_gpl12();
                gte_stcv(scr->vp->col);
                scr->vp->lit = 1;
            }
            v++;
            scr->vp++;
        } while (--scr->cnt);
        scr->lit = 1;
    next:
        scr->l++;
    } while (--scr->n);
    }
    return scr->lit;
}
