/* Window box render. Needs --rodata 0x8004B420 --no-rodata-align-fix (2nd jump table at 0x8004B480 is 8-aligned in retail). w/h split per vertex block (w,h / w2,h2) so each block gets its own pseudos. */
#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 )" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 );lwc2 $2, 0( %1 );lwc2 $3, 4( %1 );lwc2 $4, 0( %2 );lwc2 $5, 4( %2 )" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rtps() __asm__ volatile ("nop;nop;.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop;nop;.word 0x4A280030")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0( %0 )" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0( %0 );swc2 $13, 0( %1 );swc2 $14, 0( %2 )" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0( %0 )" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop;nop;.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0( %0 )" : : "r"(r0) : "memory")

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} BoxTag;

#define setaddr(p, _addr) (((BoxTag *)(p))->addr = (u32)(_addr))
#define getaddr(p) (u32)(((BoxTag *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)

typedef struct { u32 tag; u32 code[2]; } BoxTwin;
typedef struct { u32 tag; u32 code[2]; } BoxArea;
typedef struct { u32 tag; u32 code[2]; } BoxMode;
typedef struct { u8 r, g, b, cd; } BoxRgb;

typedef struct {                /* 0x28 */
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} BoxFT4;

typedef struct {                /* 0x24 */
    u32 tag;
    BoxRgb c0;
    s16 x0, y0;
    BoxRgb c1;
    s16 x1, y1;
    BoxRgb c2;
    s16 x2, y2;
    BoxRgb c3;
    s16 x3, y3;
} BoxG4;

typedef struct {
    u8 idx;
    u8 pad[3];
    u8 *prim;
} GfxCur;

typedef struct {
    DRAWENV env;
    u8 pad[0x64 - 0x5C];
} DrawSlot;

typedef struct {                /* 0x19458 per frame */
    DrawSlot slot[10];
    u8 rest[0x19458 - 0x3E8];
} FrameEnvs;

typedef struct {
    u8 pad7c[0x08];
    s16 frame;                  /* +0x84 frame style, 0..11 */
    u8 pad86[4];
    s16 plain;                  /* +0x8A 0: textured FT4 panel, else gouraud G4 */
} BoxDraw;

extern FrameEnvs D_801BCE88[2];
extern GfxCur D_801EF6C8;
extern SVECTOR D_800B8D78;
extern BoxRgb D_800B755C;
extern RECT D_800B7560[2];
extern BoxRgb D_800B7570;
extern BoxRgb D_800B7574;
extern RECT D_800B7578[2];
extern MATRIX D_800B8DC4;
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void SetColorMatrix(MATRIX *m);
extern void SetTexWindow(BoxTwin *p, RECT *tw);
extern void SetDrawArea(BoxArea *p, RECT *r);
extern void SetDrawMode(BoxMode *p, int dfe, int dtd, int tpage, RECT *tw);
extern void func_80060168(UiObj *o);
extern void func_8006035C(UiObj *o, SVECTOR *v);
extern void func_80060ABC(UiObj *o, SVECTOR *v);
extern void func_80060E10(UiObj *o, SVECTOR *v);
extern void func_80061164(UiObj *o, SVECTOR *v);
extern void func_800617A8(UiObj *o, SVECTOR *v);
extern void func_80061B00(UiObj *o, SVECTOR *v);
extern void func_80061E58(UiObj *o, SVECTOR *v);
extern void func_80062D58(UiObj *o, SVECTOR *v);
extern void func_800634C4(UiObj *o, SVECTOR *v);
extern void func_8006381C(UiObj *o, SVECTOR *v);
extern void func_80063F64(UiObj *o, SVECTOR *v);

/* Render a window box: panel quad clipped to the box draw area, then its frame style. */
void func_8005F5E8(UiObj *o)
{
    SVECTOR v[4];
    BoxTwin *tw[2];
    RECT twr0 = { 0, 0, 64, 64 };
    RECT twr1 = { 0, 0, 256, 256 };
    s32 sxy[4];
    RECT clip;
    RECT full;
    BoxFT4 *ft;
    BoxG4 *g;
    BoxArea *ar0;
    BoxArea *ar1;
    BoxArea *ar2;
    BoxArea *ar3;
    BoxArea *ar4;
    u16 depth;
    s16 slot;
    s16 w;
    s16 h;
    s16 w2;
    s16 h2;

    depth = o->unk14;
    if (o->id == 1) {
        func_80060168(o);
        return;
    }
    tw[0] = (BoxTwin *)D_801EF6C8.prim;
    D_801EF6C8.prim += sizeof(BoxTwin);
    tw[1] = (BoxTwin *)D_801EF6C8.prim;
    D_801EF6C8.prim += sizeof(BoxTwin);
    SetTexWindow(tw[0], &twr0);
    SetTexWindow(tw[1], &twr1);
    switch (o->id - 1) {
    case 0:
        slot = 0;
        break;
    case 2:
        slot = 1;
        break;
    case 4:
        slot = 2;
        break;
    case 6:
        slot = 3;
        break;
    case 8:
        slot = 4;
        break;
    case 10:
        slot = 5;
        break;
    case 12:
        slot = 6;
        break;
    case 14:
        slot = 7;
        break;
    case 16:
        slot = 8;
        break;
    case 18:
        slot = 9;
        break;
    }
    w = D_801BCE88[0].slot[slot].env.clip.w;
    h = D_801BCE88[0].slot[slot].env.clip.h;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = 256;
    v[0].vx = v[2].vx = -w / 2 + 4;
    v[0].vy = v[1].vy = -h / 2 + 3;
    v[1].vx = v[3].vx = w / 2 + 4;
    v[2].vy = v[3].vy = h / 2 + 3;
    o->mat.t[2] = 0;
    SetRotMatrix(&o->mat);
    SetTransMatrix(&o->mat);
    gte_ldv3(&v[0], &v[1], &v[2]);
    gte_rtpt();
    gte_stsxy3(&sxy[0], &sxy[1], &sxy[2]);
    gte_ldv0(&v[3]);
    gte_rtps();
    gte_stsxy(&sxy[3]);
    if (((BoxDraw *)&o->w)->plain == 0) {
        ft = (BoxFT4 *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(BoxFT4);
        ((BoxTag *)ft)->len = 9;
        ft->code = 0x2E;
        ft->u0 = 200;
        ft->v0 = 200;
        ft->u1 = 207;
        ft->v1 = 200;
        ft->u2 = 200;
        ft->v2 = 207;
        ft->u3 = 207;
        ft->v3 = 207;
        *(s32 *)&ft->x0 = sxy[0];
        *(s32 *)&ft->x1 = sxy[1];
        *(s32 *)&ft->x2 = sxy[2];
        ft->tpage = 69;
        *(s32 *)&ft->x3 = sxy[3];
        ft->clut = 14359;
        gte_ldv0(&D_800B8D78);
        gte_ldrgb(&D_800B755C);
        gte_nccs();
        gte_strgb(&ft->r0);
        clip = D_801BCE88[D_801EF6C8.idx].slot[slot].env.clip;
        ar0 = (BoxArea *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(BoxArea);
        SetDrawArea(ar0, &clip);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, ar0);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, tw[1]);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, ft);
        full = D_800B7560[D_801EF6C8.idx];
        ar1 = (BoxArea *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(BoxArea);
        SetDrawArea(ar1, &full);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, ar1);
    } else {
        g = (BoxG4 *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(BoxG4);
        ((BoxTag *)g)->len = 8;
        g->c0.cd = 0x3A;
        *(s32 *)&g->x0 = sxy[0];
        *(s32 *)&g->x1 = sxy[1];
        *(s32 *)&g->x2 = sxy[2];
        *(s32 *)&g->x3 = sxy[3];
        g->x1 -= 4;
        g->y2 -= 4;
        g->x3 -= 4;
        g->y3 -= 4;
        g->c0 = D_800B7570;
        g->c1 = D_800B7570;
        g->c2 = D_800B7574;
        g->c3 = D_800B7574;
        clip = D_801BCE88[D_801EF6C8.idx].slot[slot].env.clip;
        ar2 = (BoxArea *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(BoxArea);
        SetDrawArea(ar2, &clip);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, ar2);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, tw[1]);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, g);
        full = D_800B7578[D_801EF6C8.idx];
        ar3 = (BoxArea *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(BoxArea);
        SetDrawArea(ar3, &full);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, ar3);
        ar4 = (BoxArea *)D_801EF6C8.prim;
        D_801EF6C8.prim += sizeof(BoxMode);
        SetDrawMode((BoxMode *)ar4, 1, 0, 261, 0);
        addPrim(o->ot[D_801EF6C8.idx] + (s16)depth, ar4);
    }
    w2 = D_801BCE88[0].slot[slot].env.clip.w;
    h2 = D_801BCE88[0].slot[slot].env.clip.h;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = 256;
    v[0].vx = v[2].vx = -w2 / 2;
    v[0].vy = v[1].vy = -h2 / 2;
    v[1].vx = v[3].vx = w2 / 2;
    v[2].vy = v[3].vy = h2 / 2;
    SetColorMatrix(&D_800B8DC4);
    switch (((BoxDraw *)&o->w)->frame) {
    case 0:
        break;
    case 1:
        func_8006035C(o, v);
        break;
    case 2:
        func_80060ABC(o, v);
        break;
    case 3:
        func_80060E10(o, v);
        break;
    case 4:
        func_80061164(o, v);
        break;
    case 5:
        func_800617A8(o, v);
        break;
    case 6:
        func_80061B00(o, v);
        break;
    case 7:
        func_80061E58(o, v);
        break;
    case 8:
        func_80062D58(o, v);
        break;
    case 9:
        func_800634C4(o, v);
        break;
    case 10:
        func_8006381C(o, v);
        break;
    case 11:
        func_80063F64(o, v);
        break;
    }
}
