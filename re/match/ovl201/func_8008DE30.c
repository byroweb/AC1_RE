#include "ac1/ac1.h"
/* ovl201 0x8008DE30 - draw the name of shop category `kind` into the VRAM
 * text area for list row `row` (five rows per 24-pixel column). */

extern void func_8005D15C(char *s, s32 x, s32 y);

void func_8008DE30(u8 kind, u8 row)
{
    char name[10][13] = {
        "HEAD        ",
        "CORE        ",
        "ARMS        ",
        "LEGS        ",
        "GENERATOR   ",
        "FCS         ",
        "OPTION PARTS",
        "BOOSTERS    ",
        "BACK WEAPON ",
        "ARM WEAPON  "
    };

    func_8005D15C(name[kind], (u8)(row / 5) * 24 + 960, (u8)(row % 5) * 16 + 176);
    DrawSync(0);
}
