#include "ac1_types.h"
typedef struct {
    u8 pad;
    u8 flag;        /* +0x01 */
} Func80099ECCObj;
void func_80099ECC(Func80099ECCObj *o)
{
    o->flag = 1;
}
