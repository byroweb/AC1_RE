/* tgt declared before dir (retail slots); first call through static inline get_dir() so &dir is not CSEd into an s-reg; the call sits in two identical arms of if (dx != 0 && u == w): dx/dz become global (no local-alloc tie with the p[0]/pos.x temps), jump2 cross-jumps the arms, and the earlier dx branch stops delete_computation from removing the (dead) abs() of the uninitialised u, w. */
#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x108];
    s16 hp;             /* +0x108 */
    u8 pad10a[2];
    s16 tag;            /* +0x10C */
    u8 pad10e[0x62];
} Obj;

extern int abs(int);
extern void func_801C62CC(void);
extern void func_80014728(s32, AcPos *);
extern void func_8004CC30(s32, s32, void *, s32, s32, s32, s32);
extern void func_800529F0(s32, s32, s32);
extern Obj *func_8006D5BC(AcEntity *, AcPos *, s32, s32);
extern s16 *func_80074280(Obj *);
extern void func_800835E8(s32, Obj *, void *, AcPos *, AcPos *);
extern s32 D_801A5DC0;
extern u8 D_801CDC74[];

static inline void get_dir(s32 yaw, AcPos *d) { func_80014728(yaw, d); }

void func_801C66DC(AcEntity *e)
{
    AcPos tgt;
    AcPos dir;
    Obj *hit;
    s16 *p;
    s32 u;
    s32 w;

    func_801C62CC();
    if (((Obj *)e)->hp > 0) {
        get_dir(e->yaw, &dir);
        tgt.x = e->pos.x + ((dir.x * 625) >> 12);
        tgt.z = e->pos.z + ((dir.z * 625) >> 12);
        tgt.y = e->pos.y;
        hit = func_8006D5BC(e, &tgt, 250, 250);
        if (hit != 0) {
            p = func_80074280(hit);
            {
                s32 dx, dz;
                dx = p[0] - e->pos.x;
                u = abs(u);
                dz = p[2] - e->pos.z;
                w = abs(w);
                if (dx != 0 && u == w) {
                    dir.x = dx;
                    dir.z = dz;
                    func_800835E8(0, hit, D_801CDC74, &dir, &e->pos);
                } else {
                    dir.x = dx;
                    dir.z = dz;
                    func_800835E8(0, hit, D_801CDC74, &dir, &e->pos);
                }
            }
        }
    }
    if ((e->unk42[0] == 0 && ((Obj *)e)->tag == 22) || (e->unk42[0] == 1 && ((Obj *)e)->tag == 66)) {
        func_8004CC30(140, 0, &e->pos, 127, 60000, 66000, 0);
    }
    if (e->unk42[0] == 1 && ((Obj *)e)->tag == 110) {
        func_800529F0(0, 0, 5);
    }
    func_8004CC30(138, D_801A5DC0 + 4096, &e->pos, 120, 16000, 17000, ((Obj *)e)->hp - 40);
}
