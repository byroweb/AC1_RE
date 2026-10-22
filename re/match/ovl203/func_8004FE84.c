#include "ac1/ac1.h"

typedef struct {
    u8 unk000[0x254];
    u16 n254;                   /* +0x254 */
    u8 pad256[2];
    u8 stage;                   /* +0x258 */
    u8 pad259[0x394 - 0x259];
    void *body;                 /* +0x394 */
    u8 pad398[4];
} PilotBlk;                     /* 0x39C */

typedef struct {
    u8 b[0x2C];
} Slot2C;

typedef struct {
    u8 b[0x170];
} Ent170;

typedef struct {
    Slot2C slot[2];             /* 0x8019B4F8 */
    Ent170 ent[2];              /* 0x8019B550 */
} ModeWork;

typedef struct {
    u8 pad[0x14];
    void (*start)(void);        /* +0x14 */
} ModeDef;

typedef struct {
    u8 unk0[8];
    s32 off;
} ModelHdr;

typedef struct {
    u8 pad[0x0E];
    s16 model;                  /* +0x0E */
} Body;

typedef struct {
    u8 pad[0x394];
    Body *body;
} Pilot;

typedef struct {
    void *model;
    ModelHdr *tbl[1];
} RenderGlobals;

extern u8 D_80039D00;
extern PilotBlk *D_80039D18;
extern u8 D_80039D2A;
extern Pilot *D_80039D30;
extern PilotBlk D_80040FA0[2];
extern u16 D_80041EC6[];
extern u8 D_800895C8;
extern u8 D_8008D190[];
extern u8 D_800AD1B0[];
extern u32 D_800AD730[];
extern u8 D_8019B480;
extern ModeDef *D_8019B484;
extern u32 *D_8019B488;
extern s32 D_8019B48C;
extern s16 D_8019B490;
extern s16 D_8019B492;
extern s16 D_8019B494;
extern s16 D_8019B496;
extern s16 D_8019B498;
extern s16 D_8019B49A;
extern ModeWork D_8019B4F8;
typedef struct { s16 v; } S16v;
extern S16v D_8019B502_s;
extern S16v D_8019B52E_s;
extern S16v D_8019B586_s;
extern S16v D_8019B6F6_s;
#define D_8019B502 D_8019B502_s.v
#define D_8019B52E D_8019B52E_s.v
#define D_8019B586 D_8019B586_s.v
#define D_8019B6F6 D_8019B6F6_s.v
extern RenderGlobals D_8019D968;
extern ModeDef D_801B80C8;
extern u32 *D_801C40EC;
extern void func_80011FD8(u32 *p);
extern void func_800154D8(void *dst, u32 *src, u32 n);
extern void func_80015508(void *dst, u32 *src, u32 n);
extern void func_800165E4(s32 a, s32 b, void *buf);
extern void func_8004C9B0(s32 a, u32 *b, u32 *c);
extern u32 *func_8004CA5C(u32 *p);
extern void func_8004F4E0(void);
extern void func_8004F7C8(s32 n);
extern void func_8004FD18(void *m);
extern void func_80053244(s32 n, u32 *p);
extern void func_80056354(void);
extern void func_80056450(void);
extern void func_800564CC(u32 *p);
extern void func_80057390(s32 n, u32 *p);
extern void func_8006CE7C(void);
extern void func_80071198(u32 *p, u32 n);
extern void func_8007256C(u32 *p);
extern void func_800726D0(u32 *p);
extern void func_80072BDC(u32 *p);
extern void func_80077894(Ent170 *e, Slot2C *s);
extern void func_8007D704(Ent170 *e);

#define NEXT(p) ((u32 *)((u8 *)(p) + (*(p) + 4)))

void func_8004FE84(void)
{
    u32 *p;
    u16 n;
    Ent170 *e;
    ModelHdr *m;
    ModeWork *w;
    Ent170 *f;

    D_8019B484 = &D_801B80C8;
    func_8004F4E0();
    if (D_800895C8 != 0) {
        return;
    }
    if (!(D_80039D2A & 1)) {
        D_8019B480 = D_80040FA0[0].stage;
    } else {
        D_8019B480 = D_80040FA0[1].stage;
    }
    D_8019B496 = 128;
    D_8019B49A = 64;
    D_8019B490 = 0;
    D_8019B492 = 0;
    D_8019B48C = 0;
    D_8019B498 = 0;
    D_8019B488 = D_800AD730;
    if (D_80039D00 == 0) {
        func_800165E4(1, 100, D_800AD730);
        func_80011FD8(D_800AD730);
    }
    func_800165E4(1, D_8019B480, D_800AD730);
    func_80011FD8(D_800AD730);
    p = D_8019B488;
    func_800165E4(2, 200, p);
    func_80053244(40, p + 1);
    p = NEXT(p);
    D_8019B488 = p + 1;
    p = NEXT(p);
    func_8004C9B0(0, D_8019B488, p + 1);
    D_8019B488 = p;
    p = NEXT(p);
    func_80015508(D_80041EC6, p + 1, *p >> 1);
    p = NEXT(p);
    p = NEXT(p);
    p = NEXT(p);
    p = NEXT(p);
    func_800154D8(D_800AD1B0, p + 1, *p >> 2);
    func_80056354();
    func_80056450();
    func_800165E4(2, D_8019B480 * 2, &D_801B80C8);
    p = D_8019B488;
    func_800165E4(2, (D_8019B480 * 2) | 1, p);
    func_80053244(260, p + 1);
    p = NEXT(p);
    func_80053244(128, p + 1);
    p = NEXT(p);
    func_80053244(160, p + 1);
    p = NEXT(p);
    func_80057390(0, p + 1);
    p = NEXT(p);
    D_801C40EC = p + 1;
    p = NEXT(p);
    D_8019B488 = func_8004CA5C(p + 1);
    p = NEXT(p);
    func_800154D8(D_8008D190, p + 1, *p >> 2);
    p = NEXT(p);
    func_8006CE7C();
    func_80071198(p + 1, *p);
    p = NEXT(p);
    func_800726D0(p + 1);
    p = NEXT(p);
    func_8007256C(p + 1);
    p = NEXT(p);
    func_800564CC(p + 1);
    p = NEXT(p);
    p = NEXT(p);
    func_80072BDC(p + 1);
    D_8019B484->start();
    n = D_80040FA0[0].n254 < D_80040FA0[1].n254 ? D_80040FA0[1].n254 : D_80040FA0[0].n254;
    D_8019B494 = n * 22;
    D_80039D18 = &D_80040FA0[0];
    func_8004F7C8(0);
    w = &D_8019B4F8;
    e = &w->ent[0];
    D_8019B502 = 0;
    func_80077894(e, &w->slot[0]);
    func_8007D704(e);
    D_80039D18->body = e;
    D_80039D18 = &D_80040FA0[1];
    D_8019B586 = 1;
    func_8004F7C8(1);
    f = &w->ent[1];
    D_8019B52E = 1;
    func_80077894(f, &w->slot[1]);
    func_8007D704(f);
    D_80039D18->body = f;
    D_8019B6F6 = 2;
    m = D_8019D968.tbl[D_80039D30->body->model + 128];
    func_8004FD18((u8 *)m + m->off);
}
