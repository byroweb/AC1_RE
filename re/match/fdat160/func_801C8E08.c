#include "ac1/ac1.h"

typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad08[0x22 - 0x08];
    s8 lo[3];           /* +0x22 */
    s8 hi[3];           /* +0x25 */
    u8 id;              /* +0x28 */
} Func801C8E08Node;
typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Func801C8E08Box;
extern u16 D_801AB68A;
extern u8 D_80040FA0[][0x39C];
extern void func_801C8CBC(void *, Func801C8E08Box *, u8);

void func_801C8E08(Func801C8E08Node *n)
{
    Func801C8E08Box box;

    box.lo.vx = n->pos.x + (n->lo[0] << 8);
    box.hi.vx = n->pos.x + (n->hi[0] << 8);
    box.lo.vy = n->pos.y + (n->lo[1] << 8);
    box.hi.vy = n->pos.y + (n->hi[1] << 8);
    box.lo.vz = n->pos.z + (n->lo[2] << 8);
    box.hi.vz = n->pos.z + (n->hi[2] << 8);
    if (D_801AB68A != 0xFFFF) {
        func_801C8CBC(D_80040FA0[0], &box, n->id);
        func_801C8CBC(D_80040FA0[1], &box, n->id);
    }
}
