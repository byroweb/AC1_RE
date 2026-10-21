#include "ac1/ac1.h"
/* ovl201 0x80051628 - flush the CD request queue: free buffers no file slot still owns,
 * clear every request, keep only the one being read (re-queued as state 0x60) */
typedef struct CdReq {
    u8 state;                   /* +0x00 0 = free */
    u8 busy;                    /* +0x01 */
    u8 unk02[10];
    u8 *buf;                    /* +0x0C */
    u8 unk10[0x18];
} CdReq;
typedef struct CdQueue {
    CdReq req[16];              /* +0x000 */
    CdReq *wr;                  /* +0x280 */
    CdReq *rd;                  /* +0x284 */
} CdQueue;
extern CdQueue D_80049FE0;
extern u8 *D_801F36D0[];
void EnterCriticalSection(void);
void ExitCriticalSection(void);
void free2(void *p);




void func_80051628(void)
{
    CdReq save;
    s32 i;
    s16 j;
    CdReq *r;

    EnterCriticalSection();
    save = *D_80049FE0.rd;
    for (r = D_80049FE0.req, i = 0; i < 16; i++, r++) {
        if (r->state != 0) {
            for (j = 0; j < 30; j++) {
                if (D_801F36D0[j] == r->buf) {
                    goto kept;
                }
            }
            free2(r->buf);
        }
    kept:
        r->state = 0;
        r->busy = 0;
    }
    D_80049FE0.wr = D_80049FE0.rd = D_80049FE0.req;
    if (save.state != 0) {
        save.state = 0x60;
        D_80049FE0.req[0] = save;
        D_80049FE0.wr++;
    }
    ExitCriticalSection();
}
