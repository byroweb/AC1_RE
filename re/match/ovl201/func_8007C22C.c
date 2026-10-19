#include "ac1_types.h"
typedef struct {
    u8 pad[0x42];
    u16 a;          /* +0x42 */
} Func8007C22CObj;
void func_8007C22C(Func8007C22CObj *o)
{
    o->a -= 32;
}
