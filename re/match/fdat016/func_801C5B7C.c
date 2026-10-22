#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x1C];
    u16 snd;            /* +0x1C */
    u8 pad1e[2];
    u8 arg;             /* +0x20 */
} Def;

extern s32 rand(void);
extern void func_80078598(AcEntity *);
extern s32 func_800754D0(AcEntity *, void *);
extern void func_80078E1C(AcEntity *);
extern s32 func_801C59F8(AcEntity *);
extern void func_80077B78(AcEntity *, s32, u16, u8, void *, s32, s32, s32);
extern Projectile *func_80086F20(s32, AcPos *, s32, s32, s32, void *, void *, void *, void *);
extern s32 D_801A5DC4;
extern u8 D_80039C78[];
extern u8 D_8019F52E[];
extern u8 D_801CDC20[];
extern u8 D_801CDC44[];
extern u8 D_801CDC48[];
extern u8 D_801CDC4C[];

void func_801C5B7C(AcEntity *e)
{
    func_80078598(e);
    e->fire_flags = 0;
    e->flags = 0;
    if (e->ai_mode != 0xFF) {
        if (func_800754D0(e, D_801CDC20) != 0) {
            func_80078E1C(e);
        }
        if (D_801A5DC4 & 3) {
            func_80086F20(0, &e->pos, 41, 80, (rand() & 3) + 6, D_80039C78, D_8019F52E, D_801CDC44, D_801CDC48)->unk12 = 20;
        }
    } else if (func_801C59F8(e) != 0) {
        if ((*(u32 *)&e->unk3a[2] & 0xFFFF00) == 0xC000) {
            if (rand() < 2622) {
                e->ai_sub = 5;
            }
        } else if (e->ai_sub2 == 0) {
            func_80077B78(e, 23, ((Def *)e->def)->snd, ((Def *)e->def)->arg, D_801CDC4C, 1, 0, 1);
            e->ai_sub2 = 1;
            e->fire_flags = 0xF;
            e->unk14a[0] = 180;
            e->unk14a[1] = 160;
            e->unk14a[2] = 140;
            e->flags |= 1;
            e->ai_sub--;
        } else {
            e->ai_sub2--;
        }
    }
}
