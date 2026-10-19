#include "ac1_types.h"
/* fdat060 (mission 30 "Eliminate Squatters", entry 60 @ 0x801C4B40): toggle one
 * bit of the object's flag word, the bit index coming from the record's byte +0x06
 * (sibling of func_801C6E44, which sets the bit, and func_801C6E08, bit from +0x2D). */
typedef struct {
    u8 pad[0x15C];
    u32 flags;          /* +0x15C */
} Func801C6E60Obj;
typedef struct {
    u8 pad[0x06];
    u8 bit;             /* +0x06 */
} Func801C6E60Rec;
void func_801C6E60(Func801C6E60Obj *o, Func801C6E60Rec *r)
{
    o->flags ^= 1 << r->bit;
}
