#include "ac1/ac1.h"

typedef struct {                /* 0x1C */
    u8 pad00[0x0E];
    u16 n_extra;                /* +0x0E (& 0x1FF) */
    u32 *prim;                  /* +0x10 */
    u16 n_prim;                 /* +0x14 */
    u8 pad16[6];
} MdlObj;

typedef struct {
    u32 id;
    u32 flags;
    s32 n_obj;                  /* +0x08 */
    MdlObj obj[1];              /* +0x0C */
} MdlHdr;

extern void func_8004FBDC(u16 *a, u16 *b, u16 *c, u16 *d, u16 *e);
extern void func_8004FC6C(u16 *a, u16 *b, u16 *c, u16 *d, u16 *e, u16 *f);

void func_8004FD18(MdlHdr *m)
{
    MdlObj *o;
    u32 *p;
    u16 *d;
    s32 i;
    s32 j;
    u32 hdr;
    s32 unused[2];      /* retail frame slot, never touched */

    o = m->obj;
    for (i = m->n_obj; --i != -1;) {
        j = o->n_prim + (o->n_extra & 0x1FF);
        p = o->prim;
        while (--j != -1) {
            hdr = *p;
            d = (u16 *)(p + 1);
            p = (u32 *)d + ((u8 *)p)[1];
            switch ((s32)(hdr >> 24) & 0xBC) {
            case 0x24:
            case 0x34:
            case 0xA4:
            case 0xB4:
                func_8004FBDC(d + 1, d + 3, d, d + 2, d + 4);
                break;
            case 0x2C:
            case 0x3C:
            case 0xAC:
            case 0xBC:
                func_8004FC6C(d + 1, d + 3, d, d + 2, d + 4, d + 6);
                break;
            }
        }
        o++;
    }
}
