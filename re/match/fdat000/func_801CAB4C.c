/* fdat000 (mission library, entry 0 @ 0x801C4B40; 32 twins): install the mission-library
 * AI on an entity (fn_state 801C981C, fn_move 801C75BC, fn_think 801C8CBC, fn_hit
 * 801C91CC) and run its setup (func_801C9518). */
#include "ac1/ac1.h"
extern void func_801C981C();
extern void func_801C75BC();
extern void func_801C8CBC();
extern void func_801C91CC();
extern void func_801C9518(AcEntity *e);
void func_801CAB4C(AcEntity *e)
{
    e->fn_state = (void *)func_801C981C;
    e->fn_move = (void *)func_801C75BC;
    e->fn_think = (void *)func_801C8CBC;
    e->fn_hit = (void *)func_801C91CC;
    func_801C9518(e);
}
