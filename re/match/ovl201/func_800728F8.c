#include "ac1/ac1.h"
/* ovl201 0x800728F8 - func_800506EC(6), then replace the cursor object
 * (+0x7C) with a new one from func_800728D8 that keeps the old one's
 * rotation. */

typedef struct {
    UiObj *cursor;              /* +0x7C */
} CursorWork;

extern void func_800506EC(s32 kind);
extern UiObj *func_800728D8(UiObj *o);

void func_800728F8(UiObj *o)
{
    UiObj *c;

    func_800506EC(6);
    c = func_800728D8(o);
    c->rot = ((CursorWork *)&o->w)->cursor->rot;
    ((CursorWork *)&o->w)->cursor = c;
}
