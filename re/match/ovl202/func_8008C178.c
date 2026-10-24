#include "ac1/ac1.h"
/* ovl202 0x8008C178: load mission script thread set `set`: point every thread at its
 * code in the VM blob, allocate its per-kind work area from the mission heap, return
 * the set header word 0. */
typedef struct {
    u16 *pc;            /* +0x00 */
    s32 b04;            /* +0x04 */
    s32 b08;
    u16 kind;           /* +0x0C */
    u16 pad0e;
    s16 b10;            /* +0x10 */
    s16 pad12;
    u8 *mem;            /* +0x14 */
} VmThread;
extern u8 *func_8008C154(s32 n);

u16 func_8008C178(int set)
{
    u8 unused[0x212];
    u16 *hdr;
    u16 *p;
    VmThread *t;
    s32 i;
    u8 *h;

    g_mission_ctl.vm_set = set;
    hdr = (u16 *)(g_mission_ctl.vm_blob + ((u16 *)g_mission_ctl.vm_blob)[set]);
    h = g_mission_ctl.heap;
    g_mission_ctl.vm_threads = hdr[1];
    g_mission_ctl.vm_extra = hdr[2];
    g_mission_ctl.heap_cur = h;
    g_mission_ctl.vm_thread_tab = h;
    t = (VmThread *)h;
    p = hdr + 3;
    i = g_mission_ctl.vm_threads;
    while (--i != -1) {
        u16 *q;
        g_mission_ctl.heap_cur += sizeof(VmThread);
        q = (u16 *)(g_mission_ctl.vm_blob + *p++);
        t->pc = q + 1;
        t->kind = *q;
        t->b04 = 0;
        t->b10 = 0;
        t++;
    }
    t = g_mission_ctl.vm_thread_tab;
    i = g_mission_ctl.vm_threads;
    while (--i != -1) {
        switch (t->kind) {
        case 0:
            t->mem = func_8008C154(16);
            break;
        case 1:
            t->mem = func_8008C154(530);
            *(s16 *)(t->mem + 0x10) = 0;
            break;
        case 2:
            t->mem = func_8008C154(40);
            break;
        }
        t++;
    }
    return hdr[0];
}
