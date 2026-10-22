#include "ac1/ac1.h"
/* ovl202 0x8004F508 - mission load: reset the mission globals, read the mission
 * archives into the load buffer and hand each size-prefixed chunk to its owner
 * (textures, models, script blob, map, objects ...), then start the objective
 * object, the mission timer and the AC entities. */

#define NEXT(p) ((p) = (u32 *)((u8 *)(p) + (*(p) + 4)))

typedef struct {
    u8 pad00[0x14];
    void (*start)(void);        /* +0x14 */
} ObjObj;

extern u8 D_800411F8;
extern u8 D_8004121C;
extern u8 D_80041EC6[];
extern u32 D_80091228[];
extern u32 D_800B1248[];
extern u32 D_800B17C8[];
extern u8 D_8019F518;
extern u32 *D_8019F520;
extern s16 D_8019F52E;
extern s16 D_8019F530;
extern s16 D_8019F532;
extern u8 D_801C4B40[];
extern u32 D_801ED850[];
extern void func_80011FD8(u32 *buf);
extern void func_800154D8(u32 *dst, u32 *src, u32 n);
extern void func_80015508(u8 *dst, u32 *src, u32 n);
extern void func_800165E4(s32 a, s32 b, u32 *buf);
extern void func_8004C7A8(s32 a, u32 *b, u32 *c);
extern u32 *func_8004C854(u32 *p);
extern void func_8004F1A8(s32 a);
extern void func_80053848(s32 a, u32 *p);
extern void func_80056958(void);
extern void func_80056A54(void);
extern void func_80056AD0(u32 *p);
extern void func_80057994(u16 i, u32 *v);
extern void func_8006D9DC(void);
extern void func_80071E14(u32 *p, u32 n);
extern void func_800731E8(u32 *p);
extern void func_8007334C(u32 *p);
extern void func_800739AC(u32 *p);
extern void func_80073B74(u32 *p);
extern void func_80078B14(void *a, void *b);
extern void func_8007EAE8(void *a);

void func_8004F508(void)
{
    u32 *p;
    u32 n;
    u32 *src;

    g_objective_obj = D_801C4B40;
    D_8019F52E = 128;
    g_mission_end_timer = 0;
    g_mission_end_seq = 0;
    g_mission_flags = 0;
    D_8019F530 = 0;
    D_8019F532 = 64;
    D_8019F520 = D_800B17C8;
    D_8019F518 = D_800411F8;
    func_800165E4(1, 100, D_800B17C8);
    func_80011FD8(D_800B17C8);
    func_800165E4(1, D_8019F518, D_800B17C8);
    func_80011FD8(D_800B17C8);
    p = D_8019F520;
    func_800165E4(2, 200, p);
    func_80053848(40, p + 1);
    NEXT(p);
    D_8019F520 = p + 1;
    NEXT(p);
    func_8004C7A8(0, D_8019F520, p + 1);
    D_8019F520 = p;
    NEXT(p);
    func_80015508(D_80041EC6, p + 1, *p >> 1);
    NEXT(p);
    switch (D_8004121C) {
    case 0:
        func_800154D8(D_801ED850, p + 1, *p >> 2);
        NEXT(p);
        NEXT(p);
        NEXT(p);
        break;
    case 1:
        NEXT(p);
        func_800154D8(D_801ED850, p + 1, *p >> 2);
        NEXT(p);
        NEXT(p);
        break;
    case 2:
        NEXT(p);
        NEXT(p);
        func_800154D8(D_801ED850, p + 1, *p >> 2);
        NEXT(p);
        break;
    }
    func_800154D8(D_800B1248, p + 1, *p >> 2);
    func_80056958();
    func_80056A54();
    {
        u8 *m = &D_8019F518;
        func_800165E4(2, *m * 2, (u32 *)D_801C4B40);
        p = D_8019F520;
        func_800165E4(2, (*m * 2) | 1, p);
    }
    func_80053848(260, p + 1);
    NEXT(p);
    func_80053848(128, p + 1);
    NEXT(p);
    func_80053848(160, p + 1);
    NEXT(p);
    func_80057994(0, p + 1);
    NEXT(p);
    g_mission_ctl.vm_blob = (u8 *)(p + 1);
    NEXT(p);
    D_8019F520 = func_8004C854(p + 1);
    NEXT(p);
    func_800154D8(D_80091228, p + 1, *p >> 2);
    NEXT(p);
    func_8006D9DC();
    func_80071E14(p + 1, *p);
    NEXT(p);
    func_8007334C(p + 1);
    NEXT(p);
    func_800731E8(p + 1);
    NEXT(p);
    func_80056AD0(p + 1);
    NEXT(p);
    func_800739AC(p + 1);
    NEXT(p);
    func_80073B74(p + 1);
    NEXT(p);
    switch (D_8004121C) {
    case 2:
        NEXT(p);
        NEXT(p);
    case 1:
        NEXT(p);
        NEXT(p);
    case 0:
        src = p + 1;
        n = *p;
        break;
    }
    NEXT(p);
    n >>= 2;
    if (*p != 0) {
        u32 **q = &D_8019F520;
        func_800154D8(*q, src, n);
        func_8004C7A8(2, *q, p + 1);
        *q += n;
    }
    ((ObjObj *)g_objective_obj)->start();
    g_mission_timer = 0;
    func_8004F1A8(0);
    func_80078B14(g_ac_entities, (u8 *)g_ac_entities - 11264);
    func_8007EAE8(g_ac_entities);
}
