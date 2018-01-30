/*s: rc/path.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*s: global [[nullpath]] */
struct Word nullpath = { "", nil};
/*e: global [[nullpath]] */

/*s: function [[searchpath]] */
word*
searchpath(char *w)
{
    word *path;

    if(strncmp(w, "/", 1)==0
    || strncmp(w, "#", 1)==0
    || strncmp(w, "./", 2)==0
    || strncmp(w, "../", 3)==0
    || (path = vlook("path")->val)==nil)
        path=&nullpath;
    return path;
}
/*e: function [[searchpath]] */

/*e: rc/path.c */
