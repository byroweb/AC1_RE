#include "ac1/ac1.h"

/* part-list work area (UiObj +0x7C) */
typedef struct {
    u8 pad7c[0x14];
    u16 cat;            /* +0x90 part category */
    u8 pad92[0x14];
    u16 cur;            /* +0xA6 selected part in the category, 255 = none */
} PartWork;

typedef struct {
    u32 *mask;          /* owned parts, 64 bits */
    u8 pad[12];
} PartCat;

extern PartCat D_800BCABC[];

u16 func_8007C2F0(UiObj *o, u8 dir)
{
    PartWork *w = (PartWork *)&o->w;
    u32 *mask = D_800BCABC[w->cat].mask;
    s16 cur = w->cur;
    s16 i;
    u32 bits;

    if (dir == 'N') {
        if (cur == 255) {
            cur = 63;
        }
        if (cur < 31 || cur == 63) {
            if (cur == 63) {
                bits = mask[0];
                cur = -1;
            } else {
                bits = mask[0] >> (w->cur + 1);
            }
            for (i = cur + 1; i < 32; i++, bits >>= 1) {
                if (bits & 1) {
                    return i;
                }
            }
            bits = mask[1];
            for (i = 32; i < 64; i++, bits >>= 1) {
                if (bits & 1) {
                    return i;
                }
            }
            bits = mask[0];
            for (i = 0; i <= cur; i++, bits >>= 1) {
                if (bits & 1) {
                    return i;
                }
            }
        } else {
            bits = mask[1] >> (cur - 31);
            for (i = cur + 1; i < 64; i++, bits >>= 1) {
                if (bits & 1) {
                    return i;
                }
            }
            bits = mask[0];
            for (i = 0; i < 32; i++, bits >>= 1) {
                if (bits & 1) {
                    return i;
                }
            }
            bits = mask[1];
            for (i = 32; i <= cur; i++, bits >>= 1) {
                if (bits & 1) {
                    return i;
                }
            }
        }
    } else {
        if (cur == 255) {
            cur = 0;
        }
        if (cur < 33 && cur != 0) {
            bits = mask[0] << (32 - cur);
            for (i = cur - 1; i >= 0; i--, bits <<= 1) {
                if (bits & 0x80000000) {
                    return i;
                }
            }
            bits = mask[1];
            for (i = 63; i >= 32; i--, bits <<= 1) {
                if (bits & 0x80000000) {
                    return i;
                }
            }
            bits = mask[0];
            for (i = 31; i >= cur; i--, bits <<= 1) {
                if (bits & 0x80000000) {
                    return i;
                }
            }
        } else {
            if (cur == 0) {
                bits = mask[1];
                cur = 64;
            } else {
                bits = mask[1] << (64 - cur);
            }
            for (i = cur - 1; i >= 32; i--, bits <<= 1) {
                if (bits & 0x80000000) {
                    return i;
                }
            }
            bits = mask[0];
            for (i = 31; i >= 0; i--, bits <<= 1) {
                if (bits & 0x80000000) {
                    return i;
                }
            }
            bits = mask[1];
            for (i = 63; i >= w->cur; i--, bits <<= 1) {
                if (bits & 0x80000000) {
                    return i;
                }
            }
        }
    }
}
