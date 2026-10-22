#include "ac1/ac1.h"
/* ovl201 0x800A51B8 - once the box is open, draw the result panel; on a fresh
 * entry (D_80039CAD == 0) copy the 0xA0-byte block D_80031A94 into D_80042AA8
 * and zero both counters, then clamp the counters to 999. */
typedef struct { u8 b[0xA0]; } Blk160;
extern Blk160 D_80031A94;
extern Blk160 D_80042AA8;
extern u8 D_80039CAD;
extern u8 D_80039CAE;
extern u16 D_80039D08;
extern u16 D_80039D0A;
extern u8 D_800BDD08[];
extern s32 func_800517C8(void);
extern UiObj *func_8005CAE0(void *def, s32 x, s32 y, s32 z);
extern void func_800A5318(UiObj *o);

void func_800A51B8(UiObj *o)
{
    if ((s16)func_800517C8() == 0) {
        func_8005CAE0(D_800BDD08, 0x10, 0x10, 2);
        if (D_80039CAD == 0) {
            D_80039CAE = 0x5A;
            D_80042AA8 = D_80031A94;
            D_80039D0A = 0;
            D_80039D08 = 0;
        }
        o->update = func_800A5318;
        {
            u16 *p = &D_80039D08;

            if (*p >= 1000) {
                *p = 999;
            }
        }
        if (D_80039D0A >= 1000) {
            D_80039D0A = 999;
        }
    }
}
