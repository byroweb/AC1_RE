#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"

/* per-player record (0x39C, array at 0x80040FA0); D_80039D18 = current */
typedef struct {
    u8 unk0[0x24];
    s32 unk24;                  /* +0x24 */
    u8 unk28[0x58];
    TargetSlot *target;         /* +0x80 lock list head */
    u8 unk84[0x220 - 0x84];
    u16 pitch;                  /* +0x220 (0x800411C0 for player 0) */
    u8 unk222[0x254 - 0x222];
    u32 flags;                  /* +0x254 (0x800411F4) */
    u8 unk258[0x394 - 0x258];
    AcEntity *ent;              /* +0x394 */
    CamParm *cam;               /* +0x398 */
} Player;

extern Player *D_80039D18;
extern void func_80073138(AcEntity *e, SVECTOR *eye, s32 a, s32 b, s32 c, s32 d);
extern void func_8008166C(void);
extern void func_8008181C(SVECTOR *pos, SVECTOR *ang, s32 *o, SVECTOR *dir);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern s32 func_8006F798(s32 *seg, SVECTOR *hit, s32 mask);
extern s32 func_800700F0(SVECTOR *box, s32 a, SVECTOR *hit, s32 b, s32 mask);

/* Per-frame view camera: out[0] = eye, out[1] = target. */
void func_80081A00(SVECTOR *out) {
    SVECTOR p;
    SVECTOR d;
    SVECTOR v;
    SVECTOR ang;
    s32 co[6];
    SVECTOR box[2];
    SVECTOR hit;
    SVECTOR a;
    s32 i;

    p.vx = D_80039D18->ent->pos.x;
    p.vz = D_80039D18->ent->pos.z;
    p.vy = D_80039D18->ent->pos.y - *(u16 *)((u8 *)D_80039D18->ent + 0x6C) + (*(u16 *)((u8 *)D_80039D18->ent + 0x6C) >> 3);
    if (D_80039D18->flags & 0x10000) {
        func_80073138(D_80039D18->ent, &v, D_80039D18->cam->eye_a, D_80039D18->cam->eye_b, 1, 0);
        func_8008166C();
        a.vx = D_80039D18->pitch + D_80039D18->cam->ox;
        a.vy = D_80039D18->ent->yaw + D_80039D18->cam->oy - 2048;
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
        d.vx = v.vx - (d.vx >> 4);
        d.vy = v.vy - (d.vy >> 4);
        d.vz = v.vz - (d.vz >> 4);
        D_80039D18->cam->snap = 1;
    } else {
        ang.vx = D_80039D18->pitch;
        ang.vy = D_80039D18->ent->yaw;
        func_8008181C(&p, &ang, co, &d);
        if (func_8006F798(co, &hit, 7)) {
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
            if (!func_800700F0(box, 0, &hit, 0, 7)) {
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
        if (D_80039D18->cam->mode & 1) {
            v.vx = D_80039D18->cam->eye[0];
            v.vy = D_80039D18->cam->eye[1];
            v.vz = D_80039D18->cam->eye[2];
            if (D_80039D18->cam->snap != 0) {
                D_80039D18->cam->snap = 0;
                goto push;
            }
        } else {
            if (D_80039D18->cam->snap != 0) {
                D_80039D18->cam->snap = 0;
                goto push;
            }
            v.vx = D_80039D18->cam->eye[0] + ((v.vx - D_80039D18->cam->eye[0]) >> D_80039D18->cam->ease);
            v.vy = D_80039D18->cam->eye[1] + ((v.vy - D_80039D18->cam->eye[1]) >> D_80039D18->cam->ease);
            v.vz = D_80039D18->cam->eye[2] + ((v.vz - D_80039D18->cam->eye[2]) >> D_80039D18->cam->ease);
        }
        d.vx = D_80039D18->cam->at[0] + ((d.vx - D_80039D18->cam->at[0]) >> 1);
        d.vy = D_80039D18->cam->at[1] + ((d.vy - D_80039D18->cam->at[1]) >> 1);
        d.vz = D_80039D18->cam->at[2] + ((d.vz - D_80039D18->cam->at[2]) >> 1);
    push:
        i = 6;
        do {
            box[0].vx = v.vx - 60;
            box[0].vy = v.vy - 60;
            box[0].vz = v.vz - 60;
            box[1].vx = v.vx + 60;
            box[1].vy = v.vy + 60;
            box[1].vz = v.vz + 60;
            if (!func_800700F0(box, 0, &hit, 0, 7)) {
                break;
            }
            i--;
            v.vx += hit.vx >> 2;
            v.vy += hit.vy >> 2;
            v.vz += hit.vz >> 2;
        } while (i != 0);
    }
    out[0].vx = D_80039D18->cam->eye[0] = v.vx;
    out[0].vy = D_80039D18->cam->eye[1] = v.vy;
    out[0].vz = D_80039D18->cam->eye[2] = v.vz;
    out[1].vx = D_80039D18->cam->at[0] = d.vx;
    out[1].vy = D_80039D18->cam->at[1] = d.vy;
    out[1].vz = D_80039D18->cam->at[2] = d.vz;
    D_80039D18->cam->frames++;
}
