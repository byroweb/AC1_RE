/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0022): pick the
 * nearest of the 512 nodes in the 0x34-byte table at 0x80198B08 that is live, in the
 * entity's group (+0xFE), of kind 248 and is id or has id below it; nodes whose bit
 * is clear in mask[] cost an extra 0x40000000.  Stores the index at +0x126 (-1 if
 * none). */
#include "ac1/ac1.h"
typedef struct {
    AcPos pos;          /* +0x00 x, y, z; w is the kind index (-1 = unused) */
    u8 pad08[0x0E - 0x08];
    s16 bit;            /* +0x0E index into the caller's bit mask */
    u8 pad10[0x2A - 0x10];
    u8 group;           /* +0x2A */
    u8 pad2b[0x34 - 0x2B];
} Func801C6D10Node;
typedef struct {
    u8 pad[0x0E];
    s16 kind;           /* +0x0E */
    u8 pad10[0x18 - 0x10];
} Func801C6D10Kind;
typedef struct {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[0xFE - 0x10];
    u8 group;           /* +0xFE */
    u8 padff[0x126 - 0xFF];
    s16 node;           /* +0x126 */
} Func801C6D10Obj;
extern Func801C6D10Node D_80198B08[];
extern Func801C6D10Kind D_80198800[];
extern s32 func_801C6B30(Func801C6D10Node *n, s32 id);
void func_801C6D10(Func801C6D10Obj *o, s32 id, u32 *mask)
{
    Func801C6D10Node *n = D_80198B08;
    Func801C6D10Node *best = 0;
    s32 best_d = 0x7FFFFFFF;
    s32 i;
    u8 group = o->group;
    s32 dx, dy, dz, d;

    for (i = 0; i < 512; n++, i++) {
        if (n->pos.w == -1)
            continue;
        if (n->group != group)
            continue;
        if (D_80198800[n->pos.w].kind != 248)
            continue;
        if (i != id && !func_801C6B30(n, id))
            continue;
        dx = (n->pos.x - o->pos.x) >> 2;
        dy = (n->pos.y - o->pos.y) >> 2;
        dz = (n->pos.z - o->pos.z) >> 2;
        d = dx * dx + dy * dy + dz * dz;
        if (!(mask[n->bit >> 5] & (1 << (n->bit & 31))))
            d += 0x40000000;
        if (d < best_d) {
            best = n;
            best_d = d;
        }
    }
    if (best)
        o->node = best - D_80198B08;
    else
        o->node = -1;
}
