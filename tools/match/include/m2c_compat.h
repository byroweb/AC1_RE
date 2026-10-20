/* m2c_compat.h - the names m2c's --valid-syntax output uses, for machine drafts
 * (tools/loop/sweep.py).  Our own definitions of what each name means; a draft that
 * matched still compiles to the retail bytes with these, and a human cleanup can
 * replace them with real types and fields later.
 */
#ifndef M2C_COMPAT_H
#define M2C_COMPAT_H

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
typedef long long M2C_UNK64;

/* field of unknown struct type at a byte offset; type_ptr is a pointer type */
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_BITWISE(type, expr) ((type)(expr))

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif
