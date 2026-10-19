/* tools/match/include/ac1/ac1.h - forwarder to the shared headers in include/ac1.
 *
 * match_fn.py always passes -I tools/match/include and runs cpp from a temporary
 * work directory, so a repo-relative -I include would not resolve there.  This
 * file makes `#include "ac1/ac1.h"` work with no extra flag: a quoted include is
 * searched first next to the including file, i.e. relative to this directory. */
#include "../../../../include/ac1/ac1.h"
