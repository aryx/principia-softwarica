/*s: port/toupper.c */
#include    <ctype.h>

/*s: function toupper */
int
toupper(int c)
{

    if(c < 'a' || c > 'z')
        return c;
    return _toupper(c);
}
/*e: function toupper */

/*s: function tolower */
int
tolower(int c)
{

    if(c < 'A' || c > 'Z')
        return c;
    return _tolower(c);
}
/*e: function tolower */
/*e: port/toupper.c */
