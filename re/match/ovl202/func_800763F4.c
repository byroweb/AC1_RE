#include "ac1/ac1.h"

typedef struct {
    u8 pad[0xCC];
    SVECTOR v;          /* +0xCC */
} AcTurn;

extern void func_8004ECA8(AcMover *m);

void func_800763F4(AcEntity *e)
{
    func_8004ECA8(&e->mover);
    if (--((u8 *)&e->mover)[2] == 0 && e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
    ((AcTurn *)e)->v.vx += ((AcTurn *)e)->v.vy;
    if (((AcTurn *)e)->v.vy >= 0) {
        if (((AcTurn *)e)->v.vx > ((AcTurn *)e)->v.vz) {
            ((AcTurn *)e)->v.vx = ((AcTurn *)e)->v.vz;
        }
    } else {
        if (((AcTurn *)e)->v.vx < ((AcTurn *)e)->v.vz) {
            ((AcTurn *)e)->v.vx = ((AcTurn *)e)->v.vz;
        }
    }
    e->yaw += ((AcTurn *)e)->v.vx;
}
