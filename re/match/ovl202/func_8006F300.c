#include "ac1/ac1.h"
/* ovl202 0x8006F300 - pick the box corner named by code (min/max per axis), then
 * func_8006F234.  One call per case: GCC cross-jumps the identical tails. */
typedef struct BoxObj {
    u8 unk00[0x64];
    SVECTOR min;                /* +0x64 */
    SVECTOR max;                /* +0x6C */
    u8 unk74[0x70];
    SVECTOR c;                  /* +0xE4 picked corner */
} BoxObj;
void func_8006F234(BoxObj *o);

void func_8006F300(s32 code, BoxObj *o)
{
    switch (code) {
    case 1:
    case 17:
    case 33:
        o->c.vx = o->min.vx;
        o->c.vy = o->min.vy;
        o->c.vz = o->min.vz;
        func_8006F234(o);
        break;
    case 2:
    case 21:
    case 37:
        o->c.vx = o->max.vx;
        o->c.vy = o->min.vy;
        o->c.vz = o->min.vz;
        func_8006F234(o);
        break;
    case 5:
    case 18:
    case 41:
        o->c.vx = o->min.vx;
        o->c.vy = o->max.vy;
        o->c.vz = o->min.vz;
        func_8006F234(o);
        break;
    case 6:
    case 22:
    case 45:
        o->c.vx = o->max.vx;
        o->c.vy = o->max.vy;
        o->c.vz = o->min.vz;
        func_8006F234(o);
        break;
    case 9:
    case 25:
    case 34:
        o->c.vx = o->min.vx;
        o->c.vy = o->min.vy;
        o->c.vz = o->max.vz;
        func_8006F234(o);
        break;
    case 10:
    case 29:
    case 38:
        o->c.vx = o->max.vx;
        o->c.vy = o->min.vy;
        o->c.vz = o->max.vz;
        func_8006F234(o);
        break;
    case 13:
    case 26:
    case 42:
        o->c.vx = o->min.vx;
        o->c.vy = o->max.vy;
        o->c.vz = o->max.vz;
        func_8006F234(o);
        break;
    case 14:
    case 30:
    case 46:
        o->c.vx = o->max.vx;
        o->c.vy = o->max.vy;
        o->c.vz = o->max.vz;
        func_8006F234(o);
        break;
    }
}
