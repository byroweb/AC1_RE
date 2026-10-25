/* ovl202 0x8004E22C. Loop levers: the dead "rk = 0" in the loop makes rk non-invariant for loop.c (invariant_p: n_times_set != 0), so only i*6 is strength-reduced and rk + off stays an add per use; one pointer variable r (global pseudo) gives the address a0 everywhere; "k++, i++" orders the biv increments. */
#include "ac1/ac1.h"
/* ovl202 0x8004E22C - motion-clip step of a mover (AcEntity +0x64): find the
 * key pair around the current frame of clip idx in the bank, give the move
 * delta to the next key (clamped to +-limxz*4 / +-limy*2, frames rescaled by
 * the clamped length), rotate it by the facing into *out (/4), the rotation
 * delta into *drot; returns the frames to the next key (at least 1). */

typedef struct {
    u16 frame;
    s16 vx, vy, vz;
} AnimKey;

typedef struct {
    s16 vx, vy, vz;
} AnimRot;

typedef struct {
    u8 pad0[6];
    u16 n;              /* +6 key count | 0x8000 rotation keys follow */
    AnimKey keys[1];    /* +8 keys[0] = clip total */
} AnimHdr;

typedef struct {
    u8 idx;             /* +0x00 clip */
    u8 pad1[5];
    u16 limxz;          /* +0x06 */
    u16 limy;           /* +0x08 */
    s16 frame;          /* +0x0A */
    s16 dir;            /* +0x0C */
    u8 padE[0x1C - 0xE];
    struct AnimBank *bank;  /* +0x1C */
} AnimSt;

typedef struct AnimBank {
    u8 pad0[0xC];
    u16 ofs[1];         /* +0xC clip offsets from +0xC */
} AnimBank;

#define RK(j) (*({ s32 o_ = (j) * 6; (AnimRot *)((u8 *)rk + o_); }))

s32 func_8004E22C(AnimSt *a, SVECTOR *rot, SVECTOR *rot0, SVECTOR *out, SVECTOR *drot)
{
    SVECTOR d;
    SVECTOR dr;
    SVECTOR ang;
    MATRIX m;
    SVECTOR c;
    AnimHdr *h;
    AnimKey *k;
    AnimRot *rk;
    AnimRot *r;
    s32 n;
    s32 hasrot;
    s32 i;
    s32 len;
    s32 f;
    s32 prev;
    s32 lim;
    s32 limy;
    s32 clamp;
    s32 s;
    AnimKey *keys;

    h = (AnimHdr *)((u8 *)a->bank + (a->bank->ofs[a->idx] + 12));
    n = h->n;
    keys = h->keys;
    hasrot = n & 0x8000;
    n &= 0x7FFF;
    rk = (AnimRot *)(keys + n);
    if (a->dir >= 0) {
        if (a->frame == 0) {
            d.vx = keys[0].vx;
            d.vy = keys[0].vy;
            d.vz = keys[0].vz;
            if (hasrot) {
                *rot = *rot0;
                dr.vx = rk[0].vx;
                dr.vy = rk[0].vy;
                dr.vz = rk[0].vz;
            }
            len = keys[0].frame;
            goto done;
        }
    } else {
        if (a->frame == 0) {
            d.vx = ((AnimKey *)rk - 1)->vx;
            d.vy = ((AnimKey *)rk - 1)->vy;
            d.vz = ((AnimKey *)rk - 1)->vz;
            if (hasrot) {
                *rot = *rot0;
                dr.vx = ({ s32 o_ = n * 6; (AnimRot *)((u8 *)rk + o_) - 1; })->vx;
                dr.vy = ({ s32 o_ = n * 6; (AnimRot *)((u8 *)rk + o_) - 1; })->vy;
                dr.vz = ({ s32 o_ = n * 6; (AnimRot *)((u8 *)rk + o_) - 1; })->vz;
            }
            k = (AnimKey *)rk - 2;
            goto tail;
        }
        if (a->frame <= keys[0].frame) {
            d.vx = -keys[0].vx;
            d.vy = -keys[0].vy;
            d.vz = -keys[0].vz;
            if (hasrot) {
                dr.vx = -rk[0].vx;
                dr.vy = -rk[0].vy;
                dr.vz = -rk[0].vz;
            }
            len = -keys[0].frame;
            goto done;
        }
    }
    k = keys;
    for (i = 0; i < n; k++, i++) {
        if (k->frame >= (f = a->frame)) {
            if (f == k->frame) {
                d.vx = k->vx;
                d.vy = k->vy;
                d.vz = k->vz;
                len = f;
                if (hasrot) {
                    r = &RK(i);
                    dr.vx = r->vx;
                    dr.vy = r->vy;
                    dr.vz = r->vz;
                }
                break;
            }
            if (i >= n - 1) {
                k--;
            }
            prev = k->frame;
            d.vx = k->vx;
            d.vy = k->vy;
            d.vz = k->vz;
            len = f;
            if (hasrot) {
                r = &RK(i);
                dr.vx = r->vx;
                dr.vy = r->vy;
                dr.vz = r->vz;
            }
            if (a->dir >= 0) {
                k++;
            } else {
                k--;
            }
            len = k->frame - len;
            prev = k->frame - prev;
            d.vx = k->vx - d.vx;
            d.vy = k->vy - d.vy;
            d.vz = k->vz - d.vz;
            d.vx = d.vx * len / prev;
            d.vy = d.vy * len / prev;
            d.vz = d.vz * len / prev;
            if (hasrot) {
                r = &RK(i);
                dr.vx = r->vx - dr.vx;
                dr.vy = r->vy - dr.vy;
                dr.vz = r->vz - dr.vz;
                dr.vx = dr.vx * len / prev;
                dr.vy = dr.vy * len / prev;
                dr.vz = dr.vz * len / prev;
            }
            rk = 0;
            c = d;
            lim = a->limxz * 4;
            limy = a->limy * 2;
            clamp = 0;
            if (lim < d.vx) {
                d.vx = lim;
                clamp = 1;
            } else if (d.vx < -lim) {
                d.vx = -lim;
                clamp = 1;
            }
            if (lim < d.vz) {
                d.vz = lim;
                clamp = 1;
            } else if (d.vz < -lim) {
                d.vz = -lim;
                clamp = 1;
            }
            if (limy < d.vy) {
                d.vy = limy;
                clamp = 1;
            } else if (d.vy < -limy) {
                d.vy = -limy;
                clamp = 1;
            }
            if (clamp) {
                s = SquareRoot0(c.vx * c.vx + c.vy * c.vy + c.vz * c.vz);
                len = len * SquareRoot0(d.vx * d.vx + d.vy * d.vy + d.vz * d.vz) / s;
            }
            goto done;
        }
    }
    if (a->dir >= 0) {
        k++;
    } else {
        k--;
    }
tail:
    d.vx = k->vx - d.vx;
    d.vy = k->vy - d.vy;
    d.vz = k->vz - d.vz;
    if (hasrot) {
        r = &RK(i);
        dr.vx = r->vx - dr.vx;
        dr.vy = r->vy - dr.vy;
        dr.vz = r->vz - dr.vz;
    }
    len = k->frame - len;
done:
    ang.vx = rot->vx;
    ang.vy = 0x800 - rot->vy;
    ang.vz = rot->vz;
    RotMatrixYXZ(&ang, &m);
    ApplyMatrixSV(&m, &d, out);
    out->vx = out->vx >> 2;
    out->vy = out->vy >> 2;
    out->vz = out->vz >> 2;
    *drot = dr;
    if (len == 0) {
        len = 1;
    }
    return len;
}
