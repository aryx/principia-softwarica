/*s: port/atof.c */
#include <u.h>
#include <libc.h>

/*s: function atof */
double
atof(char *cp)
{
    return strtod(cp, nil);
}
/*e: function atof */
/*e: port/atof.c */
