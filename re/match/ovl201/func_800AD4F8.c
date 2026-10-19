#include "ac1_types.h"
/* ovl201 0x800AD4F8  size 568  score 5.793  2026-10-18 */

typedef struct {
    s16 x, y, w, h;
} RECT;

typedef struct {
    u8 pad[0x74];
    void (*state)(void *);
} Func800AD4F8Obj;

extern u8 D_8005BA2C;
extern u8 D_801BC4F8;
extern void *func_80050078(void *h);
extern void func_8005D71C(void);
extern void func_8002C17C(RECT *r, s32 cr, s32 cg, s32 cb);
extern void func_8005D2DC(char *s, s32 x, s32 y);
extern void func_8005D21C(char *s, s32 x, s32 y);
extern void func_80072838(void);
extern void func_800AD730(void *);

void func_800AD4F8(Func800AD4F8Obj *obj)
{
    if (func_80050078(&D_8005BA2C) == &D_801BC4F8) {
        func_8005D71C();
        {
            RECT r = { 832, 316, 64, 196 };
            func_8002C17C(&r, 0, 0, 0);
        }
        func_8005D2DC("NAME         ", 832, 316);
        func_8005D2DC("TYPE         ", 832, 324);
        func_8005D2DC("E.C.         ", 832, 332);
        func_8005D2DC("ARMOR POINT  ", 832, 340);
        func_8005D2DC("WEIGHT       ", 832, 348);
        func_8005D2DC("LEFT         ", 832, 356);
        func_8005D21C("E", 992, 192);
        func_8005D2DC(".", 994, 192);
        func_8005D21C("P          ", 995, 192);
        func_8005D2DC(".", 997, 192);
        func_8005D2DC("ENERGY POINT", 992, 200);
        func_8005D21C("A", 992, 208);
        func_8005D2DC(".", 994, 208);
        func_8005D21C("P          ", 995, 208);
        func_8005D2DC(".", 997, 208);
        func_8005D2DC("ARMOR POINT ", 992, 216);
        func_8005D21C("W", 992, 224);
        func_8005D2DC(".", 994, 224);
        func_8005D21C("P          ", 995, 224);
        func_8005D2DC(".", 997, 224);
        func_8005D2DC("WEIGHT POINT", 992, 232);
        func_80072838();
        obj->state = func_800AD730;
    }
}
