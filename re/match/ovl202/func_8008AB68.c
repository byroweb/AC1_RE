#include "ac1/ac1.h"

extern void func_8008A8F8(void);
extern void func_8008AA90(void);

void func_8008AB68(void)
{
    (*(void (**)(void))g_objective_obj)();
    if (g_mission_end_seq <= 60) {
        func_8008A8F8();
    }
    if (g_mission_end_timer != 0) {
        func_8008AA90();
    }
}
