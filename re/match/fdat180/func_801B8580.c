#include "ac1/ac1.h"

typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad08[0x22 - 0x08];
    s8 lo[3];           /* +0x22 */
    s8 hi[3];           /* +0x25 */
    u8 id;              /* +0x28 */
} Func801B8580Node;
typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Func801B8580Box;
extern u16 D_8019B492;
extern void *D_80039D30;
extern void *D_80039D34;
extern void func_801B8434(void *, Func801B8580Box *, u8);

void func_801B8580(Func801B8580Node *n)
{
    Func801B8580Box box;

    box.lo.vx = n->pos.x + (n->lo[0] << 8);
    box.hi.vx = n->pos.x + (n->hi[0] << 8);
    box.lo.vy = n->pos.y + (n->lo[1] << 8);
    box.hi.vy = n->pos.y + (n->hi[1] << 8);
    box.lo.vz = n->pos.z + (n->lo[2] << 8);
    box.hi.vz = n->pos.z + (n->hi[2] << 8);
    if (D_8019B492 != 0xFFFF) {
        func_801B8434(D_80039D30, &box, n->id);
        func_801B8434(D_80039D34, &box, n->id);
    }
}
