/*s: files/chgrp.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

//pad: ???? int readgid(char*);
int uflag;

/*s: function [[main]](chgrp.c) */
void
main(int argc, char *argv[])
{
    int i;
    Dir dir;
    char *group;
    char *errs;

    ARGBEGIN {
    default:
    usage:
        fprint(2, "usage: chgrp [ -uo ] group file ....\n");
        exits("usage");
        return;
    case 'u':
    case 'o':
        uflag++;
        break;
    } ARGEND
    if(argc < 1)
        goto usage;

    group = argv[0];
    errs = 0;
    for(i=1; i<argc; i++){
        nulldir(&dir);
        if(uflag)
            dir.uid = group;
        else
            dir.gid = group;
        if(dirwstat(argv[i], &dir) == -1) {
            fprint(2, "chgrp: can't wstat %s: %r\n", argv[i]);
            errs = "can't wstat";
            continue;
        }
    }
    exits(errs);
}
/*e: function [[main]](chgrp.c) */
/*e: files/chgrp.c */
