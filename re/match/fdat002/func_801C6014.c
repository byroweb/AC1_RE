/* fdat002 (16 twins): cylinder overlap test (func_80012794) of the object's position
 * (+0x22), radius (+0x16) and height (+0x18) against the player's (0x801A26C0, radius
 * and height at +0x20/+0x22), no velocity. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x16];
    u16 radius;         /* +0x16 */
    u16 height;         /* +0x18 */
    u8 pad1a[0x22 - 0x1A];
    SVECTOR pos;        /* +0x22 */
} Func801C6014Obj;
extern SVECTOR D_801A26C0;
extern u16 D_801A26E0;
extern u16 D_801A26E2;
extern s32 func_80012794(SVECTOR *pos, s32 r1, s32 h1, SVECTOR *vel, SVECTOR *opos, s32 r2, s32 h2);
void func_801C6014(Func801C6014Obj *o)
{
    func_80012794(&o->pos, o->radius, o->height, 0, &D_801A26C0, D_801A26E0, D_801A26E2);
}
