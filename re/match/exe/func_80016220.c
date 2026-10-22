#include "ac1/ac1.h"

extern volatile s32 D_80049F94;
extern void (*volatile D_80049F98)();

void func_80016220(void)
{
    D_80049F94++;
    if (D_80049F98 != 0) {
        D_80049F98();
    }
}
