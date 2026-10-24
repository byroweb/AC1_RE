#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {                /* 0x10 projected vertex with clip codes */
    s32 sxy;                    /* +0x00 */
    s16 otz;                    /* +0x04 */
    u8 clip;                    /* +0x06 1 left 2 right 4 above 8 below, 31 = z 0 */
    u8 near;                    /* +0x07 0 far, 1 z < 1000, 2 z < 80 */
    s32 pad08;
    s32 dp;                     /* +0x0C */
} ClipVtx;

typedef struct {
    u8 pad00[0xC];
    union {
        s32 sxy;
        struct { u16 x, y; } s;
    } p;                        /* +0x0C */
    s32 dp;                     /* +0x10 */
    s32 sz;                     /* +0x14 */
} ClipScr;

#define CLIPSCR ((ClipScr *)0x1F800000)

extern ClipVtx D_801A6A48[];
extern SVECTOR *D_801A6544;
extern void func_80067BAC(SVECTOR *v, s32 *sxy, s32 *dp, s32 *sz);

/* project the current vertex list (D_801A6544) into D_801A6A48 with near and
 * screen-edge clip codes, one vertex ahead in the GTE.  The do{}while(0)
 * blocks are load-bearing: they weight the &o->clip giv refs (register choice)
 * and the tail one makes reorg predict the sz>=80 branch taken. */
void func_80066EBC(s32 n)
{
    ClipVtx *o;
    SVECTOR *v;
    ClipScr *scr;

    scr = CLIPSCR;
    o = D_801A6A48;
    v = D_801A6544;
    func_80067BAC(v, &CLIPSCR->p.sxy, &CLIPSCR->dp, &CLIPSCR->sz);
    v++;
    while (--n != 0) {
        gte_ldv0(v);
        gte_rtps();
        o->otz = scr->sz;
        if (scr->sz < 1000) {
            if (scr->sz < 80) {
                o->near = 2;
                if (scr->sz == 0) {
                    o->clip = 31;
                    goto next;
                ymin:
                    o->clip |= 4;
                    goto yend;
                }
            } else {
                do { o->near = 1; } while (0);
            }
        } else {
            do { o->near = 0; } while (0);
        }

        o->sxy = scr->p.sxy;
        if ((s16)scr->p.s.x < 0) {
            o->clip = 1;
        } else if ((s16)scr->p.s.x > 158) {
            o->clip = 2;
        } else {
            o->clip = 0;
        }
        do {
        if ((s16)scr->p.s.y < 0) {
            goto ymin;
        }
        if ((s16)scr->p.s.y > 120) {
            o->clip |= 8;
        }
        } while (0);
    yend:
    next:
        v++;
        o++;
        gte_stsxy(&scr->p.sxy);
        gte_stszotz(&scr->sz);
    }
    o->otz = scr->sz;
    if (scr->sz < 1000) {
        if (scr->sz >= 80) {
            goto n1;
        }
        o->near = 2;
        if (scr->sz == 0) {
            o->clip = 31;
            goto done;
        }
        goto xc;
        do {
        n1:
            o->near = 1;
        } while (0);
    } else {
        o->near = 0;
    }
xc:
    o->sxy = scr->p.sxy;
    if ((s16)scr->p.s.x < 0) {
        o->clip = 1;
    } else if ((s16)scr->p.s.x > 158) {
        o->clip = 2;
    } else {
        o->clip = 0;
    }
    if ((s16)scr->p.s.y >= 0) {
        if ((s16)scr->p.s.y > 120) {
            o->clip |= 8;
        }
    } else {
        o->clip |= 4;
    }
done:
    ;
}
