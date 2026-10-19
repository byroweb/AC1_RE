#include "ac1_types.h"
/* fdat060 (mission 30 "Eliminate Squatters", entry 60 @ 0x801C4B40): toggle one
 * bit of the object's flag word, the bit index coming from the record's byte +0x2D. */
typedef struct {
    u8 pad[0x15C];
    u32 flags;          /* +0x15C */
} Func801C6E08Obj;
typedef struct {
    u8 pad[0x2D];
    u8 bit;             /* +0x2D */
} Func801C6E08Rec;
void func_801C6E08(Func801C6E08Obj *o, Func801C6E08Rec *r)
{
    o->flags ^= 1 << r->bit;
}
