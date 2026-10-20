/* exe 0x80011FD8  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK LoadImage(s32, s32);                        /* extern */
M2C_UNK OpenTIM();                                  /* extern */
s32 ReadTIM(M2C_UNK *);                             /* extern */

void func_80011FD8(void) {
    union { s8 b[0x18]; s32 w; } sp_frame;

    OpenTIM();
loop_1:
    if (ReadTIM(&(*(M2C_UNK *)(sp_frame.b + 0x0))) != 0) {
        if ((*(s32 *)(sp_frame.b + 0x8)) != 0) {
            LoadImage((*(s32 *)(sp_frame.b + 0x4)), (*(s32 *)(sp_frame.b + 0x8)));
        }
        if ((*(s32 *)(sp_frame.b + 0x10)) != 0) {
            LoadImage((*(s32 *)(sp_frame.b + 0xC)), (*(s32 *)(sp_frame.b + 0x10)));
        }
        goto loop_1;
    }
}
