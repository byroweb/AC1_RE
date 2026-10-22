#include "ac1/ac1.h"
/* ovl202 0x80053190 - message window update for the current script thread: fade the
 * window in (state 0, running the X/V sound and B/N codes at the start of the text),
 * type the text (1), hold it (2), fade out (3, B code = closing sound, then the next
 * thread); draws the semi-transparent backing quad and a draw-mode reset in OT 14. */

typedef struct {
    u8 pad00[0xC];
    u8 mode;                    /* +0x0C bit0 wait for key, bit1 auto close, bit2 always draw */
    u8 pad0d[3];
    u8 *text;                   /* +0x10 */
} MsgWin;

typedef struct {
    u32 tag;
    u32 code[2];
} DrMode;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} PolyF4;

extern u8 D_80039D10;
extern u8 D_8004121C;
extern u8 D_80041BC1;
typedef struct { volatile u8 b; } VolByte;
extern VolByte D_80041BC2;
extern DrawCtx D_801A5F24;
extern MsgWin *D_801ED844;
extern u16 D_801ED848;
extern u16 D_801ED84C;
extern void SetDrawMode(DrMode *p, s32 dfe, s32 dtd, s32 tpage, RECT *tw);
extern void func_8004DB18(s32 id);
extern s32 func_80052350(u8 **pp);
extern void func_8005239C(s16 a);
extern MsgWin *func_800523BC(MsgWin *t);
extern s32 func_80052B28(MsgWin *w, u32 n);
extern void func_80052F90(MsgWin *w, PolyF4 *p);
extern void func_80053020(MsgWin *w, PolyF4 *p, s32 a);

void func_80053190(void)
{
    MsgWin *w;
    PolyF4 *prim;
    DrMode *dm;
    u32 *ot;
    u8 *p;
    u8 *q;
    u16 st;
    s32 r;
    u16 *pc;
    u16 *pc2;

    w = D_801ED844;
    if (w->text == 0) {
        return;
    }
    if (D_80039D10 != 0 || (w->mode & 4)) {
        prim = (PolyF4 *)D_801A5F24.buf->cur;
    }
    D_801A5F24.buf->cur += sizeof(PolyF4);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    st = D_801ED848;
    switch (st) {
    case 0:
        if (D_801ED84C == 0) {
            p = w->text;
            for (;;) {
                switch (*p++) {
                case 'X':
                case 'x':
                    func_8004DB18(func_80052350(&p) + 48);
                    continue;
                case 'V':
                case 'v':
                    func_8004DB18(func_80052350(&p) + (s16)((D_8004121C << 6) + 144));
                    continue;
                case 'B':
                case 'N':
                case 'b':
                case 'n':
                    func_80052350(&p);
                    continue;
                }
                break;
            }
        }
        pc = &D_801ED84C;
        *pc += 256;
        func_80053020(w, prim, *pc);
        if (*pc >= 1024) {
            func_8005239C(1);
        }
        break;
    case 1:
        func_80052F90(w, prim);
        r = func_80052B28(w, D_801ED84C);
        if (r != 0 && r == st) {
            if (!(w->mode & 1)) {
                if (!(w->mode & 2)) {
                    func_8005239C(2);
                } else {
                    func_8005239C(3);
                }
            }
        } else {
            pc2 = &D_801ED84C;
            *pc2 += 1;
        }
        break;
    case 2:
        func_80052F90(w, prim);
        func_80052B28(w, 0x10000);
        if (D_801ED84C++ >= 60) {
            if (D_80041BC2.b != 0 || !(D_80041BC1 & 1)) {
                func_8005239C(3);
            }
        }
        break;
    case 3:
        D_801ED84C += 256;
        func_80053020(w, prim, 1024 - D_801ED84C);
        if (D_801ED84C >= 1024) {
            q = w->text;
            if (*q == 'B' || *q == 'b') {
                q++;
                func_8004DB18(func_80052350(&q));
            }
            w->text = 0;
            D_801ED844 = func_800523BC(w);
            func_8005239C(0);
        }
        break;
    }
    if (D_80039D10 != 0 || (w->mode & 4)) {
        setlen(prim, 5);
        prim->code = 0x2A;
        ot = D_801A5F24.ot;
        addPrim(ot + 14, prim);
        dm = (DrMode *)D_801A5F24.buf->cur;
        D_801A5F24.buf->cur += sizeof(DrMode);
        if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
            return;
        }
        SetDrawMode(dm, 0, 0, 0, 0);
        addPrim(ot + 14, dm);
    }
}
