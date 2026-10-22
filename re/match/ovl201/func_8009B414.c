#include "ac1/ac1.h"
#include "ac1/gfxframe.h"
/* ovl201 0x8009B414 - front-end main: set up the display, the views and the
 * object pool, pick the entry screen from the game-state flags (ending,
 * credits, new pilot, rank/movie checks, mission results), then run the
 * per-frame loop until D_801A2560 is set: flip frames, clear the layer
 * OTs, update every used object, build the active list, retire killed /
 * orphaned objects, refresh view and object matrices, compose parents and
 * attachments, draw by render type, then present all layers. */
typedef struct {
    DRAWENV draw;       /* +0x00 */
    u32 *buf;           /* +0x5C */
    s32 size;           /* +0x60 */
} Layer;
typedef struct {
    DRAWENV draw;       /* +0x00 */
    DISPENV disp;       /* +0x5C */
    Layer layer[10];    /* +0x70 */
    u8 prim[0x19000];   /* +0x458 */
} Frame;
typedef struct {
    u8 idx;             /* +0 frame being built */
    u8 pad[3];
    u8 *prim;           /* +4 */
} FrameSel;
typedef struct {
    UiObj **end;        /* 0x801BC5E8 */
    UiObj *active[510]; /* 0x801BC5EC */
} ActiveList;
typedef struct {
    DRAWENV draw;
    DISPENV disp;
} DrawDisp;
typedef struct { u8 b[36]; } TimDesc;

extern Frame D_801BCE18[2];
extern FrameSel D_801EF6C8;
extern ActiveList D_801BC5E8;
extern UiObj D_801BC448;
extern u8 D_801BC44C;
extern s16 D_801A2560;
extern s32 D_801A2574;
extern u8 D_801A2578;
typedef struct { u8 r, g, b; } BackCol;
extern BackCol D_8018E468;
extern MATRIX D_800B8D84[];
extern MATRIX D_800B8DC4[];
extern RECT D_800BCB68;
extern RECT D_800BCB70;
extern RECT D_800BCB78;
extern RECT D_800BCB80;
extern RECT D_800BCB88;
extern RECT D_800BCB90;
extern RECT D_800BCB98;
extern RECT D_8004D148;
extern RECT D_8017F3E0;
extern u32 D_8004AC18[];
extern u32 D_80042B70[];
extern TimDesc D_80040850[];
extern u8 D_80048510[];
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_80039C5E;
extern s32 D_80039CA4;
extern u8 D_80039CA9;
extern u8 D_80039CAA;
extern u8 D_80039CAB;
extern u8 D_80039CAC;
extern u8 D_80039CAD;
extern u8 D_80039CAF;
extern u8 D_80039CB0;
extern u8 D_80039D21;
extern u8 D_80041B10;
extern s8 D_80041B14;
extern s32 D_80041B3C;
extern s32 D_80041B40;
extern u8 D_80041B48;
extern u8 D_80041B78;
extern s32 D_80048610;

extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);
extern DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h);
extern DISPENV *PutDispEnv(DISPENV *env);
extern DRAWENV *PutDrawEnv(DRAWENV *env);
extern int SetDispMask(int mask);
extern int ClearImage(RECT *r, int a, int b, int c);
extern int MoveImage(RECT *r, int x, int y);
extern int StoreImage(RECT *r, u32 *p);
extern int DrawSync(int mode);
extern int VSync(int mode);
extern u32 *ClearOTagR(u32 *ot, int n);
extern void DrawOTag(u32 *p);
extern void SetBackColor(int r, int g, int b);
extern MATRIX *RotMatrix(SVECTOR *r, MATRIX *m);
extern MATRIX *TransMatrix(MATRIX *m, VECTOR *v);
extern MATRIX *CompMatrix(MATRIX *a, MATRIX *b, MATRIX *c);
extern MATRIX *MulMatrix0(MATRIX *a, MATRIX *b, MATRIX *c);
extern MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v);
extern void SetLightMatrix(MATRIX *m);
extern void SetColorMatrix(MATRIX *m);
extern void FntFlush(int id);
extern long GetRCnt(unsigned long spec);
extern void func_800510FC(void);
extern void func_800510B0(void);
extern void func_80066974(void);
extern void func_8009A7E8(void);
extern void func_8009A83C(Frame *f);
extern void func_8009B3D8(void *p);
extern void func_80050BBC(void);
extern void func_80050C18(void);
extern void func_80050CE0(void);
extern void func_80050A54(s32 a, s32 b);
extern void func_80050804(s32 kind, s32 id);
extern void func_80012298(void);
extern void func_8005B46C(void);
extern void func_800AE7A4(void);
extern void func_800AE838(void);
extern void func_800A88F8(void);
extern void func_800A4F44(void);
extern UiObj *func_800A2D0C(u8 mode);
extern void func_80086838(void);
extern void func_800962E8(void);
extern void func_800866FC(void);
extern void func_80096230(void);
extern void func_80073B7C(void);
extern void func_800866A8(void);
extern s8 func_8009C5C0(s8 id);
extern void func_8004FDF4(void);
extern void func_800A448C(s16 idx, u8 mode);
extern void func_8005B2CC(void);
extern s32 func_800517C8(void);
extern UiObj *func_8004DCC0(void);
extern void func_800653D4(UiObj *o);
extern void func_80057190(u32 *ot, u16 a, u16 b, MATRIX *m);
extern void func_800656B8(UiObj *o);
extern void func_80064874(UiObj *o);
extern void func_80059B50(u32 *ot, u16 a, u16 b, MATRIX *m);
extern void func_8006599C(UiObj *o);
extern void func_80066454(UiObj *o);
extern void func_8005F5E8(UiObj *o);
extern void func_80064EB0(UiObj *o);
extern void func_8005F260(void *tim, s32 x, s32 y, s32 a, s32 b);
extern void func_80051628(void);
extern void func_80068790(void);
extern void func_80050D58(void);

void func_8009B414(void)
{
    MATRIX m;
    DrawDisp db[2];
    s16 i;
    s32 n;
    s8 id;
    RECT *rp;
    UiObj *o;
    UiObj **pp;
    UiObj **a;

    D_801BC44C = 0;
    func_800510FC();
    func_800510B0();
    SetDefDrawEnv(&db[0].draw, 0, 0, 320, 240);
    SetDefDispEnv(&db[0].disp, 0, 240, 320, 240);
    SetDefDrawEnv(&db[1].draw, 0, 240, 320, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, 320, 240);
    db[1].draw.isbg = 0;
    db[0].draw.isbg = 0;
    db[0].draw.r0 = 0;
    db[0].draw.g0 = 0;
    db[0].draw.b0 = 0;
    db[1].draw.r0 = 0;
    db[1].draw.g0 = 0;
    db[1].draw.b0 = 0;
    PutDispEnv(&db[0].disp);
    func_80066974();
    func_8009A7E8();
    func_8009A83C(D_801BCE18);
    for (i = 0; i < 10; i++) {
        g_ui_views[i].rot.vx = 0;
        g_ui_views[i].rot.vy = 0;
        g_ui_views[i].rot.vz = 0;
        g_ui_views[i].pos.vx = 0;
        g_ui_views[i].pos.vy = 0;
        g_ui_views[i].pos.vz = 0xE00;
    }
    func_8009B3D8(g_ui_pool);
    func_80050BBC();
    func_80050C18();
    func_80050CE0();
    func_80050A54(0, 0);
    func_80050804(4, 0);
    func_80012298();
    SetDispMask(0);
    ClearImage(&D_800BCB68, 0, 0, 0);
    DrawSync(0);
    SetDispMask(1);
    func_8005B46C();
    if (D_80039CAD != 0 && D_80039CAC != 0) {
        D_80039CAD = 0;
        D_80039CA9 = 1;
    }
    if (D_80039CAB == 0 && D_80039CAD == 0 && D_80039CA9 != 0) {
        func_800AE7A4();
    }
    func_800AE838();
    if (D_80039CAA == 0) {
        D_80039CB0 = 0;
    }
    if (D_80039CAB != 0) {
        func_80050804(4, 0);
        D_80039C5E = 2;
        func_800A88F8();
    } else if (D_80039CAD != 0) {
        func_80050804(4, 0);
        D_80039C5E = 1;
        D_8017F3E0.x = 0x2C0;
        D_8017F3E0.y = 0;
        D_8017F3E0.w = 0x40;
        D_8017F3E0.h = 0x100;
        MoveImage(&D_8017F3E0, 0x2C0, 0x100);
        D_8017F3E0.x = 0x300;
        D_8017F3E0.y = 0;
        D_8017F3E0.w = 0x40;
        D_8017F3E0.h = 0x100;
        MoveImage(&D_8017F3E0, 0x300, 0x100);
        DrawSync(0);
        D_8017F3E0.x = 0;
        D_8017F3E0.y = 0x1EF;
        D_8017F3E0.w = 0x100;
        D_8017F3E0.h = 9;
        MoveImage(&D_8017F3E0, 0, 0x1E6);
        DrawSync(0);
        D_8017F3E0.x = 0x230;
        D_8017F3E0.y = 0x180;
        D_8017F3E0.w = 0x10;
        D_8017F3E0.h = 0x40;
        MoveImage(&D_8017F3E0, 0x220, 0x180);
        DrawSync(0);
        D_8017F3E0.x = 0x100;
        D_8017F3E0.y = 0x1FF;
        D_8017F3E0.w = 0x10;
        D_8017F3E0.h = 1;
        MoveImage(&D_8017F3E0, 0x100, 0x1FE);
        DrawSync(0);
        if ((D_80031A5E != 0xFF && D_80031A5E >= 31) || (D_80031A5F != 0xFF && D_80031A5F >= 31)) {
            MoveImage(&D_800BCB70, 0x180, 0x100);
            MoveImage(&D_800BCB78, 0x140, 0x1E2);
            DrawSync(0);
        }
        func_800A4F44();
    } else if (D_80039CA9 != 0 || D_80039C5E != 0) {
        if (D_80039CAC != 0) {
            func_800A2D0C(1);
            D_80039CAC = 0;
        } else {
            func_800A2D0C(0);
        }
        D_80039CA9 = 0;
        StoreImage(&D_800BCB88, D_8004AC18);
        rp = &D_800BCB80;
        goto store;
    } else {
        id = D_80041B10;
        func_80086838();
        func_800962E8();
        func_800866FC();
        func_80096230();
        if (D_80039CA4 >= 99999999) {
            D_80039CA4 = 99999999;
        }
        if (D_80039CA4 <= -9999999) {
            D_80039CA4 = -9999999;
        }
        if (D_80039CAF != 0) {
            if (!(D_80041B48 & 2)) {
                D_80039CAF = 0;
                func_800A2D0C(0);
                D_80039CA9 = 0;
                StoreImage(&D_800BCB98, D_8004AC18);
                rp = &D_800BCB90;
            store:
                StoreImage(rp, D_80042B70);
                DrawSync(0);
                goto run;
            }
            func_80073B7C();
            D_80041B40--;
            D_80041B3C--;
            func_800866A8();
            D_80039CAF = 0;
        }
        id = func_8009C5C0(id);
        if (id == 48 && (D_80041B78 & 2) && D_80041B14 != 0) {
            if (D_80039D21 != 0) {
                goto def;
            }
            func_80050804(4, 0);
            func_80050804(4, 1);
            func_80050804(4, 4);
            func_8004FDF4();
        } else if (D_80039D21 == 0 && D_80041B14 != 0 && D_80048610 == 2
                   && (id == 48 || (u8)(id - 21) < 2 || id == 47)) {
            func_800A448C(7, 0);
            func_80050804(4, 0);
            D_80039CA9 = 0;
            func_8005B2CC();
            func_800A2D0C(0);
        } else {
        def:
            func_80050804(4, 0);
            while ((s16)func_800517C8() != 0) {
            }
            func_8004DCC0();
        }
    }
run:
    while (D_801A2560 == 0) {
        D_801EF6C8.idx = D_801EF6C8.idx == 0;
        D_801EF6C8.prim = D_801BCE18[D_801EF6C8.idx].prim;
        for (i = 0; i < 10; i++) {
            ClearOTagR(D_801BCE18[D_801EF6C8.idx].layer[i].buf, D_801BCE18[D_801EF6C8.idx].layer[i].size);
        }
        SetBackColor(D_8018E468.r, D_8018E468.g, D_8018E468.b);
        D_8018E468.r = 0x40;
        D_8018E468.g = 0x40;
        D_8018E468.b = 0x40;
        for (o = g_ui_pool; o < &D_801BC448; o++) {
            if (o->used) {
                o->update(o);
            }
        }
        a = D_801BC5E8.active;
        for (o = g_ui_pool; o < &D_801BC448; o++) {
            if (o->used) {
                *a++ = o;
            }
        }
        D_801BC5E8.end = a;
        for (pp = D_801BC5E8.active; pp < D_801BC5E8.end; pp++) {
            if ((*pp)->kill) {
                (*pp)->destroy(*pp);
                (*pp)->kill = 0;
                (*pp)->used = 0;
            }
        }
        for (pp = D_801BC5E8.active; pp < D_801BC5E8.end; pp++) {
            UiObj *q = *pp;

            if (q->used && q->parent && !q->parent->used) {
                q->destroy(q);
                (*pp)->kill = 0;
                (*pp)->used = 0;
            }
        }
        for (i = 0; i < 10; i++) {
            RotMatrix(&g_ui_views[i].rot, &g_ui_views[i].mat);
            TransMatrix(&g_ui_views[i].mat, &g_ui_views[i].pos);
        }
        for (pp = D_801BC5E8.active; pp < D_801BC5E8.end; pp++) {
            UiObj *q = *pp;

            if (q->used && q->has_xform) {
                RotMatrix(&q->rot, &q->mat);
                TransMatrix(&(*pp)->mat, &(*pp)->pos);
            }
        }
        for (pp = D_801BC5E8.active; pp < D_801BC5E8.end; pp++) {
            UiObj *q = *pp;

            if (q->used && q->has_xform && q->parent) {
                CompMatrix(&q->parent->mat, &q->mat, &q->mat);
            }
        }
        for (pp = D_801BC5E8.active; pp < D_801BC5E8.end; pp++) {
            UiObj *q = *pp;

            if (q->used && q->has_xform && q->attached) {
                CompMatrix(&q->attach->mat, &q->mat, &q->mat);
            }
        }
        for (pp = D_801BC5E8.active; pp < D_801BC5E8.end; pp++) {
            UiObj *q = *pp;

            if (q->used && q->visible) {
                if (q->view != 2) {
                    MulMatrix0(&D_800B8D84[q->view], &q->mat, &m);
                    SetLightMatrix(&m);
                } else {
                    SetLightMatrix(D_800B8D84);
                }
                SetColorMatrix(&D_800B8DC4[(*pp)->light]);
                {
                    UiObj *q2 = *pp;

                    if (q2->scaled && q2->has_xform) {
                        ScaleMatrix(&q2->mat, &q2->scale);
                    }
                }
                {
                UiObj *q3 = *pp;

                if (q3->render_type == 3) {
                    func_800653D4(q3);
                } else if (q3->render_type == 0) {
                    func_80057190(q3->ot[D_801EF6C8.idx], ((u16 *)&q3->res)[0], ((u16 *)&q3->res)[1], &q3->mat);
                } else if (q3->render_type == 4) {
                    func_800656B8(q3);
                } else if (q3->render_type == 5) {
                    func_80064874(q3);
                } else if (q3->render_type == 2) {
                    func_80059B50(q3->ot[D_801EF6C8.idx], ((u16 *)&q3->res)[0], ((u16 *)&q3->res)[1], &q3->mat);
                } else if (q3->render_type == 6) {
                    func_8006599C(q3);
                } else if (q3->render_type == 7) {
                    func_80066454(q3);
                } else if (q3->render_type == 1) {
                    func_8005F5E8(q3);
                } else if (q3->render_type == 8) {
                    func_80064EB0(q3);
                }
                }
            }
        }
        DrawSync(0);
        n = 10;
        VSync(D_801A2574);
        PutDispEnv(&D_801BCE18[D_801EF6C8.idx].disp);
        PutDrawEnv(&D_801BCE18[D_801EF6C8.idx].draw);
        if (D_801A2578 != 0) {
            n = 2;
        }
        for (i = 0; i < n; i++) {
            PutDrawEnv(&D_801BCE18[D_801EF6C8.idx].layer[i].draw);
            DrawOTag(D_801BCE18[D_801EF6C8.idx].layer[i].buf + D_801BCE18[D_801EF6C8.idx].layer[i].size - 2);
        }
        PutDrawEnv(&D_801BCE18[D_801EF6C8.idx].layer[0].draw);
        FntFlush(-1);
        GetRCnt(0xF2000001);
    }
    if (D_80039CAB != 0) {
        RECT c = D_8004D148;

        ClearImage(&c, 0, 0, 0);
        func_8005F260(&D_80040850[0], 960, 0, 272, 489);
        func_8005F260(&D_80040850[1], 960, 16, 272, 489);
    } else if (D_80039CAD != 0) {
        RECT c = D_8004D148;

        ClearImage(&c, 0, 0, 0);
        func_8005F260(D_80048510, 960, 0, 272, 489);
    }
    func_80051628();
    while ((s16)func_800517C8() != 0) {
    }
    func_80068790();
    func_80050D58();
    DrawSync(0);
}
