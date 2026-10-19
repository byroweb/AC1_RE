#include "ac1_types.h"
/* ovl201 func_8006C554 - draw an n x n grid of TILE cells (n = 128 / cell size) from
 * a 4-bit-per-cell map (32 bytes per row, two cells per byte, low nibble first).
 * A non-zero nibble lights the cell colour from a palette with the GTE (NCCS on a
 * fixed normal); zero uses the default colour. Each tile goes into the OT entry of
 * the current player's layer. */

/* PsyQ inline_c.h GTE macros (no shared GTE header in the repo) */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    s16 vx, vy, vz, pad;
} SVECTOR;

typedef struct {
    u8 r, g, b, cd;
} CVECTOR;

typedef struct {
    s16 m[3][3];
    s32 t[3];
} MATRIX;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))
#define getaddr(p) (u32)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setlen(p, _len) (((P_TAG *)(p))->len = (u8)(_len))
#define setcode(p, _code) (((P_TAG *)(p))->code = (u8)(_code))
#define setTile(p) setlen(p, 3), setcode(p, 0x60)

typedef struct {
    u8 pad[0x24];
    u32 ot;     /* +0x24 */
} Func8006C554Layer;

typedef struct {
    u8 pad[0x18];
    Func8006C554Layer *layer[2];    /* +0x18 */
} Func8006C554Ots;

typedef struct {
    u8 pad[0x7C];
    s16 x;      /* +0x7C */
    s16 y;      /* +0x7E */
} Func8006C554Info;

typedef struct {
    u8 pad[8];
    u8 size;    /* +0x08 */
    u8 pad9[3];
} Func8006C554Cell;

typedef struct {
    u8 pad[0x7C];
    Func8006C554Cell cell;      /* +0x7C */
    u8 *map;                    /* +0x88 */
    u8 pad8C[0x98 - 0x8C];
    Func8006C554Ots *ots;       /* +0x98 */
    Func8006C554Info *info;     /* +0x9C */
} Func8006C554Obj;

typedef struct {
    u16 x;      /* +0x00 */
    u16 y;      /* +0x02 */
    u8 pad4[4];
    u16 w;      /* +0x08 */
    u16 h;      /* +0x0A */
    u8 padC[103512 - 0xC];
} Func8006C554Screen;

typedef struct {
    u8 player;  /* +0x00 */
    u8 pad[3];
    TILE *prim; /* +0x04 */
} Func8006C554Gpu;

extern Func8006C554Gpu D_801EF6C8;
extern Func8006C554Screen D_801BCEEC[];
extern MATRIX D_800B8E44;
extern SVECTOR D_800B8D78;
extern CVECTOR D_8017E5D8[];
extern CVECTOR D_800B7E48;

extern void SetColorMatrix(MATRIX *m);

void func_8006C554(Func8006C554Obj *obj)
{
    s16 n;
    s16 i;
    s16 j;
    s32 x;
    s32 y;
    s32 c;
    s32 t;
    TILE *prim;
    Func8006C554Ots *ots;
    Func8006C554Cell *cell;
    u8 *row;
    u8 *p;

    n = 128 / obj->cell.size;
    prim = D_801EF6C8.prim;
    ots = obj->ots;
    D_801EF6C8.prim = (TILE *)((u8 *)prim + sizeof(TILE) * n * n);
    SetColorMatrix(&D_800B8E44);
    row = obj->map + obj->info->x / 2 + obj->info->y * 32;
    cell = &obj->cell;
    p = row;
    t = D_801BCEEC[D_801EF6C8.player].y + 36;
    y = t - D_801BCEEC[D_801EF6C8.player].h;
    for (i = 0; i < n; i++, y += cell->size) {
        t = D_801BCEEC[D_801EF6C8.player].x + 172;
        x = t - D_801BCEEC[D_801EF6C8.player].w;
        for (j = 0; j < n / 2; j++) {
            setTile(prim);
            prim->x0 = x;
            prim->y0 = y;
            prim->w = prim->h = cell->size;
            c = *p & 0xF;
            if (c) {
                gte_ldv0(&D_800B8D78);
                gte_ldrgb(&D_8017E5D8[c]);
                gte_nccs();
                gte_strgb(&prim->r0);
            } else {
                *(CVECTOR *)&prim->r0 = D_800B7E48;
            }
            addPrim(&ots->layer[D_801EF6C8.player]->ot, prim);
            x += cell->size;
            prim++;

            setTile(prim);
            prim->x0 = x;
            prim->y0 = y;
            prim->w = prim->h = cell->size;
            c = *p >> 4;
            if (c) {
                gte_ldv0(&D_800B8D78);
                gte_ldrgb(&D_8017E5D8[c]);
                gte_nccs();
                gte_strgb(&prim->r0);
            } else {
                *(CVECTOR *)&prim->r0 = D_800B7E48;
            }
            addPrim(&ots->layer[D_801EF6C8.player]->ot, prim);
            x += cell->size;
            prim++;
            p++;
        }
        row = p = row + 32;
    }
}
