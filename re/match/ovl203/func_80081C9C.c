#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"

/* ovl203 port of ovl202 0x80083038: player record through D_80039D18, camera at 0x80194738 */
typedef struct {
    u8 pad000[0x220];
    u16 pitch;                  /* +0x220 (ovl202 D_80039D18->pitch) */
    u8 pad222[0x254 - 0x222];
    u32 flags;                  /* +0x254 (ovl202 D_80039D18->flags) */
    u8 pad258[0x394 - 0x258];
    AcEntity *ent;              /* +0x394 */
} Pl3;
extern Pl3 *D_80039D18;
extern CamParm D_80194738;

typedef struct {
    u32 flags;
} MFlags;

extern void func_800557B4(void *o);
extern void func_8005581C(void *o, u8 *list);
extern void func_800732FC(AcEntity *e, SVECTOR *eye, s32 a, s32 b, s32 c, s32 d);
extern void func_8008196C(void);
extern void func_80081AE0(SVECTOR *pos, SVECTOR *ang, s32 *o, SVECTOR *dir);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern s32 func_8006F95C(s32 *seg, SVECTOR *hit, s32 mask);
extern s32 func_800702B4(SVECTOR *box, s32 a, SVECTOR *hit, s32 b, s32 mask);

/* Per-frame view camera: out[0] = eye, out[1] = target. */
void func_80081C9C(SVECTOR *out) {
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
    func_800557B4((void *)D_80039D18->ent->unk60);
    if (D_80039D18->flags & 0x10000) {
        func_8005581C((void *)D_80039D18->ent->unk60, D_80194738.parts);
        func_800732FC(D_80039D18->ent, &v, D_80194738.eye_a, D_80194738.eye_b, 1, 0);
        func_8008196C();
        a.vx = D_80039D18->pitch + D_80194738.ox;
        a.vy = D_80039D18->ent->yaw + D_80194738.oy - 2048;
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
        D_80194738.snap = 1;
        d.vx = v.vx - (d.vx >> 4);
        d.vy = v.vy - (d.vy >> 4);
        d.vz = v.vz - (d.vz >> 4);
    } else {
        ang.vx = D_80039D18->pitch;
        ang.vy = D_80039D18->ent->yaw;
        func_80081AE0(&p, &ang, co, &d);
        if (func_8006F95C(co, &hit, 7)) {
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
            if (!func_800702B4(box, 0, &hit, 0, 7)) {
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
        if (D_80194738.mode & 1) {
            v.vx = D_80194738.eye[0];
            v.vy = D_80194738.eye[1];
            v.vz = D_80194738.eye[2];
            if (D_80194738.snap != 0) {
            snap:
                D_80194738.snap = 0;
                goto push;
            }
        } else {
            if (D_80194738.snap != 0) {
                goto snap;
            }
            v.vx = D_80194738.eye[0] + ((v.vx - D_80194738.eye[0]) >> D_80194738.ease);
            v.vy = D_80194738.eye[1] + ((v.vy - D_80194738.eye[1]) >> D_80194738.ease);
            v.vz = D_80194738.eye[2] + ((v.vz - D_80194738.eye[2]) >> D_80194738.ease);
        }
        d.vx = D_80194738.at[0] + ((d.vx - D_80194738.at[0]) >> 1);
        d.vy = D_80194738.at[1] + ((d.vy - D_80194738.at[1]) >> 1);
        d.vz = D_80194738.at[2] + ((d.vz - D_80194738.at[2]) >> 1);
    push:
        i = 6;
        do {
            box[0].vx = v.vx - 60;
            box[0].vy = v.vy - 60;
            box[0].vz = v.vz - 60;
            box[1].vx = v.vx + 60;
            box[1].vy = v.vy + 60;
            box[1].vz = v.vz + 60;
            if (!func_800702B4(box, 0, &hit, 0, 7)) {
                break;
            }
            i--;
            v.vx += hit.vx >> 2;
            v.vy += hit.vy >> 2;
            v.vz += hit.vz >> 2;
        } while (i != 0);
    }
    out[0].vx = D_80194738.eye[0] = v.vx;
    out[0].vy = D_80194738.eye[1] = v.vy;
    out[0].vz = D_80194738.eye[2] = v.vz;
    out[1].vx = D_80194738.at[0] = d.vx;
    out[1].vy = D_80194738.at[1] = d.vy;
    out[1].vz = D_80194738.at[2] = d.vz;
    D_80194738.frames++;
}
