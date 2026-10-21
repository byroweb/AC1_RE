#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 a;
    u16 step;
    s16 c;
    s16 d;
} Wave8;

extern Wave8 D_800906C8[];

void func_80085328(void *p)
{
    M2C_FIELD(p, s16 *, 0xBA) = (M2C_FIELD(p, u16 *, 0x1E) * M2C_FIELD(p, s16 *, 0x1A)) >> 12;
    M2C_FIELD(p, s16 *, 0x1A) += M2C_FIELD(p, u16 *, 0x1C);
    if (M2C_FIELD(p, s16 *, 0x1A) <= 0) {
        M2C_FIELD(p, s16 *, 0x12) = 0;
        return;
    }
    M2C_FIELD(p, u16 *, 0x1C) += D_800906C8[M2C_FIELD(p, u8 *, 0x19)].step;
}
