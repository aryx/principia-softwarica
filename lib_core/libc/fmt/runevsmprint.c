/*s: libc/fmt/runevsmprint.c */
#include <u.h>
#include <libc.h>
#include "fmtdef.h"

/*s: function [[runeFmtStrFlush]] */
static int
runeFmtStrFlush(Fmt *f)
{
    Rune *s;
    int n;

    if(f->start == nil)
        return 0;
    n = (int)(uintptr)f->farg;
    n *= 2;
    s = f->start;
    f->start = realloc(s, sizeof(Rune)*n);
    if(f->start == nil){
        f->farg = nil;
        f->to = nil;
        f->stop = nil;
        free(s);
        return 0;
    }
    f->farg = (void*)n;
    f->to = (Rune*)f->start + ((Rune*)f->to - s);
    f->stop = (Rune*)f->start + n - 1;
    return 1;
}
/*e: function [[runeFmtStrFlush]] */

/*s: function [[runefmtstrinit]] */
int
runefmtstrinit(Fmt *f)
{
    int n;

    memset(f, 0, sizeof *f);
    f->runes = 1;
    n = 32;
    f->start = malloc(sizeof(Rune)*n);
    if(f->start == nil)
        return -1;
    setmalloctag(f->start, getcallerpc(&f));
    f->to = f->start;
    f->stop = (Rune*)f->start + n - 1;
    f->flush = runeFmtStrFlush;
    f->farg = (void*)n;
    f->nfmt = 0;
    return 0;
}
/*e: function [[runefmtstrinit]] */

/*s: function [[runevsmprint]] */
/*
 * print into an allocated string buffer
 */
Rune*
runevsmprint(char *fmt, va_list args)
{
    Fmt f;
    int n;

    if(runefmtstrinit(&f) < 0)
        return nil;
    f.args = args;
    n = dofmt(&f, fmt);
    if(f.start == nil)      /* realloc failed? */
        return nil;
    if(n < 0){
        free(f.start);
        return nil;
    }
    setmalloctag(f.start, getcallerpc(&fmt));
    *(Rune*)f.to = '\0';
    return f.start;
}
/*e: function [[runevsmprint]] */
/*e: libc/fmt/runevsmprint.c */
