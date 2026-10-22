#include "ac1/ac1.h"
/* ovl203 0x800506A4 - radar blips for every live projectile of kind func_80082FC0 not fired by this pilot */
typedef struct Pilot {
    u8 unk000[0x394];
    u32 id;                     /* +0x394 */
} Pilot;
extern Pilot *D_80039D18;
extern Projectile D_801A60C4[256];
extern u8 D_800895DC[];
void func_80082FC0(Projectile *p);
void func_80050AF0(Projectile *p, u8 *a, u8 *b, u8 *c, s32 d, s32 e);

void func_800506A4(s32 a, s32 b)
{
    Projectile *p;
    s32 i;
    u32 me;

    me = D_80039D18->id & 0x7FFFFFFF;
    p = D_801A60C4;
    for (i = 256; i != 0; i--) {
        if (p->free == 0 && p->think == func_80082FC0 && p->unk12 > 0
            && ((u32)p->owner & 0x7FFFFFFF) != me) {
            func_80050AF0(p, D_800895DC, D_800895DC, D_800895DC, a, b);
        }
        p++;
    }
}
