#include "ac1_types.h"
typedef struct {
    u8 pad[1];
    u8 flag;        /* +0x01 */
} Func80095F4CObj;
void func_80095F4C(Func80095F4CObj *o)
{
    o->flag = 1;
}
