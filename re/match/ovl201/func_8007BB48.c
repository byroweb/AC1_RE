#include "ac1/ac1.h"
/* ovl201 0x8007BB48 - if the D_80031A58[5] flag is 15, clear it to 255 and
 * show the D_8004C1B4 message (returning to func_80078D84); else go straight
 * to func_80078D84. */

extern u8 D_80031A58[];
extern char D_8004C1B4[];      /* "Unequipped the right ARM weapon.>" */
extern void func_80078D84(UiObj *o);
extern void func_8005DED8(UiObj *o, void (*ret)(UiObj *), char *msg, s32 n);

void func_8007BB48(UiObj *o)
{
    if (D_80031A58[5] == 15) {
        D_80031A58[5] = 255;
        func_8005DED8(o, func_80078D84, D_8004C1B4, 12);
        o->update = ui_obj_nop2;
    } else {
        o->update = func_80078D84;
    }
}
