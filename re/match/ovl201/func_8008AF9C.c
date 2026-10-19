#include "ac1_types.h"
typedef struct {
    u8 pad;
    u8 flag;        /* +0x01 */
} Func8008AF9CObj;
void func_8008AF9C(Func8008AF9CObj *o)
{
    o->flag = 1;
}
