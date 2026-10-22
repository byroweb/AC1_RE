#include "ac1/ac1.h"

extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);

s32 func_800A8618(void)
{
    func_800309A8(1, 1, 2);
    for (;;) {
        if (func_800309A8(3, 1, 0) != 0) {
            break;
        }
        if (func_800309A8(0, 0, 0) & 0x80) {
            return 0;
        }
    }
    VSync(0);
    func_800309A8(1, 1, 0);
    VSync(0);
    return 1;
}
