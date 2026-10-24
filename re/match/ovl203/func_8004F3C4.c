#include "ac1/ac1.h"

typedef struct {
    u8 head;                    /* +0 'R' */
    u8 pad1;
    u16 a;                      /* +2 */
    u16 b;                      /* +4 */
    u8 pad6;
    u8 tail;                    /* +7 'C' */
} LinkPkt;

typedef struct {
    u8 unk0[0x2F4];
    s32 fd_r;                   /* +0x2F4 */
    s32 fd_w;                   /* +0x2F8 */
    LinkPkt rx;                 /* +0x2FC = 0x8004A28C */
    u8 pad304[4];
    LinkPkt tx;                 /* +0x308 = 0x8004A298 */
} LinkState;

extern LinkState D_80049F90;
extern volatile u8 D_8004A28C;           /* rx.head */
extern volatile u8 D_8004A293;           /* rx.tail */
extern volatile u8 D_8004A298;           /* tx.head */
extern volatile u8 D_8004A29F;           /* tx.tail */
extern volatile u16 D_8004A29A;          /* tx.a */
extern volatile u16 D_8004A29C;          /* tx.b */
extern volatile s32 D_800895C4;
extern volatile u8 D_800895C8;
extern s32 read(s32 fd, void *buf, s32 n);
extern s32 write(s32 fd, void *buf, s32 n);
extern s32 func_8001628C(void);
extern s32 func_8004BEB0(void);

void func_8004F3C4(u16 *src, u16 *dst, s32 n)
{
    LinkState *p;

    s32 i;
    i = n - 1;
    if (i == -1) return;
    p = &D_80049F90;
    for (; i != -1; i--) {
        D_8004A28C = D_8004A293 = 0;
        D_8004A298 = 'R';
        D_8004A29F = 'C';
        D_8004A29A = *src++;
        D_8004A29C = *src;
        read(p->fd_r, &p->rx, 8);
        D_800895C4 = func_8001628C();
        write(p->fd_w, &p->tx, 8);
        src++;
        if (D_800895C8 != 0 || func_8004BEB0() == 0) {
            return;
        }
        *dst++ = *(volatile u16 *)&p->rx.a;
        *dst++ = *(volatile u16 *)&p->rx.b;
    }
}
