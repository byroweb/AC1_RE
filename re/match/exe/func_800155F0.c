#include "ac1_types.h"
/* exe func_800155F0 - reset a 16-entry pool of 40-byte records inside a critical
 * section: save the current record, clear the first two bytes of every entry,
 * rewind both pool pointers to the first entry, and if the saved record was in
 * use, put it back as entry 0 with state 0x60. The pointers sit right after the
 * array (one object: the code addresses the array as &next - 0x280). */

typedef struct {
    u8 state;       /* +0x00 */
    u8 flag;        /* +0x01 */
    u8 pad[2];
    s32 data[9];    /* +0x04 */
} Func800155F0Entry;

typedef struct {
    Func800155F0Entry entry[16];    /* +0x000 */
    Func800155F0Entry *next;        /* +0x280 */
    Func800155F0Entry *cur;         /* +0x284 */
} Func800155F0Pool;

extern Func800155F0Pool D_80049FE0;

extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

void func_800155F0(void)
{
    Func800155F0Entry saved;
    Func800155F0Entry *p;
    s32 i;

    EnterCriticalSection();
    saved = *D_80049FE0.cur;
    p = D_80049FE0.entry;
    for (i = 0; i < 16; i++, p++) {
        p->state = 0;
        p->flag = 0;
    }
    D_80049FE0.next = D_80049FE0.cur = D_80049FE0.entry;
    if (saved.state != 0) {
        saved.state = 0x60;
        *D_80049FE0.cur = saved;
        D_80049FE0.next++;
    }
    ExitCriticalSection();
}
