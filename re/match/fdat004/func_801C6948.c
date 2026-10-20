/* fdat004 (23 twins): fire func_8004D610 over a node's box: corners = node position
 * plus the s8 lo/hi offsets * 256, then the node's effect arguments (+0x28..+0x2E). */
#include "ac1/ac1.h"
typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad08[0x22 - 0x08];
    s8 lo[3];           /* +0x22 */
    s8 hi[3];           /* +0x25 */
    u16 a;              /* +0x28 */
    u8 b;               /* +0x2A */
    u8 c;               /* +0x2B */
    u8 d;               /* +0x2C */
    s8 e;               /* +0x2D */
    u8 f;               /* +0x2E */
} Func801C6948Node;
extern void func_8004D610(SVECTOR *lo, SVECTOR *hi, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
void func_801C6948(Func801C6948Node *n)
{
    SVECTOR lo, hi;

    lo.vx = n->pos.x + (n->lo[0] << 8);
    lo.vy = n->pos.y + (n->lo[1] << 8);
    lo.vz = n->pos.z + (n->lo[2] << 8);
    hi.vx = n->pos.x + (n->hi[0] << 8);
    hi.vy = n->pos.y + (n->hi[1] << 8);
    hi.vz = n->pos.z + (n->hi[2] << 8);
    func_8004D610(&lo, &hi, n->a, n->b, n->c, n->d, n->e, n->f);
}
