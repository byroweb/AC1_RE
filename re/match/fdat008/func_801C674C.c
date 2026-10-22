#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    u8 tick;            /* +0x08 */
} Func801C674CTimer;
typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad8[6];
    s16 flag;           /* +0x0E */
    u8 pad10[4];
    Func801C674CTimer *timer; /* +0x14 */
} Func801C674CObj;
typedef struct {
    u8 pad0[0x10];
    u8 kind;            /* +0x10 */
    u8 pad11[0xA9];
    s16 ba;             /* +0xBA */
    s16 bc;             /* +0xBC */
    s16 be;             /* +0xBE */
} Func801C674CFx;
extern s32 SquareRoot0(s32);
extern Func801C674CFx *func_80086854(s32, void *, void *);
extern u32 D_801AC860[];
extern u8 D_801CC4D4[];

void func_801C674C(Func801C674CObj *o)
{
    Func801C674CTimer *t;
    Func801C674CFx *fx;
    s32 dx, dy, dz;

    t = o->timer;
    if (!(t->tick & 7)) {
        if (D_801AC860[o->flag >> 5] & (1 << (o->flag & 0x1F))) {
            dx = g_ac_entities[0].pos.x - o->pos.x;
            dy = g_ac_entities[0].pos.y - o->pos.y;
            dz = g_ac_entities[0].pos.z - o->pos.z;
            if (SquareRoot0(dx * dx + dy * dy + dz * dz) < 6000) {
                fx = func_80086854(0, o, D_801CC4D4);
                fx->kind = 5;
                fx->bc = 200;
                fx->ba = 200;
                fx->be = 100;
            }
        }
    }
    t->tick++;
}
