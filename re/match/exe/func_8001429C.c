/* exe 0x8001429C  size 156  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK VectorNormalSS(u16 *, void *);              /* extern */

void func_8001429C(void *arg0) {
    union { s8 b[0x8]; s32 w; } sp_frame;

    (*(u16 *)(sp_frame.b + 0x0)) = M2C_FIELD(arg0, u16 *, 0);
    (*(u16 *)(sp_frame.b + 0x2)) = M2C_FIELD(arg0, u16 *, 2);
    (*(u16 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, u16 *, 4);
    VectorNormalSS(&(*(u16 *)(sp_frame.b + 0x0)), arg0);
    (*(u16 *)(sp_frame.b + 0x0)) = M2C_FIELD(arg0, u16 *, 6);
    (*(u16 *)(sp_frame.b + 0x2)) = M2C_FIELD(arg0, u16 *, 8);
    (*(u16 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, u16 *, 0xA);
    VectorNormalSS(&(*(u16 *)(sp_frame.b + 0x0)), arg0 + 6);
    (*(u16 *)(sp_frame.b + 0x0)) = M2C_FIELD(arg0, u16 *, 0xC);
    (*(u16 *)(sp_frame.b + 0x2)) = M2C_FIELD(arg0, u16 *, 0xE);
    (*(u16 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, u16 *, 0x10);
    VectorNormalSS(&(*(u16 *)(sp_frame.b + 0x0)), arg0 + 0xC);
}
