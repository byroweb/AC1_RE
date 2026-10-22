#include "ac1/ac1.h"

typedef struct { u8 r, g, b, a; } EmbRgb;
extern EmbRgb D_8017E5D8[];
extern RECT D_800B7FEC[];       /* sample CLUT rects */
extern RECT D_800B806C[];       /* sample image rects */
extern s32 D_801A2550;
extern s32 D_801A2568;
int MoveImage(RECT *, int, int);
int StoreImage(RECT *, u32 *);
int DrawSync(int);
void func_800506EC(s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_8006BA08(UiObj *);
void func_8006A3EC(UiObj *);
void func_8006ADE0(UiObj *);

typedef struct EmbSampleSel {
    u8 pad[0x88];
    u16 *pixels;                /* +0x88 */
    u8 pad8c[0x38];
    u8 answer;                  /* +0xC4 */
    u8 page;                    /* +0xC5 */
    s8 col;                     /* +0xC6 */
    s8 row;                     /* +0xC7 */
} EmbSampleSel;

void func_8006ABA4(UiObj *o)
{
    EmbSampleSel *e = (EmbSampleSel *)o;
    u16 clut[16];
    RECT *crect, *irect;
    s16 idx, i;

    func_8006BA08(o);
    if (e->answer == 0) {
        idx = e->col + e->page * 4 + e->row * 4;
        crect = &D_800B7FEC[idx];
        MoveImage(crect, 0x3B0, 0);
        irect = &D_800B806C[idx];
        MoveImage(irect, 0x39A, 0);
        MoveImage(irect, 0x39A, 0x40);
        StoreImage(irect, (u32 *)e->pixels);
        StoreImage(crect, (u32 *)clut);
        DrawSync(0);
        for (i = 1; i < 16; i++) {
            D_8017E5D8[i].r = (clut[i] & 0x1F) << 3;
            D_8017E5D8[i].g = (clut[i] & 0x3E0) >> 2;
            D_8017E5D8[i].b = (clut[i] & 0x7C00) >> 7;
            D_8017E5D8[i].a = 0x60;
        }
        func_800506EC(14);
        obj_find_by_id(13)->kill = 1;
        func_8005C2D0(0x10, 0x30, 0x11C, 0x90, 13);
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        { s32 *p = &D_801A2550; *p |= D_801A2568; }
        func_8005C2D0(0x50, 0x32, 0x68, 0x90, 9);
        o->update = func_8006ADE0;
    } else if (e->answer == 1) {
        o->update = func_8006A3EC;
    }
}
