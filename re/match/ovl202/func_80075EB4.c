#include "ac1/ac1.h"

typedef struct {
    u8 pad[0xDC];
    u8 aim_mode;        /* +0xDC */
    u8 padDD[3];
    AcEntity *aim_at;   /* +0xE0 */
    s16 off_x;          /* +0xE4 */
    s16 padE6;
    s16 off_z;          /* +0xE8 */
} AcAim;

extern long ratan2(long y, long x);

s32 func_80075EB4(AcEntity *e)
{
    AcAim *a = (AcAim *)e;
    AcEntity *t;
    s32 ang;

    if (a->aim_mode < 6) {
        t = a->aim_at;
        if (t == 0 || t->def == 0) {
            return;
        }
        ang = -ratan2(t->pos.x + a->off_x - e->pos.x, t->pos.z + a->off_z - e->pos.z);
        switch (a->aim_mode) {
        case 1:
        case 5:
            ang += 2048;
            break;
        case 2:
            ang += 1024;
            break;
        case 3:
            ang += 3072;
            break;
        }
    } else if (a->aim_mode == 6) {
        ang = -ratan2(a->off_x - e->pos.x, a->off_z - e->pos.z);
    }
    return ang;
}
