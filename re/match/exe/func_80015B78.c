#include "ac1/ac1.h"

typedef struct {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
} CdLoc;

typedef struct {
    u8 cmd;             /* +0x00 */
    u8 status;          /* +0x01 */
    CdLoc loc;          /* +0x02 */
    CdLoc cur;          /* +0x06 */
    u8 pad0A[2];
    s32 dst;            /* +0x0C */
    u32 sectors;        /* +0x10 */
    s32 arg;            /* +0x14 */
    u8 pad18[0x10];
} CdReq;                /* 0x28 */

typedef struct {
    CdReq req[16];      /* 0x80049FE0 */
    CdReq *wr;          /* 0x8004A260 */
    CdReq *rd;          /* 0x8004A264 */
} CdQueue;

extern CdQueue D_80049FE0;
extern void func_80015AC8(CdReq *r);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

CdReq *func_80015B78(s32 cmd, CdLoc *loc, s32 size, s32 dst, s32 arg)
{
    CdReq *r;

    r = D_80049FE0.wr;
    func_80015AC8(r);
    EnterCriticalSection();
    r->cmd = cmd;
    r->status = 0;
    r->loc = *loc;
    r->cur = r->loc;
    r->dst = dst;
    r->sectors = (u32)(size + 0x7FF) >> 11;
    r->arg = arg;
    D_80049FE0.wr++;
    if (D_80049FE0.wr == &D_80049FE0.req[16]) {
        D_80049FE0.wr = D_80049FE0.req;
    }
    if (D_80049FE0.rd == r) {
        ExitCriticalSection();
        while (CdControl(0x16, (u8 *)loc, 0) == 0) {
        }
    } else {
        ExitCriticalSection();
    }
    return r;
}
