#include "ac1/ac1.h"
/* ovl201 0x8008D8AC - after selling: unequip every equipped part (bytes at
 * 0x80031A58, 0xFF = none) the player no longer owns (counts at
 * 0x80031A94), and the second back weapon when both back slots hold the
 * same part but fewer than two are owned. */

extern u8 D_80031A58[];
extern u8 D_80031A94[];
extern s16 func_8008CFD4(s8 kind, s8 idx);

void func_8008D8AC(void)
{
    if (D_80031A58[2] != 0xFF && D_80031A94[func_8008CFD4(0, D_80031A58[2])] == 0) {
        D_80031A58[2] = 0xFF;
    }
    if (D_80031A58[0] != 0xFF && D_80031A94[func_8008CFD4(1, D_80031A58[0])] == 0) {
        D_80031A58[0] = 0xFF;
    }
    if (D_80031A58[3] != 0xFF && D_80031A94[func_8008CFD4(2, D_80031A58[3])] == 0) {
        D_80031A58[3] = 0xFF;
    }
    if (D_80031A58[11] != 0xFF && D_80031A94[func_8008CFD4(3, D_80031A58[11])] == 0) {
        D_80031A58[11] = 0xFF;
    }
    if (D_80031A58[1] != 0xFF && D_80031A94[func_8008CFD4(7, D_80031A58[1])] == 0) {
        D_80031A58[1] = 0xFF;
    }
    if (D_80031A58[8] != 0xFF && D_80031A94[func_8008CFD4(4, D_80031A58[8])] == 0) {
        D_80031A58[8] = 0xFF;
    }
    if (D_80031A58[9] != 0xFF && D_80031A94[func_8008CFD4(5, D_80031A58[9])] == 0) {
        D_80031A58[9] = 0xFF;
    }
    if (D_80031A58[5] != 0xFF && D_80031A94[func_8008CFD4(9, D_80031A58[5])] == 0) {
        D_80031A58[5] = 0xFF;
    }
    if (D_80031A58[4] != 0xFF && D_80031A94[func_8008CFD4(9, D_80031A58[4]) + 16] == 0) {
        D_80031A58[4] = 0xFF;
    }
    if (D_80031A58[7] != 0xFF && D_80031A94[func_8008CFD4(8, D_80031A58[7])] == 0) {
        D_80031A58[7] = 0xFF;
    }
    if (D_80031A58[6] != 0xFF && D_80031A94[func_8008CFD4(8, D_80031A58[6])] == 0) {
        D_80031A58[6] = 0xFF;
    }
    if (D_80031A58[10] != 0xFF && D_80031A94[func_8008CFD4(6, D_80031A58[10])] == 0) {
        D_80031A58[10] = 0xFF;
    }
    if (D_80031A58[7] == D_80031A58[6] && D_80031A94[func_8008CFD4(8, D_80031A58[7])] < 2) {
        D_80031A58[7] = 0xFF;
    }
}
