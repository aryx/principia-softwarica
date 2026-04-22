/*s: webfs/plumb.c */
#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>
#include <thread.h>
#include <plumb.h>
#include <9p.h>

#include "dat.h"
#include "fns.h"

static int              plumbsendfd;
static int              plumbwebfd;
static Channel  *plumbchan;

static void     plumbwebproc(void*);
static void     plumbwebthread(void*);
static void plumbsendproc(void*);

/*s: function [[plumbinit]](webfs) */
void
plumbinit(void)
{
    plumbsendfd = plumbopen("send", OWRITE|OCEXEC);
    plumbwebfd = plumbopen("web", OREAD|OCEXEC);
}
/*e: function [[plumbinit]](webfs) */

/*s: function [[plumbstart]](webfs) */
void
plumbstart(void)
{
    plumbchan = chancreate(sizeof(Plumbmsg*), 0);
    proccreate(plumbwebproc, nil, STACK);
    threadcreate(plumbwebthread, nil, STACK);
}
/*e: function [[plumbstart]](webfs) */

/*s: function [[plumbwebthread]](webfs) */
static void
plumbwebthread(void*)
{
    char *base;
    Plumbmsg *m;

    for(;;){
        m = recvp(plumbchan);
        if(m == nil)
            threadexits(nil);
        base = plumblookup(m->attr, "baseurl");
        if(base == nil)
            base = m->wdir;
        plumburl(m->data, base);
        plumbfree(m);
    }
}
/*e: function [[plumbwebthread]](webfs) */

/*s: function [[plumbwebproc]](webfs) */
static void
plumbwebproc(void*)
{
    Plumbmsg *m;

    for(;;){
        m = plumbrecv(plumbwebfd);
        sendp(plumbchan, m);
        if(m == nil)
            threadexits(nil);
    }
}
/*e: function [[plumbwebproc]](webfs) */

/*s: function [[addattr]](webfs) */
static void
addattr(Plumbmsg *m, char *name, char *value)
{
    Plumbattr *a;

    a = malloc(sizeof(Plumbattr));
    a->name = name;
    a->value = value;
    a->next = m->attr;
    m->attr = a;
}
/*e: function [[addattr]](webfs) */

/*s: function [[freeattrs]](webfs) */
static void
freeattrs(Plumbmsg *m)
{
    Plumbattr *a, *next;

    a = m->attr;
    while(a != nil) {
        next = a->next;
        free(a);
        a = next;
    }
}
/*e: function [[freeattrs]](webfs) */

static struct
{
    char        *ctype;
    char        *ext;
}
ctypes[] =
{
    { "application/msword", "doc" },
    { "application/pdf", "pdf" },
    { "application/postscript", "ps" },
    { "application/rtf", "rtf" },
    { "image/gif", "gif" },
    { "image/jpeg", "jpg" },
    { "image/png", "png" },
    { "image/ppm", "ppm" },
    { "image/tiff", "tiff" },
    { "text/html", "html" },
    { "text/plain", "txt" },
    { "text/xml", "xml" },
};

/*s: function [[replumb]](webfs) */
void
replumb(Client *c)
{
    int i;
    Plumbmsg *m;
    char name[128], *ctype, *ext, *p;

    if(!c->plumbed)
        return;
    m = emalloc(sizeof(Plumbmsg));
    m->src = "webfs";
    m->dst = nil;
    m->wdir = "/";
    m->type = "text";
    m->attr = nil;
    addattr(m, "url", c->url->url);
    ctype = c->contenttype;
    ext = nil;
    if(ctype != nil) {
        addattr(m, "content-type", ctype);
        for(i = 0; i < nelem(ctypes); i++) {
            if(strcmp(ctype, ctypes[i].ctype) == 0) {
                ext = ctypes[i].ext;
                break;
            }
        }
    }
    if(ext == nil) {
        p = strrchr(c->url->url, '/');
        if(p != nil)
            p = strrchr(p+1, '.');
        if(p != nil && strlen(p) <= 5)
            ext = p+1;
        else
            ext = "txt";                /* punt */
    }
    c->ext = ext;
if(0)fprint(2, "content type %s -> extension .%s\n", ctype, ext);
    m->ndata = snprint(name, sizeof name, "/mnt/web/%d/body.%s", c->num, ext);
    m->data = estrdup(name);
    proccreate(plumbsendproc, m, STACK);        /* separate proc to avoid a deadlock */
}
/*e: function [[replumb]](webfs) */

/*s: function [[plumbsendproc]](webfs) */
static void
plumbsendproc(void *x)
{
    Plumbmsg *m;

    m = x;
    plumbsend(plumbsendfd, m);
    freeattrs(m);
    free(m->data);
    free(m);
}
/*e: function [[plumbsendproc]](webfs) */
/*e: webfs/plumb.c */
