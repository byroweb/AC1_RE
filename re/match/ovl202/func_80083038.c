#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"

typedef struct {
    u32 flags;
} MFlags;

extern u32 D_800411F4;
extern u16 D_800411C0;
extern void func_80055DB8(void *o);
extern void func_80055E20(void *o, u8 *list);
extern void func_800745D0(AcEntity *e, SVECTOR *eye, s32 a, s32 b, s32 c, s32 d);
extern void func_80082D04(void);
extern void func_80082E7C(SVECTOR *pos, SVECTOR *ang, s32 *o, SVECTOR *dir);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern s32 func_800705D8(s32 *seg, SVECTOR *hit, s32 mask);
extern s32 func_80070F30(SVECTOR *box, s32 a, SVECTOR *hit, s32 b, s32 mask);

/* Per-frame view camera: out[0] = eye, out[1] = target. */
void func_80083038(SVECTOR *out) {
    SVECTOR p;
    SVECTOR d;
    SVECTOR v;
    SVECTOR ang;
    s32 co[6];
    SVECTOR box[2];
    SVECTOR hit;
    SVECTOR a;
    AcEntity *e = &g_ac_entities[0];
    s32 i;

    p.vx = e->pos.x;
    p.vz = e->pos.z;
    p.vy = e->pos.y - *(u16 *)((u8 *)e + 0x6C) + (*(u16 *)((u8 *)e + 0x6C) >> 3);
    func_80055DB8((void *)e->unk60);
    if (D_800411F4 & 0x10000) {
        func_80055E20((void *)e->unk60, D_801987D0.parts);
        func_800745D0(e, &v, D_801987D0.eye_a, D_801987D0.eye_b, 1, 0);
        func_80082D04();
        a.vx = D_800411C0 + D_801987D0.ox;
        a.vy = e->yaw + D_801987D0.oy - 2048;
        if (a.vx >= 2048) {
            a.vx -= 4096;
        }
        if (a.vx > 400) {
            a.vx = 400;
        }
        if (a.vx < -400) {
            a.vx = -400;
        }
        func_80014774(&a, &d);
        D_801987D0.snap = 1;
        d.vx = v.vx - (d.vx >> 4);
        d.vy = v.vy - (d.vy >> 4);
        d.vz = v.vz - (d.vz >> 4);
    } else {
        ang.vx = D_800411C0;
        ang.vy = e->yaw;
        func_80082E7C(&p, &ang, co, &d);
        if (func_800705D8(co, &hit, 7)) {
            co[3] = hit.vx;
            co[4] = hit.vy;
            co[5] = hit.vz;
        }
        d.vx = p.vx - (d.vx >> 3);
        d.vy = p.vy - (d.vy >> 3);
        d.vz = p.vz - (d.vz >> 3);
        i = 6;
        do {
            box[0].vx = co[3] - 60;
            box[0].vy = co[4] - 100;
            box[0].vz = co[5] - 60;
            box[1].vx = co[3] + 60;
            box[1].vy = co[4] + 100;
            box[1].vz = co[5] + 60;
            if (!func_80070F30(box, 0, &hit, 0, 7)) {
                break;
            }
            i--;
            co[3] += hit.vx >> 2;
            co[4] += hit.vy >> 2;
            co[5] += hit.vz >> 2;
        } while (i != 0);
        v.vx = co[3];
        v.vy = co[4];
        v.vz = co[5];
        if (D_801987D0.mode & 1) {
            v.vx = D_801987D0.eye[0];
            v.vy = D_801987D0.eye[1];
            v.vz = D_801987D0.eye[2];
            if (D_801987D0.snap != 0) {
            snap:
                D_801987D0.snap = 0;
                goto push;
            }
        } else {
            if (D_801987D0.snap != 0) {
                goto snap;
            }
            v.vx = D_801987D0.eye[0] + ((v.vx - D_801987D0.eye[0]) >> D_801987D0.ease);
            v.vy = D_801987D0.eye[1] + ((v.vy - D_801987D0.eye[1]) >> D_801987D0.ease);
            v.vz = D_801987D0.eye[2] + ((v.vz - D_801987D0.eye[2]) >> D_801987D0.ease);
        }
        d.vx = D_801987D0.at[0] + ((d.vx - D_801987D0.at[0]) >> 1);
        d.vy = D_801987D0.at[1] + ((d.vy - D_801987D0.at[1]) >> 1);
        d.vz = D_801987D0.at[2] + ((d.vz - D_801987D0.at[2]) >> 1);
    push:
        i = 6;
        do {
            box[0].vx = v.vx - 60;
            box[0].vy = v.vy - 60;
            box[0].vz = v.vz - 60;
            box[1].vx = v.vx + 60;
            box[1].vy = v.vy + 60;
            box[1].vz = v.vz + 60;
            if (!func_80070F30(box, 0, &hit, 0, 7)) {
                break;
            }
            i--;
            v.vx += hit.vx >> 2;
            v.vy += hit.vy >> 2;
            v.vz += hit.vz >> 2;
        } while (i != 0);
    }
    out[0].vx = D_801987D0.eye[0] = v.vx;
    out[0].vy = D_801987D0.eye[1] = v.vy;
    out[0].vz = D_801987D0.eye[2] = v.vz;
    out[1].vx = D_801987D0.at[0] = d.vx;
    out[1].vy = D_801987D0.at[1] = d.vy;
    out[1].vz = D_801987D0.at[2] = d.vz;
    D_801987D0.frames++;
}
