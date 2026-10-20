/* fdat004 (25 twins): node box trigger.  The box is the node position plus its s8
 * lo/hi offsets * 256 (as in func_801C6948).  Unless the mission is ending, a player
 * inside the box sets bit +0x28 of 0x80040FB0, and box 1 / 2 is published at
 * 0x800411C6 / 0x800411D6.  For box 1 every other active AC with faction bit 0x100
 * that is outside the box gets fn_move(e, 8, &box). */
#include "ac1/ac1.h"
typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad08[0x22 - 0x08];
    s8 lo[3];           /* +0x22 */
    s8 hi[3];           /* +0x25 */
    u8 id;              /* +0x28 */
} Func801C6A64Node;
typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Func801C6A64Box;
extern SVECTOR D_801A26C0;
extern u16 D_80040FB0[];  /* flag words; [0] read-modify-written through the la form */
extern Func801C6A64Box D_800411C6;
extern Func801C6A64Box D_800411D6;
void func_801C6A64(Func801C6A64Node *n)
{
    Func801C6A64Box box;
    AcEntity *e;
    s32 i;

    box.lo.vx = n->pos.x + (n->lo[0] << 8);
    box.hi.vx = n->pos.x + (n->hi[0] << 8);
    box.lo.vy = n->pos.y + (n->lo[1] << 8);
    box.hi.vy = n->pos.y + (n->hi[1] << 8);
    box.lo.vz = n->pos.z + (n->lo[2] << 8);
    box.hi.vz = n->pos.z + (n->hi[2] << 8);
    if (g_mission_end_seq != 0xFFFF) {
        if (D_801A26C0.vx >= box.lo.vx && D_801A26C0.vx <= box.hi.vx && D_801A26C0.vy >= box.lo.vy &&
            D_801A26C0.vy <= box.hi.vy && D_801A26C0.vz >= box.lo.vz && D_801A26C0.vz <= box.hi.vz)
            D_80040FB0[0] |= 1 << n->id;
        switch (n->id) {
        case 1:
            D_800411C6 = box;
            break;
        case 2:
            D_800411D6 = box;
            break;
        }
    }
    if (n->id == 1) {
        e = &g_ac_entities[1];
        i = 15;
        do {
            if (e->def != 0 && (e->faction & 0x100)) {
                if (e->pos.x < box.lo.vx || e->pos.x > box.hi.vx || e->pos.y < box.lo.vy ||
                    e->pos.y > box.hi.vy || e->pos.z < box.lo.vz || e->pos.z > box.hi.vz) {
                    if (e->fn_move != 0)
                        ((void (*)(AcEntity *, s32, Func801C6A64Box *))e->fn_move)(e, 8, &box);
                }
            }
            e++;
        } while (--i != 0);
    }
}
