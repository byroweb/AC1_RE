#include "ac1/ac1.h"
extern s32 D_8017E5C4;
extern s32 D_8017E5C8;
extern s32 D_8017E5CC;
extern s32 D_8017E5D0;
s32 TestEvent(s32);

s32 func_800688AC(void)
{
    while (1) {
        if (TestEvent(D_8017E5C4) == 1) return 0;
        if (TestEvent(D_8017E5C8) == 1) return 1;
        if (TestEvent(D_8017E5CC) == 1) return 2;
        if (TestEvent(D_8017E5D0) == 1) return 3;
    }
}
