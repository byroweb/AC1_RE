#include "ac1/ac1.h"

extern u8 D_80031A58; /* core */
extern u8 D_80031A59; /* booster */
extern u8 D_80031A5A; /* head */
extern u8 D_80031A5B; /* arms */
extern u8 D_80031A60;
extern u8 D_80031A61; /* FCS */
extern u8 D_80031A63; /* legs */

s32 func_800892AC(void)
{
    if (D_80031A5A == 255 || D_80031A58 == 255 || D_80031A5B == 255 || D_80031A63 == 255
        || (D_80031A59 == 255 && D_80031A63 < 27) || (D_80031A59 != 255 && D_80031A63 >= 27)
        || D_80031A60 == 255 || D_80031A61 == 255) {
        return 1;
    }
    return 0;
}
