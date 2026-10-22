#include "ac1/ac1.h"

typedef struct CdReq {
    u8 cmd;             /* +0x00 */
    u8 status;          /* +0x01 */
    u8 loc[4];          /* +0x02 */
    u8 cur[4];          /* +0x06 */
    u8 pad0A[2];
    u32 *dst;           /* +0x0C */
    s32 sectors;        /* +0x10 */
    void (*done)();     /* +0x14 */
    u8 pad18[0x10];
} CdReq;                /* 0x28 */

extern CdReq *D_8004A264;
extern s32 CdRead(s32 sectors, u32 *buf, s32 mode);
extern s32 func_80015B24(u32 *dst, s32 sectors);
extern void func_80015580(CdReq *r);

s32 func_80015710(void)
{
    CdReq *r;

    r = D_8004A264;
    switch (r->cmd) {
    case 0:
        break;
    case 0x10:
    case 0x30:
    case 0x40:
        switch (r->status) {
        case 0:
            while (CdRead(r->sectors, r->dst, 0x80) == 0) {
            }
            r->status = 1;
            break;
        case 1:
            if (r->cmd == 0x10) {
                if (func_80015B24(r->dst, r->sectors) != 0) {
                    r->status = 0;
                    while (CdControl(0x16, r->cur, 0) == 0) {
                    }
                    break;
                }
                if (r->done != 0) {
                    r->done(r->dst);
                }
finish:
                func_80015580(r);
            } else if (r->done != 0) {
                r->done(r);
            }
            break;
        }
        break;
    case 0x60:
        goto finish;
    }
}
