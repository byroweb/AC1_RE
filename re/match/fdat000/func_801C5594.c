/* fdat000 (mission library, entry 0 @ 0x801C4B40; 26 twins): tail wrapper around
 * func_8008A80C. */
#include "ac1/ac1.h"
extern void func_8008A80C();
void func_801C5594(void)
{
    func_8008A80C();
}
