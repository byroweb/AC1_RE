#include "ac1/ac1.h"
/* ovl201 0x8007BA10 - once func_800517C8 says 0: if either D_80031A58[6] or
 * [7] holds a part 24..30, drop those >= 24 and show "Unequipped the radar.>"
 * (returning to func_8007BB48); else the func_8007BB48 check inline. */

extern u8 D_80031A58[];
extern char D_8004C19C[];      /* "Unequipped the radar.>" */
extern char D_8004C1B4[];       /* "Unequipped the right ARM weapon.>" */
extern s16 func_800517C8(void);
extern void func_8007BB48(UiObj *o);
extern void func_80078D84(UiObj *o);
extern void func_8005DED8(UiObj *o, void (*ret)(UiObj *), char *msg, s32 n);

void func_8007BA10(UiObj *o)
{
    if (func_800517C8() == 0) {
        if ((D_80031A58[6] != 255 && D_80031A58[6] >= 24 && D_80031A58[6] < 31)
            || (D_80031A58[7] != 255 && D_80031A58[7] >= 24 && D_80031A58[7] < 31)) {
            if (D_80031A58[6] >= 24) {
                D_80031A58[6] = 255;
            }
            if (D_80031A58[7] >= 24) {
                D_80031A58[7] = 255;
            }
            func_8005DED8(o, func_8007BB48, D_8004C19C, 12);
            o->update = ui_obj_nop2;
        } else if (D_80031A58[5] == 15) {
            D_80031A58[5] = 255;
            func_8005DED8(o, func_80078D84, D_8004C1B4, 12);
            o->update = ui_obj_nop2;
        } else {
            o->update = func_80078D84;
        }
    }
}
