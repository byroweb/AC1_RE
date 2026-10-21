#include "ac1/ac1.h"

typedef struct {
    s16 a[3];       /* +0x00 */
    s16 unk06;      /* +0x06 */
    s16 b[3];       /* +0x08 */
    s16 unk0e;
    s16 org[3];     /* +0x10 */
    s16 unk16;
    s16 bbox_min[3]; /* +0x18 */
    s16 mesh;
    s16 bbox_max[3]; /* +0x20 */
    s16 unk26;
    u8 unk28[4];
    void *on_hit;   /* +0x2C */
    u32 a0 : 12;
    u32 a12 : 1;
    u32 a13 : 3;
    u32 a16 : 1;
    u32 a17 : 15;
    u32 lo12 : 12;
    u32 mid : 12;
    u32 b24 : 1;
    u32 hi : 7;
    void *stats;
    u8 unk3c[4];
} WorldReset;

extern void func_8006DD00(WorldReset *w, s32 idx);

void func_8006D39C(WorldReset *w)
{
    func_8006DD00(w, w - (WorldReset *)g_world_structs);
    w->unk06 = -1;
    w->bbox_max[2] = 0x7FFF;
    w->bbox_max[1] = 0x7FFF;
    w->bbox_max[0] = 0x7FFF;
    w->bbox_min[2] = 0x7FFF;
    w->bbox_min[1] = 0x7FFF;
    w->bbox_min[0] = 0x7FFF;
    w->b[2] = 0x7FFF;
    w->b[1] = 0x7FFF;
    w->b[0] = 0x7FFF;
    w->a[2] = 0x7FFF;
    w->a[1] = 0x7FFF;
    w->a[0] = 0x7FFF;
    w->org[2] = 0x7FFF;
    w->org[1] = 0x7FFF;
    w->org[0] = 0x7FFF;
    w->on_hit = 0;
    w->b24 = 0;
    w->a16 = 0;
    w->a12 = 1;
    w->lo12 = 0;
}
