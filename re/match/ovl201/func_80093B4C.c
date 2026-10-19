#include "ac1_types.h"
typedef struct {
    u8 pad;
    u8 flag;        /* +0x01 */
} Func80093B4CObj;
void func_80093B4C(Func80093B4CObj *o)
{
    o->flag = 1;
}
