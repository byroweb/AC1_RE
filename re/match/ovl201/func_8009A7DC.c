#include "ac1_types.h"
typedef struct {
    u8 pad[1];
    u8 flag;        /* +0x01 */
} Func8009A7DCObj;
void func_8009A7DC(Func8009A7DCObj *o)
{
    o->flag = 1;
}
