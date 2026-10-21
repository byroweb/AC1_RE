#include "ac1/ac1.h"
/* ovl202 0x80071768 - walk the polygon buckets still set in mask; test every polygon
 * whose normal can face the probe (func_80071610) */
typedef struct Probe {
    u8 unk00[0x4C];
    s32 dy;                     /* +0x4C */
    u8 unk50[0x3C];
    s32 group;                  /* +0x8C polygons per bucket bit */
    u8 unk90[0xC];
    CollPoly poly;              /* +0x9C copy of the polygon under test */
    CollPoly *cur;              /* +0xB0 */
    CollPoly *end;              /* +0xB4 */
    CollPoly *group_end;        /* +0xB8 */
    u32 mask;                   /* +0xBC buckets still to test */
} Probe;
void func_80071610(Probe *o);

void func_80071768(Probe *o)
{
    do {
        if (o->mask & 1) {
            o->group_end = o->cur + o->group;
            if (o->end < o->group_end) {
                o->group_end = o->end;
            }
            do {
                if (!((o->dy > 0 && o->cur->n[1] <= 0) || (o->dy < 0 && o->cur->n[1] >= 0))) {
                    o->poly = *o->cur;
                    func_80071610(o);
                }
                o->cur++;
            } while (o->cur < o->group_end);
        } else {
            o->cur += o->group;
        }
        o->mask >>= 1;
    } while (o->cur < o->end);
}
