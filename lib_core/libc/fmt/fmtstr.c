/*s: libc/fmt/fmtstr.c */
#include <u.h>
#include <libc.h>

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
