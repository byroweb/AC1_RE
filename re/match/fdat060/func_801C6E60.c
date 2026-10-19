/* fdat060 (mission 30 "Eliminate Squatters", entry 60 @ 0x801C4B40): toggle one
 * AI flag bit of the entity, the bit index coming from the record's byte +0x06. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x06];
    u8 bit;             /* +0x06 */
} Func801C6E60Rec;
void func_801C6E60(AcEntity *e, Func801C6E60Rec *r)
{
    e->flags ^= 1 << r->bit;
}
