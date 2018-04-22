/*s: networking/ip/httpd/anonymous.c */
#include <u.h>
#include <libc.h>
#include "httpd.h"
#include "httpsrv.h"

/*s: function [[anonymous]] */
void
anonymous(HConnect *c)
{
    if(bind(webroot, "/", MREPL) < 0){
        hfail(c, HInternal);
        exits(nil);
    }
    chdir("/");
}
/*e: function [[anonymous]] */
/*e: networking/ip/httpd/anonymous.c */
