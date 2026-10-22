#include "ac1/ac1.h"

typedef struct {                /* 0x30 */
    s32 id;                     /* +0x00 */
    SVECTOR pos;                /* +0x04 (pad = kind) */
    u8 pad0c[0x18];
    DVECTOR uv;                 /* +0x24 */
    u8 pad28[2];
    s16 f2a;                    /* +0x2A */
    s16 f2c;                    /* +0x2C */
    u8 pad2e[2];
} FxEnt;

typedef struct {                /* 0x801AC820 */
    u32 mask[0x80D];            /* bit per id */
    FxEnt ent[16];              /* 0x801AE82C */
    FxEnt *cur;                 /* 0x801AEB2C */
    s32 n;                      /* 0x801AEB30 */
} FxPool;

extern FxPool D_801B4118;

void func_80055BA0(s32 id, SVECTOR *pos, s32 kind, s32 bit, s32 f2a, DVECTOR *uv, s32 f2c) {
    FxEnt *e;

    /* retail reads uv/f2c from their stack slots at the use: addressable */
    { void *dummy = &uv; dummy = &f2c; }

    if (D_801B4118.n < 16 && (D_801B4118.mask[bit >> 5] & (1 << (bit & 0x1F)))) {
        e = D_801B4118.cur;
        if (kind == 0) {
            e->f2c = f2c;
            if (e->f2c == 0) {
                return;
            }
        }
        e->id = id;
        e->pos = *pos;
        e->pos.pad = kind;
        e->f2a = f2a;
        e->uv = *uv;
        D_801B4118.cur++;
        D_801B4118.n++;
    }
}
