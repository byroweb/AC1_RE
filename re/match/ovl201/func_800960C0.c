#include "ac1/ac1.h"

typedef struct {
    u8 b[0xC4];
} Blob800960C0;

extern Blob800960C0 D_80049EC8;
extern Blob800960C0 D_8017F6D0;

void func_800960C0(void)
{
    D_80049EC8 = D_8017F6D0;
}
