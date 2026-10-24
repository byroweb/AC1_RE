#include "ac1/ac1.h"
/* matched (wave4 a5): same shape as ovl202 0x8005CE3C (sect_draw_visible), ported */
/* ovl204: collect the visible world cells around the current cell (portal list,
 * distance cull) into two draw lists, then draw them. */
typedef struct {                /* 0x18 */
    s32 t[3];
    s16 mi;                     /* +0x0C -1 ends the list */
    u16 part;
    void *ws;
    void *lt;
} DrawEnt4;
typedef struct Cell4 {          /* 0x40 */
    u8 pad0[0x10];
    s16 cx, cy, cz;             /* +0x10 */
    u8 pad16[0x26 - 0x16];
    s16 n;                      /* +0x26 */
    u16 *list;                  /* +0x28 */
    u8 pad2c[4];
    u32 flags;                  /* +0x30 */
    u32 range;                  /* +0x34 */
    u8 pad38[8];
} Cell4;
typedef struct {                /* 0x801B4118 */
    u32 vis[16];                /* cell visible bits (fx pool tests these) */
    u32 drawn[16];              /* +0x40 */
    u8 pad80[0xE70 - 0x80];
    Cell4 *cur;                 /* +0xE70 0x801B4F88 */
} World4;
extern World4 D_801B4118;
extern s32 D_801B4FB0;
extern s32 D_801B4FB4;
extern s32 D_801B4FB8;
extern s32 D_801A4960;
extern s32 D_801A4964;
extern Cell4 D_801D4488[];
extern void func_8005BAA8(Cell4 *c, s32 flag, DrawEnt4 **a, DrawEnt4 **b, SVECTOR *cam);
extern void func_800284DC(VECTOR *a, VECTOR *b);
extern void func_8005A134(DrawEnt4 *l);
extern void func_80062CB4(DrawEnt4 *l);

#define SQ ((VECTOR *)0x1F800000)

void func_8005BFE0(void)
{
    SVECTOR cam;
    VECTOR unused[2];
    DrawEnt4 la[129];
    DrawEnt4 lb[193];
    DrawEnt4 *pa;
    DrawEnt4 *pb;
    Cell4 *c;
    u16 *list;
    u16 *last;
    s32 n;
    s32 x;
    s32 k;
    u32 bit;
    Cell4 *cells;

    c = D_801B4118.cur;
    pa = la;
    pb = lb;
    if (c == 0) {
        return;
    }
    D_801A4964 = -1;
    cam.vx = D_801B4FB0;
    cam.vy = D_801B4FB4;
    cam.vz = D_801B4FB8;
    D_801A4960 = 2;
    func_8005BAA8(c, 0, &pa, &pb, &cam);
    x = c - D_801D4488;
    D_801B4118.vis[x >> 5] = D_801B4118.drawn[x >> 5] = 1 << (x & 0x1F);
    n = c->n;
    list = c->list;
    last = list + n - 1;
    if (--n != -1) {
        cells = D_801D4488;
        do {
            x = *list;
            k = x & 0x1FF;
            c = (Cell4 *)((k << 6) + (u32)cells);
            list++;
            if (c->flags & 0x1000) {
                SQ->vx = (cam.vx - c->cx) >> 1;
                SQ->vy = (cam.vy - c->cy) >> 1;
                SQ->vz = (cam.vz - c->cz) >> 1;
                func_800284DC(SQ, SQ);
                if (((SQ->vx + SQ->vy + SQ->vz) >> 18) >= (s32)(c->range & 0xFFF)) {
                    continue;
                }
            }
            D_801B4118.vis[k >> 5] |= 1 << (x & 0x1F);
            if ((x & 0x3800) && (cells[last[(x & 0x3800) >> 11]].flags & 0x100)) {
                continue;
            }
            D_801B4118.drawn[k >> 5] |= 1 << (x & 0x1F);
            bit = x & 0x4000;
            func_8005BAA8(c, bit != 0, &pa, &pb, &cam);
        } while (--n != -1);
    }
    pa->mi = pb->mi = -1;
    func_8005A134(la);
    func_80062CB4(lb);
}
