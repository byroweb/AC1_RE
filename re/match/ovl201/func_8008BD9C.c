#include "ac1_types.h"
typedef struct {
    u8 pad[1];
    u8 flag;        /* +0x01 */
} Func8008BD9CObj;
void func_8008BD9C(Func8008BD9CObj *o)
{
    o->flag = 1;
}
