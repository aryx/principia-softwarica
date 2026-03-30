/*s: libc/port/hangup.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <ctype.h>

/*s: function [[hangup]] */
/*
 *  force a connection to hangup
 */
int
hangup(int ctl)
{
    return write(ctl, "hangup", sizeof("hangup")-1) != sizeof("hangup")-1;
}
/*e: function [[hangup]] */
/*e: libc/port/hangup.c */
