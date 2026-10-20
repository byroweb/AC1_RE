/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0034): clear the low
 * nibble of fire_flags and the AI flag bits named by the record's mask at +0x38. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x38];
    u16 mask;           /* +0x38 */
} Func801C5634Rec;
void func_801C5634(AcEntity *e, Func801C5634Rec *r)
{
    e->fire_flags &= 0xFFF0;
    e->flags &= ~r->mask;
}
