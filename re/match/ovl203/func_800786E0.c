#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

extern void func_80078258(AcEntity *, u16, u16);
extern s32 func_8004F29C(AcMover *, AcPos *);

/* Actor step: refresh, run the mover, then the state handler unless the mover ended it. */
void func_800786E0(AcEntity *e) {
    func_80078258(e, D_80039D18->unk214, D_80039D18->speed_cap);
    if (func_8004F29C(&e->mover, &e->pos) == 0) {
        if (e->fn_state != 0) {
            e->fn_state(e, e->state);
        }
    }
}
