/*s: port/strcat.c */
#include <u.h>
#include <libc.h>

/*s: function [[strcat]] */
char*
strcat(char *s1, char *s2)
{

    strcpy(strchr(s1, '\0'), s2);
    return s1;
}
/*e: function [[strcat]] */
/*e: port/strcat.c */
