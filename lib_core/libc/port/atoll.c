/*s: libc/port/atoll.c */
#include <u.h>
#include <libc.h>

/*s: function [[atoll]] */
vlong
atoll(char *s)
{
    return strtoll(s, nil, 0);
}
/*e: function [[atoll]] */
/*e: libc/port/atoll.c */
