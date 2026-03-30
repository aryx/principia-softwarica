/*s: libc/9sys/nulldir.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[nulldir]] */
void
nulldir(Dir *d)
{
    memset(d, ~0, sizeof(Dir));
    d->name = d->uid = d->gid = d->muid = "";
}
/*e: function [[nulldir]] */
/*e: libc/9sys/nulldir.c */
