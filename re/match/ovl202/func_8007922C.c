#include "ac1/ac1.h"

typedef struct {
    u8 b[0x2C];
} Obj2C;

typedef struct {
    Obj2C objs[256];        /* 0x8019FAB8 */
    AcEntity ac[16];        /* 0x801A26B8 */
    u8 pad3db8[0x200C];
    s32 frame;              /* 0x801A5DC4 */
    u8 pad5dc8[0x10];
    u8 ctx[1];              /* 0x801A5DD8 */
} WorldState;

extern WorldState D_8019FAB8;
extern u16 D_801987D8;
extern void *D_801AD668;
extern void func_8006E7B8(void *ctx);
extern void func_8006E7D4(void *ctx, void *obj);
extern void func_80078F00(void *ctx, Obj2C *o);
extern void func_8007909C(void *ctx, AcEntity *e);

void func_8007922C(void)
{
    s32 i;
    Obj2C *p;
    AcEntity *e;

    func_8006E7B8(D_8019FAB8.ctx);
    if (D_8019FAB8.ac[0].section != -1) {
        func_8006E7D4(D_8019FAB8.ctx, &g_world_structs[D_8019FAB8.ac[0].section]);
    }
    if (D_801987D8 & 1) {
        func_8006E7D4(D_8019FAB8.ctx, D_801AD668);
    }
    i = (D_8019FAB8.frame & 0xF) + 4;
    for (p = &D_8019FAB8.objs[i]; i < 256; i += 16, p += 16) {
        func_80078F00(D_8019FAB8.ctx, p);
    }
    e = &D_8019FAB8.ac[D_8019FAB8.frame % 16];
    if (e != D_8019FAB8.ac && e->def != 0) {
        func_8007909C(D_8019FAB8.ctx, e);
    }
}
