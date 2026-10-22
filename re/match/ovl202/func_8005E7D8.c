#include "ac1/ac1.h"

typedef struct {                /* libgpu POLY_F4 */
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} PolyF4;

typedef struct {                /* libgpu DR_MODE */
    u32 tag;
    u32 code[2];
} DrMode;

extern void SetDrawMode(DrMode *p, s32 dfe, s32 dtd, s32 tpage, RECT *tw);
extern DrawCtx D_801A5F24;

/* flat rectangle (x0,y0)-(x1,y1) in colour *rgb at OT slot z; abr != -1
 * makes it semi-transparent with that blend mode */
void func_8005E7D8(s32 x0, s32 y0, s32 x1, s32 y1, u32 *rgb, s32 z, s32 abr)
{
    PolyF4 *p;
    DrMode *dm;
    u32 *ot;
    u32 c;

    p = (PolyF4 *)D_801A5F24.buf->cur;
    ot = &D_801A5F24.ot[z];
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    c = *rgb;
    p->x0 = p->x2 = x0;
    p->x1 = p->x3 = x1;
    p->y0 = p->y1 = y0;
    p->y2 = p->y3 = y1;
    setlen(p, 5);
    *(u32 *)&p->r0 = c;
    setcode(p, 0x28);
    if (abr != -1) {
        setcode(p, 0x2A);
    }
    addPrim(ot, p);
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
