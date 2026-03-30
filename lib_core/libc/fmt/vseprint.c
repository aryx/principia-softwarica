/*s: libc/fmt/vseprint.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[vseprint]] */
char*
vseprint(char *buf, char *e, char *fmt, va_list args)
{
    Fmt f;

    if(e <= buf)
        return nil;
    f.runes = 0;
    f.start = buf;
    f.to = buf;
    f.stop = e - 1;
    f.flush = nil;
    f.farg = nil;
    f.nfmt = 0;
    f.args = args;
    dofmt(&f, fmt);
    *(char*)f.to = '\0';
    return f.to;
}
/*e: function [[vseprint]] */
/*e: libc/fmt/vseprint.c */
