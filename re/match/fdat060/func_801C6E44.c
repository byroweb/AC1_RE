/* fdat060 (mission 30 "Eliminate Squatters", entry 60 @ 0x801C4B40): set one
 * AI flag bit of the entity, the bit index coming from the record's byte +0x06. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x06];
    u8 bit;             /* +0x06 */
} Func801C6E44Rec;
void func_801C6E44(AcEntity *e, Func801C6E44Rec *r)
{
    e->flags |= 1 << r->bit;
}
