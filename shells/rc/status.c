/*s: rc/status.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*s: function [[setstatus]] */
void
setstatus(char *s)
{
    setvar("status", newword(s, (word *)nil));
}
/*e: function [[setstatus]] */

/*s: function [[getstatus]] */
char*
getstatus(void)
{
    var *status = vlook("status");
    return status->val ? status->val->word : "";
}
/*e: function [[getstatus]] */

/*s: function [[truestatus]] */
bool
truestatus(void)
{
    char *s;
    for(s = getstatus();*s;s++)
        if(*s!='|' && *s!='0')
            return false;
    return true;
}
/*e: function [[truestatus]] */

/*s: function [[concstatus]] */
char*
concstatus(char *s, char *t)
{
    static char v[NSTATUS+1];
    int n = strlen(s);
    strncpy(v, s, NSTATUS);
    if(n<NSTATUS){
        v[n]='|';
        strncpy(v+n+1, t, NSTATUS-n-1);
    }
    v[NSTATUS]='\0';
    return v;
}
/*e: function [[concstatus]] */

/*e: rc/status.c */
