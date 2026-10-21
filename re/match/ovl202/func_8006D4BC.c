#include "ac1/ac1.h"
/* ovl202 0x8006D4BC - nearest entry of the 0x801E8B74 table whose class
 * bits (low 3 of +0x0E of what it points at) meet mask. */

typedef struct {
    u8 unk00[0xE];
    u16 flags;          /* +0x0E low 3 bits: class */
} NearObj;

typedef struct {
    NearObj *obj;       /* +0x00; 0 ends the table */
    u8 unk04[0xA];
    s16 dist;           /* +0x0E */
    u8 unk10[0xC];
} NearEnt;

extern NearEnt D_801E8B74[];

NearEnt *func_8006D4BC(s32 mask)
{
    s32 min = 0x7FFFFFFF;
    NearEnt *e = D_801E8B74;
    NearEnt *best = 0;

    while (e->obj) {
        if ((e->obj->flags & 7) & mask) {
            if (e->dist < min) {
                min = e->dist;
                best = e;
            }
        }
        e++;
    }
    return best;
}
