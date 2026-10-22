#include "ac1/ac1.h"
/* ovl202 0x8008C2FC: run a mission cut-scene: load VM thread set (flags & 0xFF), optional
 * per-frame hook (flags 0x100, variadic arg), freeze the ACs (+0x32 |= 0x4000), step the
 * VM for vm_extra frames with the camera following thread 0's object, then restore the
 * camera pose / view unless flags 0x200; flags 0x400 waits for 0x80052338. */
typedef struct {
    SVECTOR pos;
    SVECTOR rot;
} CamPose;
typedef struct {
    u8 pad0[4];
    struct VmObj *obj;  /* +0x04 */
    u8 pad08[4];
    u16 kind;           /* +0x0C */
    u8 pad0e[6];
    SVECTOR *mem;       /* +0x14 */
} VmThr;
typedef struct VmObj {
    u8 pad0[4];
    u8 *ent;            /* +0x04 */
    u8 pad08[0x14 - 0x08];
    SVECTOR *mem;       /* +0x14 */
} VmObj;
extern CamPose D_801AD66C;
extern void (*D_801D0B54)(void);
extern VmThr *D_801D0B60;
extern s32 D_801AD668;
extern s32 D_801AC80C[];
extern u32 D_800411F4;
extern s32 D_801987D4;
extern s32 func_8008C178(s32 set);
extern void func_8008B380(void);
extern void func_80055DB8(void *a);
extern void func_80062BD8(CamPose *p);
extern void func_8004CF7C(CamPose *p);
extern s32 func_80052338(void);
extern void func_80055E20(void *a, s32 b);

void func_8008C2FC(s32 flags, ...)
{
    CamPose saved;
    CamPose p;
    u32 mflags;
    s32 old80c;
    s32 old668;
    AcEntity *e;
    VmThr *t;
    VmObj *w;
    s32 i;
    SVECTOR *a, *b;

    g_mission_ctl.unk14 = 0;
    if (flags & 0x100) {
        g_mission_ctl.unk14 = (&flags)[1];
    }
    switch (func_8008C178(flags & 0xFF)) {
    case 0:
        mflags = g_mission_flags;
        g_ac_entities[0].fire_flags = 0;
        g_ac_entities[0].flags = 0;
        for (e = g_ac_entities, i = 16; i != 0; e++, i--) {
            if (e->def != 0) {
                *(u16 *)((u8 *)e + 0x32) |= 0x4000;
            }
        }
        saved = D_801AD66C;
        old80c = D_801AC80C[0];
        old668 = D_801AD668;
        D_801AC80C[0] = 0;
        D_801AD668 = 0;
        do {
            func_8008B380();
            if (D_801D0B54 != 0) {
                D_801D0B54();
            }
            t = D_801D0B60;
            if (t->kind == 1 && (w = t->obj) != 0) {
                a = t->mem;
                b = w->mem;
                p.pos = *a;
                p.rot = *b;
                if ((AcEntity *)w->ent >= g_ac_entities && (AcEntity *)w->ent <= &g_ac_entities[15]) {
                    p.rot.vy -= *(u16 *)(w->ent + 0x2A) >> 1;
                }
                func_80055DB8(g_ac_entities[0].unk60);
                func_80062BD8(&p);
                func_8004CF7C(&p);
            } else {
                func_80062BD8(0);
            }
        } while (g_mission_ctl.vm_extra-- != 0);
        if (flags & 0x400) {
            while (func_80052338() == 0) {
                func_80062BD8(0);
            }
        }
        for (e = g_ac_entities, i = 16; i != 0; e++, i--) {
            if (e->def != 0) {
                *(u16 *)((u8 *)e + 0x32) &= 0x3FFF;
            }
        }
        if (g_mission_end_timer == 0) {
            D_801AC80C[0] = old80c;
        }
        if (D_800411F4 & 0x10000) {
            func_80055E20(g_ac_entities[0].unk60, D_801987D4);
        }
        g_mission_flags = (g_mission_flags & ~0x400) | (mflags & 0x400);
        if (!(flags & 0x200)) {
            D_801AD66C = saved;
            D_801AD668 = old668;
            if (old668 != 0) {
                func_80062BD8(&saved);
                func_8004CF7C(&saved);
            }
        }
        break;
    case 1:
        func_8008B380();
        break;
    }
}
