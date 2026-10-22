#include "ac1/ac1.h"
/* ovl202 0x8007D068: player weapon fire step for one weapon (lock-on missiles):
 * gather locked targets (flag 8) into a volley count, then fire two per step. */
typedef struct {
    u8 pad0[0x16];
    u16 flags;          /* +0x16 */
    u8 pad18[0x26 - 0x18];
    s16 locks;          /* +0x26 */
    u8 pad28[0x30 - 0x28];
} LockSlot;
typedef struct {
    u32 held;           /* 0x80040FA8 */
    u8 pad04[0x1A - 0x04];
    u16 b1a;            /* 0x80040FC2 */
    struct { u8 pad[6]; s16 id; } *cur; /* 0x80040FC4 */
    u8 pad20[0x78 - 0x20];
    LockSlot *head;     /* 0x80041020 */
    LockSlot list[8];   /* 0x80041024 */
} PadCtl7D;
typedef struct {
    u8 pad0;
    u8 kind;            /* +0x01 */
    u8 a2, a3, a4, a5;  /* +0x02 */
    u8 pad6[4];
    s8 b0a;             /* +0x0A */
    u8 pad0b;
    u8 b0c;             /* +0x0C */
    u8 pad0d[0x23 - 0x0D];
    u8 reload;          /* +0x23 */
} WpnSlot;
typedef struct {
    u8 pad0[2];
    u8 s2, s3;          /* +0x02 */
    u8 pad4[4];
} WpnRec8;
extern PadCtl7D D_80040FA8;
extern u16 D_8004123C;
extern SVECTOR D_80039C78;
extern WpnRec8 D_8008E610[];
extern void func_8007C02C(AcEntity *o, WpnSlot *w, WpnRec8 *r);
extern LockSlot *func_8007CD74(AcEntity *o, WpnSlot *w, WpnRec8 *r, s32 a, s32 b, s32 c);
extern void func_80089F7C(AcEntity *t, s32 a, AcEntity *o);

void func_8007D068(AcEntity *o, WpnSlot *w)
{
    WpnRec8 *r;
    LockSlot *best;
    LockSlot *s;
    LockSlot *hit;
    s32 max;
    s32 i;

    r = &D_8008E610[w->kind];
    *(SVECTOR *)((u8 *)o + 0x76) = D_80039C78;
    *(s16 *)((u8 *)o + 0x7C) = w->b0a;
    if (o->ai_sub2 == 0) {
        if (o->ai_sub == 0) {
            if (D_80040FA8.cur != 0 && D_80040FA8.cur->id == -1) {
                D_80040FA8.cur = 0;
            }
            func_8007C02C(o, w, r);
            best = 0;
            if ((D_80040FA8.held & D_8004123C) || (D_80040FA8.b1a == 0 && (w->b0c & 0x80))) {
                max = 0;
                for (i = 8, s = D_80040FA8.list; i != 0; i--, s++) {
                    if (*(void **)s != 0 && (s->flags & 8)) {
                        if (max < s->locks) {
                            best = s;
                            max = s->locks;
                        }
                        o->ai_sub += s->locks;
                    }
                }
                if (o->ai_sub != 0) {
                    D_80040FA8.cur = 0;
                    D_80040FA8.head = best;
                    o->ai_sub *= 4;
                }
            }
        } else {
            if (!(o->ai_sub & 3)) {
                hit = func_8007CD74(o, w, r, r->s2, w->a2, w->a3);
                func_8007CD74(o, w, r, r->s3, w->a4, w->a5);
                if (hit != 0) {
                    func_80089F7C(*(AcEntity **)D_80040FA8.list, 2, o);
                    if (--hit->locks == 0) {
                        hit->flags &= ~8;
                    }
                }
            }
            if (--o->ai_sub == 0) {
                o->ai_sub2 = w->reload;
            }
        }
    } else {
        o->ai_sub2--;
    }
}
