#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x1C];
    u16 snd;            /* +0x1C */
    u8 pad1e[2];
    u8 arg;             /* +0x20 */
} Def;

extern s32 rand(void);
extern s32 func_800781E8(AcEntity *);
extern void func_80077B78(AcEntity *, s32, u16, u8, void *, s32, s32);
extern void func_80078E1C(AcEntity *);
extern s32 func_801C5828(AcEntity *);
extern s32 func_801C6880(AcEntity *, void *);
extern u8 D_801C84BC[];
extern u8 D_801C84E0[];

void func_801C5EEC(AcEntity *e)
{
    e->fire_flags = 0;
    e->flags = 0;
    if (e->ai_mode != 0xFF) {
        if (func_801C6880(e, D_801C84BC) != 0) {
            func_80078E1C(e);
        }
    } else if (rand() < 1967 && func_801C5828(e) != 0 && func_800781E8(e) != 0) {
        func_80077B78(e, 0x27, ((Def *)e->def)->snd, ((Def *)e->def)->arg, D_801C84E0, 1, 0);
        func_80077B78(e, 0x27, ((Def *)e->def)->snd, ((Def *)e->def)->arg, D_801C84E0, 1, 2);
        e->fire_flags = 0xF;
        e->unk14a[0] = 180;
        e->unk14a[1] = 180;
        e->unk14a[2] = 170;
        e->flags = 0x2000;
    }
}
