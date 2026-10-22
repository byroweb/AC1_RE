#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")


typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;                    /* +0x08 */
    s32 xy1;                    /* +0x0C */
} LineF2;

typedef struct {
    u32 tag;
    u32 code[2];
} DrMode;

typedef struct {
    u16 unk0;
    u16 tpage;                  /* +0x02 */
} LineTex;

typedef struct LineNode {
    SVECTOR a;                  /* +0x00 */
    SVECTOR b;                  /* +0x08 */
    s32 unk10;
    s32 sxy;                    /* +0x14 saved screen xy of b */
    s32 z;                      /* +0x18 saved otz of b */
} LineNode;                     /* 0x1C, the next level follows directly */

typedef struct {
    u32 *ot;                    /* +0x00 */
    u16 depth;                  /* +0x04 */
    u8 unk6[4];
    u16 tpage;                  /* +0x0A */
    u8 unkC[4];
    u32 rgb;                    /* +0x10 */
    LineNode node;              /* +0x14 first level */
    u8 unk30[0x68 - 0x30];
    s32 sxy0;                   /* +0x68 */
    s32 sxy1;                   /* +0x6C */
    s32 z0;                     /* +0x70 */
    s32 z1;                     /* +0x74 */
    u8 unk78[4];
    s32 half;                   /* +0x7C */
} LineScratch;

#define SCR ((LineScratch *)0x1F800000)

extern DrawCtx D_801ADCCC;
extern void SetDrawMode(DrMode *p, s32 dfe, s32 dtd, s32 tpage, void *tw);

/* recursive line clipper: halve the segment (SCR->depth levels) until both ends
 * are in front of the near plane, then emit LINE_F2 + DR_MODE */
void func_8005DEE4(LineNode *nd)
{
    LineNode *nx;
    LineF2 *p;
    DrMode *dm;
    s32 n;
    LineScratch *scr = SCR;

    if (SCR->half == 0) {
        nd->sxy = SCR->sxy1;
        nd->z = SCR->z1;
        gte_ldv0(&nd->b);
        gte_rtps();
        gte_stsxy(0x1F80006C);
        gte_stszotz(0x1F800074);
    } else {
        SCR->sxy0 = SCR->sxy1;
        SCR->z0 = SCR->z1;
        {
            s32 xy = nd->sxy;
            s32 z = nd->z;
            SCR->sxy1 = xy;
            SCR->z1 = z;
        }
    }
    if (scr->z0 <= 160 || scr->z1 <= 160) {
        if (scr->depth == 0) {
            if (scr->z0 >= 8 && scr->z1 >= 8) {
                goto draw;
            }
        } else {
            scr->depth--;
            nx = nd + 1;
            nx->a = nd->a;
            nx->b.vx = (nd->a.vx + nd->b.vx) >> 1;
            nx->b.vy = (nd->a.vy + nd->b.vy) >> 1;
            nx->b.vz = (nd->a.vz + nd->b.vz) >> 1;
            scr->half = 0;
            func_8005DEE4(nx);
            nx->a = nx->b;
            nx->b = nd->b;
            scr->half = 1;
            func_8005DEE4(nx);
            scr->depth++;
        }
        return;
    }
draw:
    p = (LineF2 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    p->xy0 = scr->sxy0;
    p->xy1 = scr->sxy1;
    *(u32 *)&p->r0 = scr->rgb;
    setlen(p, 3);
    setcode(p, 0x42);
    n = (scr->z0 + scr->z1) >> 2;
    if (n >= 0) {
        addPrim(scr->ot + n, p);
    }
    dm = (DrMode *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(dm + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    SetDrawMode(dm, 0, 1, scr->tpage, 0);
    if (n >= 0) {
        addPrim(scr->ot + n, dm);
    }
}
