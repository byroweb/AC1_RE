#include "ac1/ac1.h"

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    u8 r, g, b, cd;
} CVec;

typedef struct {                /* 0x10 */
    s32 sxy;
    s32 otz;
    CVec col;                   /* +0x08 */
    s32 dp;                     /* +0x0C */
} CVtx;

typedef struct {
    CVtx v[1729];
    s32 pad[3];
    s32 depth_cue;              /* +0x6C1C = 0x801AD664 */
} CVtxBuf;

typedef struct {
    SVECTOR *verts;
    u16 n_verts;                /* +0x04 */
    u16 n_extra;                /* +0x06 */
} TPart;

typedef struct {
    u8 pad00[0x10];
    TPart *part;                /* +0x10 */
    u8 pad14[0xCC];
    CVec col;                   /* +0xE0 */
} TObj;

extern CVtxBuf D_801A6A48;
extern SVECTOR *D_801A6544;
extern CVec D_80090928[4][16];

/* project the part's vertices, tinting each by the 4-bit groups in its pad
 * (table D_80090928) over the object's colour; then the extra vertices */
void func_80065CF8(TObj *obj, u16 mask, s32 extra)
{
    CVtx *o;
    SVECTOR *v;
    s32 n;
    CVec *tb;
    CVec *c;
    s32 sh;
    s32 bits;
    s32 r;
    s32 g;
    s32 b;
    s32 k;
    s32 k2;

    o = D_801A6A48.v;
    v = D_801A6544;
    n = obj->part->n_verts;
    if (mask == 0xF) {
        tb = D_80090928[0];
        sh = 0;
    single:
        do {
            gte_ldv0(v);
            gte_rtps();
            bits = v->pad & mask;
            if (bits) {
                c = &tb[bits >> sh];
                r = obj->col.r + c->r;
                if (r > 255) {
                    o->col.r = 255;
                } else {
                    o->col.r = r;
                }
                g = obj->col.g + c->g;
                if (g > 255) {
                    o->col.g = 255;
                } else {
                    o->col.g = g;
                }
                b = obj->col.b + c->b;
                if (b > 255) {
                    o->col.b = 255;
                } else {
                    o->col.b = b;
                }
            } else {
                o->col = obj->col;
            }
            v++;
            gte_stsxy(&o->sxy);
            gte_stdp(&o->dp);
            gte_stszotz(&o->otz);
            o++;
        } while (--n);
    } else if (mask == 0xF0) {
        tb = D_80090928[1];
        sh = 4;
        goto single;
    } else if (mask == 0xF00) {
        tb = D_80090928[2];
        sh = 8;
        goto single;
    } else if (mask == 0xF000) {
        tb = D_80090928[3];
        sh = 12;
        goto single;
    } else {
        do {
            gte_ldv0(v);
            gte_rtps();
            bits = v->pad & mask;
            if (bits) {
                tb = D_80090928[0];
                r = obj->col.r;
                g = obj->col.g;
                b = obj->col.b;
                k = 4;
                do {
                    k2 = bits & 0xF;
                    bits >>= 4;
                    if (k2) {
                        c = &tb[k2];
                        r += c->r;
                        g += c->g;
                        b += c->b;
                    }
                    tb += 16;
                } while (--k);
                if (r > 255) {
                    o->col.r = 255;
                } else {
                    o->col.r = r;
                }
                if (g > 255) {
                    o->col.g = 255;
                } else {
                    o->col.g = g;
                }
                if (b > 255) {
                    o->col.b = 255;
                } else {
                    o->col.b = b;
                }
            } else {
                o->col = obj->col;
            }
            v++;
            gte_stsxy(&o->sxy);
            gte_stdp(&o->dp);
            gte_stszotz(&o->otz);
            o++;
        } while (--n);
    }
done:
    if (D_801A6A48.depth_cue == 0) {
        o = D_801A6A48.v;
        n = obj->part->n_verts;
        do {
            o->dp = 0;
            o++;
        } while (--n);
    }
    if (extra) {
        n = obj->part->n_extra;
        while (--n != -1) {
            gte_ldv0(v);
            gte_rtps();
            v++;
            gte_stsxy(&o->sxy);
            gte_stdp(&o->dp);
            gte_stszotz(&o->otz);
            o++;
        }
    }
}
