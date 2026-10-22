#include "ac1/ac1.h"
/* ovl202 0x80058E04 - transform the model's vertices into D_801A6A48 and give each a
 * colour: vertices whose light bits (SVECTOR pad) hit `mask` get the fade-ramp colour
 * (one ramp: ramp + 0x404040; several: 64 + sum, clamped to 255), the others the
 * object's colour.  Depth cue is cleared when fog is off; with `extra` the trailing
 * vertices are transformed without colour. */

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct {
    u8 r, g, b, cd;
} Col4;

typedef struct {
    s32 sxy;                    /* +0x00 */
    s32 otz;                    /* +0x04 */
    Col4 col;                   /* +0x08 */
    s32 dp;                     /* +0x0C */
} LitVtx;

typedef struct {
    u8 pad00[4];
    u16 nvert;                  /* +0x04 */
    u16 nextra;                 /* +0x06 */
} VtxHdr;

typedef struct {
    u8 pad00[0x10];
    VtxHdr *hdr;                /* +0x10 */
    u8 pad14[0x18];
    Col4 col;                   /* +0x2C */
} LitObj;

typedef struct {                /* 0x801A5F30 */
    u8 pad0000[0xB18];
    LitVtx vtx[1];              /* +0x0B18 (0x801A6A48) */
    u8 pad0b28[0x7734 - 0xB28];
    s32 fog;                    /* +0x7734 (0x801AD664) */
} RenderGlobals;

extern Col4 D_80090928[4][16];
extern SVECTOR *D_801A6544;
extern RenderGlobals D_801A5F30;

void func_80058E04(LitObj *o, u16 mask, s32 extra)
{
    LitVtx *out;
    SVECTOR *v;
    s32 n;
    Col4 *ramp;
    s32 sh;
    s32 k;
    s32 i;
    s32 r;
    s32 g;
    s32 b;
    s32 j;
    Col4 *c;

    out = D_801A5F30.vtx;
    v = D_801A6544;
    n = o->hdr->nvert;
    if (mask == 0x000F) {
        ramp = D_80090928[0];
        sh = 0;
    single:
        do {
            gte_ldv0(v);
            gte_rtps();
            k = v->pad & mask;
            if (k != 0) {
                c = &ramp[k >> sh];
                *(u32 *)&out->col = *(u32 *)c + 0x404040;
            } else {
                *(u32 *)&out->col = *(u32 *)&o->col;
            }
            v++;
            gte_stsxy(&out->sxy);
            gte_stdp(&out->dp);
            gte_stszotz(&out->otz);
            out++;
        } while (--n);
    } else if (mask == 0x00F0) {
        ramp = D_80090928[1];
        sh = 4;
        goto single;
    } else if (mask == 0x0F00) {
        ramp = D_80090928[2];
        sh = 8;
        goto single;
    } else if (mask == 0xF000) {
        ramp = D_80090928[3];
        sh = 12;
        goto single;
    } else {
        do {
            gte_ldv0(v);
            gte_rtps();
            k = v->pad & mask;
            if (k != 0) {
                i = 4;
                b = 64;
                g = 64;
                r = 64;
                ramp = D_80090928[0];
                do {
                    j = k & 0xF;
                    k >>= 4;
                    if (j != 0) {
                        c = &ramp[j];
                        r += c->r;
                        g += c->g;
                        b += c->b;
                    }
                    ramp += 16;
                } while (--i);
                if (r > 255) {
                    out->col.r = 255;
                } else {
                    out->col.r = r;
                }
                if (g > 255) {
                    out->col.g = 255;
                } else {
                    out->col.g = g;
                }
                if (b > 255) {
                    out->col.b = 255;
                } else {
                    out->col.b = b;
                }
            } else {
                out->col = o->col;
            }
            v++;
            gte_stsxy(&out->sxy);
            gte_stdp(&out->dp);
            gte_stszotz(&out->otz);
            out++;
        } while (--n);
    }
    if (D_801A5F30.fog == 0) {
        out = D_801A5F30.vtx;
        n = o->hdr->nvert;
        do {
            out->dp = 0;
            out++;
        } while (--n);
    }
    if (extra) {
        n = o->hdr->nextra;
        while (--n != -1) {
            gte_ldv0(v);
            gte_rtps();
            v++;
            gte_stsxy(&out->sxy);
            gte_stszotz(&out->otz);
            out++;
        }
    }
}
