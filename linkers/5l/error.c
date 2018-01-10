/*s: linkers/5l/error.c */
#include "l.h"

/*s: global [[nerrors]] */
int	nerrors = 0;
/*e: global [[nerrors]] */

/*s: function [[errorexit]] */
void
errorexit(void)
{

    if(nerrors) {
        if(cout >= 0)
            remove(outfile);
        exits("error");
    }
    exits(0);
}
/*e: function [[errorexit]] */

/*s: function [[diag]] */
void
diag(char *fmt, ...)
{
    char buf[STRINGSZ];
    char *tn;
    va_list arg;

    tn = "??none??";
    if(curtext != P && curtext->from.sym != S)
        tn = curtext->from.sym->name;
    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);
    print("%s: %s\n", tn, buf);

    nerrors++;
    if(nerrors > 20 && !debug['A']) {
        print("too many errors\n");
        errorexit();
    }
}
/*e: function [[diag]] */
/*e: linkers/5l/error.c */
