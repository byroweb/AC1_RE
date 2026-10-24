/* fdat024: proximity trap.  Every `reload` frames, if the player is inside the
 * box (pos + lo/hi << 8), hit him with a blast and spawn an effect by type. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[4];
    u8 at[0x1C];        /* +0x04 */
} Func801C6498Debris;
typedef struct {
    u8 type;            /* +0x00 */
    u8 pad1;
    u16 dmg;            /* +0x02 */
    u8 b4;              /* +0x04 */
    u8 pad5[3];
} Func801C6498Blast;
typedef struct {
    u8 pad0[8];
    s32 timer;          /* +0x08 */
} Func801C6498Ctl;
typedef struct {
    s16 x, y, z;        /* +0x00 */
    u8 pad6[0x14 - 0x06];
    Func801C6498Ctl *ctl; /* +0x14 */
    u8 pad18[0x22 - 0x18];
    s8 lo[3];           /* +0x22 */
    s8 hi[3];           /* +0x25 */
    u8 reload;          /* +0x28 */
    u8 type;            /* +0x29 */
    u16 dmg;            /* +0x2A */
} Func801C6498Obj;
typedef struct {
    u8 pad0[0x12];
    s16 life;           /* +0x12 */
    void (*think)();    /* +0x14 */
    u8 pad18[0x26 - 0x18];
    s16 h26;            /* +0x26 */
    u8 pad28[0xBE - 0x28];
    u16 hbe;            /* +0xBE */
    s16 hc0;            /* +0xC0 */
} Func801C6498Fx;
extern AcPos D_80039C78;
extern s32 D_801A2718;
extern s16 D_801A26C6;
extern u16 D_801A26E0;
extern u16 D_801A26E2;
extern u8 D_801C70BC[];
extern u8 D_801C70C4[];
extern u8 D_801C70CC[];
extern u8 D_801C70D0[];
extern s32 func_80014D78(s32 a);
extern s32 func_80014DAC(s32 a);
extern s32 func_800564AC(s32 model, s32 a, AcPos *pos, void *b, s32 n, Func801C6498Debris *out);
extern void func_800835E8(s32 owner, AcEntity *e, Func801C6498Blast *b, AcPos *d, AcPos *p);
extern void func_80086854(s32 owner, AcPos *pos, s32 *col);
extern Func801C6498Fx *func_80086F20(s32 a, void *at, s32 b, s32 c, s32 d, void *e, void *f, void *g, void *h);
extern void func_801C63CC();
void func_801C6498(Func801C6498Obj *o)
{
    AcPos lo;
    AcPos hi;
    Func801C6498Blast b;
    AcPos pos;
    s32 col;
    Func801C6498Ctl *c;
    Func801C6498Fx *fx;

    c = o->ctl;
    if (c->timer == 0) {
        c->timer = o->reload;
        lo.x = o->x + (o->lo[0] << 8);
        hi.x = o->x + (o->hi[0] << 8);
        lo.y = o->y + (o->lo[1] << 8);
        hi.y = o->y + (o->hi[1] << 8);
        lo.z = o->z + (o->lo[2] << 8);
        hi.z = o->z + (o->hi[2] << 8);
        if (g_ac_entities[0].pos.x >= lo.x && hi.x >= g_ac_entities[0].pos.x && g_ac_entities[0].pos.y >= lo.y && hi.y >= g_ac_entities[0].pos.y
            && g_ac_entities[0].pos.z >= lo.z && hi.z >= g_ac_entities[0].pos.z) {
            b.type = 7;
            b.dmg = o->dmg;
            b.b4 = 0;
            func_800835E8(0, &g_ac_entities[0], &b, &D_80039C78, &D_80039C78);
            switch (o->type) {
            case 0:
                col = 0x01901D88;
                pos.x = g_ac_entities[0].pos.x + func_80014DAC(D_801A26E0);
                pos.z = g_ac_entities[0].pos.z + func_80014DAC(D_801A26E0);
                pos.y = g_ac_entities[0].pos.y - func_80014D78(D_801A26E2);
                func_80086854(0, &pos, &col);
                break;
            case 1: {
                Func801C6498Debris debris[2];

                if (func_800564AC(D_801A2718, D_801A26C6 + 128, &g_ac_entities[0].pos,
                                  g_ac_entities[0].unk10, 1, debris)) {
                    fx = func_80086F20(0, debris[0].at, 294, 5, 6, D_801C70BC, D_801C70C4,
                                       D_801C70CC, D_801C70D0);
                    fx->life = 40;
                    fx->hc0 = 80;
                    fx->think = func_801C63CC;
                    fx->h26 = fx->hbe * 16;
                }
                break;
            }
            }
        }
    } else {
        c->timer--;
    }
}
