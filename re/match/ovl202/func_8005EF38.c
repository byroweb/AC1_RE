#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

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

typedef struct {                /* 0x1C: one level of the line splitter */
    SVECTOR a;                  /* +0x00 */
    SVECTOR b;                  /* +0x08 */
    s32 pad10;
    s32 sxy;                    /* +0x14 saved end */
    s32 sz;                     /* +0x18 */
} LNode;

typedef struct {
    u32 *ot;                    /* +0x00 */
    u16 depth;                  /* +0x04 */
    u8 pad06[4];
    u16 tpage;                  /* +0x0A */
    u8 pad0c[4];
    u32 col;                    /* +0x10 */
    LNode top;                  /* +0x14 */
    u8 pad30[0x38];
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

/* draw the line a-b, halving it recursively (depth LSCR->depth) while an
 * end is closer than z 320; segments closer than 16 are dropped */
void func_8005EF38(LNode *n)
{
    LineScr *scr;
    LNode *c;
    LineF2 *p;
    DrMode *dm;
    s32 z;
    s32 a;
    s32 b;

    scr = LSCR;
    if (LSCR->half == 0) {
        n->sxy = LSCR->sxy1;
        n->sz = LSCR->sz1;
        gte_ldv0(&n->b);
        gte_rtps();
        gte_stsxy(&LSCR->sxy1);
        gte_stszotz(&LSCR->sz1);
    } else {
        LSCR->sxy0 = LSCR->sxy1;
        LSCR->sz0 = LSCR->sz1;
        a = n->sxy;
        b = n->sz;
        LSCR->sxy1 = a;
        LSCR->sz1 = b;
    }
    if (scr->sz0 <= 320 || scr->sz1 <= 320) {
        if (scr->depth == 0) {
            if (scr->sz0 >= 16 && scr->sz1 >= 16) {
                goto draw;
            }
            return;
        } else {
            scr->depth--;
            c = n + 1;
            c->a = n->a;
            c->b.vx = (n->a.vx + n->b.vx) >> 1;
            c->b.vy = (n->a.vy + n->b.vy) >> 1;
            c->b.vz = (n->a.vz + n->b.vz) >> 1;
            scr->half = 0;
            func_8005EF38(c);
            c->a = c->b;
            c->b = n->b;
            scr->half = 1;
            func_8005EF38(c);
            scr->depth++;
            return;
        }
    }
draw:
    p = (LineF2 *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    p->xy0 = scr->sxy0;
    p->xy1 = scr->sxy1;
    *(u32 *)&p->r0 = scr->col;
    setlen(p, 3);
    setcode(p, 0x42);
    z = (scr->sz0 + scr->sz1) >> 2;
    if (z >= 0) {
        addPrim(scr->ot + z, p);
    }
    dm = (DrMode *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(dm + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    SetDrawMode(dm, 0, 1, scr->tpage, 0);
    if (z >= 0) {
        addPrim(scr->ot + z, dm);
    }
}
