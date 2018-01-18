/*s: libc/9sys/setnetmtpt.c */
#include <u.h>
#include <libc.h>

/*s: function [[setnetmtpt]] */
void
setnetmtpt(char *net, int n, char *x)
{
    if(x == nil)
        x = "/net";

    if(*x == '/'){
        strncpy(net, x, n);
        net[n-1] = 0;
    } else {
        snprint(net, n, "/net%s", x);
    }
}
/*e: function [[setnetmtpt]] */
/*e: libc/9sys/setnetmtpt.c */
