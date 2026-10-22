#include "ac1/ac1.h"

typedef struct {                /* libgpu LINE_F3 */
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0;
    s32 xy1;
    s32 xy2;
    u32 pad;                    /* 0x55555555 terminator */
} LineF3;

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

extern DrawCtx D_801A5F24;
extern void SetDrawMode(DrMode *p, s32 dfe, s32 dtd, s32 tpage, RECT *tw);

/* flat polyline through n screen points: LINE_F3 per pair of segments,
 * a LINE_F2 for an odd last one, optional semi-transparency mode abr */
void func_800610A4(s32 *xy, u32 *col, s32 n, s32 z, s32 abr)
{
    LineF3 *p;
    LineF2 *q;
    DrMode *dm;
    u32 *ot;
    s32 i;
    s32 st;

    i = n >> 1;
    ot = &D_801A5F24.ot[z];
    st = (abr != -1) << 1;
    while (--i != -1) {
        p = (LineF3 *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur = (u8 *)(p + 1);
        if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
            return;
        }
        *(u32 *)&p->r0 = *col;
        p->xy0 = xy[0];
        p->xy1 = xy[1];
        p->xy2 = xy[2];
        xy += 2;
        setlen(p, 5);
        setcode(p, st | 0x48);
        p->pad = 0x55555555;
        addPrim(ot, p);
    }
    if (n & 1) {
        q = (LineF2 *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur = (u8 *)(q + 1);
        if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
            return;
        }
        *(u32 *)&q->r0 = *col;
        q->xy0 = xy[0];
        q->xy1 = xy[1];
        setlen(q, 3);
        setcode(q, st | 0x40);
        addPrim(ot, q);
    }
    if (abr != -1) {
        dm = (DrMode *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur = (u8 *)(dm + 1);
        if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
            return;
        }
        SetDrawMode(dm, 0, 0, (abr & 3) << 5, 0);
        addPrim(ot, dm);
    }
}
