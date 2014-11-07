/*s: mk/match.c */
#include	"mk.h"

/*s: function match */
bool
match(char *name, char *template, char *stem)
{
    Rune r;
    int n;

    while(*name && *template){
        n = chartorune(&r, template);
        if (PERCENT(r))
            break;
        while (n--)
            if(*name++ != *template++)
                return false;
    }
    if(!PERCENT(*template))
        return false;
    n = strlen(name)-strlen(template+1);
    if (n < 0)
        return false;
    if (strcmp(template+1, name+n))
        return false;

    strncpy(stem, name, n);
    stem[n] = '\0';

    if(*template == '&')
        return !charin(stem, "./");

    return true;
}
/*e: function match */

/*s: function subst */
void
subst(char *stem, char *template,   char *dest, int dlen)
{
    Rune r;
    char *s, *e;
    int n;

    e = dest+dlen-1;
    while(*template){
        n = chartorune(&r, template);
        if (PERCENT(r)) {
            template += n;
            for (s = stem; *s; s++)
                if(dest < e)
                    *dest++ = *s;
        } else
            while (n--){
                if(dest < e)
                    *dest++ = *template;
                template++;
            }
    }
    *dest = '\0';
}
/*e: function subst */
/*e: mk/match.c */
