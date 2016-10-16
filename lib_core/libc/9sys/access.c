/*s: 9sys/access.c */
#include <u.h>
#include <libc.h>

/*s: function access */
int
access(char *name, int mode)
{
    int fd;
    Dir *db;
    static char omode[] = {
        0,
        OEXEC,
        OWRITE,
        ORDWR,
        OREAD,
        OEXEC,  /* only approximate */
        ORDWR,
        ORDWR   /* only approximate */
    };

    if(mode == AEXIST){
        db = dirstat(name);
        free(db);
        if(db != nil)
            return 0;
        return -1;
    }
    fd = open(name, omode[mode&7]);
    if(fd >= 0){
        close(fd);
        return 0;
    }
    return -1;
}
/*e: function access */
/*e: 9sys/access.c */
