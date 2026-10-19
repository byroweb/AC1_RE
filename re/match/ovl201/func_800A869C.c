#include "ac1_types.h"

extern u32 D_801F07E8;
extern u32 D_801BC448;
extern u8 func_800A86D0(u32 a, u32 b, u32 c);

u8 func_800A869C(u32 c)
{
    return func_800A86D0(D_801F07E8, D_801BC448, c);
}
