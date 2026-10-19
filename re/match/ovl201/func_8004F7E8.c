#include "ac1_types.h"
typedef struct {
    u8 pad[0x40];
    u16 x;          /* +0x40 */
    u16 y;          /* +0x42 */
    u16 w;          /* +0x44 */
} Func8004F7E8Obj;
void func_8004F7E8(Func8004F7E8Obj *o)
{
    o->x += 64;
    o->y += 32;
    o->w += 16;
}
