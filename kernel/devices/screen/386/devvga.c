/*s: kernel/devices/screen/386/devvga.c */
/*
 * VGA controller
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"

#include <draw.h>
#include <memdraw.h>
#include <cursor.h>

#include "../port/portscreen.h"
#include "screen.h"

/*s: enum [[_anon_ (kernel/devices/screen/386/devvga.c)]] */
enum {
    Qdir,

    Qvgabios,
    Qvgactl,

    Qvgaovl,
    Qvgaovlctl,
};
/*e: enum [[_anon_ (kernel/devices/screen/386/devvga.c)]] */

/*s: global [[vgadir]](x86) */
static Dirtab vgadir[] = {
    ".",    { Qdir, 0, QTDIR },     0,  0550,
    "vgabios",  { Qvgabios, 0 },    0x100000, 0440,
    "vgactl",       { Qvgactl, 0 },     0,  0660,
    "vgaovl",       { Qvgaovl, 0 },     0,  0660,
    "vgaovlctl",    { Qvgaovlctl, 0 },  0,  0660,
};
/*e: global [[vgadir]](x86) */

/*s: enum [[_anon_ (kernel/devices/screen/386/devvga.c)2]] */
enum {
    CMactualsize,
    CMblank,
    CMblanktime,
    CMdrawinit,
    CMhwaccel,
    CMhwblank,
    CMhwgc,
    CMlinear,
    CMpalettedepth,
    CMpanning,
    CMsize,
    CMtextmode,
    CMtype,
    CMunblank,
};
/*e: enum [[_anon_ (kernel/devices/screen/386/devvga.c)2]] */

/*s: global [[vgactlmsg]](x86) */
static Cmdtab vgactlmsg[] = {
    CMactualsize,   "actualsize",   2,
    CMblank,    "blank",    1,
    CMblanktime,    "blanktime",    2,
    CMdrawinit, "drawinit", 1,
    CMhwaccel,  "hwaccel",  2,
    CMhwblank,  "hwblank",  2,
    CMhwgc,     "hwgc",     2,
    CMlinear,   "linear",   0,
    CMpalettedepth, "palettedepth", 2,
    CMpanning,  "panning",  2,
    CMsize,     "size",     3,
    CMtextmode, "textmode", 1,
    CMtype,     "type",     2,
    CMunblank,  "unblank",  1,
};
/*e: global [[vgactlmsg]](x86) */

/*s: function [[vgareset]](x86) */
static void
vgareset(void)
{
    /* reserve the 'standard' vga registers */
    if(ioalloc(0x2b0, 0x2df-0x2b0+1, 0, "vga") < 0)
        panic("vga ports already allocated"); 
    if(ioalloc(0x3c0, 0x3da-0x3c0+1, 0, "vga") < 0)
        panic("vga ports already allocated"); 
}
/*e: function [[vgareset]](x86) */

/*s: function [[vgaattach]](x86) */
static Chan*
vgaattach(char* spec)
{
    if(*spec && strcmp(spec, "0"))
        error(Eio);
    return devattach('v', spec);
}
/*e: function [[vgaattach]](x86) */

/*s: function [[vgawalk]](x86) */
Walkqid*
vgawalk(Chan* c, Chan *nc, char** name, int nname)
{
    return devwalk(c, nc, name, nname, vgadir, nelem(vgadir), devgen);
}
/*e: function [[vgawalk]](x86) */

/*s: function [[vgastat]](x86) */
static int
vgastat(Chan* c, uchar* dp, int n)
{
    return devstat(c, dp, n, vgadir, nelem(vgadir), devgen);
}
/*e: function [[vgastat]](x86) */

/*s: function [[vgaopen]](x86) */
static Chan*
vgaopen(Chan* c, int omode)
{
    VGAscr *scr;
    static char *openctl = "openctl\n";

    scr = &vgascreen;
    if ((ulong)c->qid.path == Qvgaovlctl) {
        if (scr->dev && scr->dev->ovlctl)
            scr->dev->ovlctl(scr, c, openctl, strlen(openctl));
        else 
            error(Enonexist);
    }
    return devopen(c, omode, vgadir, nelem(vgadir), devgen);
}
/*e: function [[vgaopen]](x86) */

/*s: function [[vgaclose]](x86) */
static void
vgaclose(Chan* c)
{
    VGAscr *scr;
    static char *closectl = "closectl\n";

    scr = &vgascreen;
    if((ulong)c->qid.path == Qvgaovlctl)
        if(scr->dev && scr->dev->ovlctl){
            if(waserror()){
                print("ovlctl error: %s\n", up->errstr);
                return;
            }
            scr->dev->ovlctl(scr, c, closectl, strlen(closectl));
            poperror();
        }
}
/*e: function [[vgaclose]](x86) */

/*s: function [[vgaread]](x86) */
static long
vgaread(Chan* c, void* a, long n, vlong off)
{
    int len;
    char *p, *s;
    VGAscr *scr;
    ulong offset = off;
    char chbuf[30];

    switch((ulong)c->qid.path){

    case Qdir:
        return devdirread(c, a, n, vgadir, nelem(vgadir), devgen);

    case Qvgabios:
        if(offset >= 0x100000)
            return 0;
        if(offset+n >= 0x100000)
            n = 0x100000 - offset;
        memmove(a, (uchar*)arch_kaddr(0)+offset, n); // why not KADDR()?
        return n;

    case Qvgactl:
        scr = &vgascreen;

        p = malloc(READSTR);
        if(p == nil)
            error(Enomem);
        if(waserror()){
            free(p);
            nexterror();
        }

        len = 0;

        if(scr->dev)
            s = scr->dev->name;
        else
            s = "cga";
        len += snprint(p+len, READSTR-len, "type %s\n", s);

        if(gscreen) {
            len += snprint(p+len, READSTR-len, "size %dx%dx%d %s\n",
                gscreen->r.max.x, gscreen->r.max.y,
                gscreen->depth, chantostr(chbuf, gscreen->chan));

            if(Dx(gscreen->r) != Dx(physgscreenr) 
            || Dy(gscreen->r) != Dy(physgscreenr))
                len += snprint(p+len, READSTR-len, "actualsize %dx%d\n",
                    physgscreenr.max.x, physgscreenr.max.y);
        }

        len += snprint(p+len, READSTR-len, "blank time %lud idle %d state %s\n",
            blanktime, drawidletime(), scr->isblank ? "off" : "on");
        len += snprint(p+len, READSTR-len, "hwaccel %s\n", hwaccel ? "on" : "off");
        len += snprint(p+len, READSTR-len, "hwblank %s\n", hwblank ? "on" : "off");
        len += snprint(p+len, READSTR-len, "panning %s\n", panning ? "on" : "off");
        len += snprint(p+len, READSTR-len, "addr p 0x%lux v 0x%p size 0x%ux\n", scr->paddr, scr->vaddr, scr->apsize);
        USED(len);

        n = readstr(offset, a, n, p);
        poperror();
        free(p);

        return n;

    case Qvgaovl:
    case Qvgaovlctl:
        error(Ebadusefd);
        break;

    default:
        error(Egreg);
        break;
    }

    return 0;
}
/*e: function [[vgaread]](x86) */

/*s: function [[vgactl]](x86) */
static void
vgactl(Cmdbuf *cb)
{
    int align, i, size, x, y, z;
    char *chanstr, *p;
    ulong chan;
    Cmdtab *ct;
    VGAscr *scr;
    extern VGAdev *vgadev[];
    extern VGAcur *vgacur[];

    scr = &vgascreen;
    ct = lookupcmd(cb, vgactlmsg, nelem(vgactlmsg));
    switch(ct->index){
    /*s: [[vgactl]] cases(x86) */
    case CMblank:
        drawblankscreen(true);
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMunblank:
        drawblankscreen(false);
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMblanktime:
        blanktime = strtoul(cb->f[1], 0, 0);
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMhwblank:
        if(strcmp(cb->f[1], "on") == 0)
            hwblank = true;
        else if(strcmp(cb->f[1], "off") == 0)
            hwblank = false;
        else
            break;
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMhwgc:
        if(strcmp(cb->f[1], "off") == 0){
            lock(&cursor);
            if(scr->cur){
                if(scr->cur->disable)
                    scr->cur->disable(scr);
                scr->cur = nil;
            }
            unlock(&cursor);
            return;
        }
        if(strcmp(cb->f[1], "soft") == 0){
            lock(&cursor);
            swcursorinit_wrapper();
            if(scr->cur && scr->cur->disable)
                scr->cur->disable(scr);
            scr->cur = &swcursor;
            if(scr->cur->enable)
                scr->cur->enable(scr);
            unlock(&cursor);
            return;
        }
        for(i = 0; vgacur[i]; i++){
            if(strcmp(cb->f[1], vgacur[i]->name))
                continue;
            lock(&cursor);
            if(scr->cur && scr->cur->disable)
                scr->cur->disable(scr);
            scr->cur = vgacur[i];
            if(scr->cur->enable)
                scr->cur->enable(scr);
            unlock(&cursor);
            return;
        }
        break;
    /*x: [[vgactl]] cases(x86) */
    case CMtype:
        for(i = 0; vgadev[i]; i++){
            if(strcmp(cb->f[1], vgadev[i]->name))
                continue;
            if(scr->dev && scr->dev->disable)
                scr->dev->disable(scr);
            scr->dev = vgadev[i];
            if(scr->dev->enable)
                scr->dev->enable(scr);
            return;
        }
        break;
    /*x: [[vgactl]] cases(x86) */
    case CMsize:
        x = strtoul(cb->f[1], &p, 0);
        if(x == 0 || x > 10240)
            error(Ebadarg);
        if(*p)
            p++;

        y = strtoul(p, &p, 0);
        if(y == 0 || y > 10240)
            error(Ebadarg);
        if(*p)
            p++;

        z = strtoul(p, &p, 0);

        chanstr = cb->f[2];
        if((chan = strtochan(chanstr)) == 0)
            error("bad channel");

        if(chantodepth(chan) != z)
            error("depth, channel do not match");

        arch_cursoroff(1);
        deletescreenimage();
        if(screensize(x, y, z, chan))
            error(Egreg);
        vgascreenwin(scr);
        resetscreenimage();
        arch_cursoron(1);
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMactualsize:
        if(gscreen == nil)
            error("set the screen size first");

        x = strtoul(cb->f[1], &p, 0);
        if(x == 0 || x > 2048)
            error(Ebadarg);
        if(*p)
            p++;

        y = strtoul(p, nil, 0);
        if(y == 0 || y > 2048)
            error(Ebadarg);

        if(x > gscreen->r.max.x || y > gscreen->r.max.y)
            error("physical screen bigger than virtual");

        physgscreenr = Rect(0,0,x,y);
        gscreen->clipr = physgscreenr;
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMpalettedepth:
        x = strtoul(cb->f[1], &p, 0);
        if(x != 8 && x != 6)
            error(Ebadarg);

        scr->palettedepth = x;
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMdrawinit:
        if(gscreen == nil)
            error("drawinit: no gscreen");
        if(scr->dev && scr->dev->drawinit)
            scr->dev->drawinit(scr);
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMlinear:
        if(cb->nf!=2 && cb->nf!=3)
            error(Ebadarg);
        size = strtoul(cb->f[1], 0, 0);
        if(cb->nf == 2)
            align = 0;
        else
            align = strtoul(cb->f[2], 0, 0);
        if(screenaperture(size, align) < 0)
            error("not enough free address space");
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMpanning:
        if(strcmp(cb->f[1], "on") == 0){
            if(scr == nil || scr->cur == nil)
                error("set screen first");
            if(!scr->cur->doespanning)
                error("panning not supported");
            gscreen->clipr = gscreen->r;
            panning = true;
        }
        else if(strcmp(cb->f[1], "off") == 0){
            gscreen->clipr = physgscreenr;
            panning = false;
        }else
            break;
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMhwaccel:
        if(strcmp(cb->f[1], "on") == 0)
            hwaccel = 1;
        else if(strcmp(cb->f[1], "off") == 0)
            hwaccel = 0;
        else
            break;
        return;
    /*x: [[vgactl]] cases(x86) */
    case CMtextmode:
        arch__screeninit();
        return;
    /*e: [[vgactl]] cases(x86) */
    }

    cmderror(cb, "bad VGA control message");
}
/*e: function [[vgactl]](x86) */

/*s: global [[Enooverlay]](x86) */
char Enooverlay[] = "No overlay support";
/*e: global [[Enooverlay]](x86) */

/*s: function [[vgawrite]](x86) */
static long
vgawrite(Chan* c, void* a, long n, vlong off)
{
    ulong offset = off;
    Cmdbuf *cb;
    VGAscr *scr;

    switch((ulong)c->qid.path){

    case Qdir:
        error(Eperm);

    case Qvgactl:
        if(offset || n >= READSTR)
            error(Ebadarg);
        cb = parsecmd(a, n);
        if(waserror()){
            free(cb);
            nexterror();
        }
        vgactl(cb);
        poperror();
        free(cb);
        return n;

    case Qvgaovl:
        scr = &vgascreen;
        if (scr->dev == nil || scr->dev->ovlwrite == nil) {
            error(Enooverlay);
            break;
        }
        return scr->dev->ovlwrite(scr, a, n, off);

    case Qvgaovlctl:
        scr = &vgascreen;
        if (scr->dev == nil || scr->dev->ovlctl == nil) {
            error(Enooverlay);
            break;
        }
        scr->dev->ovlctl(scr, c, a, n);
        return n;

    default:
        error(Egreg);
        break;
    }

    return 0;
}
/*e: function [[vgawrite]](x86) */

/*s: global [[vgadevtab]](x86) */
Dev vgadevtab = {
    .dc       =    'v',
    .name     =    "vga",
               
    .attach   =    vgaattach,
    .walk     =    vgawalk,

    .open     =    vgaopen,
    .close    =    vgaclose,
    .read     =    vgaread,
    .write    =    vgawrite,
    .stat     =    vgastat,

    .reset    =    vgareset,

    // generic
    .create   =    devcreate,
    .remove   =    devremove,
    .wstat    =    devwstat,
    .bread    =    devbread,
    .bwrite   =    devbwrite,
    .init     =    devinit,
    .shutdown =    devshutdown,
};
/*e: global [[vgadevtab]](x86) */
/*e: kernel/devices/screen/386/devvga.c */
