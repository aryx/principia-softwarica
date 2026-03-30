/*s: libc/port/atof.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[atof]] */
double
atof(char *cp)
{
    return strtod(cp, nil);
}
/*e: function [[atof]] */
/*e: libc/port/atof.c */
