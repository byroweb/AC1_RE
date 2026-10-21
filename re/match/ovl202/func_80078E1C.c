#include "ac1/ac1.h"

typedef struct {
    u8 state;           /* +0x00 */
    u8 pad01[0x13];
    u16 flags;          /* +0x14 */
    u8 pad16[4];
    u16 bounty;         /* +0x1A */
} AcDefKill;

extern s32 D_80048614[];
extern void func_80078D78(AcEntity *e, AcDefKill *d);

void func_80078E1C(AcEntity *e)
{
    AcDefKill *d;

    d = e->def;
    if (e->faction & 2) {
        D_80048614[0] += (u16)(d->bounty / 200);
    }
    (*(void (**)(AcEntity *))((u8 *)g_objective_obj + 0x10))(e);
    func_80078D78(e, d);
    if (!(d->flags & 0x10)) {
        d->state = 3;
    }
}
