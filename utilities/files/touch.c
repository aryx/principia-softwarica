/*s: files/touch.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

// forward decls
error1 touch(bool, char *);

/*s: global [[now]](touch.c) */
ulong now;
/*e: global [[now]](touch.c) */

/*s: function [[usage]](touch.c) */
void
usage(void)
{
    fprint(STDERR, "usage: touch [-c] [-t time] files\n");
    exits("usage");
}
/*e: function [[usage]](touch.c) */
/*s: function [[main]](touch.c) */
void
main(int argc, char **argv)
{
    char *t, *s;
    bool nocreate = false;
    errorn status = OK_0;

    now = time(0);
    ARGBEGIN{
    case 't':
        t = EARGF(usage());
        now = strtoul(t, &s, 0);
        if(s == t || *s != '\0')
            usage();
        break;
    case 'c':
        nocreate = true;
        break;
    default:    
        usage();
    }ARGEND

    if(!*argv)
        usage();
    while(*argv)
        status += touch(nocreate, *argv++);
    if(status)
        exits("touch");
    exits(nil);
}
/*e: function [[main]](touch.c) */

/*s: function [[touch]] */
error1
touch(bool nocreate, char *name)
{
    Dir stbuff;
    fdt fd;

    nulldir(&stbuff);
    stbuff.mtime = now;
    if(dirwstat(name, &stbuff) >= 0)
        return OK_0;
    //else
    if(nocreate){
        fprint(STDERR, "touch: %s: cannot wstat: %r\n", name);
        return ERROR_1;
    }
    if((fd = create(name, OREAD|OEXCL, 0666)) < 0){
        fprint(STDERR, "touch: %s: cannot create: %r\n", name);
        return ERROR_1;
    }
    dirfwstat(fd, &stbuff);
    close(fd);
    return OK_0;
}
/*e: function [[touch]] */
/*e: files/touch.c */
