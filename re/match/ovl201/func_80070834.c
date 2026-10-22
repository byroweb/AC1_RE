#include "ac1/ac1.h"
/* ovl201 0x80070834 - clear the 64 x 196 VRAM area at (832, 316) to black. */

extern int ClearImage(RECT *rect, u8 r, u8 g, u8 b);
extern int DrawSync(int mode);

void func_80070834(void)
{
    RECT r = { 832, 316, 64, 196 };

    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
}
