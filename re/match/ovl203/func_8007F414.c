#include "ac1/ac1.h"
/* ovl203 (jumping legs, kind 0xB0): pick the AC action (o+0x41) from the pad buttons held by the side in
 * D_80039D18 (bit masks +0x290..+0x2A4); message handler, always returns 0. */
typedef struct {
    u8 pad0[0x3C];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[3];
    u8 act;             /* +0x41 */
    u8 pad42[0x64 - 0x42];
    u8 mode;            /* +0x64 */
    u8 pad65[0xCE - 0x65];
    s16 hCE;            /* +0xCE */
} PObj3;
typedef struct {
    u8 pad0[8];
    u32 held;           /* +0x08 */
    u8 padc[0x12 - 0xC];
    s16 h12;            /* +0x12 */
    u8 pad14[0x20 - 0x14];
    u16 h20;            /* +0x20 */
    u8 pad22[0x20A - 0x22];
    u16 h20A;           /* +0x20A */
    u8 pad20c[0x27A - 0x20C];
    u8 b27A;            /* +0x27A */
    u8 pad27b[0x290 - 0x27B];
    u16 k_up;           /* +0x290 */
    u16 k_down;         /* +0x292 */
    u16 k_294;          /* +0x294 */
    u16 k_296;          /* +0x296 */
    u8 pad298[2];
    u16 k_29A;          /* +0x29A */
    u8 pad29c[4];
    u16 k_left;         /* +0x2A0 */
    u8 pad2a2[2];
    u16 k_right;        /* +0x2A4 */
} Side3;
extern Side3 *D_80039D18;

s32 func_8007F414(PObj3 *o, s32 msg, ...)
{

    if (msg != 0) {
        return 0;
    }
    if (!(D_80039D18->held & D_80039D18->k_29A)) {
        D_80039D18->h20 &= ~2;
    }
    if (o->mode == 22) {
        return 0;
    }
    if (o->kind == 0xB0) {
        if (D_80039D18->h12 >= 0) {
            if ((D_80039D18->held & D_80039D18->k_29A) && !(D_80039D18->h20 & 1)) {
                if ((D_80039D18->held & (D_80039D18->k_up | D_80039D18->k_left)) == (D_80039D18->k_up | D_80039D18->k_left)) {
                    o->act = 49;
                set:
                    D_80039D18->h12 = 5;
                    return 0;
                }
                if ((D_80039D18->held & (D_80039D18->k_up | D_80039D18->k_right)) == (D_80039D18->k_up | D_80039D18->k_right)) {
                    o->act = 50;
                    goto set;
                }
                if ((D_80039D18->held & (D_80039D18->k_down | D_80039D18->k_left)) == (D_80039D18->k_down | D_80039D18->k_left)) {
                    o->act = 54;
                    goto set;
                }
                if ((D_80039D18->held & (D_80039D18->k_down | D_80039D18->k_right)) == (D_80039D18->k_down | D_80039D18->k_right)) {
                    o->act = 55;
                    goto set;
                }
                if (D_80039D18->k_up & D_80039D18->held) {
                    o->act = 48;
                    goto set;
                }
                if (D_80039D18->k_down & D_80039D18->held) {
                    o->act = 53;
                    goto set;
                }
                if (D_80039D18->k_left & D_80039D18->held) {
                    o->act = 51;
                    goto set;
                }
                if (D_80039D18->k_right & D_80039D18->held) {
                    o->act = 52;
                    goto set;
                }
                if (D_80039D18->h12 == 0) {
                    D_80039D18->h20 |= 2;
                    o->act = 0x38;
                    return 0;
                }
                D_80039D18->h12--;
            } else {
                D_80039D18->h12 = -6;
            }
            return 0;
        }
        if ((D_80039D18->held & D_80039D18->k_29A) && !(D_80039D18->h20 & 1)) {
            o->hCE = -D_80039D18->h20A / 4;
            o->act = 0x40;
            D_80039D18->h12 = 0;
            return 0;
        }
        if (D_80039D18->h12 == -1) {
            o->act = 0x38;
        } else {
            D_80039D18->h12++;
        }
        return 0;
    }
    if (o->kind == 192) {
        if ((D_80039D18->held & (D_80039D18->k_up | D_80039D18->k_left)) == (D_80039D18->k_up | D_80039D18->k_left)) {
            do {
                o->act = 66;
            } while (0);
        } else if ((D_80039D18->held & (D_80039D18->k_up | D_80039D18->k_right)) == (D_80039D18->k_up | D_80039D18->k_right)) {
            do {
                o->act = 67;
            } while (0);
        } else if ((D_80039D18->held & (D_80039D18->k_down | D_80039D18->k_left)) == (D_80039D18->k_down | D_80039D18->k_left)) {
            do {
                o->act = 69;
            } while (0);
        } else if ((D_80039D18->held & (D_80039D18->k_down | D_80039D18->k_right)) == (D_80039D18->k_down | D_80039D18->k_right)) {
            do {
                o->act = 70;
            } while (0);
        } else if (D_80039D18->k_up & D_80039D18->held) {
            do {
                o->act = 65;
            } while (0);
        } else if (D_80039D18->k_down & D_80039D18->held) {
            do {
                o->act = 68;
            } while (0);
        } else if (D_80039D18->k_left & D_80039D18->held) {
            o->act = 71;
        } else if (D_80039D18->k_right & D_80039D18->held) {
            o->act = 72;
        } else if (D_80039D18->held & D_80039D18->k_294) {
            o->act = 0x49;
        } else if (D_80039D18->held & D_80039D18->k_296) {
            o->act = 0x4A;
        } else if (D_80039D18->held & D_80039D18->k_29A) {
        act40:
            o->act = 0x40;
        } else {
            o->act = 0x4B;
        }
    } else {
        if ((D_80039D18->held & (D_80039D18->k_up | D_80039D18->k_left)) == (D_80039D18->k_up | D_80039D18->k_left)) {
            o->act = 2;
        } else if ((D_80039D18->held & (D_80039D18->k_up | D_80039D18->k_right)) == (D_80039D18->k_up | D_80039D18->k_right)) {
            o->act = 3;
        } else if ((D_80039D18->held & (D_80039D18->k_down | D_80039D18->k_left)) == (D_80039D18->k_down | D_80039D18->k_left)) {
            o->act = 7;
        } else if ((D_80039D18->held & (D_80039D18->k_down | D_80039D18->k_right)) == (D_80039D18->k_down | D_80039D18->k_right)) {
            o->act = 8;
        } else if (D_80039D18->k_up & D_80039D18->held) {
            o->act = 1;
        } else if (D_80039D18->k_down & D_80039D18->held) {
            o->act = 6;
        } else if (D_80039D18->k_left & D_80039D18->held) {
            o->act = 4;
        } else if (D_80039D18->k_right & D_80039D18->held) {
            o->act = 5;
        } else if (D_80039D18->held & D_80039D18->k_294) {
            o->act = 9;
        } else if (D_80039D18->held & D_80039D18->k_296) {
            o->act = 10;
        } else if (o->dir != 96 && !(D_80039D18->h20 & 2) && (D_80039D18->held & D_80039D18->k_29A)) {
            goto act40;
        } else {
            D_80039D18->h12 = 0;
            o->act = 0;
            return 0;
        }
    }
    return 0;
}
