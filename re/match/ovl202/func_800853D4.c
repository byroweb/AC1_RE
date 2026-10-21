#include "ac1/ac1.h"
#include "m2c_compat.h"
#define F(o) M2C_FIELD(p, s16 *, o)

void func_800853D4(void *p)
{
    F(0xBC) += F(0x1A);
    if (F(0xBC) < F(0x20)) {
kill:
        F(0x12) = 0;
        return;
    }
    F(0xBA) += F(0x18);
    if (F(0xBA) <= 0) goto kill;
    F(0x1A) += F(0x1E);
    F(0x18) += F(0x1C);
}
