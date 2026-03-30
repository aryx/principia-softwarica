/*s: libc/port/atoll.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[atoll]] */
vlong
atoll(char *s)
{
    return strtoll(s, nil, 0);
}
/*e: function [[atoll]] */
/*e: libc/port/atoll.c */
