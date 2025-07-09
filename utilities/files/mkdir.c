/*s: files/mkdir.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: global [[e]](mkdir.c) */
// error
char *e;
/*e: global [[e]](mkdir.c) */
/*s: global [[mode]](mkdir.c) */
ulong mode = 0777L;
/*e: global [[mode]](mkdir.c) */

/*s: function [[usage]](mkdir.c) */
void
usage(void)
{
    fprint(STDERR, "usage: mkdir [-p] [-m mode] dir...\n");
    exits("usage");
}
/*e: function [[usage]](mkdir.c) */
/*s: function [[makedir]] */
errorneg1
makedir(char *s)
{
    fdt f;

    if(access(s, AEXIST) == 0){
        fprint(STDERR, "mkdir: %s already exists\n", s);
        e = "error";
        return ERROR_NEG1;
    }
    f = create(s, OREAD, DMDIR | mode);
    if(f < 0){
        fprint(STDERR, "mkdir: can't create %s: %r\n", s);
        e = "error";
        return ERROR_NEG1;
    }
    close(f);
    return OK_0;
}
/*e: function [[makedir]] */
/*s: function [[mkdirp]] */
void
mkdirp(char *s)
{
    char *p;

    for(p=strchr(s+1, '/'); p; p=strchr(p+1, '/')){
        *p = '\0';
        if(access(s, AEXIST) != 0 && makedir(s) == ERROR_NEG1)
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
    int i;
    bool pflag = false;
    char *m;

    ARGBEGIN{
    case 'm':
        m = ARGF();
        if(m == nil)
            usage();
        mode = strtoul(m, &m, 8);
        if(mode > 0777)
            usage();
        break;
    case 'p':
        pflag = true;
        break;
    default:
        usage();
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
