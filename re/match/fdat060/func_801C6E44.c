#include "ac1_types.h"
/* fdat060 (mission 30 "Eliminate Squatters", entry 60 @ 0x801C4B40): set one bit of
 * the object's flag word, the bit index coming from the record's byte +0x06. */
typedef struct {
    u8 pad[0x15C];
    u32 flags;          /* +0x15C */
} Func801C6E44Obj;
typedef struct {
    u8 pad[0x06];
    u8 bit;             /* +0x06 */
} Func801C6E44Rec;
void func_801C6E44(Func801C6E44Obj *o, Func801C6E44Rec *r)
{
    o->flags |= 1 << r->bit;
}
