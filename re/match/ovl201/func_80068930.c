/* ovl201 0x80068930  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK TestEvent(s32);                             /* extern */
extern s32 D_8017E5B4;
extern s32 D_8017E5B8;
extern s32 D_8017E5BC;
extern s32 D_8017E5C0;

void func_80068930(void) {
    TestEvent(D_8017E5B4);
    TestEvent(D_8017E5B8);
    TestEvent(D_8017E5BC);
    TestEvent(D_8017E5C0);
}
