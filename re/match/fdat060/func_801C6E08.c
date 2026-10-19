/* fdat060 (mission 30 "Eliminate Squatters", entry 60 @ 0x801C4B40): toggle one
 * AI flag bit of the entity, the bit index coming from the record's byte +0x2D. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x2D];
    u8 bit;             /* +0x2D */
} Func801C6E08Rec;
void func_801C6E08(AcEntity *e, Func801C6E08Rec *r)
{
    e->flags ^= 1 << r->bit;
}
