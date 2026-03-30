/*s: libc/fmt/fmtstr.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[fmtstrflush]] */
char*
fmtstrflush(Fmt *f)
{
    if(f->start == nil)
        return nil;
    *(char*)f->to = '\0';
    return f->start;
}
/*e: function [[fmtstrflush]] */
/*e: libc/fmt/fmtstr.c */
