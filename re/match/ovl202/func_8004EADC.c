/* NOT MATCHED: n/a, inventory mis-split: the 5 words at 0x8004EADC (lbu/lw/sll/addu/lhu, no
 * jr ra) are loads hoisted above the prologue of the 264-byte function at 0x8004EAF0
 * (addiu sp,sp,-16 is 5 words after the real start, beyond inventory.py's 4-word
 * prologue window); a standalone 20-byte function cannot exist here, 0 attempts */
#include "ac1_types.h"
