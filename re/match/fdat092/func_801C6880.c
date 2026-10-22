#include "ac1/ac1.h"

typedef struct { s16 a, b, c; } Blk6;
typedef struct {
    u8 pad0[8];
    u16 x, y, z;        /* +0x08 */
    u8 pad0e[0x1C];
    u16 h;              /* +0x2A */
    u8 pad2c[0x144];
} Ent;
extern void func_80085e98();
extern Blk6 D_801C4CA0;
extern u8 D_8019FB69[];
extern u8 D_8019FB95;
extern u16 D_801D0B52;
extern u8 D_8008E530[];
extern s16 D_8008E538;
extern s16 D_8008E53C;
extern s16 D_8008E53E;
extern s16 D_8008E540;
extern u8 D_8008E545;

void func_801C6880(void)
{
    Blk6 sp40;
    u16 sp48[3];
    u16 sp50[3];
    Ent *a;
    Ent *b;
    u8 *g;

    if (D_801D0B52 == 0xA0) {
        g = D_8008E530;
        sp40 = D_801C4CA0;
        a = &((Ent *)(D_8019FB69 + 0x2B4F))[D_8019FB69[0]];
        b = &((Ent *)(D_8019FB69 + 0x2B4F))[D_8019FB95];
        sp50[0] = a->x - b->x;
        sp50[1] = a->y - b->y;
        sp50[2] = a->z - b->z;
        sp48[0] = b->x;
        sp48[1] = b->y - (b->h >> 1);
        sp48[2] = b->z;
        func_80085e98(b, &sp40, sp48, sp50, D_8008E538, 15000, D_8008E540, g + 0x18, D_8008E53C, D_8008E53E, 0, g, g + 4, 0, D_8008E545);
    }
}
