#include "ac1/ac1.h"

typedef struct Body {
    u8 unk00[0x3C];
    u8 state;                   /* +0x3C */
    u8 sub;                     /* +0x3D */
} Body;

typedef struct Slot {
    u8 unk00[0x30];
} Slot;

typedef struct Mode {           /* 0x3C */
    u8 kind;                    /* +0x00 */
    u8 unk01[0x0B];
    u8 flags;                   /* +0x0C */
    u8 unk0D[0x2F];
} Mode;

typedef struct Pilot {
    u8 unk000[0x18];
    u8 mode;                    /* +0x18 */
    u8 unk019[0x67];
    s32 *marker_on;             /* +0x80 */
    Slot slot[8];               /* +0x84 */
    u8 unk204[0x76];
    u8 lock;                    /* +0x27A */
    u8 unk27B[0x5D];
    u8 hud_on;                  /* +0x2D8 */
    u8 unk2D9;
    Mode modes[1];              /* +0x2DA */
    u8 pad[0x394 - 0x2DA - 0x3C];
    Body *body;                 /* +0x394 */
} Pilot;

extern Pilot *D_80039D18;
extern s32 D_801A3D94;
typedef struct Screen {
    u8 rgb[3];                  /* +0x000 0x801A3E28 fade colour */
    u8 fade;                    /* +0x003 */
    u8 unk004[0xDDC - 4];
    MATRIX cam;                 /* +0xDDC 0x801A4C04 */
} Screen;
extern Screen D_801A3E28;
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void func_80050D78(void);
extern void func_80051B84(void);
extern void func_80052238(Mode *m, Slot *s);
extern void func_800526C4(Slot *s);
extern void func_800529D0(Slot *s);
extern void func_80052BF4(s32 *on);
extern void func_80052D18(Slot *s);
extern void func_8005DFF8(s32 x, s32 y, s32 w, s32 h, u8 *col, s32 a, s32 b);

void func_80061A44(void)
{
    Mode *m;
    Slot *s;
    Body *b;
    s32 i;
    s32 kind;

    if (D_801A3E28.fade != 0) {
        func_8005DFF8(0, 0, 320, 240, D_801A3E28.rgb, 1, D_801A3E28.fade);
    }
    func_80051B84();
    if (D_801A3D94 == 0) {
        return;
    }
    func_80050D78();
    if (D_80039D18->hud_on == 0) {
        return;
    }
    if (D_80039D18->body->state == 0x20) {
        return;
    }
    SetRotMatrix(&D_801A3E28.cam);
    SetTransMatrix((MATRIX *)&D_801A3E28.cam.m[2][0]);
    m = &D_80039D18->modes[D_80039D18->mode];
    kind = m->kind;
    switch (kind) {
    case 0:
        if (!(m->flags & 4)) {
            func_800526C4(D_80039D18->slot);
        }
        break;
    case 1:
        i = 8;
        s = D_80039D18->slot;
        for (; i != 0; i--) {
            func_800529D0(s);
            s++;
        }
        func_80052BF4(D_80039D18->marker_on);
        func_80052238(m, 0);
        return;
    case 2:
        b = D_80039D18->body;
        if (b->state == 0x22 || D_80039D18->lock == 1 || D_80039D18->lock == 3
            || (D_80039D18->lock == kind && b->sub != 0xC0)) {
            func_80052D18(D_80039D18->slot);
            break;
        }
        return;
    default:
        return;
    }
    func_80052238(m, D_80039D18->slot);
}
