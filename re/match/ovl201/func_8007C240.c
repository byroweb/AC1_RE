#include "ac1/ac1.h"
/* ovl201 0x8007C240 - spin and fly sideways by dx; once past the limit of
 * entry sub of table set (or +-800 with no entry) retire and hand the spin
 * and dx back to the owner. */

typedef struct {
    s16 lim;
    u8 pad[14];
} FlyLim;

typedef struct {
    FlyLim *lims;
    u8 pad[12];
} FlySet;

typedef struct {
    s16 dx;                     /* +0x7C */
    u8 pad7e[2];
    u16 set;                    /* +0x80 */
    u8 pad82[2];
    UiObj *owner;               /* +0x84 */
    u16 sub;                    /* +0x88 255 = no limit entry */
} FlyWork;

typedef struct {
    u8 pad7c[0x26];
    s16 rot;                    /* +0xA2 */
    s16 dx;                     /* +0xA4 */
} OwnerWork;

extern FlySet D_800BCAC0[];

#define FLY(o) ((FlyWork *)&(o)->w)

void func_8007C240(UiObj *o)
{
    OwnerWork *ow;
    s32 x;
    s16 lim;

    o->rot.vy -= 32;
    ow = (OwnerWork *)&FLY(o)->owner->w;
    x = o->pos.vx += FLY(o)->dx;
    if (FLY(o)->sub != 255) {
        lim = D_800BCAC0[FLY(o)->set].lims[FLY(o)->sub].lim;
        if (x >= -lim && lim >= x) {
            return;
        }
    } else if ((u32)(x + 800) < 1601) {
        return;
    }
    o->kill = 1;
    ow->rot = o->rot.vy;
    ow->dx = FLY(o)->dx;
}
