/*s: files/mkdir.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

char *e;
ulong mode = 0777L;

/*s: function [[usage]](mkdir.c) */
void
usage(void)
{
    fprint(2, "usage: mkdir [-p] [-m mode] dir...\n");
    exits("usage");
}
/*e: function [[usage]](mkdir.c) */
/*s: function [[makedir]] */
int
makedir(char *s)
{
    int f;

    if(access(s, AEXIST) == 0){
        fprint(2, "mkdir: %s already exists\n", s);
        e = "error";
        return -1;
    }
    f = create(s, OREAD, DMDIR | mode);
    if(f < 0){
        fprint(2, "mkdir: can't create %s: %r\n", s);
        e = "error";
        return -1;
    }
    close(f);
    return 0;
}
/*e: function [[makedir]] */
/*s: function [[mkdirp]] */
void
mkdirp(char *s)
{
    char *p;

    for(p=strchr(s+1, '/'); p; p=strchr(p+1, '/')){
        *p = 0;
        if(access(s, AEXIST) != 0 && makedir(s) < 0)
            return;
        *p = '/';
    }
    if(access(s, AEXIST) != 0)
        makedir(s);
}
/*e: function [[mkdirp]] */
/*s: function [[main]](mkdir.c) */
void
main(int argc, char *argv[])
{
    int i, pflag;
    char *m;

    pflag = 0;
    ARGBEGIN{
    default:
        usage();
    case 'm':
        m = ARGF();
        if(m == nil)
            usage();
        mode = strtoul(m, &m, 8);
        if(mode > 0777)
            usage();
        break;
    case 'p':
        pflag = 1;
        break;
    }ARGEND

    for(i=0; i<argc; i++){
        if(pflag)
            mkdirp(argv[i]);
        else
            makedir(argv[i]);
    }
    exits(e);
}
/*e: function [[main]](mkdir.c) */
/*e: files/mkdir.c */
