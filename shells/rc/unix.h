#include <u.h>
#include <libc.h>

// In the original rc in the plan9 repo this file is far bigger and contains
// includes such as '#include <fcntl.h>' and many ifdefs. This is
// probably because it was written before plan9port existed.
// We can now simply include u.h and libc.h and that's it.

