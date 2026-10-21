#include "ac1/ac1.h"

s32 func_80073598(s32, s32, s32, Projectile *);
s32 func_80074728(s32, s32, s32, Projectile *);

s32 func_8006D534(s32 arg0, s32 arg1, s32 arg2, Projectile *arg3) {
    s32 r;

    r = func_80073598(arg0, arg1, arg2, arg3);
    if (r == 0) {
        r = func_80074728(arg0, arg1, arg2, arg3);
        if (r == 0) {
            return 0;
        }
    }
    arg3->free = 1;
    return r;
}
