#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    u8 on;              /* +0x06 */
} Func801C6050Def;
typedef struct {
    u8 pad0[0x38];
    Func801C6050Def *def; /* +0x38 */
    u8 pad3C[4];
} Func801C6050Obj;      /* 0x40 */
extern Func801C6050Obj D_801D0B68[];

void func_801C6050(s32 i)
{
    D_801D0B68[i].def->on = 0;
}
