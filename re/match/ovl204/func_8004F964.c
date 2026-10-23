/* ovl204 0x8004F964  size 1236  permuter tier (tools/loop/permute.py): levers swap D_80039D34 = &D_80040FA0 / D_80040FA0[0].aux = D_80, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"

/* ovl204 (2-player VS) mode init: load the two AC records' data blocks,
 * walk the chunked file at D_801AB680, spawn both ACs. */
typedef struct {                /* 0x39C: per-player record, [2] at 0x80040FA0 */
    u8 pad000[0x254];
    u16 flags;                  /* +0x254 */
    u8 pad256[2];
    u8 stage;                   /* +0x258 */
    u8 pad259[0x394 - 0x259];
    AcEntity *ent;              /* +0x394 */
    void *aux;                  /* +0x398 */
} PlRec;

typedef struct {                /* 0x2C */
    u8 pad00[0x0A];
    s16 f0a;                    /* +0x0A */
    u8 pad0c[0x2C - 0x0C];
} Spawn4;

typedef struct {                /* 0x801AB6F0 */
    Spawn4 def[2];
    AcEntity ent[2];            /* +0x58 */
} VsEnts;

typedef struct {
    u8 pad00[0x14];
    void (*init)(void);         /* +0x14 */
} Scene4;

typedef struct {
    u8 pad00[8];
    s32 off;
} ModelHdr;

extern PlRec D_80040FA0[2];
extern PlRec *D_80039D18;
extern PlRec *D_80039D30;
extern PlRec *D_80039D34;
extern u8 D_80039D2A;
extern u8 D_80039D00;
extern u8 D_800AD368[2][0x38];
extern Scene4 D_801C8460;
extern Scene4 *D_801AB67C;
extern u32 *D_801AB680;
extern u8 D_801AB678;
extern s32 D_801AB684;
extern s16 D_801AB688;
extern s16 D_801AB68A;
extern s16 D_801AB68C;
extern s16 D_801AB68E;
extern s16 D_801AB690;
extern s16 D_801AB692;
extern u32 D_800AD960[];
extern u8 D_80041EC6[];
extern u8 D_800AD3D8[];
extern u8 D_8008D348[];
extern void *D_801D4484;
extern VsEnts D_801AB6F0;
extern s16 D_801AB6FA;
extern s16 D_801AB726;
extern ModelHdr *D_801ADCDC[];

extern void func_800165E4(s32, s32, void *);
extern void func_80011FD8(void *);
extern void func_80052894(s32, void *);
extern void func_8004C840(s32, void *, void *);
extern void func_80015508(void *, void *, u32);
extern void func_800154D8(void *, void *, u32);
extern void func_800559A4(void);
extern void func_80055AA0(void);
extern void func_80056B44(s32, void *);
extern u32 *func_8004C8EC(void *);
extern void func_8006CCB8(void);
extern void func_80070FD4(void *, u32);
extern void func_8007250C(void *);
extern void func_800723A8(void *);
extern void func_80055B1C(void *);
extern void func_80072A18(void *);
extern void func_8004F2A8(s32);
extern void func_800776B4(AcEntity *, Spawn4 *);
extern void func_8007D3B0(AcEntity *);
extern void func_8004F7F8(void *);

#define NEXT(p) ((u32 *)((u8 *)(p) + (*(p) + 4)))

void func_8004F964(void)
{
    u32 *p;
    u16 n;
    ModelHdr *m;
    PlRec *old;
    Spawn4 *d;

    D_801AB67C = &D_801C8460;
    D_80039D30 = &D_80040FA0[0];
    D_80040FA0[0].aux = D_800AD368[0];
    D_80039D34 = &D_80040FA0[1];
    D_80040FA0[1].aux = D_800AD368[1];
    if (!(D_80039D2A & 1)) {
        D_801AB678 = D_80040FA0[0].stage;
    } else {
        D_801AB678 = D_80040FA0[1].stage;
    }
    D_801AB68E = 128;
    D_801AB692 = 64;
    D_801AB688 = 0;
    D_801AB68A = 0;
    D_801AB684 = 0;
    D_801AB690 = 0;
    D_801AB680 = D_800AD960;
    if (D_80039D00 == 0) {
        func_800165E4(1, 100, D_800AD960);
        func_80011FD8(D_800AD960);
    }
    func_800165E4(1, D_801AB678, D_800AD960);
    func_80011FD8(D_800AD960);

    p = D_801AB680;
    func_800165E4(2, 200, p);
    func_80052894(40, p + 1);
    p = NEXT(p);
    D_801AB680 = p + 1;
    p = NEXT(p);
    func_8004C840(0, D_801AB680, p + 1);
    D_801AB680 = p;
    p = NEXT(p);
    func_80015508(D_80041EC6, p + 1, *p >> 1);
    p = NEXT(p);
    p = NEXT(p);
    p = NEXT(p);
    p = NEXT(p);
    func_800154D8(D_800AD3D8, p + 1, *p >> 2);
    func_800559A4();
    func_80055AA0();

    func_800165E4(2, D_801AB678 * 2, &D_801C8460);
    p = D_801AB680;
    func_800165E4(2, (D_801AB678 * 2) | 1, p);
    func_80052894(260, p + 1);
    p = NEXT(p);
    func_80052894(128, p + 1);
    p = NEXT(p);
    func_80052894(160, p + 1);
    p = NEXT(p);
    func_80056B44(0, p + 1);
    p = NEXT(p);
    D_801D4484 = p + 1;
    p = NEXT(p);
    D_801AB680 = func_8004C8EC(p + 1);
    p = NEXT(p);
    func_800154D8(D_8008D348, p + 1, *p >> 2);
    p = NEXT(p);
    func_8006CCB8();
    func_80070FD4(p + 1, *p);
    p = NEXT(p);
    func_8007250C(p + 1);
    p = NEXT(p);
    func_800723A8(p + 1);
    p = NEXT(p);
    func_80055B1C(p + 1);
    p = NEXT(p);
    p = NEXT(p);
    func_80072A18(p + 1);
    D_801AB67C->init();

    n = D_80040FA0[0].flags < D_80040FA0[1].flags ? D_80040FA0[1].flags : D_80040FA0[0].flags;
    D_801AB68C = n * 22;
    D_80039D18 = &D_80040FA0[0];
    func_8004F2A8(0);
    d = D_801AB6F0.def;
    D_801AB6FA = 0;
    func_800776B4(&D_801AB6F0.ent[0], d);
    func_8007D3B0(&D_801AB6F0.ent[0]);
    old = D_80039D18;
    D_80039D18 = &D_80040FA0[1];
    old->ent = &D_801AB6F0.ent[0];
    D_801AB6F0.ent[0].faction = 1;
    func_8004F2A8(1);
    D_801AB726 = 1;
    func_800776B4(&D_801AB6F0.ent[1], d + 1);
    func_8007D3B0(&D_801AB6F0.ent[1]);
    D_80039D18->ent = &D_801AB6F0.ent[1];
    D_801AB6F0.ent[1].faction = 2;
    m = D_801ADCDC[*(s16 *)((u8 *)&D_801AB6F0.ent[0] + 0x0E) + 128];
    func_8004F7F8((u8 *)m + m->off);
}
