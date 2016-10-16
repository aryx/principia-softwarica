/*s: 9sys/nulldir.c */
#include <u.h>
#include <libc.h>

/*s: function nulldir */
void
nulldir(Dir *d)
{
    memset(d, ~0, sizeof(Dir));
    d->name = d->uid = d->gid = d->muid = "";
}
/*e: function nulldir */
/*e: 9sys/nulldir.c */
