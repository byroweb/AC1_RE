#include "ac1/ac1.h"
extern s32 D_8017E5B4;
extern s32 D_8017E5B8;
extern s32 D_8017E5BC;
extern s32 D_8017E5C0;
s32 TestEvent(s32);

s32 func_80068828(void)
{
    while (1) {
        if (TestEvent(D_8017E5B4) == 1) return 0;
        if (TestEvent(D_8017E5B8) == 1) return 1;
        if (TestEvent(D_8017E5BC) == 1) return 2;
        if (TestEvent(D_8017E5C0) == 1) return 3;
    }
}
