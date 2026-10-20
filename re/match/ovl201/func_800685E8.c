/* ovl201 0x800685E8  size 424  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK EnableEvent(s32);                           /* extern */
M2C_UNK EnterCriticalSection();                     /* extern */
M2C_UNK ExitCriticalSection();                      /* extern */
s32 OpenEvent(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK);  /* extern */
extern s32 D_8017E5B4;
extern s32 D_8017E5B8;
extern s32 D_8017E5BC;
extern s32 D_8017E5C0;
extern s32 D_8017E5C4;
extern s32 D_8017E5C8;
extern s32 D_8017E5CC;
extern s32 D_8017E5D0;

void func_800685E8(void) {
    EnterCriticalSection();
    D_8017E5B4 = OpenEvent(0xF4000001, 4, 0x2000, 0);
    D_8017E5B8 = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    D_8017E5BC = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    D_8017E5C0 = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    D_8017E5C4 = OpenEvent(0xF0000011, 4, 0x2000, 0);
    D_8017E5C8 = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    D_8017E5CC = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    D_8017E5D0 = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    EnableEvent(D_8017E5B4);
    EnableEvent(D_8017E5B8);
    EnableEvent(D_8017E5BC);
    EnableEvent(D_8017E5C0);
    EnableEvent(D_8017E5C4);
    EnableEvent(D_8017E5C8);
    EnableEvent(D_8017E5CC);
    EnableEvent(D_8017E5D0);
    ExitCriticalSection();
}
