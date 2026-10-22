#include "ac1/ac1.h"

typedef struct {
    u8 pad0;
    s8 b1;                      /* +0x001 0x801F3751 */
    u8 pad2[2];
    s8 b4;                      /* +0x004 0x801F3754 */
    u8 pad5[0x38 - 5];
    u8 flags[64];               /* +0x038 0x801F3788 */
    volatile u8 busy;           /* +0x078 0x801F37C8 */
    s8 cursor;                  /* +0x079 0x801F37C9 */
    u8 pad7A[0x84 - 0x7A];
    s8 order[64];               /* +0x084 0x801F37D4 */
    u8 padC4[0x1944 - 0xC4];
    void *list;                 /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;
extern s8 D_8017F791;
extern u32 D_801A2568;
extern s32 D_801A2574;
extern u8 D_80039D22;
extern s32 func_800867C4(void);
extern void *func_80050FC4(s32 size);
extern void func_80086FD4(void);
extern void func_80086FFC(void);
extern void func_80087AEC(UiObj *o);
extern void func_8008878C(UiObj *o);
extern UiObj *func_80088DE4(void);
extern void func_8008AF0C(void);
extern void func_8005D71C(void);
extern void func_8005CC48(void);

UiObj *func_8008729C(void)
{
    UiObj *o;
    RECT r = { 960, 0, 2, 24 };

    if (D_8017F791 == 1) {
        if ((D_801A2568 & 0xC) == 0xC) {
            if (D_801F3750.b4 == D_8017F791) {
                D_801F3750.b4 = 0;
            } else if (func_800867C4() == 0) {
                D_801F3750.b4 = 1;
            }
        }
        if ((D_801A2568 & 3) == 3) {
            s32 i;

            D_801F3750.b4 = 0;
            for (i = 63; i != -1; i--) {
                D_801F3750.flags[i] |= 2;
            }
            D_80039D22 = 1;
        }
    }
    D_801F3750.list = func_80050FC4(0x6F0);
    if (D_801F3750.b1 >= 0) {
        while (D_801F3750.busy != 0) {
        }
        func_80086FD4();
        func_80086FFC();
        if (D_801F3750.b1 != -1) {
            D_801F3750.cursor = 0;
            while (D_801F3750.b1 != D_801F3750.order[D_801F3750.cursor]) {
                if (D_801F3750.cursor >= 64) break;
                D_801F3750.cursor++;
            }
        }
        o = ui_obj_alloc(0);
        o->ctor = func_8008729C;
        o->update = ui_obj_nop2;
        o->destroy = func_8008878C;
        func_80088DE4();
    } else {
        o = ui_obj_alloc(0);
        o->ctor = func_8008729C;
        o->update = func_80087AEC;
        o->destroy = func_8008878C;
    }
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    func_8008AF0C();
    func_8005D71C();
    r.x = 548;
    r.y = 24;
    MoveImage(&r, 960, 128);
    r.x = 550;
    r.y = 24;
    MoveImage(&r, 962, 128);
    MoveImage(&r, 964, 128);
    MoveImage(&r, 966, 128);
    MoveImage(&r, 968, 128);
    MoveImage(&r, 970, 128);
    MoveImage(&r, 972, 128);
    MoveImage(&r, 974, 128);
    MoveImage(&r, 976, 128);
    MoveImage(&r, 978, 128);
    MoveImage(&r, 980, 128);
    MoveImage(&r, 982, 128);
    MoveImage(&r, 984, 128);
    MoveImage(&r, 986, 128);
    MoveImage(&r, 988, 128);
    MoveImage(&r, 990, 128);
    MoveImage(&r, 992, 128);
    MoveImage(&r, 994, 128);
    MoveImage(&r, 996, 128);
    MoveImage(&r, 998, 128);
    MoveImage(&r, 1000, 128);
    MoveImage(&r, 1002, 128);
    MoveImage(&r, 1004, 128);
    MoveImage(&r, 1006, 128);
    MoveImage(&r, 1008, 128);
    MoveImage(&r, 1010, 128);
    MoveImage(&r, 1012, 128);
    MoveImage(&r, 1014, 128);
    MoveImage(&r, 1016, 128);
    MoveImage(&r, 1018, 128);
    MoveImage(&r, 1020, 128);
    r.x = 552;
    r.y = 24;
    MoveImage(&r, 1022, 128);
    r.x = 554;
    r.y = 24;
    MoveImage(&r, 960, 152);
    r.x = 556;
    r.y = 24;
    MoveImage(&r, 962, 152);
    MoveImage(&r, 964, 152);
    MoveImage(&r, 966, 152);
    MoveImage(&r, 968, 152);
    MoveImage(&r, 970, 152);
    MoveImage(&r, 972, 152);
    MoveImage(&r, 974, 152);
    MoveImage(&r, 976, 152);
    MoveImage(&r, 978, 152);
    MoveImage(&r, 980, 152);
    MoveImage(&r, 982, 152);
    MoveImage(&r, 984, 152);
    MoveImage(&r, 986, 152);
    MoveImage(&r, 988, 152);
    MoveImage(&r, 990, 152);
    MoveImage(&r, 992, 152);
    MoveImage(&r, 994, 152);
    MoveImage(&r, 996, 152);
    MoveImage(&r, 998, 152);
    MoveImage(&r, 1000, 152);
    MoveImage(&r, 1002, 152);
    MoveImage(&r, 1004, 152);
    MoveImage(&r, 1006, 152);
    MoveImage(&r, 1008, 152);
    MoveImage(&r, 1010, 152);
    MoveImage(&r, 1012, 152);
    MoveImage(&r, 1014, 152);
    MoveImage(&r, 1016, 152);
    MoveImage(&r, 1018, 152);
    MoveImage(&r, 1020, 152);
    r.x = 558;
    r.y = 24;
    MoveImage(&r, 1022, 152);
    DrawSync(0);
    func_8005CC48();
    D_801A2574 = 2;
    return o;
}
