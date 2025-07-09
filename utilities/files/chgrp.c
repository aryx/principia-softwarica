/*s: files/chgrp.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

//pad: ???? int readgid(char*);
/*s: global [[uflag]](chgrp.c) */
// change uid of file (instead of gid)
bool uflag;
/*e: global [[uflag]](chgrp.c) */

/*s: function [[main]](chgrp.c) */
void
main(int argc, char *argv[])
{
    int i;
    Dir dir;
    char *group;
    char *errs = nil;

    ARGBEGIN {
    default:
    usage:
        fprint(STDERR, "usage: chgrp [ -uo ] group file ....\n");
        exits("usage");
        return;
    case 'u':
    case 'o':
        uflag = true;
        break;
    } ARGEND
    if(argc < 1)
        goto usage;

    group = argv[0];
    for(i=1; i<argc; i++){
        nulldir(&dir);
        if(uflag)
            dir.uid = group;
        else
            dir.gid = group;
        if(dirwstat(argv[i], &dir) == ERROR_NEG1) {
            fprint(STDERR, "chgrp: can't wstat %s: %r\n", argv[i]);
            errs = "can't wstat";
            continue;
        }
    }
    exits(errs);
}
/*e: function [[main]](chgrp.c) */
/*e: files/chgrp.c */
