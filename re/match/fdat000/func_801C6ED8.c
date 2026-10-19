/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0010): choose the
 * entity's area node (+0x124) from the 512-node table at 0x80198B08.  A sticky (+0x2D)
 * current node with a target set is kept; otherwise the best live node of the entity's
 * group and kind 248 wins: type 1 scores by how close the armour ratio is to the node's
 * +0x2E, type 3 by a random roll (+0x2F bonus for the current node).  Inside the node's
 * box (+0x22..+0x27, in 256-unit steps) the target (+0x126) is cleared and 0 returned;
 * outside, the reachable-node mask of the section is rebuilt and, when needed, a new
 * target picked with 801C6D10 (returns 1).  No qualifying node: +0x124 = -1, 0. */
#include "ac1/ac1.h"
typedef struct {
    AcPos pos;          /* +0x00 x, y, z; w is the kind index (-1 = unused) */
    u8 pad08[0x0E - 0x08];
    s16 bit;            /* +0x0E */
    u8 pad10[0x22 - 0x10];
    s8 lo[3];           /* +0x22 box, in 256-unit steps from pos */
    s8 hi[3];           /* +0x25 */
    u8 kids[2];         /* +0x28 */
    u8 group;           /* +0x2A */
    u8 anim;            /* +0x2B */
    u8 type;            /* +0x2C */
    u8 sticky;          /* +0x2D */
    u8 weight;          /* +0x2E */
    u8 bonus;           /* +0x2F */
    u8 pad30[0x34 - 0x30];
} Func801C6ED8Node;
typedef struct {
    u8 pad[0x0E];
    s16 kind;           /* +0x0E */
    u8 pad10[0x18 - 0x10];
} Func801C6ED8Kind;
typedef struct {
    u8 pad0[0x08];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x16 - 0x10];
    s16 section;        /* +0x16 */
    u8 pad18[0x42 - 0x18];
    u8 anim;            /* +0x42 */
    u8 pad43;
    s16 timer;          /* +0x44 */
    u8 pad46[0xFE - 0x46];
    u8 group;           /* +0xFE */
    u8 padff[0x124 - 0xFF];
    s16 area;           /* +0x124 */
    s16 node;           /* +0x126 */
    u8 pad128[0x160 - 0x128];
    u16 ap;             /* +0x160 */
    u16 ap_max;         /* +0x162 */
} Func801C6ED8Obj;
extern Func801C6ED8Node D_80198B08[];
extern Func801C6ED8Kind D_80198800[];
extern u8 D_801D0B68[][64];
extern s32 func_80014D78(s32 n);
extern void func_8006E780(u32 *mask, void *section);
extern void func_801C6D10(Func801C6ED8Obj *o, s32 id, u32 *mask);
extern s32 func_80073904();
int abs(int);
s32 func_801C6ED8(Func801C6ED8Obj *o)
{
    Func801C6ED8Node *best = 0;
    s32 best_score = -1;
    Func801C6ED8Node *n = D_80198B08;
    s16 cur = o->area;  /* s16: costs the 8-byte frame slot retail has */
    u8 group = o->group;
    s32 i, score;
    u32 mask[16];       /* one bit per node */

    if (cur != -1 && o->node != -1 && D_80198B08[cur].sticky == 1) {
        best = &D_80198B08[cur];
    } else {
        for (i = 0; i < 512; i++, n++) {
            if (n->pos.w == -1)
                continue;
            if (n->group != group)
                continue;
            if (n->type == 0)
                continue;
            if (D_80198800[n->pos.w].kind != 248)
                continue;
            switch (n->type) {
            case 1:
                score = ((o->ap << 8) / o->ap_max) - n->weight;
                score = 256 - abs(score);
                break;
            case 3:
                score = n->weight;
                if (i == cur)
                    score += n->bonus;
                score = func_80014D78(score);
                break;
            }
            if (best_score < score) {
                best = n;
                best_score = score;
            }
        }
        if (best == 0)
            goto none;
        o->area = best - D_80198B08;
    }
    if (o->pos.x >= best->pos.x + (best->lo[0] << 8) && best->pos.x + (best->hi[0] << 8) >= o->pos.x
        && o->pos.y >= best->pos.y + (best->lo[1] << 8) - 8 && best->pos.y + (best->hi[1] << 8) >= o->pos.y
        && o->pos.z >= best->pos.z + (best->lo[2] << 8) && best->pos.z + (best->hi[2] << 8) >= o->pos.z) {
        o->node = -1;
        return 0;
    }
    func_8006E780(mask, D_801D0B68[o->section]);
    if (cur != o->area) {
    retarget:
        func_801C6D10(o, o->area, mask);
        if (o->node != -1) {
            o->anim = D_80198B08[o->node].anim;
            o->timer = func_80073904();
        }
    } else if (o->node == -1 || !(mask[D_80198B08[o->node].bit >> 5] & (1 << (D_80198B08[o->node].bit & 31)))) {
        goto retarget;
    }
    return 1;
none:
    o->area = -1;
    return 0;
}
