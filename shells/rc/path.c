/*s: rc/path.c */
/*s: includes */
#include "rc.h"
#include "getflags.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
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
