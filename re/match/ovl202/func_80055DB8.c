#include "ac1/ac1.h"

typedef struct {
    u8 pad00[0x12];
    s16 ang;                    /* +0x12 */
    u8 pad14[0x68];
} Part7C;

typedef struct {
    u8 pad00[5];
    u8 count;                   /* +0x05 */
} PartHdr;

typedef struct {
    u8 pad00[0x1C];
    PartHdr *hdr;               /* +0x1C */
    Part7C *parts;              /* +0x20 */
} PartObj;

void func_80055DB8(PartObj *o) {
    Part7C *p;
    s32 n;

    if (o != 0 && o->hdr != 0) {
        n = o->hdr->count;
        p = o->parts;
        while (--n != -1) {
            if (p->ang < -0x7FF) {
                p->ang += 0x1000;
            }
            p++;
        }
    }
}
