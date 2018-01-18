/*s: kernel/devices/screen/drawname.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "../port/error.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

#include    <draw.h>
#include    <memdraw.h>
#include    <memlayer.h>

#include    "devdraw.h"

/*s: function [[drawcmp]] */
static
int
drawcmp(char *a, char *b, int n)
{
    if(strlen(a) != n)
        return 1;
    return memcmp(a, b, n);
}
/*e: function [[drawcmp]] */

/*s: function [[drawlookupname]] */
DName*
drawlookupname(int n, char *str)
{
    DName *name, *ename;

    name = sdraw.name;
    ename = &name[sdraw.nname];
    for(; name<ename; name++)
        if(drawcmp(name->name, str, n) == 0)
            return name;
    return nil;
}
/*e: function [[drawlookupname]] */

/*s: function [[drawgoodname]] */
bool
drawgoodname(DImage *d)
{
    DName *n;

    /*s: [[drawgoodname()]] if DImage [[d]] is a window */
    /* if window, validate the screen's own images */
    if(d->dscreen)
        if(!drawgoodname(d->dscreen->dimage)
        || !drawgoodname(d->dscreen->dfill))
            return false;
    /*e: [[drawgoodname()]] if DImage [[d]] is a window */

    if(d->name == nil)
        return true;
    n = drawlookupname(strlen(d->name), d->name);
    if(n == nil || n->vers != d->vers)
        return false;
    return true;
}
/*e: function [[drawgoodname]] */

/*s: function [[drawdelname]] */
void
drawdelname(DName *name)
{
    int i;

    i =  name - sdraw.name;
    memmove(name, name+1, (sdraw.nname-(i+1))*sizeof(DName));
    sdraw.nname--;
}
/*e: function [[drawdelname]] */

/*s: function [[drawaddname]] */
void
drawaddname(Client *client, DImage *di, int n, char *str)
{
    DName *name, *ename, *new, *t;

    name = sdraw.name;
    ename = &name[sdraw.nname];
    for(; name<ename; name++)
        if(drawcmp(name->name, str, n) == 0)
            error(Enameused);

    // grow array
    /*s: [[drawaddname()]] grow array by 1 element */
    t = smalloc((sdraw.nname+1)*sizeof(DName));
    memmove(t, sdraw.name, sdraw.nname*sizeof(DName));
    free(sdraw.name);
    sdraw.name = t;
    /*e: [[drawaddname()]] grow array by 1 element */

    new = &sdraw.name[sdraw.nname++];
    new->name = smalloc(n+1);
    memmove(new->name, str, n);
    new->name[n] = '\0';

    new->dimage = di;
    new->client = client;

    new->vers = ++sdraw.vers;
}
/*e: function [[drawaddname]] */

/*e: kernel/devices/screen/drawname.c */
