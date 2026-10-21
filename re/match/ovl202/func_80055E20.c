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

void func_80055E20(PartObj *o, u8 *list) {
    s32 i;
    Part7C *p;

    if (o != 0) {
        while ((i = *list++) != 0xFF) {
            p = &o->parts[i];
            if (p->ang >= 0) {
                p->ang -= 0x1000;
            }
        }
    }
}
