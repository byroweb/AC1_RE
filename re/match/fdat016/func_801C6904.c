#include "ac1/ac1.h"

typedef struct {
    u8 kind;            /* +0x00 */
    u8 ent;             /* +0x01 */
    u8 pad2[0x2A];
} Func801C6904Link;     /* 0x2C */
typedef struct {
    u8 pad0[0x2C];
    Func801C6904Link links[1]; /* +0x2C */
} Func801C6904Def;
extern void func_80075418(void *, s32, s32, s32, s32);

void func_801C6904(AcEntity *o, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    Func801C6904Link *l;
    s32 i;

    if (o->ap == 0) {
        o->unk42[0] = 3;
        if (o->ai_mode == 0xFF) {
            l = ((Func801C6904Def *)o->def)->links;
            i = *((u8 *)o + 0xFE);
            while (--i != -1) {
                if (l->kind == 1) {
                    g_ac_entities[l->ent].unk42[0] = 3;
                }
                l++;
            }
        }
        func_80075418(o, arg1, arg4, 100, -1);
    }
}
