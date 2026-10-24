/* ovl202 0x80071E14: copy-loop source through a block-local base pointer (b = p + 0x6800) so the sum gets its own temporary, as retail. From regtie.py's best draft. */
#include "ac1/ac1.h"

typedef struct {
    s16 min[3];         /* +0x00 */
    s16 id;             /* +0x06 -1 = unused */
    s16 max[3];         /* +0x08 */
    s16 n_extra;        /* +0x0E */
    s16 org[3];         /* +0x10 */
    u16 h16;            /* +0x16 */
    s16 bmin[3];        /* +0x18 */
    s16 mesh;           /* +0x1E */
    s16 bmax[3];        /* +0x20 */
    s16 n_parts;        /* +0x26 */
    u8 *parts;          /* +0x28 */
    void *on_hit;       /* +0x2C */
    u32 flags;          /* +0x30 */
    u32 w34;            /* +0x34 */
    u8 *stats;          /* +0x38 */
    u8 n_stats;         /* +0x3C */
    u8 cls;             /* +0x3D */
    u8 pad3e[2];
} Ws;

extern Ws D_801D0B68[];
extern u32 D_801D8B68[];
extern u8 *D_801E8B68;
extern s32 D_801E8B70;
extern void func_8006DA20(void);
extern void func_8006DC3C(void *box, s32 idx);

void func_80071E14(u8 *data, s32 size)
{
    u32 *src;
    u32 *dst;
    u32 *rs;
    u32 *rd;
    s32 n;
    s32 i;
    s32 j;
    s32 k;
    Ws *ws;
    u8 *p;
    u8 *heap;
    s32 a;
    s32 b;
    s32 c;
    s32 d;

    g_world_struct_count = 0;
    func_8006DA20();
    p = data;
    size = (size + 3) & ~3;
    n = (u32)(size - 0x6800) >> 2;
    { u8 *b = p + 0x6800; src = (u32 *)b + n; }
    dst = D_801D8B68 + n;
    while (--n != -1) {
        *--dst = *--src;
    }
    rs = (u32 *)(p + 0x6800);
    rd = D_801D8B68;
    n = 512;
    do {
        rs -= 13;
        rd -= 16;
        src = rs + 13;
        dst = rd + 13;
        j = 13;
        do {
            *--dst = *--src;
        } while (--j);
    } while (--n);
    ws = D_801D0B68;
    p = (u8 *)ws;
    heap = (u8 *)D_801D8B68;
    for (i = 0; i < 512; ws++, i++) {
        p += 0x28;
        a = *(s32 *)p;
        p += 4;
        b = *(s32 *)p;
        p += 4;
        c = *p;
        p += 1;
        d = *p;
        p += 15;
        ws->flags = a;
        ws->w34 = b;
        if (ws->flags & 0x8000) {
            ws->w34 = (b & ~0xFFF) | ((u32)((b & 0xFFF) * (b & 0xFFF)) >> 12);
        }
        ws->n_stats = c;
        ws->cls = d;
        if (ws->id != -1 && ws->mesh != -1) {
            ws->parts = heap;
            heap += (ws->n_parts + ws->n_extra) * 2;
            ws->h16 = (ws->h16 >> 10) & 3;
            if (c == 0) {
                ws->stats = 0;
            } else {
                ws->stats = heap;
                heap += c * 2;
            }
            func_8006DC3C((ws->flags & 0x200) ? (void *)ws->bmin : (void *)ws, i);
            g_world_struct_count = i + 1;
            ws->on_hit = 0;
        } else {
            ws->id = -1;
            ws->on_hit = 0;
            ws->parts = 0;
        }
    }
    D_801E8B70 = (g_world_struct_count + 31) / 32;
    D_801E8B68 = (u8 *)(((u32)heap + 3) & ~3);
    ws = D_801D0B68;
    for (i = 512; i != 0; ws++, i--) {
        if (ws->id != -1) {
            if (ws->cls != 255) {
                ((void (**)(Ws *))((u8 *)g_objective_obj + 0x60))[ws->cls](ws);
            }
            if (ws->flags & 0x200) {
                ws->min[0] = ws->bmin[0];
                ws->min[1] = ws->bmin[1];
                ws->min[2] = ws->bmin[2];
                ws->max[0] = ws->bmax[0];
                ws->max[1] = ws->bmax[1];
                ws->max[2] = ws->bmax[2];
            }
        }
    }
}
