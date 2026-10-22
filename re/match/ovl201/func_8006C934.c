#include "ac1_types.h"

typedef struct {
    u8 r, g, b, cd;
} CVECTOR;

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
    u8 pad[0x1C];
    u32 ot[3];      /* +0x1C */
} EmbLayer;

typedef struct {
    u8 pad[0x18];
    EmbLayer *layer[2];     /* +0x18 */
} EmbOts;

typedef struct {
    u8 pad[0x86];
    u8 color;               /* +0x86 selected palette entry */
    u8 pad87[0x98 - 0x87];
    EmbOts *ots;            /* +0x98 */
} EmbEdit;

typedef struct {
    u16 x;      /* +0x00 */
    u16 y;      /* +0x02 */
    u8 pad4[4];
    u16 w;      /* +0x08 */
    u16 h;      /* +0x0A */
    u8 padC[103512 - 0xC];
} Screen;

typedef struct {
    u8 player;  /* +0x00 */
    u8 pad[3];
    TILE *prim; /* +0x04 */
} Gpu;

extern Gpu D_801EF6C8;
extern Screen D_801BCEEC[];
extern CVECTOR D_8017E5D8[];
extern CVECTOR D_800B7E48;

void func_8006C934(EmbEdit *e)
{
    s16 i;
    TILE *prim;
    EmbOts *ots;
    u8 c;

    prim = D_801EF6C8.prim;
    ots = e->ots;
    D_801EF6C8.prim = prim + 16;
    for (i = 0; i < 16; prim++, i++) {
        setTile(prim);
        c = e->color;
        if (c != i) {
            prim->x0 = D_801BCEEC[D_801EF6C8.player].x + (i * 8 - (u16)(D_801BCEEC[D_801EF6C8.player].w - 172));
            prim->y0 = D_801BCEEC[D_801EF6C8.player].y + 192 - D_801BCEEC[D_801EF6C8.player].h;
            prim->w = 8;
            prim->h = 16;
            if (i == 0) {
                *(CVECTOR *)&prim->r0 = D_800B7E48;
            } else {
                *(CVECTOR *)&prim->r0 = D_8017E5D8[i];
            }
            addPrim(&ots->layer[D_801EF6C8.player]->ot[1], prim);
        } else {
            prim->x0 = D_801BCEEC[D_801EF6C8.player].x + (c * 8 - (u16)(D_801BCEEC[D_801EF6C8.player].w - 168));
            prim->y0 = D_801BCEEC[D_801EF6C8.player].y + 188 - D_801BCEEC[D_801EF6C8.player].h;
            prim->w = 15;
            prim->h = 23;
            if (c == 0) {
                *(CVECTOR *)&prim->r0 = D_800B7E48;
            } else {
                *(CVECTOR *)&prim->r0 = D_8017E5D8[c];
            }
            addPrim(&ots->layer[D_801EF6C8.player]->ot[0], prim);
        }
    }
}
