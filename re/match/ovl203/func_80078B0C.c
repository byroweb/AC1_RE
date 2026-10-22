#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

void func_80078B0C(AcEntity *e) {
    e->fire_flags &= 0xFFF0;
    e->flags &= ~D_80039D18->flag_mask;
}
