/*s: port/hangup.c */
#include <u.h>
#include <libc.h>
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
/*e: port/hangup.c */
