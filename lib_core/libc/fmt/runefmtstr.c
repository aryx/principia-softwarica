/*s: fmt/runefmtstr.c */
#include <u.h>
#include <libc.h>

/*s: function runefmtstrflush */
Rune*
runefmtstrflush(Fmt *f)
{
    if(f->start == nil)
        return nil;
    *(Rune*)f->to = '\0';
    return f->start;
}
/*e: function runefmtstrflush */
/*e: fmt/runefmtstr.c */
