#include "ac1/ac1.h"
/* ovl202 0x80073170 - rebuild the list of live 52-byte objects (unk06 != -1, flag 0x4000 clear) */
typedef struct Obj52 {
    u8 unk00[6];
    s16 unk06;
    u8 unk08[0x14];
    u16 flags;                  /* +0x1C */
    u8 unk1E[0x16];
} Obj52;
typedef struct ObjTab {
    Obj52 obj[512];             /* +0x0000 */
    Obj52 *list[128];           /* +0x6800 */
    u16 n;                      /* +0x6A00 */
} ObjTab;
extern ObjTab D_80198B08;

void func_80073170(void)
{
    Obj52 *o;
    Obj52 **lp;
    s32 i;

    o = D_80198B08.obj;
    lp = D_80198B08.list;
    i = 512;
    D_80198B08.n = 0;
    do {
        if (o->unk06 != -1 && !(o->flags & 0x4000)) {
            D_80198B08.n++;
            *lp++ = o;
        }
        o++;
        i--;
    } while (i != 0);
}
