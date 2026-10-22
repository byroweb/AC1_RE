#include "ac1/ac1.h"

extern u32 D_801A2568;          /* pad: buttons pressed this frame */
extern void func_8006E148(UiObj *o, s32 dir);

void func_8006E0A8(UiObj *o)
{
    u32 *pad = &D_801A2568;

    if (*pad & 0x1000) {
        func_8006E148(o, 'U');
    }
    if (*pad & 0x2000) {
        func_8006E148(o, 'R');
    }
    if (*pad & 0x4000) {
        func_8006E148(o, 'D');
    }
    if (*pad & 0x8000) {
        func_8006E148(o, 'L');
    }
}
