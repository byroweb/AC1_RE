#include "ac1/ac1.h"
/* ovl201 0x800843A8 - draw up to 5 rows of two frames, one per set bit of D_800485F0 */
extern UiObj *func_8005D754(s32 layer, void *frame, s32 x, s32 y, s32 kind);
extern u8 func_800842D4(void);
extern u8 D_800B8574[];
extern u8 D_800B855C[];

void func_800843A8(void)
{
    s16 y;
    s16 i;
    u8 n;

    y = 0;
    n = func_800842D4();
    for (i = 0; i < n; i++) {
        if (i >= 5) {
            break;
        }
        func_8005D754(4, D_800B8574, 0, y, 4);
        func_8005D754(3, D_800B855C, 16, y, 4);
        y += 24;
    }
}
